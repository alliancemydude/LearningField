#include "unit_blueprint.h"
#include "unit.h"


const UnitBlueprint unit_blueprints[UNIT_TYPE_COUNT] = {
    // German Infantry
    [UNIT_SWORDSMAN] = {
        .type = UNIT_SWORDSMAN,
        .faction = FACTION_GERMAN,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 6,
        .morale = 7,
        .armor_class = 4,
        .point_value = 10,
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 3,
        .preferred_range = 1,
        .weapons = {
            {WEAPON_SWORD}
        },
        .weapon_count = 1
    },
    [UNIT_ELITE_SWORDSMAN] = {
        .type = UNIT_ELITE_SWORDSMAN,
        .faction = FACTION_GERMAN,
        .hp = 2,
        .hp_per_soldier = 2,
        .movement = 6,
        .morale = 7,
        .armor_class = 5,
        .point_value = 30,
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 3,
        .preferred_range = 1,
        .weapons = {
            {WEAPON_SWORD}
        },
        .weapon_count = 1
    },
    [UNIT_JAVELIN_THROWER] = {
        .type = UNIT_JAVELIN_THROWER,
        .faction = FACTION_GERMAN,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 6,
        .morale = 7,
        .armor_class = 4,
        .point_value = 35,
        .is_ranged = true,
        .units_count = 1,
        .attacks_per_unit = 1,
        .preferred_range = 10,
        .weapons = {
            {WEAPON_JAVELIN}
        },
        .weapon_count = 1
    },
    [UNIT_HORSEMAN] = {
        .type = UNIT_HORSEMAN,
        .faction = FACTION_GERMAN,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 18,
        .morale = 7,
        .armor_class = 5,
        .point_value = 20,
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 2,
        .preferred_range = 1,
        .weapons = {
            {WEAPON_SWORD}
        },
        .weapon_count = 1
    },
    
    
    // Roman Infantry
    [UNIT_SPEARMAN] = {
        .type = UNIT_SPEARMAN,
        .faction = FACTION_ROMAN,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 6,
        .morale = 7,
        .armor_class = 4,
        .point_value = 10,
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .preferred_range = 2,
        .weapons = {
            {WEAPON_SPEAR}
        },
        .weapon_count = 1
    },
    [UNIT_ELITE_SPEARMAN] = {
        .type = UNIT_ELITE_SPEARMAN,
        .faction = FACTION_ROMAN,
        .hp = 2,
        .hp_per_soldier = 2,
        .movement = 6,
        .morale = 8,
        .armor_class = 5,
        .point_value = 25,
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 2,
        .preferred_range = 3,
        .weapons = {
            {WEAPON_SPEAR}
        },
        .weapon_count = 1
    },
    [UNIT_BOWMAN] = {
        .type = UNIT_BOWMAN,
        .faction = FACTION_ROMAN,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 6,
        .morale = 7,
        .armor_class = 4,
        .point_value = 40,
        .is_ranged = true,
        .units_count = 1,
        .attacks_per_unit = 1,
        .preferred_range = 20,
        .weapons = {
            {WEAPON_BOW}
        },
        .weapon_count = 1
    },
    [UNIT_CAVALRY] = {
        .type = UNIT_CAVALRY,
        .faction = FACTION_ROMAN,
        .hp = 2,
        .hp_per_soldier = 2,
        .movement = 14,
        .morale = 7,
        .armor_class = 5,
        .point_value = 20,
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .preferred_range = 2,
        .weapons = {
            {WEAPON_SPEAR}
        },
        .weapon_count = 1
    }
};