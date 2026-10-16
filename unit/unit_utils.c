#include "unit.h"
#include "unit_utils.h"

// Point values for troops. Useful for when point estimations get added
int unit_point_values[UNIT_TYPE_COUNT] = {
    // German Infantry
    [UNIT_BARBARIAN] = 10,
    [UNIT_ELITE_BARBARIAN] = 20,
    [UNIT_JAVELIN_THROWER] = 35,
    [UNIT_GERMAN_CAVALRY] = 15,

    // Roman Infantry
    [UNIT_LEGIONARE] = 10,
    [UNIT_CENTURION] = 25,
    [UNIT_ARCHER] = 35,
    [UNIT_ROMAN_CAVALRY] = 15
};