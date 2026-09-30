#include "unit.h"
#include "unit_utils.h"

// Point values for troops. Useful for when point estimations get added
int unit_point_values[UNIT_TYPE_COUNT] = {
    // Republic Infantry
    [UNIT_CLONE_TROOPER] = 5,
    [UNIT_ELITE_CLONE] = 20,
    [UNIT_CLONE_SNIPER] = 20,
    [UNIT_CLONE_OFFICER] = 10,
    [UNIT_BARC_SPEEDER] = 20,

    // Separatist Infantry
    [UNIT_BATTLE_DROID] = 5,
    [UNIT_SUPER_BATTLE_DROID] = 30,
    [UNIT_DROID_SNIPER] = 15,
    [UNIT_DROID_OFFICER] = 10,
    [UNIT_STAP] = 30
};