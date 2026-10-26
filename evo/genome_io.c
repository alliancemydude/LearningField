#include "genome_io.h"
#include "evaluate.h"
#include "strategy.h"
#include "composition.h"
#include "unit_genome.h"
#include "unit_utils.h"
#include "unit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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