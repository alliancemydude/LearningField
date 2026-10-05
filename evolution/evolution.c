#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "evolution.h"
#include "battlefield.h"
#include "strategy.h"
#include "unit_genome.h"
#include "utils.h"
#include "queries.h"
#include "unit.h"

// Gene definitions
const GeneInfo REP_GENES[] = {
    // Strategy (0-5)
    {"strategy.aggression_bias",          0.5f,  2.0f, 1.2f},
    {"strategy.threat_weight",            0.0f,  1.0f, 0.5f},
    {"strategy.flank_percentage",         0.0f,  1.0f, 0.2f},
    {"strategy.flanking_bias",            0.5f,  2.0f, 1.0f},
    {"strategy.deploy_horizontal_spread", 0.0f, 10.0f, 6.0f},
    {"strategy.deploy_vertical_offset",   0.0f, 10.0f, 4.0f},
    // Swordsman (6-9)
    {"swordsman.aggression",      0.0f,  1.0f, 0.5f},
    {"swordsman.preferred_range", 0.0f, 36.0f, 12.0f},
    {"swordsman.spacing_x",       1.0f,  8.0f, 3.0f},
    {"swordsman.spacing_y",       1.0f,  8.0f, 3.0f}
};
const size_t REP_GENE_COUNT = sizeof(REP_GENES) / sizeof(GeneInfo);

const GeneInfo SEP_GENES[] = {
    // Strategy (0-5)
    {"strategy.aggression_bias",          0.5f,  2.0f, 1.2f},
    {"strategy.threat_weight",            0.0f,  1.0f, 0.5f},
    {"strategy.flank_percentage",         0.0f,  1.0f, 0.2f},
    {"strategy.flanking_bias",            0.5f,  2.0f, 1.0f},
    {"strategy.deploy_horizontal_spread", 0.0f, 10.0f, 6.0f},
    {"strategy.deploy_vertical_offset",   0.0f, 10.0f, 4.0f},
    // Spearman (6-9)
    {"spearman.aggression",      0.0f,  1.0f, 0.5f},
    {"spearman.preferred_range", 0.0f, 36.0f, 12.0f},
    {"spearman.spacing_x",       1.0f,  8.0f, 3.0f},
    {"spearman.spacing_y",       1.0f,  8.0f, 3.0f}
};
const size_t SEP_GENE_COUNT = sizeof(SEP_GENES) / sizeof(GeneInfo);

// Evolution parameters
int battles_min = 20;
int battles_max = 40;

int pop_size = 20;
int num_generations = 10;

float mutation_rate_start = 0.5f;
float mutation_rate_end = 0.2f;
float mutation_delta_start = 0.4f;
float mutation_delta_end = 0.1f;

int max_plateau_generations = 8;
float improvement_threshold = 0.005f;
float crossover_rate = 0.8f;

typedef struct {
    float* weights;
    float fitness;
} Individual;

float rand_float(float low, float high) {
    return low + ((float)rand() / RAND_MAX) * (high - low);
}

float gaussian_rand(void) {
    float u = (float)rand() / RAND_MAX;
    float v = (float)rand() / RAND_MAX;
    return sqrt(-2.0f * log(u)) * cos(2.0f * M_PI * v);
}

void random_individual(Individual* ind, const GeneInfo* genes, size_t gene_count) {
    ind->weights = malloc(gene_count * sizeof(float));
    for (size_t i = 0; i < gene_count; i++) {
        ind->weights[i] = rand_float(genes[i].min, genes[i].max);
    }
    ind->fitness = 0.0f;
}

int tournament_select(const Individual pop[], int size, int tournament_size) {
    int best_idx = rand() % size;
    for (int i = 1; i < tournament_size; i++) {
        int idx = rand() % size;
        if (pop[idx].fitness > pop[best_idx].fitness)
            best_idx = idx;
    }
    return best_idx;
}

void crossover(const float* p1, const float* p2, float* child, size_t gene_count, float rate) {
    if (rand_float(0.0f, 1.0f) < rate) {
        for (size_t i = 0; i < gene_count; i++) {
            child[i] = (rand() % 2) ? p1[i] : p2[i];
        }
    } else {
        memcpy(child, p1, gene_count * sizeof(float));
    }
}

