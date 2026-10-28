#include "unit.h"
#include "unit_utils.h"
#include "unit_registry.h"

int get_unit_point_value(UnitType type) {
    return unit_registry[type].point_value;
}

const char* short_unit_name(UnitType type) {
    return unit_registry[type].short_name;
}