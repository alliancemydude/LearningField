#include "unit_genome.h"
#include "config.h"

UnitGenome unit_genomes[UNIT_TYPE_COUNT] = {
    // === REPUBLIC INFANTRY ===
    [UNIT_CLONE_TROOPER] = {
        .unit_aggression = 0.6f,
        .unit_explosive_threshold = 1.0f,
        .unit_ally_proximity = 4.9f,
        .unit_cover_preference = 0.11f,
        .unit_retreat_hp_ratio = 0.33f,
        .preferred_range = 10.45f,
        .danger_range = 6.83f
    },
    [UNIT_ELITE_CLONE] = {
        .unit_aggression = 0.0f,
        .unit_explosive_threshold = 1.00f,
        .unit_ally_proximity = 8.00f,
        .unit_cover_preference = 1.00f,
        .unit_retreat_hp_ratio = 0.46f,
        .preferred_range = 10.45f,
        .danger_range = 6.83f
    },
    [UNIT_CLONE_SNIPER] = {
        .unit_aggression = 0.52f,
        .unit_explosive_threshold = 4.00f,
        .unit_ally_proximity = 2.00f,
        .unit_cover_preference = 1.00f,
        .unit_retreat_hp_ratio = 0.60f,
        .preferred_range = 7.85f,
        .danger_range = 0.35f
    },
    [UNIT_CLONE_OFFICER] = {
        .unit_aggression = 0.02f,
        .unit_explosive_threshold = 4.00f,
        .unit_ally_proximity = 2.00f,
        .unit_cover_preference = 1.00f,
        .unit_retreat_hp_ratio = 0.60f,
        .preferred_range = 0.61f,
        .danger_range = 2.69f
    },
    [UNIT_CLONE_ROCKET] = {
        .unit_aggression = 0.4f,
        .unit_explosive_threshold = 2.0f,
        .unit_ally_proximity = 5.0f,
        .unit_cover_preference = 0.6f,
        .unit_retreat_hp_ratio = 0.35f,
        .preferred_range = 12.0f,
        .danger_range = 2.0f
    },

    // === REPUBLIC GUNNERS ===
    [UNIT_LIGHT_CLONE_GUNNER] = {
        .unit_aggression = 0.6f,
        .unit_explosive_threshold = 2.0f,
        .unit_ally_proximity = 4.0f,
        .unit_cover_preference = 0.6f,
        .unit_retreat_hp_ratio = 0.3f,
        .preferred_range = 12.0f,
        .danger_range = 2.0f
    },
    [UNIT_MEDIUM_CLONE_GUNNER] = {
        .unit_aggression = 0.7f,
        .unit_explosive_threshold = 1.5f,
        .unit_ally_proximity = 5.0f,
        .unit_cover_preference = 0.7f,
        .unit_retreat_hp_ratio = 0.3f,
        .preferred_range = 12.0f,
        .danger_range = 2.0f
    },
    [UNIT_HEAVY_CLONE_GUNNER] = {
        .unit_aggression = 0.9f,
        .unit_explosive_threshold = 1.0f,
        .unit_ally_proximity = 6.0f, 
        .unit_cover_preference = 0.8f,
        .unit_retreat_hp_ratio = 0.2f,
        .preferred_range = 12.0f,
        .danger_range = 2.0f
    },
    
    // === REPUBLIC ARMOR ===
    [UNIT_SWAMP_SPEEDER] = {
        .unit_aggression = 0.00f,
        .unit_explosive_threshold = 1.0f,
        .unit_ally_proximity = 2.0f,
        .unit_cover_preference = 0.0f,
        .unit_retreat_hp_ratio = 0.1f,
        .preferred_range = 0.0f,
        .danger_range = 0.0f
    },
    [UNIT_BARC_SPEEDER] = {
        .unit_aggression = 0.0f,
        .unit_explosive_threshold = 1.0f,
        .unit_ally_proximity = 2.0f,
        .unit_cover_preference = 0.0f,
        .unit_retreat_hp_ratio = 0.1f,
        .preferred_range = 0.0f,
        .danger_range = 0.0f
    },
    [UNIT_SABER_TANK] = {
        .unit_aggression = 0.7f,
        .unit_explosive_threshold = 1.5f,
        .unit_ally_proximity = 7.0f,
        .unit_cover_preference = 0.6f,
        .unit_retreat_hp_ratio = 0.1f,
        .preferred_range = 12.0f,
        .danger_range = 2.0f
    },

    // === SEPARATIST INFANTRY ===
    [UNIT_BATTLE_DROID] = {
        .unit_aggression = 0.00f,
        .unit_explosive_threshold = 1.0f,
        .unit_ally_proximity = 2.00f,
        .unit_cover_preference = 0.00f,
        .unit_retreat_hp_ratio = 0.6f,
        .preferred_range = 0.00f,
        .danger_range = 3.37f
    },
    [UNIT_SUPER_BATTLE_DROID] = {
        .unit_aggression = 0.9f,
        .unit_explosive_threshold = 1.00f,
        .unit_ally_proximity = 4.00f,
        .unit_cover_preference = 0.50f,
        .unit_retreat_hp_ratio = 0.30f,
        .preferred_range = 10.00f,
        .danger_range = 6.00f
    },
    [UNIT_DROID_SNIPER] = {
        .unit_aggression = 1.0f,
        .unit_explosive_threshold = 3.0f,
        .unit_ally_proximity = 2.0f,
        .unit_cover_preference = 1.0f,
        .unit_retreat_hp_ratio = 0.6f,
        .preferred_range = 0.2f,
        .danger_range = 6.0f
    },
    [UNIT_DROID_OFFICER] = {
        .unit_aggression = 1.0f,
        .unit_explosive_threshold = 2.0f,
        .unit_ally_proximity = 2.0f,
        .unit_cover_preference = 1.0f,
        .unit_retreat_hp_ratio = 0.6f,
        .preferred_range = 0.8f,
        .danger_range = 2.0f
    },
    [UNIT_DROID_ROCKET] = {
        .unit_aggression = 0.4f,
        .unit_explosive_threshold = 2.0f,
        .unit_ally_proximity = 5.0f,
        .unit_cover_preference = 0.6f,
        .unit_retreat_hp_ratio = 0.35f,
        .preferred_range = 12.0f,
        .danger_range = 2.0f
    },

    // === SEPARATIST GUNNERS ===
    [UNIT_MEDIUM_DROID_GUNNER] = {
        .unit_aggression = 0.7f,
        .unit_explosive_threshold = 1.5f,
        .unit_ally_proximity = 5.0f,
        .unit_cover_preference = 0.7f,
        .unit_retreat_hp_ratio = 0.3f,
        .preferred_range = 12.0f,
        .danger_range = 2.0f
    },
    [UNIT_HEAVY_DROID_GUNNER] = {
        .unit_aggression = 0.9f,
        .unit_explosive_threshold = 1.0f,
        .unit_ally_proximity = 6.0f,
        .unit_cover_preference = 0.8f,
        .unit_retreat_hp_ratio = 0.0f,
        .preferred_range = 12.0f,
        .danger_range = 0.0f
    },

    // === SEPARATIST ARMOR ===
    [UNIT_SPIDER_DROID] = {
        .unit_aggression = 1.0f,
        .unit_explosive_threshold = 2.0f,
        .unit_ally_proximity = 8.0f,
        .unit_cover_preference = 1.0f,
        .unit_retreat_hp_ratio = 0.1f,
        .preferred_range = 36.0f,
        .danger_range = 20.0f
    },
    [UNIT_STAP] = {
        .unit_aggression = 1.0f,
        .unit_explosive_threshold = 2.0f,
        .unit_ally_proximity = 2.0f,
        .unit_cover_preference = 1.0f,
        .unit_retreat_hp_ratio = 0.6f,
        .preferred_range = 0.0f,
        .danger_range = 20.0f
    },
    [UNIT_AAT] = {
        .unit_aggression = 0.7f,
        .unit_explosive_threshold = 1.5f,
        .unit_ally_proximity = 7.0f,
        .unit_cover_preference = 0.6f,
        .unit_retreat_hp_ratio = 0.1f,
        .preferred_range = 12.0f,
        .danger_range = 2.0f
    }
};