void mutate(float* weights, const GeneInfo* genes, size_t gene_count, float rate, float delta) {
    for (size_t i = 0; i < gene_count; i++) {
        if (rand_float(0.0f, 1.0f) < rate) {
            float range = genes[i].max - genes[i].min;
            float scaled_delta = delta * range / 10.0f;
            weights[i] += gaussian_rand() * scaled_delta;
            if (weights[i] < genes[i].min) weights[i] = genes[i].min;
            if (weights[i] > genes[i].max) weights[i] = genes[i].max;
        }
    }
}

int compare_individual(const void *a, const void *b) {
    const Individual *ia = (const Individual*)a;
    const Individual *ib = (const Individual*)b;
    if (ia->fitness > ib->fitness) return -1;
    if (ia->fitness < ib->fitness) return 1;
    return 0;
}

// Combined evaluator
PairFitness evaluate_all(const float* rep_weights, const float* sep_weights, int num_battles) {
    // Stash current state so we can restore after the batch.
    FactionStrategy old_rep = republic_strategy;
    FactionStrategy old_sep = separatist_strategy;
    UnitGenome old_genomes[UNIT_TYPE_COUNT];
    memcpy(old_genomes, unit_genomes, sizeof(UnitGenome) * UNIT_TYPE_COUNT);

    // Apply strategy genes (indices 0..STRATEGY_GENE_COUNT-1)
    set_republic_strategy(rep_weights);
    set_separatist_strategy(sep_weights);

    // Apply unit genes. Iteration order must match ALL_GENES layout.
    int rep_idx = STRATEGY_GENE_COUNT;
    int sep_idx = STRATEGY_GENE_COUNT;
    for (unsigned int i = 0; i < UNIT_TYPE_COUNT; i++) {
        UnitType type = (UnitType)i;
        Faction f = get_faction_of_unit(type);
        if (f == FACTION_UNKNOWN) continue;
        if (!is_unit_used_in_composition(type)) continue;
        
        if (f == FACTION_REPUBLIC) {
            unit_genomes[i].unit_aggression = rep_weights[rep_idx + 0];
            unit_genomes[i].preferred_range = rep_weights[rep_idx + 1];
            unit_genomes[i].spacing_x       = rep_weights[rep_idx + 2];
            unit_genomes[i].spacing_y       = rep_weights[rep_idx + 3];
            rep_idx += UNIT_TYPE_GENE_COUNT;
        } else {
            unit_genomes[i].unit_aggression = sep_weights[sep_idx + 0];
            unit_genomes[i].preferred_range = sep_weights[sep_idx + 1];
            unit_genomes[i].spacing_x       = sep_weights[sep_idx + 2];
            unit_genomes[i].spacing_y       = sep_weights[sep_idx + 3];
            sep_idx += UNIT_TYPE_GENE_COUNT;
        }
    }

    // Run battles.
    int rep_wins = 0, sep_wins = 0, timeouts = 0;
    for (int i = 0; i < num_battles; i++) {
        Battlefield field;
        initialize_battlefield(&field);
        spawn_republic_forces(&field, default_republic_composition, default_republic_composition_count);
        spawn_separatist_forces(&field, default_separatist_composition, default_separatist_composition_count);

        int winner = run_battle(&field, MAX_TURNS);
        if (winner == 0) rep_wins++;
        else if (winner == 1) sep_wins++;
        else if (winner == 2) timeouts++;

        cleanup_battlefield(&field);
    }

    // Restore.
    republic_strategy = old_rep;
    separatist_strategy = old_sep;
    memcpy(unit_genomes, old_genomes, sizeof(UnitGenome) * UNIT_TYPE_COUNT);

    float penalty = 1.0f;
    PairFitness out;
    out.rep_fitness = (float)rep_wins / num_battles - penalty * (float)timeouts / num_battles;
    out.sep_fitness = (float)sep_wins / num_battles - penalty * (float)timeouts / num_battles;
    return out;
}

