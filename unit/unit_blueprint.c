#include "unit_blueprint.h"
#include "unit.h"

// Enemies are given a 4-digit number to determine their preferences, each number corresponds to their order of attack
// The numbers are in order of precedence
// 1. Target the lowest HP enemy
// 2. Target the highest pin value enemy
// 3. Target the highest value enemy
// 4. Target the most armored unit
// So a clone trooper might be 1234, and a sniper might be 3124

const UnitBlueprint unit_blueprints[UNIT_TYPE_COUNT] = {
    // Republic Infantry
    [UNIT_CLONE_TROOPER] = {
        .type = UNIT_CLONE_TROOPER,
        .faction = FACTION_REPUBLIC,
        .hp = 4,
        .hp_per_soldier = 1,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 5,
        .is_armor = false,
        .units_count = 4,
        .attacks_per_unit = 1,
        .attack_order = 1234,
        .preferred_range = 20,
        .danger_range = 6,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_BLASTER, 4},
            {WEAPON_GRENADE, 1}
        },
        .weapon_count = 2 // total weapons, including grenades
    },
    [UNIT_ELITE_CLONE] = {
        .type = UNIT_ELITE_CLONE,
        .faction = FACTION_REPUBLIC,
        .hp = 8,
        .hp_per_soldier = 2,
        .movement = 12,
        .morale = 9,
        .armor_class = 5,
        .point_value = 20,
        .is_armor = false,
        .units_count = 4,
        .attacks_per_unit = 1,
        .attack_order = 1234,
        .preferred_range = 20,
        .danger_range = 6,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_BLASTER, 4},
            {WEAPON_GRENADE, 3}
        },
        .weapon_count = 2
    },
    [UNIT_CLONE_SNIPER] = {
        .type = UNIT_CLONE_SNIPER,
        .faction = FACTION_REPUBLIC,
        .hp = 2,
        .hp_per_soldier = 2,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 20,
        .is_armor = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .attack_order = 3124,
        .preferred_range = 32,
        .danger_range = 24,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SNIPER, 1}
        },
        .weapon_count = 1
    },
    [UNIT_CLONE_OFFICER] = {
        .type = UNIT_CLONE_OFFICER,
        .faction = FACTION_REPUBLIC,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 15,
        .is_armor = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .attack_order = 1234,
        .preferred_range = 20,
        .danger_range = 12,
        .rally_bonus = 4,
        .weapons = {
            {WEAPON_BLASTER, 1}
        },
        .weapon_count = 1
    },
    [UNIT_CLONE_ROCKET] = {
        .type = UNIT_CLONE_ROCKET,
        .faction = FACTION_REPUBLIC,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 15,
        .is_armor = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .attack_order = 4312,
        .preferred_range = 20,
        .danger_range = 12,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_MISSILE_POD, 1}
        },
        .weapon_count = 1
    },

    // Republic Gunners
    [UNIT_LIGHT_CLONE_GUNNER] = {
        .type = UNIT_LIGHT_CLONE_GUNNER,
        .faction = FACTION_REPUBLIC,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 15,
        .is_armor = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .attack_order = 2134,
        .preferred_range = 20,
        .danger_range = 12,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_LIGHT_MG, 1}
        },
        .weapon_count = 1
    },
    [UNIT_MEDIUM_CLONE_GUNNER] = {
        .type = UNIT_MEDIUM_CLONE_GUNNER,
        .faction = FACTION_REPUBLIC,
        .hp = 3,
        .hp_per_soldier = 3,
        .movement = 8,
        .morale = 9,
        .armor_class = 4,
        .point_value = 20,
        .is_armor = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .attack_order = 2134,
        .preferred_range = 32,
        .danger_range = 8,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_MEDIUM_MG, 1}
        },
        .weapon_count = 1
    },
    [UNIT_HEAVY_CLONE_GUNNER] = {
        .type = UNIT_HEAVY_CLONE_GUNNER,
        .faction = FACTION_REPUBLIC,
        .hp = 3,
        .hp_per_soldier = 3,
        .movement = 2,
        .morale = 9,
        .armor_class = 4,
        .point_value = 40,
        .is_armor = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .attack_order = 2431,
        .preferred_range = 32,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_HEAVY_MG, 1}
        },
        .weapon_count = 1
    },

    // Republic Armor
    [UNIT_SWAMP_SPEEDER] = {
        .type = UNIT_SWAMP_SPEEDER,
        .faction = FACTION_REPUBLIC,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 24,
        .morale = 9,
        .armor_class = 6,
        .point_value = 35,
        .is_armor = true,
        .units_count = 1,
        .attacks_per_unit = 2,
        .attack_order = 1234,
        .preferred_range = 18,
        .danger_range = 4,
        .rally_bonus = 0,
        .weapons = {
            //{WEAPON_DUAL_BLASTER_CANNON, 2},
            {WEAPON_BEAM_CANNON, 2}//,
            //{WEAPON_MISSILE_POD, 16}
        },
        .weapon_count = 3
    },
    [UNIT_BARC_SPEEDER] = {
    .type = UNIT_BARC_SPEEDER,
    .faction = FACTION_REPUBLIC,
    .hp = 1,
    .hp_per_soldier = 1,
    .movement = 24,
    .morale = 9,
    .armor_class = 6,
    .point_value = 35,
    .is_armor = true,
    .units_count = 1,
    .attacks_per_unit = 2,
    .attack_order = 3124,
    .preferred_range = 20,
    .danger_range = 4,
    .rally_bonus = 0,
    .weapons = {
        {WEAPON_BLASTER_CANNON, 2},
        {WEAPON_BLASTER, 2}
    },
    .weapon_count = 2
},
    [UNIT_SABER_TANK] = {
        .type = UNIT_SABER_TANK,
        .faction = FACTION_REPUBLIC,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 24,
        .morale = 9,
        .armor_class = 9,
        .point_value = 100,
        .is_armor = true,
        .units_count = 1,
        .attacks_per_unit = 2,
        .attack_order = 4321,
        .preferred_range = 18,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_LIGHT_AT, 2},
            {WEAPON_BEAM_CANNON, 1},
            {WEAPON_MISSILE_POD, 6}
        },
        .weapon_count = 3
    },
    
    
    // Separatist Infantry
    [UNIT_BATTLE_DROID] = {
        .type = UNIT_BATTLE_DROID,
        .faction = FACTION_SEPARATIST,
        .hp = 6,
        .hp_per_soldier = 1,
        .movement = 12,
        .morale = 8,
        .armor_class = 4,
        .point_value = 5,
        .is_armor = false,
        .units_count = 6,
        .attacks_per_unit = 1,
        .attack_order = 1234,
        .preferred_range = 16,
        .danger_range = 6,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_CARBINE, 6}
        },
        .weapon_count = 1
    },
    [UNIT_SUPER_BATTLE_DROID] = {
        .type = UNIT_SUPER_BATTLE_DROID,
        .faction = FACTION_SEPARATIST,
        .hp = 8,
        .hp_per_soldier = 2,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 30,
        .is_armor = false,
        .units_count = 4,
        .attacks_per_unit = 1,
        .attack_order = 1234,
        .preferred_range = 12,
        .danger_range = 6,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SUB_MG, 4},
            {WEAPON_ROCKET, 4}
        },
        .weapon_count = 2
    },
    [UNIT_DROID_SNIPER] = {
        .type = UNIT_DROID_SNIPER,
        .faction = FACTION_SEPARATIST,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 15,
        .is_armor = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .attack_order = 3124,
        .preferred_range = 32,
        .danger_range = 24,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SNIPER, 1}
        },
        .weapon_count = 1
    },
    [UNIT_DROID_OFFICER] = {
        .type = UNIT_DROID_OFFICER,
        .faction = FACTION_SEPARATIST,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 15,
        .is_armor = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .attack_order = 1234,
        .preferred_range = 16,
        .danger_range = 10,
        .rally_bonus = 4,
        .weapons = {
            {WEAPON_CARBINE, 1}
        },
        .weapon_count = 1
    },
    [UNIT_DROID_ROCKET] = {
        .type = UNIT_DROID_ROCKET,
        .faction = FACTION_SEPARATIST,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 15,
        .is_armor = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .attack_order = 4312,
        .preferred_range = 20,
        .danger_range = 12,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_MISSILE_POD, 1}
        },
        .weapon_count = 1
    },

    // Separatist Gunners
    [UNIT_MEDIUM_DROID_GUNNER] = {
        .type = UNIT_MEDIUM_DROID_GUNNER,
        .faction = FACTION_SEPARATIST,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 8,
        .morale = 9,
        .armor_class = 4,
        .point_value = 15,
        .is_armor = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .attack_order = 2134,
        .preferred_range = 32,
        .danger_range = 12,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_MEDIUM_MG, 1}
        },
        .weapon_count = 1
    },
    [UNIT_HEAVY_DROID_GUNNER] = {
        .type = UNIT_HEAVY_DROID_GUNNER,
        .faction = FACTION_SEPARATIST,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 2,
        .morale = 9,
        .armor_class = 4,
        .point_value = 35,
        .is_armor = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .attack_order = 2431,
        .preferred_range = 32,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_HEAVY_MG, 1}
        },
        .weapon_count = 1
    },
    
    // Separatist Armor
    [UNIT_SPIDER_DROID] = {
        .type = UNIT_SPIDER_DROID,
        .faction = FACTION_SEPARATIST,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 6,
        .morale = 9,
        .armor_class = 7,
        .point_value = 60,
        .is_armor = true,
        .units_count = 1,
        .attacks_per_unit = 1,
        .attack_order = 4312,
        .preferred_range = 42,
        .danger_range = 8,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_LIGHT_AT, 1}
        },
        .weapon_count = 1
    },
    [UNIT_STAP] = {
        .type = UNIT_STAP,
        .faction = FACTION_SEPARATIST,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 24,
        .morale = 9,
        .armor_class = 6,
        .point_value = 30,
        .is_armor = true,
        .units_count = 1,
        .attacks_per_unit = 2,
        .attack_order = 3214,
        .preferred_range = 18,
        .danger_range = 6,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_BLASTER_CANNON, 2}
        },
        .weapon_count = 1
    },
    [UNIT_AAT] = {
        .type = UNIT_AAT,
        .faction = FACTION_SEPARATIST,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 12,
        .morale = 9,
        .armor_class = 9,
        .point_value = 100,
        .is_armor = true,
        .units_count = 1,
        .attacks_per_unit = 2,
        .attack_order = 4321,
        .preferred_range = 18,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_MEDIUM_AT, 1},
            {WEAPON_BLASTER_CANNON, 2},
            {WEAPON_BLASTER, 2},
            {WEAPON_MISSILE_POD, 6}
        },
        .weapon_count = 4
    }
};