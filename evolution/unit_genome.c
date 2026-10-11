#include "unit_genome.h"
#include "config.h"

UnitGenome unit_genomes[UNIT_TYPE_COUNT] = {
    [UNIT_SWORDSMAN] = {
        .unit_aggression = 0.5f,
        .preferred_range = 0.5f,
        .spacing_x = 4.0f,
        .spacing_y = 4.0f,
        .local_force_ratio = 0.7
    },
    [UNIT_ELITE_SWORDSMAN] = {
        .unit_aggression = 0.5f,
        .preferred_range = 0.5f,
        .spacing_x = 4.0f,
        .spacing_y = 4.0f,
        .local_force_ratio = 0.7
    },
    [UNIT_SPEAR_THROWER] = {
        .unit_aggression = 0.5f,
        .preferred_range = 0.5f,
        .spacing_x = 4.0f,
        .spacing_y = 4.0f,
        .local_force_ratio = 0.7
    },
    [UNIT_HORSEMAN] = {
        .unit_aggression = 0.5f,
        .preferred_range = 0.5f,
        .spacing_x = 4.0f,
        .spacing_y = 4.0f,
        .local_force_ratio = 0.7
    },
    [UNIT_SPEARMAN] = {
        .unit_aggression = 0.5f,
        .preferred_range = 0.5f,
        .spacing_x = 4.0f,
        .spacing_y = 4.0f,
        .local_force_ratio = 0.7
    },
    [UNIT_ELITE_SPEARMAN] = {
        .unit_aggression = 0.5f,
        .preferred_range = 0.5f,
        .spacing_x = 4.0f,
        .spacing_y = 4.0f,
        .local_force_ratio = 0.7
    },
    [UNIT_BOWMAN] = {
        .unit_aggression = 0.5f,
        .preferred_range = 0.5f,
        .spacing_x = 4.0f,
        .spacing_y = 4.0f,
        .local_force_ratio = 0.7
    },
    [UNIT_CAVALRY] = {
        .unit_aggression = 0.5f,
        .preferred_range = 0.5f,
        .spacing_x = 4.0f,
        .spacing_y = 4.0f,
        .local_force_ratio = 0.7
    }
};