// Co-evolution
void run_coevolution(const GeneInfo* rep_genes, size_t rep_gene_count,
                     const GeneInfo* sep_genes, size_t sep_gene_count,
                     int pop_size, int num_generations,
                     PairFitness (*evaluate_pair)(const float*, const float*, int),
                     const char* name_prefix) {
    Individual* rep_pop = malloc(pop_size * sizeof(Individual));
    Individual* sep_pop = malloc(pop_size * sizeof(Individual));
    Individual* next_rep = malloc(pop_size * sizeof(Individual));
    Individual* next_sep = malloc(pop_size * sizeof(Individual));
    if (!rep_pop || !sep_pop || !next_rep || !next_sep) {
        printf("Memory allocation failed.\n");
        return;
    }

    for (int i = 0; i < pop_size; i++) {
        random_individual(&rep_pop[i], rep_genes, rep_gene_count);
        random_individual(&sep_pop[i], sep_genes, sep_gene_count);
        next_rep[i].weights = malloc(rep_gene_count * sizeof(float));
        next_sep[i].weights = malloc(sep_gene_count * sizeof(float));
        next_rep[i].fitness = 0.0f;
        next_sep[i].fitness = 0.0f;
    }

    float best_rep_fitness = -9999.0f;
    int generations_without_improvement = 0;
    int gen;

    for (gen = 0; gen < num_generations; gen++) {
        float gen_ratio = (float)gen / num_generations;
        int num_battles = battles_min + (int)((battles_max - battles_min) * gen_ratio);
        if (num_battles > battles_max) num_battles = battles_max;

        float current_mutation_rate = mutation_rate_start + (mutation_rate_end - mutation_rate_start) * gen_ratio;
        float current_mutation_delta = mutation_delta_start + (mutation_delta_end - mutation_delta_start) * gen_ratio;

        for (int i = 0; i < pop_size; i++) {
            float total = 0.0f;
            for (int j = 0; j < pop_size; j++) {
                PairFitness pf = evaluate_pair(rep_pop[i].weights, sep_pop[j].weights, num_battles);
                total += pf.rep_fitness;
            }
            rep_pop[i].fitness = total / pop_size;
        }

        for (int j = 0; j < pop_size; j++) {
            float total = 0.0f;
            for (int i = 0; i < pop_size; i++) {
                PairFitness pf = evaluate_pair(rep_pop[i].weights, sep_pop[j].weights, num_battles);
                total += pf.sep_fitness;
            }
            sep_pop[j].fitness = total / pop_size;
        }

        qsort(rep_pop, pop_size, sizeof(Individual), compare_individual);
        qsort(sep_pop, pop_size, sizeof(Individual), compare_individual);

        float avg_rep = 0.0f, avg_sep = 0.0f;
        for (int i = 0; i < pop_size; i++) {
            avg_rep += rep_pop[i].fitness;
            avg_sep += sep_pop[i].fitness;
        }
        avg_rep /= pop_size;
        avg_sep /= pop_size;

        printf("Gen %2d: %s Rep best=%.4f avg=%.4f | Sep best=%.4f avg=%.4f\n",
               gen, name_prefix, rep_pop[0].fitness, avg_rep, sep_pop[0].fitness, avg_sep);

        if (rep_pop[0].fitness > best_rep_fitness + improvement_threshold) {
            best_rep_fitness = rep_pop[0].fitness;
            generations_without_improvement = 0;
        } else {
            generations_without_improvement++;
        }
        if (generations_without_improvement >= max_plateau_generations) {
            printf("Plateau detected - stopping early after %d generations.\n", gen + 1);
            break;
        }

        int elite_count = 3;

        for (int i = 0; i < elite_count; i++) {
            memcpy(next_rep[i].weights, rep_pop[i].weights, rep_gene_count * sizeof(float));
            next_rep[i].fitness = rep_pop[i].fitness;
        }
        for (int i = elite_count; i < pop_size; i++) {
            int p1 = tournament_select(rep_pop, pop_size, 3);
            int p2 = tournament_select(rep_pop, pop_size, 3);
            crossover(rep_pop[p1].weights, rep_pop[p2].weights, next_rep[i].weights, rep_gene_count, crossover_rate);
            mutate(next_rep[i].weights, rep_genes, rep_gene_count, current_mutation_rate, current_mutation_delta);
            next_rep[i].fitness = 0.0f;
        }

        for (int i = 0; i < elite_count; i++) {
            memcpy(next_sep[i].weights, sep_pop[i].weights, sep_gene_count * sizeof(float));
            next_sep[i].fitness = sep_pop[i].fitness;
        }
        for (int i = elite_count; i < pop_size; i++) {
            int p1 = tournament_select(sep_pop, pop_size, 3);
            int p2 = tournament_select(sep_pop, pop_size, 3);
            crossover(sep_pop[p1].weights, sep_pop[p2].weights, next_sep[i].weights, sep_gene_count, crossover_rate);
            mutate(next_sep[i].weights, sep_genes, sep_gene_count, current_mutation_rate, current_mutation_delta);
            next_sep[i].fitness = 0.0f;
        }

        Individual* tmp = rep_pop;
        rep_pop = next_rep;
        next_rep = tmp;
        tmp = sep_pop;
        sep_pop = next_sep;
        next_sep = tmp;
    }

    int generations_run = gen + 1;
    float completion = (float)generations_run / num_generations;
    if (completion >= 0.75f) {
        printf("\n=== CO-EVOLUTION COMPLETE (%s) ===\n", name_prefix);
        printf("Best Republic fitness: %.4f\n", rep_pop[0].fitness);
        printf("Best Separatist fitness: %.4f\n", sep_pop[0].fitness);

        print_genome(rep_pop[0].weights, rep_genes, rep_gene_count, "Republic");
        print_genome(sep_pop[0].weights, sep_genes, sep_gene_count, "Separatist");
        save_genomes(rep_pop[0].weights, rep_gene_count, sep_pop[0].weights, sep_gene_count, rep_genes, sep_genes, name_prefix);
    } else {
        printf("\nWARNING: Stopped early (%.1f%%). Skipping save.\n", completion * 100);
    }

    for (int i = 0; i < pop_size; i++) {
        free(rep_pop[i].weights);
        free(sep_pop[i].weights);
        free(next_rep[i].weights);
        free(next_sep[i].weights);
    }
    free(rep_pop);
    free(sep_pop);
    free(next_rep);
    free(next_sep);
}

