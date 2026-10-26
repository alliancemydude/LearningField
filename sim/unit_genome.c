#include "unit_genome.h"
#include "config.h"

UnitGenome unit_genomes[UNIT_TYPE_COUNT] = {
    [UNIT_BARBARIAN] = {
        .unit_aggression = 0.08f,
        .preferred_range = 0.59f,
        .spacing_x = 1.11f,
        .spacing_y = 2.62f,
        .local_force_ratio = 0.63
    },
    [UNIT_ELITE_BARBARIAN] = {
        .unit_aggression = 0.8f,
        .preferred_range = 0.01f,
        .spacing_x = 1.89f,
        .spacing_y = 3.80f,
        .local_force_ratio = 0.56
    },
    [UNIT_JAVELIN_THROWER] = {
        .unit_aggression = 0.84f,
        .preferred_range = 0.51f,
        .spacing_x = 1.11f,
        .spacing_y = 4.93f,
        .local_force_ratio = 0.51
    },
    [UNIT_GERMAN_CAVALRY] = {
        .unit_aggression = 0.04f,
        .preferred_range = 0.99f,
        .spacing_x = 4.43f,
        .spacing_y = 1.30f,
        .local_force_ratio = 0.5
    },
    [UNIT_LEGIONARE] = {
        .unit_aggression = 0.26f,
        .preferred_range = 0.92f,
        .spacing_x = 1.47f,
        .spacing_y = 1.22f,
        .local_force_ratio = 0.79
    },
    [UNIT_CENTURION] = {
        .unit_aggression = 0.24f,
        .preferred_range = 0.58f,
        .spacing_x = 1.07f,
        .spacing_y = 7.93f,
        .local_force_ratio = 0.50
    },
    [UNIT_ARCHER] = {
        .unit_aggression = 0.75f,
        .preferred_range = 0.47f,
        .spacing_x = 5.30f,
        .spacing_y = 8.00f,
        .local_force_ratio = 0.5
    },
    [UNIT_ROMAN_CAVALRY] = {
        .unit_aggression = 0.22f,
        .preferred_range = 0.39f,
        .spacing_x = 7.04f,
        .spacing_y = 5.25f,
        .local_force_ratio = 1.03
    }
};