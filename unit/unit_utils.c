#include "unit.h"
#include "unit_utils.h"

// Point values for troops. Useful for when point estimations get added
int unit_point_values[UNIT_TYPE_COUNT] = {
    // Republic Infantry
    [UNIT_SWORDSMAN] = 5,
    [UNIT_ELITE_SWORDSMAN] = 20,
    [UNIT_SPEAR_THROWER] = 10,
    [UNIT_HORSEMAN] = 20,

    // Separatist Infantry
    [UNIT_SPEARMAN] = 5,
    [UNIT_ELITE_SPEARMAN] = 20,
    [UNIT_BOWMAN] = 10,
    [UNIT_CAVALRY] = 20
};