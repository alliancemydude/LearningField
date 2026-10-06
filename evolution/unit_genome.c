#include "unit_genome.h"
#include "config.h"

UnitGenome unit_genomes[UNIT_TYPE_COUNT] = {
    [UNIT_SWORDSMAN] = {
        .unit_aggression = 0.06f,
        .preferred_range = 8.68f,
        .spacing_x = 3.28f,
        .spacing_y = 7.82f,
        .local_force_ratio = 0.68
    },
    [UNIT_ELITE_SWORDSMAN] = {
        .unit_aggression = 0.06f,
        .preferred_range = 8.68f,
        .spacing_x = 3.28f,
        .spacing_y = 7.82f,
        .local_force_ratio = 0.68
    },
    [UNIT_SPEAR_THROWER] = {
        .unit_aggression = 0.06f,
        .preferred_range = 8.68f,
        .spacing_x = 3.28f,
        .spacing_y = 7.82f,
        .local_force_ratio = 0.68
    },
    [UNIT_HORSEMAN] = {
        .unit_aggression = 0.06f,
        .preferred_range = 8.68f,
        .spacing_x = 3.28f,
        .spacing_y = 7.82f,
        .local_force_ratio = 0.68
    },
    [UNIT_SPEARMAN] = {
        .unit_aggression = 0.03f,
        .preferred_range = 4.65f,
        .spacing_x = 4.16f,
        .spacing_y = 1.86f,
        .local_force_ratio = 0.59
    },
    [UNIT_ELITE_SPEARMAN] = {
        .unit_aggression = 0.03f,
        .preferred_range = 4.65f,
        .spacing_x = 4.16f,
        .spacing_y = 1.86f,
        .local_force_ratio = 0.59
    },
    [UNIT_BOWMAN] = {
        .unit_aggression = 0.03f,
        .preferred_range = 4.65f,
        .spacing_x = 4.16f,
        .spacing_y = 1.86f,
        .local_force_ratio = 0.59
    },
    [UNIT_CAVALRY] = {
        .unit_aggression = 0.03f,
        .preferred_range = 4.65f,
        .spacing_x = 4.16f,
        .spacing_y = 1.86f,
        .local_force_ratio = 0.59
    }
};