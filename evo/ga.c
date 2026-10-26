#include "ga.h"
#include "evaluate.h"
#include "genome_io.h"
#include "genes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

static float rand_float(float low, float high) {
    return low + ((float)rand() / RAND_MAX) * (high - low);
}

static float gaussian_rand(void) {
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

void run_evolution(void) {
    printf("\n=== STARTING EVOLUTION ===\n");
    printf("Population: %d, Generations: %d, Genes: %zu\n", pop_size, num_generations, GERMAN_GENE_COUNT);
    printf("Mutation rate: %.2f -> %.2f, Delta: %.2f -> %.2f\n", mutation_rate_start, mutation_rate_end, mutation_delta_start, mutation_delta_end);
    printf("Crossover rate: %.2f\n", crossover_rate);
    printf("Best possible fitness: 1.4. Good fitness: 0.6-0.8.\n");

    run_coevolution(GERMAN_GENES, GERMAN_GENE_COUNT, ROMAN_GENES, ROMAN_GENE_COUNT, pop_size, num_generations, evaluate_all, "combined");
}