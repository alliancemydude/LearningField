#include "unit_genome.h"
#include "config.h"

UnitGenome unit_genomes[UNIT_TYPE_COUNT] = {
    [UNIT_SWORDSMAN] = {
        .unit_aggression = 0.68f,
        .unit_ally_proximity = 4.16f,
        .preferred_range = 2.13f
    },
    [UNIT_ELITE_SWORDSMAN] = {
        .unit_aggression = 0.0f,
        .unit_ally_proximity = 2.00f,
        .preferred_range = 1.0f
    },
    [UNIT_LONGBOWMAN] = {
        .unit_aggression = 0.52f,
        .unit_ally_proximity = 2.00f,
        .preferred_range = 8.85f
    },
    [UNIT_HORSEMAN] = {
        .unit_aggression = 0.02f,
        .unit_ally_proximity = 2.00f,
        .preferred_range = 2.0f
    },

    [UNIT_SPEARMAN] = {
        .unit_aggression = 0.70f,
        .unit_ally_proximity = 3.00f,
        .preferred_range = 2.10f
    },
    [UNIT_ELITE_SPEARMAN] = {
        .unit_aggression = 0.00f,
        .unit_ally_proximity = 2.00f,
        .preferred_range = 2.00f
    },
    [UNIT_SHORTBOWMAN] = {
        .unit_aggression = 0.9f,
        .unit_ally_proximity = 4.00f,
        .preferred_range = 10.00f
    },
    [UNIT_CAMELMAN] = {
        .unit_aggression = 1.0f,
        .unit_ally_proximity = 2.0f,
        .preferred_range = 2.2f
    }
};