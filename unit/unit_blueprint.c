#include "unit_blueprint.h"
#include "unit.h"


const UnitBlueprint unit_blueprints[UNIT_TYPE_COUNT] = {
    // German Infantry
    [UNIT_BARBARIAN] = {
        .type = UNIT_BARBARIAN,
        .faction = FACTION_GERMAN,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 6,
        .morale = 7,
        .armor_class = 4,
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 3,
        .preferred_range = 1,
        .weapons = {
            {WEAPON_AXE}
        },
        .weapon_count = 1
    },
    [UNIT_ELITE_BARBARIAN] = {
        .type = UNIT_ELITE_BARBARIAN,
        .faction = FACTION_GERMAN,
        .hp = 2,
        .hp_per_soldier = 2,
        .movement = 6,
        .morale = 7,
        .armor_class = 5,
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 3,
        .preferred_range = 1,
        .weapons = {
            {WEAPON_AXE}
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
        .is_ranged = true,
        .units_count = 1,
        .attacks_per_unit = 1,
        .preferred_range = 10,
        .weapons = {
            {WEAPON_JAVELIN}
        },
        .weapon_count = 1
    },
    [UNIT_GERMAN_CAVALRY] = {
        .type = UNIT_GERMAN_CAVALRY,
        .faction = FACTION_GERMAN,
        .hp = 2,
        .hp_per_soldier = 2,
        .movement = 18,
        .morale = 7,
        .armor_class = 5,
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .preferred_range = 1,
        .weapons = {
            {WEAPON_AXE}
        },
        .weapon_count = 1
    },
    
    
    // Roman Infantry
    [UNIT_LEGIONARE] = {
        .type = UNIT_LEGIONARE,
        .faction = FACTION_ROMAN,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 6,
        .morale = 7,
        .armor_class = 4,
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 1,
        .preferred_range = 2,
        .weapons = {
            {WEAPON_SPEAR}
        },
        .weapon_count = 1
    },
    [UNIT_CENTURION] = {
        .type = UNIT_CENTURION,
        .faction = FACTION_ROMAN,
        .hp = 2,
        .hp_per_soldier = 2,
        .movement = 6,
        .morale = 8,
        .armor_class = 5,
        .is_ranged = false,
        .units_count = 1,
        .attacks_per_unit = 2,
        .preferred_range = 2,
        .weapons = {
            {WEAPON_SPEAR}
        },
        .weapon_count = 1
    },
    [UNIT_ARCHER] = {
        .type = UNIT_ARCHER,
        .faction = FACTION_ROMAN,
        .hp = 1,
        .hp_per_soldier = 1,
        .movement = 6,
        .morale = 7,
        .armor_class = 4,
        .is_ranged = true,
        .units_count = 1,
        .attacks_per_unit = 1,
        .preferred_range = 20,
        .weapons = {
            {WEAPON_BOW}
        },
        .weapon_count = 1
    },
    [UNIT_ROMAN_CAVALRY] = {
        .type = UNIT_ROMAN_CAVALRY,
        .faction = FACTION_ROMAN,
        .hp = 2,
        .hp_per_soldier = 2,
        .movement = 18,
        .morale = 7,
        .armor_class = 5,
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