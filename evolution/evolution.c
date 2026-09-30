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

#define MAX_TURNS 35

EvolutionMode current_mode = EVOLVE_ALL;

// Gene definitions

const GeneInfo STRATEGY_GENES[] = {
    // Tactical (CAT_TACTICAL)
    {"advance_distance", 8.0f, 25.0f, 12.00f, CAT_TACTICAL},    // If the enemy center > than advance_dist * aggr_bias, advance. Smaller = aggressive
    {"flanking_bias", 0.5f, 2.0f, 1.3f, CAT_TACTICAL},          // Lateral movement when advancing 0.5 = straight, 2.0 da long way
    {"explosive_threshold", 1.0f, 4.0f, 2.0f, CAT_TACTICAL},   // Minimum enemies in a blast radius. 1 = single enemies, 4 = dense enemies
    {"retreat_hp_ratio", 0.1f, 0.6f, 0.3f, CAT_TACTICAL},      // If HP is below max_hp * retreat_hp_ratio, retreat. lower = in fight longer
    {"cohesion_threshold", 2.0f, 8.0f, 5.0f, CAT_TACTICAL},    // Maximum distance from the squad center. 2 = tight formation, 8 = loose formation
    
    // Strategic (CAT_STRATEGIC)
    {"aggression_bias", 0.5f, 2.0f, 0.8f, CAT_STRATEGIC},       // Global aggression. Higher = more aggressive (more advance, less retreat)
    {"aggression_curve", 0.5f, 6.0f, 3.14f, CAT_STRATEGIC},     // How aggression changes with HP loss. >1 = more aggr. when wounded, <1 = more cautious
    {"target_priority", 0.0f, 1.0f, 0.5f, CAT_STRATEGIC},       // Which enemy to target. 0 = lowest HP, 1 = highest point-value
    {"ally_proximity", 2.0f, 8.0f, 6.14f, CAT_STRATEGIC},       // Preferred distance to nearest ally. (* 1.5) higher = stay with the squad
    {"threat_weight", 0.0f, 1.0f, 0.9f, CAT_STRATEGIC},         // 0 = prefer low-hp enemies, 1 = prefer high-value enemies
    {"focus_fire_tendency", 0.0f, 2.0f, 1.00f, CAT_STRATEGIC},  // Whether a squad prefers to focus or spread fire. Higher = more focus
    {"flank_left_weight", 0.0f, 10.0f, 5.0f, CAT_STRATEGIC},    // Preference for flanking to the left
    {"flank_right_weight", 0.0f, 10.0f, 5.0f, CAT_STRATEGIC},   // Preference for flanking to the right
    {"breakthrough_weight", 0.0f, 10.0f, 5.0f, CAT_STRATEGIC},  // Preference for having a unit break through the center
    {"fallback_weight", 0.0f, 10.0f, 5.0f, CAT_STRATEGIC},      // Preference for having units fall back while preserving formation
    {"edge_avoidance_weight", 0.0f, 1.0f, 0.6f, CAT_STRATEGIC}, // Preference for avoiding the map edge
    
    // Deployment (CAT_DEPLOYMENT)
    {"deploy_horizontal_spread", 0.0f, 10.0f, 6.00f, CAT_DEPLOYMENT},   // How wide the deployment is. 0 = clumped, 10 = spread across the width of the map
    {"deploy_vertical_offset", 0.0f, 10.0f, 4.00f, CAT_DEPLOYMENT}, // How far back units are placed in their spawn zone. 0 = front line, 10 = far back
    {"deploy_flank_bias", -5.0f, 5.0f, 0.0f, CAT_DEPLOYMENT}   // Which side the deployment is. -5 = far left, 5 = far right
};
const size_t STRATEGY_GENE_COUNT = sizeof(STRATEGY_GENES) / sizeof(GeneInfo);

const GeneInfo UNIT_GENES[] = {
    {"aggression", 0.0f, 1.0f, 0.5f, CAT_ALL},
    {"explosive_threshold", 1.0f, 4.0f, 2.0f, CAT_ALL},
    {"ally_proximity", 2.0f, 8.0f, 5.0f, CAT_ALL},
    {"retreat_hp_ratio", 0.1f, 0.6f, 0.3f, CAT_ALL},
    {"preferred_range", 0.0f, 36.0f, 12.0f, CAT_ALL},
    {"danger_range", 0.0f, 20.0f, 4.0f, CAT_ALL}
};
const size_t UNIT_TYPE_GENE_COUNT = sizeof(UNIT_GENES) / sizeof(GeneInfo);
const size_t UNIT_GENE_COUNT = UNIT_TYPE_COUNT * UNIT_TYPE_GENE_COUNT;

