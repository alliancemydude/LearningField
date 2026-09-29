#include "unit.h"
#include "unit_utils.h"

// Point values for troops. Useful for when point estimations get added
int unit_point_values[UNIT_TYPE_COUNT] = {
    // Republic Infantry
    [UNIT_CLONE_TROOPER] = 5,
    [UNIT_ELITE_CLONE] = 20,
    [UNIT_CLONE_SNIPER] = 20,
    [UNIT_CLONE_OFFICER] = 10,
    [UNIT_CLONE_ROCKET] = 15,

    // Republic Gunners
    [UNIT_LIGHT_CLONE_GUNNER] = 10,
    [UNIT_MEDIUM_CLONE_GUNNER] = 20,
    [UNIT_HEAVY_CLONE_GUNNER] = 40,

    // Republic Armor
    [UNIT_SWAMP_SPEEDER] = 20,
    [UNIT_BARC_SPEEDER] = 30,
    [UNIT_SABER_TANK] = 100,

    // Separatist Infantry
    [UNIT_BATTLE_DROID] = 5,
    [UNIT_SUPER_BATTLE_DROID] = 30,
    [UNIT_DROID_SNIPER] = 15,
    [UNIT_DROID_OFFICER] = 10,
    [UNIT_DROID_ROCKET] = 15,

    // Separatist Gunners
    [UNIT_MEDIUM_DROID_GUNNER] = 15,
    [UNIT_HEAVY_DROID_GUNNER] = 30,

    // Separatist Armor
    [UNIT_SPIDER_DROID] = 25,
    [UNIT_STAP] = 30,
    [UNIT_AAT] = 100
};