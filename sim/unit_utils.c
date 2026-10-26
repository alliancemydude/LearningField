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
    [UNIT_CENTURION] = 20,
    [UNIT_ARCHER] = 35,
    [UNIT_ROMAN_CAVALRY] = 15
};

const char* short_unit_name(UnitType type) {
    switch (type) {
        case UNIT_BARBARIAN:        return "barbarian";
        case UNIT_ELITE_BARBARIAN:  return "elite_barb";
        case UNIT_JAVELIN_THROWER:  return "javelin";
        case UNIT_GERMAN_CAVALRY:   return "g_cav";
        case UNIT_LEGIONARE:        return "legionare";
        case UNIT_CENTURION:        return "centurion";
        case UNIT_ARCHER:           return "archer";
        case UNIT_ROMAN_CAVALRY:    return "r_cav";
        default: return "???";
    }
}