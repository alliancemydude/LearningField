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
        .unit_aggression = 0.70f,
        .preferred_range = 1.0f,
        .spacing_x = 3.0f,
        .spacing_y = 2.0f
    },
    [UNIT_LONGBOWMAN] = {
        .unit_aggression = 0.52f,
        .preferred_range = 18.0f,
        .spacing_x = 3.0f,
        .spacing_y = 3.0f
    },
    [UNIT_HORSEMAN] = {
        .unit_aggression = 0.80f,
        .preferred_range = 4.0f,
        .spacing_x = 4.0f,
        .spacing_y = 4.0f
    },

    [UNIT_SPEARMAN] = {
        .unit_aggression = 0.03f,
        .preferred_range = 4.65f,
        .spacing_x = 4.16f,
        .spacing_y = 1.86f,
        .local_force_ratio = 0.59
    },
    [UNIT_ELITE_SPEARMAN] = {
        .unit_aggression = 0.70f,
        .preferred_range = 3.0f,
        .spacing_x = 3.0f,
        .spacing_y = 2.0f
    },
    [UNIT_SHORTBOWMAN] = {
        .unit_aggression = 0.90f,
        .preferred_range = 12.0f,
        .spacing_x = 3.0f,
        .spacing_y = 3.0f
    },
    [UNIT_CAMELMAN] = {
        .unit_aggression = 1.00f,
        .preferred_range = 2.0f,
        .spacing_x = 4.0f,
        .spacing_y = 4.0f
    }
};