// Evolution parameters
int battles_min = 50;
int battles_max = 100;
int current_battles = 20;

int pop_size = 30;
int num_generations = 30;

float mutation_rate_start = 0.5f;
float mutation_rate_end = 0.2f;
float mutation_delta_start = 0.4f;
float mutation_delta_end = 0.1f;

int max_plateau_generations = 25;
float improvement_threshold = 0.005f;
float crossover_rate = 0.8f;

/*
10 min unit evolution: 20-40 battles, 25 population, 30 generations, 0.45-0.15 mutation rate, 0.35-0.08 mutation delta, 15 plateau
10 min strat evolution: 40-80 battles, 20 population, 25 generations, 0.35-0.08 mutation rate, 0.25-0.05 mutation delta, 15 plateau

30 min unit evolution: 30-60 battles, 30 population, 50 generations, 0.5-0.2 mutation rate, 0.4-0.1 mutation delta, 25 plateau
30 min strat evolution: 60-120 battles, 30 population, 40 generations, 0.35-0.08 mutation rate, 0.25-0.05 mutation delta, 20 plateau

60 min unit evolution: 40-80 battles, 40 population, 80 generations, 0.55-0.25 mutation rate, 0.45-0.12 mutation delta, 35 plateau
60 min strat evolution: 80-150 battles, 40 population, 60 generations, 0.35-0.08 mutation rate, 0.25-0.05 mutation delta, 25 plateau
*/

typedef struct {
    float* weights;
    float fitness;
} Individual;

float rand_float(float low, float high) {
    return low + ((float)rand() / RAND_MAX) * (high - low);
}

float gaussian_rand() {
    float u = (float)rand() / RAND_MAX;
    float v = (float)rand() / RAND_MAX;
    return sqrt(-2.0f * log(u)) * cos(2.0f * M_PI * v);
}

