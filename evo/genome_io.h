#ifndef GENOME_IO_H
#define GENOME_IO_H

#include <stddef.h>
#include "genes.h"

void print_genome(const float* w, const GeneInfo* genes, size_t gene_count, const char* name);
void save_genomes(const float* german_weights, size_t german_count,
                  const float* roman_weights, size_t roman_count,
                  const GeneInfo* german_genes, const GeneInfo* roman_genes,
                  const char* prefix);

#endif