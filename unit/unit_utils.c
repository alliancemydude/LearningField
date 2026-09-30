#include "unit.h"
#include "unit_utils.h"

// Point values for troops. Useful for when point estimations get added
int unit_point_values[UNIT_TYPE_COUNT] = {
    // Republic Infantry
    [UNIT_SWORDSMAN] = 5,
    [UNIT_ELITE_SWORDSMAN] = 20,
    [UNIT_LONGBOWMAN] = 20,
    [UNIT_HORSEMAN] = 10,

    // Separatist Infantry
    [UNIT_SPEARMAN] = 5,
    [UNIT_ELITE_SPEARMAN] = 30,
    [UNIT_SHORTBOWMAN] = 15,
    [UNIT_CAMELMAN] = 30
};