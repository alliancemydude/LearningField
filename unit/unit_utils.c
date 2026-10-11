#include "unit.h"
#include "unit_utils.h"

// Point values for troops. Useful for when point estimations get added
int unit_point_values[UNIT_TYPE_COUNT] = {
    // Republic Infantry
    [UNIT_SWORDSMAN] = 10,
    [UNIT_ELITE_SWORDSMAN] = 17,
    [UNIT_SPEAR_THROWER] = 27,
    [UNIT_HORSEMAN] = 24,

    // Separatist Infantry
    [UNIT_SPEARMAN] = 10,
    [UNIT_ELITE_SPEARMAN] = 20,
    [UNIT_BOWMAN] = 22,
    [UNIT_CAVALRY] = 12
};