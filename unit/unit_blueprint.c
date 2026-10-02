#include "unit_blueprint.h"
#include "unit.h"


const UnitBlueprint unit_blueprints[UNIT_TYPE_COUNT] = {
    // Republic Infantry
    [UNIT_SWORDSMAN] = {
        .type = UNIT_SWORDSMAN,
        .faction = FACTION_REPUBLIC,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 5,
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .preferred_range = 1,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SWORD}
        },
        .weapon_count = 1
    },
    [UNIT_ELITE_SWORDSMAN] = {
        .type = UNIT_ELITE_SWORDSMAN,
        .faction = FACTION_REPUBLIC,
        .hp = 2,
        .hp_per_soldier = 2,
        .movement = 12,
        .morale = 9,
        .armor_class = 5,
        .point_value = 20,
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 2,
        .preferred_range = 1,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SWORD}
        },
        .weapon_count = 1
    },
    [UNIT_LONGBOWMAN] = {
        .type = UNIT_LONGBOWMAN,
        .faction = FACTION_REPUBLIC,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 20,
        .is_ranged = true,
        .units_count = 1,
        .attacks_per_unit = 1,
        .preferred_range = 18,
        .danger_range = 12,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_LONGBOW}
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
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .preferred_range = 4,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SWORD}
        },
        .weapon_count = 1
    },
    
    
    // Separatist Infantry
    [UNIT_SPEARMAN] = {
        .type = UNIT_SPEARMAN,
        .faction = FACTION_SEPARATIST,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 8,
        .morale = 7,
        .armor_class = 4,
        .point_value = 5,
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .preferred_range = 3,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SPEAR}
        },
        .weapon_count = 1
    },
    [UNIT_ELITE_SPEARMAN] = {
        .type = UNIT_ELITE_SPEARMAN,
        .faction = FACTION_SEPARATIST,
        .hp = 2,
        .hp_per_soldier = 2,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 30,
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 2,
        .preferred_range = 3,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SPEAR}
        },
        .weapon_count = 1
    },
    [UNIT_SHORTBOWMAN] = {
        .type = UNIT_SHORTBOWMAN,
        .faction = FACTION_SEPARATIST,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 12,
        .morale = 9,
        .armor_class = 4,
        .point_value = 15,
        .is_ranged = true,
        .units_count = 1,
        .attacks_per_unit = 1,
        .preferred_range = 12,
        .danger_range = 8,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SHORTBOW}
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
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 2,
        .preferred_range = 2,
        .danger_range = 0,
        .rally_bonus = 0,
        .weapons = {
            {WEAPON_SWORD}
        },
        .weapon_count = 1
    }
};