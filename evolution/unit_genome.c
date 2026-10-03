#include "unit_genome.h"
#include "config.h"

UnitGenome unit_genomes[UNIT_TYPE_COUNT] = {
    [UNIT_SWORDSMAN] = {
        .unit_aggression = 0.70f,
        .unit_ally_proximity = 3.00f,
        .unit_retreat_hp_ratio = 0.10f,
        .preferred_range = 0.10f,
        .danger_range = 0.00f
    },
    [UNIT_ELITE_SWORDSMAN] = {
        .unit_aggression = 0.0f,
        .unit_ally_proximity = 2.00f,
        .unit_retreat_hp_ratio = 0.1f,
        .preferred_range = 1.0f,
        .danger_range = 0.83f
    },
    [UNIT_LONGBOWMAN] = {
        .unit_aggression = 0.52f,
        .unit_ally_proximity = 2.00f,
        .unit_retreat_hp_ratio = 0.60f,
        .preferred_range = 8.85f,
        .danger_range = 5.35f
    },
    [UNIT_HORSEMAN] = {
        .unit_aggression = 0.02f,
        .unit_ally_proximity = 2.00f,
        .unit_retreat_hp_ratio = 0.60f,
        .preferred_range = 2.0f,
        .danger_range = 0.69f
    },

    [UNIT_SPEARMAN] = {
        .unit_aggression = 0.70f,
        .unit_ally_proximity = 3.00f,
        .unit_retreat_hp_ratio = 0.10f,
        .preferred_range = 0.10f,
        .danger_range = 0.00f
    },
    [UNIT_ELITE_SPEARMAN] = {
        .unit_aggression = 0.00f,
        .unit_ally_proximity = 2.00f,
        .unit_retreat_hp_ratio = 0.6f,
        .preferred_range = 2.00f,
        .danger_range = 0.37f
    },
    [UNIT_SHORTBOWMAN] = {
        .unit_aggression = 0.9f,
        .unit_ally_proximity = 4.00f,
        .unit_retreat_hp_ratio = 0.30f,
        .preferred_range = 10.00f,
        .danger_range = 6.00f
    },
    [UNIT_CAMELMAN] = {
        .unit_aggression = 1.0f,
        .unit_ally_proximity = 2.0f,
        .unit_retreat_hp_ratio = 0.6f,
        .preferred_range = 2.2f,
        .danger_range = 1.0f
    }
};