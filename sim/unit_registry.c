#include "unit_registry.h"
#include <string.h>

const UnitDefinition unit_registry[UNIT_TYPE_COUNT] = {
    [UNIT_BARBARIAN] = {
        .short_name = "barbarian",
        .long_name  = "UNIT_BARBARIAN",
        .faction    = FACTION_GERMAN,
        .point_value = 10,
        .hp = 1, .hp_per_soldier = 1, .movement = 8, .morale = 7,
        .armor_class = 4, .is_ranged = false,
        .units_count = 1, .attacks_per_unit = 2, .preferred_range = 1,
        .weapons = { WEAPON_AXE }, .weapon_count = 1,
    },
    [UNIT_ELITE_BARBARIAN] = {
        .short_name = "elite_barb",
        .long_name  = "UNIT_ELITE_BARBARIAN",
        .faction    = FACTION_GERMAN,
        .point_value = 20,
        .hp = 2, .hp_per_soldier = 2, .movement = 8, .morale = 7,
        .armor_class = 4, .is_ranged = false,
        .units_count = 1, .attacks_per_unit = 2, .preferred_range = 1,
        .weapons = { WEAPON_AXE }, .weapon_count = 1,
    },
    [UNIT_JAVELIN_THROWER] = {
        .short_name = "javelin",
        .long_name  = "UNIT_JAVELIN_THROWER",
        .faction    = FACTION_GERMAN,
        .point_value = 35,
        .hp = 1, .hp_per_soldier = 1, .movement = 6, .morale = 7,
        .armor_class = 4, .is_ranged = true,
        .units_count = 1, .attacks_per_unit = 1, .preferred_range = 14,
        .weapons = { WEAPON_JAVELIN }, .weapon_count = 1,
    },
    [UNIT_GERMAN_CAVALRY] = {
        .short_name = "g_cav",
        .long_name  = "UNIT_GERMAN_CAVALRY",
        .faction    = FACTION_GERMAN,
        .point_value = 15,
        .hp = 2, .hp_per_soldier = 2, .movement = 12, .morale = 7,
        .armor_class = 4, .is_ranged = false,
        .units_count = 1, .attacks_per_unit = 1, .preferred_range = 1,
        .weapons = { WEAPON_AXE }, .weapon_count = 1,
    },
    [UNIT_LEGIONARE] = {
        .short_name = "legionare",
        .long_name  = "UNIT_LEGIONARE",
        .faction    = FACTION_ROMAN,
        .point_value = 10,
        .hp = 1, .hp_per_soldier = 1, .movement = 6, .morale = 7,
        .armor_class = 4, .is_ranged = false,
        .units_count = 1, .attacks_per_unit = 1, .preferred_range = 2,
        .weapons = { WEAPON_SPEAR }, .weapon_count = 1,
    },
    [UNIT_CENTURION] = {
        .short_name = "centurion",
        .long_name  = "UNIT_CENTURION",
        .faction    = FACTION_ROMAN,
        .point_value = 20,
        .hp = 2, .hp_per_soldier = 2, .movement = 6, .morale = 8,
        .armor_class = 5, .is_ranged = false,
        .units_count = 1, .attacks_per_unit = 1, .preferred_range = 2,
        .weapons = { WEAPON_SPEAR }, .weapon_count = 1,
    },
    [UNIT_ARCHER] = {
        .short_name = "archer",
        .long_name  = "UNIT_ARCHER",
        .faction    = FACTION_ROMAN,
        .point_value = 35,
        .hp = 1, .hp_per_soldier = 1, .movement = 6, .morale = 7,
        .armor_class = 4, .is_ranged = true,
        .units_count = 1, .attacks_per_unit = 1, .preferred_range = 20,
        .weapons = { WEAPON_BOW }, .weapon_count = 1,
    },
    [UNIT_ROMAN_CAVALRY] = {
        .short_name = "r_cav",
        .long_name  = "UNIT_ROMAN_CAVALRY",
        .faction    = FACTION_ROMAN,
        .point_value = 15,
        .hp = 2, .hp_per_soldier = 2, .movement = 14, .morale = 7,
        .armor_class = 4, .is_ranged = false,
        .units_count = 1, .attacks_per_unit = 1, .preferred_range = 2,
        .weapons = { WEAPON_SPEAR }, .weapon_count = 1,
    },
};

UnitType parse_unit_type_from_name(const char* short_name) {
    for (int i = 0; i < UNIT_TYPE_COUNT; i++) {
        if (strcmp(unit_registry[i].short_name, short_name) == 0) {
            return (UnitType)i;
        }
    }
    return UNIT_TYPE_COUNT;
}