void random_individual(Individual* ind, const GeneInfo* genes, size_t gene_count, EvolutionMode mode) {
    ind->weights = malloc(gene_count * sizeof(float));
    for (size_t i = 0; i < gene_count; i++) {
        if (mode == EVOLVE_ALL || mode == EVOLVE_UNIT || (genes[i].category & mode)) {
            ind->weights[i] = rand_float(genes[i].min, genes[i].max);
        } else {
            ind->weights[i] = genes[i].default_val;
        }
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

void mutate(float* weights, const GeneInfo* genes, size_t gene_count, EvolutionMode mode, float rate, float delta) {
    for (size_t i = 0; i < gene_count; i++) {
        if (mode != EVOLVE_ALL && mode != EVOLVE_UNIT && !(genes[i].category & mode)) continue;
        if (rand_float(0.0f, 1.0f) < rate) {
            float range = genes[i].max - genes[i].min;
            float scaled_delta = delta * range / 10.0f;
            float d = gaussian_rand() * scaled_delta;
            weights[i] += d;
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

// Evaluation function
float evaluate_unit_pair(const float* rep_weights, const float* sep_weights, int num_battles) {
    UnitGenome old_genomes[UNIT_TYPE_COUNT];
    memcpy(old_genomes, unit_genomes, sizeof(UnitGenome) * UNIT_TYPE_COUNT);

    // Helper macro to clamp a value to a range
    #define CLAMP(value, min_val, max_val) \
        do { if ((value) < (min_val)) (value) = (min_val); \
             if ((value) > (max_val)) (value) = (max_val); } while(0)

    for (unsigned int i = 0; i < UNIT_TYPE_COUNT; i++) {
        UnitType type = (UnitType)i;
        Faction f = get_faction_of_unit(type);
        if (f == FACTION_UNKNOWN) continue;
        int base = i * UNIT_TYPE_GENE_COUNT;

        if (f == FACTION_REPUBLIC) {
            unit_genomes[i].unit_aggression = rep_weights[base + 0];
            CLAMP(unit_genomes[i].unit_aggression, 0.0f, 1.0f);

            unit_genomes[i].unit_ally_proximity = rep_weights[base + 2];
            CLAMP(unit_genomes[i].unit_ally_proximity, 2.0f, 8.0f);

            unit_genomes[i].unit_retreat_hp_ratio = rep_weights[base + 4];
            CLAMP(unit_genomes[i].unit_retreat_hp_ratio, 0.1f, 0.6f);

            unit_genomes[i].preferred_range = rep_weights[base + 5];
            CLAMP(unit_genomes[i].preferred_range, 0.0f, 36.0f);

            unit_genomes[i].danger_range = rep_weights[base + 6];
            CLAMP(unit_genomes[i].danger_range, 0.0f, 20.0f);

        } else if (f == FACTION_SEPARATIST) {
            unit_genomes[i].unit_aggression = sep_weights[base + 0];
            CLAMP(unit_genomes[i].unit_aggression, 0.0f, 1.0f);

            unit_genomes[i].unit_ally_proximity = sep_weights[base + 2];
            CLAMP(unit_genomes[i].unit_ally_proximity, 2.0f, 8.0f);

            unit_genomes[i].unit_retreat_hp_ratio = sep_weights[base + 4];
            CLAMP(unit_genomes[i].unit_retreat_hp_ratio, 0.1f, 0.6f);

            unit_genomes[i].preferred_range = sep_weights[base + 5];
            CLAMP(unit_genomes[i].preferred_range, 0.0f, 36.0f);

            unit_genomes[i].danger_range = sep_weights[base + 6];
            CLAMP(unit_genomes[i].danger_range, 0.0f, 20.0f);
        }
    }

    int rep_wins = 0;
    int timeouts = 0;

    for (int i = 0; i < num_battles; i++) {
        Battlefield field;
        initialize_battlefield(&field);

        spawn_republic_forces(&field, default_republic_composition, default_republic_composition_count);
        spawn_separatist_forces(&field, default_separatist_composition, default_separatist_composition_count);

        int winner = run_battle(&field, MAX_TURNS);
        if (winner == 0) rep_wins++;
        else if (winner == 2) {
            timeouts++;
        }
        cleanup_battlefield(&field);
    }

    memcpy(unit_genomes, old_genomes, sizeof(UnitGenome) * UNIT_TYPE_COUNT);


    float win_rate = (float)rep_wins / num_battles;
    float timeout_rate = (float)timeouts / num_battles;
    float penalty_factor = 1.0f; // make sure to adjust this to control aggressiveness
    float fitness = win_rate - (timeout_rate * penalty_factor);

    return fitness;

    #undef CLAMP
}

// Co-evolution
void run_coevolution(const GeneInfo* genes, size_t gene_count, int pop_size, int num_generations,
    float (*evaluate_pair)(const float* rep_genome, const float* sep_genome, int num_battles), const char* name_prefix, EvolutionMode mode) {
    // Allocate populations (weights will be allocated inside)
    Individual* rep_pop = malloc(pop_size * sizeof(Individual));
    Individual* sep_pop = malloc(pop_size * sizeof(Individual));
    Individual* next_rep = malloc(pop_size * sizeof(Individual));
    Individual* next_sep = malloc(pop_size * sizeof(Individual));
    if (!rep_pop || !sep_pop || !next_rep || !next_sep) {
        printf("Memory allocation failed.\n");
        return;
    }

    // Initialise all four populations with allocated weight arrays
    for (int i = 0; i < pop_size; i++) {
        random_individual(&rep_pop[i], genes, gene_count, mode);
        random_individual(&sep_pop[i], genes, gene_count, mode);
        // Allocate weight arrays for the "next" populations
        next_rep[i].weights = malloc(gene_count * sizeof(float));
        next_sep[i].weights = malloc(gene_count * sizeof(float));
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

        // Evaluate Republic vs Separatist population
        for (int i = 0; i < pop_size; i++) {
            float total = 0.0f;
            for (int j = 0; j < pop_size; j++) {
                total += evaluate_pair(rep_pop[i].weights, sep_pop[j].weights, num_battles);
            }
            rep_pop[i].fitness = total / pop_size;
        }

        for (int j = 0; j < pop_size; j++) {
            float total = 0.0f;
            for (int i = 0; i < pop_size; i++) {
                total += 1.0f - evaluate_pair(rep_pop[i].weights, sep_pop[j].weights, num_battles);
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

        // ---- Republic: write into next_rep ----
        // Elitism: copy best individuals
        for (int i = 0; i < elite_count; i++) {
            memcpy(next_rep[i].weights, rep_pop[i].weights, gene_count * sizeof(float));
            next_rep[i].fitness = rep_pop[i].fitness;
        }
        // Fill the rest via crossover + mutation
        for (int i = elite_count; i < pop_size; i++) {
            int p1 = tournament_select(rep_pop, pop_size, 3);
            int p2 = tournament_select(rep_pop, pop_size, 3);
            crossover(rep_pop[p1].weights, rep_pop[p2].weights, next_rep[i].weights, gene_count, crossover_rate);
            mutate(next_rep[i].weights, genes, gene_count, mode, current_mutation_rate, current_mutation_delta);
            next_rep[i].fitness = 0.0f;
        }

        // ---- Separatists: write into next_sep ----
        for (int i = 0; i < elite_count; i++) {
            memcpy(next_sep[i].weights, sep_pop[i].weights, gene_count * sizeof(float));
            next_sep[i].fitness = sep_pop[i].fitness;
        }
        for (int i = elite_count; i < pop_size; i++) {
            int p1 = tournament_select(sep_pop, pop_size, 3);
            int p2 = tournament_select(sep_pop, pop_size, 3);
            crossover(sep_pop[p1].weights, sep_pop[p2].weights, next_sep[i].weights, gene_count, crossover_rate);
            mutate(next_sep[i].weights, genes, gene_count, mode, current_mutation_rate, current_mutation_delta);
            next_sep[i].fitness = 0.0f;
        }

        // Swap pointers (no freeing here)
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

        if (genes == UNIT_GENES) {
            // Unit genome mode
            print_unit_genome(rep_pop[0].weights, FACTION_REPUBLIC, "Republic");
            print_unit_genome(sep_pop[0].weights, FACTION_SEPARATIST, "Separatist");
            save_unit_genome(rep_pop[0].weights, FACTION_REPUBLIC, name_prefix);
            save_unit_genome(sep_pop[0].weights, FACTION_SEPARATIST, name_prefix);
        } else {
            // Strategy genome mode
            print_genome(rep_pop[0].weights, genes, gene_count, "Republic");
            print_genome(sep_pop[0].weights, genes, gene_count, "Separatist");
            save_genomes(rep_pop[0].weights, sep_pop[0].weights, genes, gene_count, name_prefix);
        }
    } else {
        printf("\nWARNING: Stopped early (%.1f%%). Skipping save.\n", completion * 100);
    }

    // Cleanup: free all weight arrays
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

// Print and save functions
void print_genome(const float* w, const GeneInfo* genes, size_t gene_count, const char* name) {
    printf("%s genome:\n", name);
    for (size_t i = 0; i < gene_count; i++) {
        printf("  %s: %.2f\n", genes[i].name, w[i]);
    }
}

void save_genomes(const float* rep_weights, const float* sep_weights, const GeneInfo* genes, size_t gene_count, const char* prefix) {
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
    for (size_t i = 0; i < gene_count; i++) {
        fprintf(f, "%s %.2f\n", genes[i].name, rep_weights[i]);
    }
    fprintf(f, "\n[SEPARATIST]\n");
    for (size_t i = 0; i < gene_count; i++) {
        fprintf(f, "%s %.2f\n", genes[i].name, sep_weights[i]);
    }
    fclose(f);
    printf("Genomes saved to %s\n", filename);
}

void print_unit_genome(const float* w, Faction faction, const char* name) {
    printf("%s unit genome:\n", name);
    for (unsigned int i = 0; i < UNIT_TYPE_COUNT; i++) {
        UnitType type = (UnitType)i;
        if (get_faction_of_unit(type) != faction) continue;
        // Only print if this unit type is used in the composition
        if (!is_unit_used_in_composition(type)) continue;
        int base = i * UNIT_TYPE_GENE_COUNT;
        const char* type_name = get_unit_type_name(type);
        printf("  [%s]\n", type_name);
        for (size_t g = 0; g < UNIT_TYPE_GENE_COUNT; g++) {
            // Clamp value to bounds for display
            float val = w[base + g];
            // Apply clamping (same as in evaluate_unit_pair)
            #define CLAMP_PRINT(value, min_val, max_val) \
                do { if ((value) < (min_val)) (value) = (min_val); \
                     if ((value) > (max_val)) (value) = (max_val); } while(0)
            if (g == 0) CLAMP_PRINT(val, 0.0f, 1.0f);
            else if (g == 1) CLAMP_PRINT(val, 1.0f, 4.0f);
            else if (g == 2) CLAMP_PRINT(val, 2.0f, 8.0f);
            else if (g == 3) CLAMP_PRINT(val, 0.0f, 1.0f);
            else if (g == 4) CLAMP_PRINT(val, 0.1f, 0.6f);
            else if (g == 5) CLAMP_PRINT(val, 0.0f, 36.0f);
            else if (g == 6) CLAMP_PRINT(val, 0.0f, 20.0f);
            #undef CLAMP_PRINT
            printf("    %s: %.2f\n", UNIT_GENES[g].name, val);
        }
    }
}

void save_unit_genome(const float* weights, Faction faction, const char* prefix) {
    (void)prefix; // shut up about the parameters
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char filename[128];
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y%m%d_%H%M%S", tm);
    const char* faction_name = (faction == FACTION_REPUBLIC) ? "republic" : "separatist";
    snprintf(filename, sizeof(filename), "best_unit_%s_%s.txt", faction_name, time_str);
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Error: Could not save unit genome.\n");
        return;
    }
    fprintf(f, "# Best unit genome for %s\n\n", (faction == FACTION_REPUBLIC) ? "Republic" : "Separatist");

    for (unsigned int i = 0; i < UNIT_TYPE_COUNT; i++) {
        UnitType type = (UnitType)i;
        if (get_faction_of_unit(type) != faction) continue;
        // Only save units that are used in the composition
        if (!is_unit_used_in_composition(type)) continue;

        int base = i * UNIT_TYPE_GENE_COUNT;
        const char* type_name = get_unit_type_name(type);
        fprintf(f, "[%s]\n", type_name);
        for (size_t g = 0; g < UNIT_TYPE_GENE_COUNT; g++) {
            // Clamp value to bounds before saving)
            float val = weights[base + g];
            #define CLAMP_SAVE(value, min_val, max_val) \
                do { if ((value) < (min_val)) (value) = (min_val); \
                     if ((value) > (max_val)) (value) = (max_val); } while(0)
            if (g == 0) CLAMP_SAVE(val, 0.0f, 1.0f);
            else if (g == 1) CLAMP_SAVE(val, 1.0f, 4.0f);
            else if (g == 2) CLAMP_SAVE(val, 2.0f, 8.0f);
            else if (g == 3) CLAMP_SAVE(val, 0.0f, 1.0f);
            else if (g == 4) CLAMP_SAVE(val, 0.1f, 0.6f);
            else if (g == 5) CLAMP_SAVE(val, 0.0f, 36.0f);
            else if (g == 6) CLAMP_SAVE(val, 0.0f, 20.0f);
            #undef CLAMP_SAVE
            fprintf(f, "%s %.2f\n", UNIT_GENES[g].name, val);
        }
        fprintf(f, "\n");
    }
    fclose(f);
    printf("Unit genomes saved to %s\n", filename);
}

// Main evolution function
void run_evolution(EvolutionMode mode) {
    current_mode = mode;
    
    printf("\n=== STARTING EVOLUTION ===\n");
    printf("Population: %d, Generations: %d\n", pop_size, num_generations);
    printf("Mutation rate: %.2f -> %.2f, Delta: %.2f -> %.2f\n", mutation_rate_start, mutation_rate_end, mutation_delta_start, mutation_delta_end);
    printf("Crossover rate: %.2f\n", crossover_rate);

    if (mode == EVOLVE_UNIT) {
        run_coevolution(UNIT_GENES, UNIT_GENE_COUNT, pop_size, num_generations, evaluate_unit_pair, "unit", mode);
        return;
    }

    // Strategy evolution (TACTICAL, STRATEGIC, DEPLOYMENT, ALL)
    run_coevolution(STRATEGY_GENES, STRATEGY_GENE_COUNT, pop_size, num_generations, evaluate_matchup, "strategy", mode);
}