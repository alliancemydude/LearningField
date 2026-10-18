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
#include "composition.h"
#include "strategy.h"
#include "unit_genome.h"
#include "unit_utils.h"
#include "utils.h"
#include "queries.h"
#include "unit.h"

// Gene definitions
const GeneInfo GERMAN_GENES[] = {
    // Strategy (0-6)
    {"strategy.aggression_bias",          0.5f,  2.0f, 1.2f},
    {"strategy.threat_weight",            0.0f,  1.0f, 0.5f},
    {"strategy.flank_percentage",         0.0f,  1.0f, 0.2f},
    {"strategy.flanking_bias",            0.5f,  2.0f, 1.0f},
    {"strategy.deploy_horizontal_spread", 0.0f, 10.0f, 6.0f},
    {"strategy.deploy_vertical_offset",   0.0f, 10.0f, 4.0f},
    {"strategy.flank_depth",              0.0f, 20.0f, 4.0f},
    {"strategy.face_threat_weight",       0.0f,  1.0f, 0.0f},
    {"strategy.flank_targeting_weight",   0.0f,  1.0f, 0.0f},
    {"strategy.cohesion_weight",          0.0f,  1.0f, 0.0f},

    {"barbarian.aggression",      0.0f,  1.0f, 0.5f},
    {"barbarian.preferred_range", 0.0f, 1.0f, 0.5f},
    {"barbarian.spacing_x",       1.0f,  8.0f, 3.0f},
    {"barbarian.spacing_y",       1.0f,  8.0f, 3.0f},
    {"barbarian.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"elite_barbarian.aggression",      0.0f,  1.0f, 0.5f},
    {"elite_barbarian.preferred_range", 0.0f, 1.0f, 0.5f},
    {"elite_barbarian.spacing_x",       1.0f,  8.0f, 3.0f},
    {"elite_barbarian.spacing_y",       1.0f,  8.0f, 3.0f},
    {"elite_barbarian.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"javelin_thrower.aggression",      0.0f,  1.0f, 0.5f},
    {"javelin_thrower.preferred_range", 0.0f, 1.0f, 0.5f},
    {"javelin_thrower.spacing_x",       1.0f,  8.0f, 3.0f},
    {"javelin_thrower.spacing_y",       1.0f,  8.0f, 3.0f},
    {"javelin_thrower.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"german_cavalry.aggression",      0.0f,  1.0f, 0.5f},
    {"german_cavalry.preferred_range", 0.0f, 1.0f, 0.5f},
    {"german_cavalry.spacing_x",       1.0f,  8.0f, 3.0f},
    {"german_cavalry.spacing_y",       1.0f,  8.0f, 3.0f},
    {"german_cavalry.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"composition.barbarian_share",        0.0f, 1.0f, 0.5f},
    {"composition.elite_barbarian_share",  0.0f, 1.0f, 0.5f},
    {"composition.javelin_thrower_share",  0.0f, 1.0f, 0.5f},
    {"composition.german_cavalry_share",         0.0f, 1.0f, 0.5f}
};
const size_t GERMAN_GENE_COUNT = sizeof(GERMAN_GENES) / sizeof(GeneInfo);

const GeneInfo ROMAN_GENES[] = {
    // Strategy (0-6)
    {"strategy.aggression_bias",          0.5f,  2.0f, 1.2f},
    {"strategy.threat_weight",            0.0f,  1.0f, 0.5f},
    {"strategy.flank_percentage",         0.0f,  1.0f, 0.2f},
    {"strategy.flanking_bias",            0.5f,  2.0f, 1.0f},
    {"strategy.deploy_horizontal_spread", 0.0f, 10.0f, 6.0f},
    {"strategy.deploy_vertical_offset",   0.0f, 10.0f, 4.0f},
    {"strategy.flank_depth",              0.0f, 20.0f, 4.0f},
    {"strategy.face_threat_weight",       0.0f,  1.0f, 0.0f},
    {"strategy.flank_targeting_weight",   0.0f,  1.0f, 0.0f},
    {"strategy.cohesion_weight",          0.0f,  1.0f, 0.0f},

    {"legionare.aggression",      0.0f,  1.0f, 0.5f},
    {"legionare.preferred_range", 0.0f, 1.0f, 0.5f},
    {"legionare.spacing_x",       1.0f,  8.0f, 3.0f},
    {"legionare.spacing_y",       1.0f,  8.0f, 3.0f},
    {"legionare.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"centurion.aggression",      0.0f,  1.0f, 0.5f},
    {"centurion.preferred_range", 0.0f, 1.0f, 0.5f},
    {"centurion.spacing_x",       1.0f,  8.0f, 3.0f},
    {"centurion.spacing_y",       1.0f,  8.0f, 3.0f},
    {"centurion.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"archer.aggression",      0.0f,  1.0f, 0.5f},
    {"archer.preferred_range", 0.0f, 1.0f, 0.5f},
    {"archer.spacing_x",       1.0f,  8.0f, 3.0f},
    {"archer.spacing_y",       1.0f,  8.0f, 3.0f},
    {"archer.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"roman_cavalry.aggression",      0.0f,  1.0f, 0.5f},
    {"roman_cavalry.preferred_range", 0.0f, 1.0f, 0.5f},
    {"roman_cavalry.spacing_x",       1.0f,  8.0f, 3.0f},
    {"roman_cavalry.spacing_y",       1.0f,  8.0f, 3.0f},
    {"roman_cavalry.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"composition.legionare_share",        0.0f, 1.0f, 0.5f},
    {"composition.centurion_share",  0.0f, 1.0f, 0.5f},
    {"composition.archer_share",  0.0f, 1.0f, 0.5f},
    {"composition.roman_cavalry_share",         0.0f, 1.0f, 0.5f}
};
const size_t ROMAN_GENE_COUNT = sizeof(ROMAN_GENES) / sizeof(GeneInfo);

// Evolution parameters
int battles_min = 40;
int battles_max = 60;

int pop_size = 30;
int num_generations = 60;

float mutation_rate_start = 0.5f;
float mutation_rate_end = 0.15f;
float mutation_delta_start = 0.4f;
float mutation_delta_end = 0.08f;
float crossover_rate = 0.8f;

typedef struct {
    float* weights;
    float fitness;           // raw fitness, logging
    float selection_score;   // rank-based, tournament selection
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
    ind->selection_score = 0.0f;
}

int tournament_select(const Individual pop[], int size, int tournament_size) {
    int best_idx = rand() % size;
    for (int i = 1; i < tournament_size; i++) {
        int idx = rand() % size;
        if (pop[idx].selection_score > pop[best_idx].selection_score)
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

static void apply_genes(const float* german_weights, const float* roman_weights) {
    set_german_strategy(german_weights);
    set_roman_strategy(roman_weights);

    int german_idx = STRATEGY_GENE_COUNT;
    int roman_idx = STRATEGY_GENE_COUNT;
    for (unsigned int i = 0; i < UNIT_TYPE_COUNT; i++) {
        UnitType type = (UnitType)i;
        Faction f = get_faction_of_unit(type);
        if (f == FACTION_UNKNOWN) continue;

        if (f == FACTION_GERMAN) {
            unit_genomes[i].unit_aggression = german_weights[german_idx + 0];
            unit_genomes[i].preferred_range = german_weights[german_idx + 1];
            unit_genomes[i].spacing_x       = german_weights[german_idx + 2];
            unit_genomes[i].spacing_y       = german_weights[german_idx + 3];
            unit_genomes[i].local_force_ratio    = german_weights[german_idx + 4];
            german_idx += UNIT_TYPE_GENE_COUNT;
        } else {
            unit_genomes[i].unit_aggression = roman_weights[roman_idx + 0];
            unit_genomes[i].preferred_range = roman_weights[roman_idx + 1];
            unit_genomes[i].spacing_x       = roman_weights[roman_idx + 2];
            unit_genomes[i].spacing_y       = roman_weights[roman_idx + 3];
            unit_genomes[i].local_force_ratio    = roman_weights[roman_idx + 4];
            roman_idx += UNIT_TYPE_GENE_COUNT;
        }
    }

    // Read composition genes
    german_strategy.composition.count = 4;
    roman_strategy.composition.count  = 4;
    for (int i = 0; i < 4; i++) {
        german_strategy.composition.share[i] = german_weights[german_idx + i];
        roman_strategy.composition.share[i]  = roman_weights[roman_idx + i];
    }
}

static void print_comp_line(ForceComposition* comp, int n) {
    int total = 0;
    for (int i = 0; i < n; i++) {
        total += comp[i].count;
        if (comp[i].count > 0) {
            printf("%d%s ", comp[i].count, short_unit_name(comp[i].type));
        }
    }
    printf("(%d units)", total);
}

static void print_generation_diagnostics(const float* german_weights, const float* roman_weights, int num_battles, const char* label)
{
    FactionStrategy old_german = german_strategy;
    FactionStrategy old_roman  = roman_strategy;
    UnitGenome old_genomes[UNIT_TYPE_COUNT];
    memcpy(old_genomes, unit_genomes, sizeof(UnitGenome) * UNIT_TYPE_COUNT);

    apply_genes(german_weights, roman_weights);

    ForceComposition german_comp[COMPOSITION_MAX_TYPES];
    ForceComposition roman_comp[COMPOSITION_MAX_TYPES];
    int german_comp_n = build_composition(&german_strategy.composition, FACTION_GERMAN, german_comp);
    int roman_comp_n  = build_composition(&roman_strategy.composition,  FACTION_ROMAN,  roman_comp);

    int german_wins = 0, roman_wins = 0, timeouts = 0;
    long total_german_dmg = 0, total_roman_dmg = 0;
    long total_turns = 0;
    long total_german_surv = 0, total_roman_surv = 0;

    for (int i = 0; i < num_battles; i++) {
        Battlefield field;
        initialize_battlefield(&field);
        spawn_german_forces(&field, german_comp, german_comp_n);
        spawn_roman_forces(&field, roman_comp, roman_comp_n);

        int winner = run_battle(&field, MAX_TURNS);
        if (winner == 0) german_wins++;
        else if (winner == 1) roman_wins++;
        else timeouts++;

        total_german_dmg += battle_stats.damage_taken_roman;
        total_roman_dmg  += battle_stats.damage_taken_german;
        total_turns      += turn_count;

        // Count survivors per faction
        int g_surv = 0, r_surv = 0;
        for (int u = 0; u < field.unit_count; u++) {
            Unit* unit = field.units[u];
            if (!unit || unit->hp <= 0) continue;
            if (unit->faction == FACTION_GERMAN) g_surv++;
            else                                 r_surv++;
        }
        total_german_surv += g_surv;
        total_roman_surv  += r_surv;

        cleanup_battlefield(&field);
    }

    german_strategy = old_german;
    roman_strategy = old_roman;
    memcpy(unit_genomes, old_genomes, sizeof(UnitGenome) * UNIT_TYPE_COUNT);

    long total_dmg = total_german_dmg + total_roman_dmg;
    float g_dmg_pct = (total_dmg > 0) ? 100.0f * total_german_dmg / total_dmg : 0.0f;
    float r_dmg_pct = (total_dmg > 0) ? 100.0f * total_roman_dmg  / total_dmg : 0.0f;

    printf("  [%s]\n", label);
    printf("    G: "); print_comp_line(german_comp, german_comp_n); printf("\n");
    printf("    R: "); print_comp_line(roman_comp,  roman_comp_n);  printf("\n");
    printf("    G %2dW / R %2dW / %2dTO  |  Dmg %3.0f/%3.0f  |  "
           "Turns %.1f  |  Surv %.1f/%.1f\n",
           german_wins, roman_wins, timeouts,
           g_dmg_pct, r_dmg_pct,
           (double)total_turns / num_battles,
           (double)total_german_surv / num_battles,
           (double)total_roman_surv / num_battles);
}

// Combined evaluator
PairFitness evaluate_all(const float* german_weights, const float* roman_weights, int num_battles) {
    // Stash current state so we can restore after the batch.
    FactionStrategy old_german = german_strategy;
    FactionStrategy old_roman = roman_strategy;
    UnitGenome old_genomes[UNIT_TYPE_COUNT];
    memcpy(old_genomes, unit_genomes, sizeof(UnitGenome) * UNIT_TYPE_COUNT);

    // Apply strategy genes
    apply_genes(german_weights, roman_weights);

    // Build force compositions from the genes.
    ForceComposition german_comp[COMPOSITION_MAX_TYPES];
    ForceComposition roman_comp[COMPOSITION_MAX_TYPES];
    int german_comp_n = build_composition(&german_strategy.composition, FACTION_GERMAN, german_comp);
    int roman_comp_n  = build_composition(&roman_strategy.composition,  FACTION_ROMAN,  roman_comp);

    // Weights (TUNE THESE IF NEEDED)
    const float W_WIN           = 1.0f;
    const float W_DAMAGE_SHARE  = 0.3f;
    const float W_TURN_PENALTY  = 0.15f;
    const float W_TIMEOUT       = 0.5f;
    const float W_NO_ENGAGE     = 0.05f;

    float german_score = 0.0f;
    float roman_score = 0.0f;
    int   timeouts  = 0;

    for (int i = 0; i < num_battles; i++) {
        Battlefield field;
        initialize_battlefield(&field);
        spawn_german_forces(&field, german_comp, german_comp_n);
        spawn_roman_forces(&field, roman_comp, roman_comp_n);

        int winner = run_battle(&field, MAX_TURNS);

        // Outcome
        if (winner == 0) {
            german_score += W_WIN;
        } else if (winner == 1) {
            roman_score += W_WIN;
        } else {
            timeouts++;
        }

        // Damage share
        int german_dmg = battle_stats.damage_taken_roman;
        int roman_dmg = battle_stats.damage_taken_german;
        int total_dmg = german_dmg + roman_dmg;

        if (total_dmg > 0) {
            german_score += W_DAMAGE_SHARE * ((float)german_dmg / total_dmg);
            roman_score += W_DAMAGE_SHARE * ((float)roman_dmg / total_dmg);
        } else {
            // Nobody engaged at all
            german_score -= W_NO_ENGAGE;
            roman_score -= W_NO_ENGAGE;
        }

        // Turn efficiency penalty (both sides)
        float turn_ratio = (float)turn_count / MAX_TURNS;
        german_score -= W_TURN_PENALTY * turn_ratio;
        roman_score -= W_TURN_PENALTY * turn_ratio;

        cleanup_battlefield(&field);
    }

    // Average over battles
    german_score /= num_battles;
    roman_score /= num_battles;

    // Timeout penalty applied to both sides
    float timeout_rate = (float)timeouts / num_battles;
    german_score -= W_TIMEOUT * timeout_rate;
    roman_score -= W_TIMEOUT * timeout_rate;

    // Restore
    german_strategy = old_german;
    roman_strategy = old_roman;
    memcpy(unit_genomes, old_genomes, sizeof(UnitGenome) * UNIT_TYPE_COUNT);

    PairFitness out;
    out.german_fitness = german_score;
    out.roman_fitness = roman_score;
    return out;
}

// Co-evolution
void run_coevolution(const GeneInfo* german_genes, size_t german_gene_count, const GeneInfo* roman_genes, size_t roman_gene_count,
                     int pop_size, int num_generations, PairFitness (*evaluate_pair)(const float*, const float*, int), const char* name_prefix) {
    
    // Create populations of the current and next units
    Individual* german_pop = malloc(pop_size * sizeof(Individual));
    Individual* roman_pop = malloc(pop_size * sizeof(Individual));
    Individual* next_german = malloc(pop_size * sizeof(Individual));
    Individual* next_roman = malloc(pop_size * sizeof(Individual));
    if (!german_pop || !roman_pop || !next_german || !next_roman) {
        printf("Memory allocation failed.\n");
        return;
    }

    // Select random individuals and assign weights, resetting fitness
    for (int i = 0; i < pop_size; i++) {
        random_individual(&german_pop[i], german_genes, german_gene_count);
        random_individual(&roman_pop[i], roman_genes, roman_gene_count);
        next_german[i].weights = malloc(german_gene_count * sizeof(float));
        next_roman[i].weights = malloc(roman_gene_count * sizeof(float));
        next_german[i].fitness = 0.0f;
        next_roman[i].fitness = 0.0f;
    }

    // Loop through the generations
    int gen;
    for (gen = 0; gen < num_generations; gen++) {
        float gen_ratio = (float)gen / num_generations;
        int num_battles = battles_min + (int)((battles_max - battles_min) * gen_ratio);
        if (num_battles > battles_max) {
            num_battles = battles_max;
        }

        // Get current mutation rate and delta
        float current_mutation_rate = mutation_rate_start + (mutation_rate_end - mutation_rate_start) * gen_ratio;
        float current_mutation_delta = mutation_delta_start + (mutation_delta_end - mutation_delta_start) * gen_ratio;

        // Make a new cache for holding info on pairings in the form of a 2D array
        PairFitness* cache = malloc(pop_size * pop_size * sizeof(PairFitness));
        if (!cache) {
            printf("Memory allocation failed (cache).\n");
            for (int i = 0; i < pop_size; i++) {
                free(german_pop[i].weights);
                free(roman_pop[i].weights);
                free(next_german[i].weights);
                free(next_roman[i].weights);
            }
            free(german_pop);
            free(roman_pop);
            free(next_german);
            free(next_roman);
            return;
        }

        // Compute every pairing once and cache both scores.
        // cache[i * pop_size + j] holds the scores for German individual i
        // vs roman individual j.
        for (int i = 0; i < pop_size; i++) {
            for (int j = 0; j < pop_size; j++) {
                cache[i * pop_size + j] = evaluate_pair(german_pop[i].weights, roman_pop[j].weights, num_battles);
            }
        }

        // German scores: average across each row
        for (int i = 0; i < pop_size; i++) {
            float total = 0.0f;
            for (int j = 0; j < pop_size; j++) {
                total += cache[i * pop_size + j].german_fitness;
            }
            german_pop[i].fitness = total / pop_size;
        }

        // Roman scores: average down each column
        for (int j = 0; j < pop_size; j++) {
            float total = 0.0f;
            for (int i = 0; i < pop_size; i++) {
                total += cache[i * pop_size + j].roman_fitness;
            }
            roman_pop[j].fitness = total / pop_size;
        }

        free(cache);

        // Sort the data in ascending order
        qsort(german_pop, pop_size, sizeof(Individual), compare_individual);
        qsort(roman_pop, pop_size, sizeof(Individual), compare_individual);

        // Compute average scores for both sides
        float avg_german = 0.0f;
        float avg_roman = 0.0f;
        for (int i = 0; i < pop_size; i++) {
            avg_german += german_pop[i].fitness;
            avg_roman += roman_pop[i].fitness;
        }
        avg_german /= pop_size;
        avg_roman /= pop_size;

        // Print best and average fitness
        printf("Gen %2d: %s German best=%.4f avg=%.4f | Roman best=%.4f avg=%.4f\n", gen, name_prefix, german_pop[0].fitness, avg_german, roman_pop[0].fitness, avg_roman);

        // Run several matchups. Uncomment later if you want?
        print_generation_diagnostics(german_pop[0].weights, roman_pop[0].weights, num_battles, "best G vs best R");
        // print_generation_diagnostics(german_pop[0].weights, roman_pop[pop_size/2].weights, num_battles, "best G vs mid R");
        // print_generation_diagnostics(german_pop[pop_size/2].weights, roman_pop[pop_size/2].weights, num_battles, "mid G vs mid R");

        // Rank-based selection scores. 
        for (int i = 0; i < pop_size; i++) {
            float score = 1.0f - (float)i / (pop_size - 1);
            german_pop[i].selection_score = score;
            roman_pop[i].selection_score = score;
        }

        // Take the best 3 genomes to repopulate future generations. Adjust this count if need be
        int elite_count = 3;

        // Populate next German units
        for (int i = 0; i < elite_count; i++) {
            memcpy(next_german[i].weights, german_pop[i].weights, german_gene_count * sizeof(float));
            next_german[i].fitness = german_pop[i].fitness;
            next_german[i].selection_score = german_pop[i].selection_score;
        }
        
        // Do a tournament select for crossover genes, and do mutation.
        for (int i = elite_count; i < pop_size; i++) {
            int p1 = tournament_select(german_pop, pop_size, 3);
            int p2 = tournament_select(german_pop, pop_size, 3);
            crossover(german_pop[p1].weights, german_pop[p2].weights, next_german[i].weights, german_gene_count, crossover_rate);
            mutate(next_german[i].weights, german_genes, german_gene_count, current_mutation_rate, current_mutation_delta);
            next_german[i].fitness = 0.0f;
            next_german[i].selection_score = 0.0f;
        }

        // Populate next Roman units
        for (int i = 0; i < elite_count; i++) {
            memcpy(next_roman[i].weights, roman_pop[i].weights, roman_gene_count * sizeof(float));
            next_roman[i].fitness = roman_pop[i].fitness;
            next_roman[i].selection_score = roman_pop[i].selection_score;
        }

        // Same as above
        for (int i = elite_count; i < pop_size; i++) {
            int p1 = tournament_select(roman_pop, pop_size, 3);
            int p2 = tournament_select(roman_pop, pop_size, 3);
            crossover(roman_pop[p1].weights, roman_pop[p2].weights, next_roman[i].weights, roman_gene_count, crossover_rate);
            mutate(next_roman[i].weights, roman_genes, roman_gene_count, current_mutation_rate, current_mutation_delta);
            next_roman[i].fitness = 0.0f;
            next_roman[i].selection_score = 0.0f;
        }

        // Checkpoint save every 20 generations, currently not needed but very safe
        /*
        if ((gen + 1) % 20 == 0) {
            char ckpt[64];
            snprintf(ckpt, sizeof(ckpt), "checkpoint_gen%d", gen + 1);
            printf("\n--- Checkpoint at generation %d ---\n", gen + 1);
            save_genomes(german_pop[0].weights, german_gene_count, roman_pop[0].weights, roman_gene_count, german_genes, roman_genes, ckpt);
            printf("\n");
        }
        */

        // Add fresh random individuals every 10 generations to maintain diversity.
        if ((gen + 1) % 10 == 0) {
            int immigrants = pop_size / 10;   // 3 out of 30
            for (int i = 0; i < immigrants; i++) {
                int slot = pop_size - 1 - i;   // replace the worst performers
                random_individual(&next_german[slot], german_genes, german_gene_count);
                random_individual(&next_roman[slot], roman_genes, roman_gene_count);
            }
        }

        // Swap pointers
        Individual* tmp = german_pop;
        german_pop = next_german;
        next_german = tmp;
        tmp = roman_pop;
        roman_pop = next_roman;
        next_roman = tmp;
    }

    // Finish 
    int generations_run = gen + 1;
    float completion = (float)generations_run / num_generations;
    if (completion >= 0.75f) {
        printf("\n=== CO-EVOLUTION COMPLETE (%s) ===\n", name_prefix);
        printf("Best German fitness: %.4f\n", german_pop[0].fitness);
        printf("Best Roman fitness: %.4f\n", roman_pop[0].fitness);

        print_genome(german_pop[0].weights, german_genes, german_gene_count, "German");
        print_genome(roman_pop[0].weights, roman_genes, roman_gene_count, "Roman");
        save_genomes(german_pop[0].weights, german_gene_count, roman_pop[0].weights, roman_gene_count, german_genes, roman_genes, name_prefix);
    } else {
        printf("\nWARNING: Stopped early (%.1f%%). Skipping save.\n", completion * 100);
    }

    for (int i = 0; i < pop_size; i++) {
        free(german_pop[i].weights);
        free(roman_pop[i].weights);
        free(next_german[i].weights);
        free(next_roman[i].weights);
    }
    free(german_pop);
    free(roman_pop);
    free(next_german);
    free(next_roman);
}

// Print and save
void print_genome(const float* w, const GeneInfo* genes, size_t gene_count, const char* name) {
    printf("%s genome:\n", name);
    for (size_t i = 0; i < gene_count; i++) {
        printf("  %s: %.2f\n", genes[i].name, w[i]);
    }
}

void save_genomes(const float* german_weights, size_t german_count, const float* roman_weights, size_t roman_count,
                  const GeneInfo* german_genes, const GeneInfo* roman_genes, const char* prefix) {
    // Stash global state so we can build the actual compositions from the winner's genes
    FactionStrategy old_german = german_strategy;
    FactionStrategy old_roman  = roman_strategy;
    UnitGenome old_genomes[UNIT_TYPE_COUNT];
    memcpy(old_genomes, unit_genomes, sizeof(UnitGenome) * UNIT_TYPE_COUNT);

    apply_genes(german_weights, roman_weights);

    ForceComposition german_comp[COMPOSITION_MAX_TYPES];
    ForceComposition roman_comp[COMPOSITION_MAX_TYPES];
    int german_n = build_composition(&german_strategy.composition, FACTION_GERMAN, german_comp);
    int roman_n  = build_composition(&roman_strategy.composition,  FACTION_ROMAN,  roman_comp);

    // Write the file
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char filename[128];
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y%m%d_%H%M%S", tm);
    snprintf(filename, sizeof(filename), "best_%s_%s.txt", prefix, time_str);

    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Error: Could not save genomes.\n");
        german_strategy = old_german;
        roman_strategy = old_roman;
        memcpy(unit_genomes, old_genomes, sizeof(UnitGenome) * UNIT_TYPE_COUNT);
        return;
    }
    fprintf(f, "# Best co-evolved genomes for %s\n\n", prefix);

    fprintf(f, "[GERMAN]\n");
    for (size_t i = 0; i < german_count; i++) {
        fprintf(f, "%s %.2f\n", german_genes[i].name, german_weights[i]);
    }
    for (int i = 0; i < german_n; i++) {
        fprintf(f, "composition.%s_count %d\n",
                short_unit_name(german_comp[i].type), german_comp[i].count);
    }

    fprintf(f, "\n[ROMAN]\n");
    for (size_t i = 0; i < roman_count; i++) {
        fprintf(f, "%s %.2f\n", roman_genes[i].name, roman_weights[i]);
    }
    for (int i = 0; i < roman_n; i++) {
        fprintf(f, "composition.%s_count %d\n",
                short_unit_name(roman_comp[i].type), roman_comp[i].count);
    }

    fclose(f);
    printf("Genomes saved to %s\n", filename);

    // Restore state
    german_strategy = old_german;
    roman_strategy = old_roman;
    memcpy(unit_genomes, old_genomes, sizeof(UnitGenome) * UNIT_TYPE_COUNT);
}

void run_evolution(void) {
    printf("\n=== STARTING EVOLUTION ===\n");
    printf("Population: %d, Generations: %d, Genes: %zu\n", pop_size, num_generations, GERMAN_GENE_COUNT);
    printf("Mutation rate: %.2f -> %.2f, Delta: %.2f -> %.2f\n", mutation_rate_start, mutation_rate_end, mutation_delta_start, mutation_delta_end);
    printf("Crossover rate: %.2f\n", crossover_rate);
    printf("Best possible fitness: 1.4. Good fitness: 0.6-0.8.\n");

    run_coevolution(GERMAN_GENES, GERMAN_GENE_COUNT, ROMAN_GENES, ROMAN_GENE_COUNT, pop_size, num_generations, evaluate_all, "combined");
}