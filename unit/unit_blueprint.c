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
    [UNIT_SWORDSMAN] = {
        .type = UNIT_SWORDSMAN,
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
        .preferred_range = 2,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SWORD, 4}
        },
        .weapon_count = 1
    },
    [UNIT_ELITE_SWORDSMAN] = {
        .type = UNIT_ELITE_SWORDSMAN,
        .faction = FACTION_REPUBLIC,
        .hp = 8,
        .hp_per_soldier = 2,
        .movement = 12,
        .morale = 9,
        .armor_class = 5,
        .point_value = 20,
        .is_armor = false,
        .units_count = 4,
        .attacks_per_unit = 2,
        .attack_order = 1234,
        .preferred_range = 2,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SWORD, 4}
        },
        .weapon_count = 2
    },
    [UNIT_LONGBOWMAN] = {
        .type = UNIT_LONGBOWMAN,
        .faction = FACTION_REPUBLIC,
        .hp = 4,
        .hp_per_soldier = 1,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 20,
        .is_armor = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .attack_order = 3124,
        .preferred_range = 18,
        .danger_range = 12,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_LONGBOW, 4}
        },
        .weapon_count = 1
    },
    [UNIT_HORSEMAN] = {
        .type = UNIT_HORSEMAN,
        .faction = FACTION_REPUBLIC,
        .hp = 2,
        .hp_per_soldier = 1,
        .movement = 24,
        .morale = 9,
        .armor_class = 6,
        .point_value = 35,
        .is_armor = true,
        .units_count = 1,
        .attacks_per_unit = 1,
        .attack_order = 1234,
        .preferred_range = 4,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SWORD, 2}
        },
        .weapon_count = 1
    },
    
    
    // Separatist Infantry
    [UNIT_SPEARMAN] = {
        .type = UNIT_SPEARMAN,
        .faction = FACTION_SEPARATIST,
        .hp = 6,
        .hp_per_soldier = 1,
        .movement = 10,
        .morale = 8,
        .armor_class = 4,
        .point_value = 5,
        .is_armor = false,
        .units_count = 6,
        .attacks_per_unit = 1,
        .attack_order = 1234,
        .preferred_range = 4,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SPEAR, 6}
        },
        .weapon_count = 1
    },
    [UNIT_ELITE_SPEARMAN] = {
        .type = UNIT_ELITE_SPEARMAN,
        .faction = FACTION_SEPARATIST,
        .hp = 8,
        .hp_per_soldier = 2,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 30,
        .is_armor = false,
        .units_count = 4,
        .attacks_per_unit = 2,
        .attack_order = 1234,
        .preferred_range = 4,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SPEAR, 4}
        },
        .weapon_count = 1
    },
    [UNIT_SHORTBOWMAN] = {
        .type = UNIT_SHORTBOWMAN,
        .faction = FACTION_SEPARATIST,
        .hp = 4,
        .hp_per_soldier = 1,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 15,
        .is_armor = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .attack_order = 3124,
        .preferred_range = 12,
        .danger_range = 8,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SHORTBOW, 4}
        },
        .weapon_count = 1
    },
    [UNIT_CAMELMAN] = {
        .type = UNIT_CAMELMAN,
        .faction = FACTION_SEPARATIST,
        .hp = 2,
        .hp_per_soldier = 1,
        .movement = 24,
        .morale = 9,
        .armor_class = 6,
        .point_value = 30,
        .is_armor = true,
        .units_count = 1,
        .attacks_per_unit = 2,
        .attack_order = 3214,
        .preferred_range = 2,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SWORD, 2}
        },
        .weapon_count = 1
    }
};