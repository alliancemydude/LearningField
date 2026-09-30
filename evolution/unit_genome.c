#include "unit_genome.h"
#include "config.h"

UnitGenome unit_genomes[UNIT_TYPE_COUNT] = {
    // === REPUBLIC INFANTRY ===
    [UNIT_CLONE_TROOPER] = {
        .unit_aggression = 0.6f,
        .unit_explosive_threshold = 1.0f,
        .unit_ally_proximity = 4.9f,
        .unit_retreat_hp_ratio = 0.33f,
        .preferred_range = 10.45f,
        .danger_range = 6.83f
    },
    [UNIT_ELITE_CLONE] = {
        .unit_aggression = 0.0f,
        .unit_explosive_threshold = 1.00f,
        .unit_ally_proximity = 8.00f,
        .unit_retreat_hp_ratio = 0.46f,
        .preferred_range = 10.45f,
        .danger_range = 6.83f
    },
    [UNIT_CLONE_SNIPER] = {
        .unit_aggression = 0.52f,
        .unit_explosive_threshold = 4.00f,
        .unit_ally_proximity = 2.00f,
        .unit_retreat_hp_ratio = 0.60f,
        .preferred_range = 7.85f,
        .danger_range = 0.35f
    },
    [UNIT_CLONE_OFFICER] = {
        .unit_aggression = 0.02f,
        .unit_explosive_threshold = 4.00f,
        .unit_ally_proximity = 2.00f,
        .unit_retreat_hp_ratio = 0.60f,
        .preferred_range = 0.61f,
        .danger_range = 2.69f
    },
    [UNIT_BARC_SPEEDER] = {
        .unit_aggression = 0.0f,
        .unit_explosive_threshold = 1.0f,
        .unit_ally_proximity = 2.0f,
        .unit_retreat_hp_ratio = 0.1f,
        .preferred_range = 0.0f,
        .danger_range = 0.0f
    },

    // === SEPARATIST INFANTRY ===
    [UNIT_BATTLE_DROID] = {
        .unit_aggression = 0.00f,
        .unit_explosive_threshold = 1.0f,
        .unit_ally_proximity = 2.00f,
        .unit_retreat_hp_ratio = 0.6f,
        .preferred_range = 0.00f,
        .danger_range = 3.37f
    },
    [UNIT_SUPER_BATTLE_DROID] = {
        .unit_aggression = 0.9f,
        .unit_explosive_threshold = 1.00f,
        .unit_ally_proximity = 4.00f,
        .unit_retreat_hp_ratio = 0.30f,
        .preferred_range = 10.00f,
        .danger_range = 6.00f
    },
    [UNIT_DROID_SNIPER] = {
        .unit_aggression = 1.0f,
        .unit_explosive_threshold = 3.0f,
        .unit_ally_proximity = 2.0f,
        .unit_retreat_hp_ratio = 0.6f,
        .preferred_range = 0.2f,
        .danger_range = 6.0f
    },
    [UNIT_DROID_OFFICER] = {
        .unit_aggression = 1.0f,
        .unit_explosive_threshold = 2.0f,
        .unit_ally_proximity = 2.0f,
        .unit_retreat_hp_ratio = 0.6f,
        .preferred_range = 0.8f,
        .danger_range = 2.0f
    },
    [UNIT_STAP] = {
        .unit_aggression = 1.0f,
        .unit_explosive_threshold = 2.0f,
        .unit_ally_proximity = 2.0f,
        .unit_retreat_hp_ratio = 0.6f,
        .preferred_range = 0.0f,
        .danger_range = 20.0f
    }
};