#include "unit.h"
#include "unit_utils.h"

// Point values for troops. Useful for when point estimations get added
int unit_point_values[UNIT_TYPE_COUNT] = {
    // German Infantry
    [UNIT_SWORDSMAN] = 10,
    [UNIT_ELITE_SWORDSMAN] = 20,
    [UNIT_JAVELIN_THROWER] = 35,
    [UNIT_HORSEMAN] = 15,

    // Roman Infantry
    [UNIT_SPEARMAN] = 10,
    [UNIT_ELITE_SPEARMAN] = 25,
    [UNIT_BOWMAN] = 35,
    [UNIT_CAVALRY] = 15
};