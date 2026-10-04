#include "unit_genome.h"
#include "config.h"

UnitGenome unit_genomes[UNIT_TYPE_COUNT] = {
    [UNIT_SWORDSMAN] = {
        .unit_aggression = 0.83f,
        .preferred_range = 1.64f,
        .spacing_x = 1.079f,
        .spacing_y = 1.77f
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
        .unit_aggression = 0.70f,
        .preferred_range = 3.0f,
        .spacing_x = 3.0f,
        .spacing_y = 2.0f
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