// Print and save
void print_genome(const float* w, const GeneInfo* genes, size_t gene_count, const char* name) {
    printf("%s genome:\n", name);
    for (size_t i = 0; i < gene_count; i++) {
        printf("  %s: %.2f\n", genes[i].name, w[i]);
    }
}

void save_genomes(const float* rep_weights, size_t rep_count,
                  const float* sep_weights, size_t sep_count,
                  const GeneInfo* rep_genes, const GeneInfo* sep_genes,
                  const char* prefix) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char filename[128];
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y%m%d_%H%M%S", tm);
    snprintf(filename, sizeof(filename), "best_%s_%s.txt", prefix, time_str);
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Error: Could not save genomes.\n");
        return;
    }
    fprintf(f, "# Best co-evolved genomes for %s\n\n", prefix);
    fprintf(f, "[REPUBLIC]\n");
    for (size_t i = 0; i < rep_count; i++) {
        fprintf(f, "%s %.2f\n", rep_genes[i].name, rep_weights[i]);
    }
    fprintf(f, "\n[SEPARATIST]\n");
    for (size_t i = 0; i < sep_count; i++) {
        fprintf(f, "%s %.2f\n", sep_genes[i].name, sep_weights[i]);
    }
    fclose(f);
    printf("Genomes saved to %s\n", filename);
}

// Entry point
void run_evolution(void) {
    printf("\n=== STARTING EVOLUTION ===\n");
    printf("Population: %d, Generations: %d, Genes: %zu\n",
           pop_size, num_generations, REP_GENE_COUNT);
    printf("Mutation rate: %.2f -> %.2f, Delta: %.2f -> %.2f\n",
           mutation_rate_start, mutation_rate_end,
           mutation_delta_start, mutation_delta_end);
    printf("Crossover rate: %.2f\n", crossover_rate);

    run_coevolution(REP_GENES, REP_GENE_COUNT,
                SEP_GENES, SEP_GENE_COUNT,
                pop_size, num_generations,
                evaluate_all, "combined");
}