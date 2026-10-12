#include "unit.h"
#include "unit_blueprint.h"
#include "battlefield.h"
#include "unit_utils.h"
#include "queries.h"
#include <stdlib.h>

Unit* create_unit(UnitType type, int id, int x, int y) {
    if (type < 0 || type >= UNIT_TYPE_COUNT) return NULL;

    const UnitBlueprint* bp = &unit_blueprints[type];
    Unit* unit = calloc(1, sizeof(Unit));
    if (!unit) return NULL;

    // Identity + stats (unchanged)
    unit->id = id;
    unit->type = type;
    unit->faction = bp->faction;
    unit->hp = bp->hp;
    unit->max_hp = bp->hp;
    unit->hp_per_soldier = bp->hp_per_soldier;
    unit->movement = bp->movement;
    unit->half_movement = bp->movement / 2;
    unit->morale = bp->morale;
    unit->armor_class = bp->armor_class;
    unit->point_value = bp->point_value;
    unit->is_ranged = bp->is_ranged;
    unit->units_count = bp->units_count;
    unit->attacks_per_unit = bp->attacks_per_unit;

    // Weapons FIRST, so max_range can be computed correctly
    clear_weapons(unit);
    for (int i = 0; i < bp->weapon_count; i++) {
        add_weapon(unit, bp->weapons[i].type);
    }

    // Now max_range is meaningful
    unit->max_range = get_weapon_range(unit);
    unit->max_range_sq = unit->max_range * unit->max_range;

    // Genome unpacking
    UnitGenome* ug = &unit_genomes[type];
    unit->effective_aggression = ug->unit_aggression;
    unit->spacing_x = ug->spacing_x;
    unit->spacing_y = ug->spacing_y;
    unit->effective_local_force_ratio = ug->local_force_ratio;

    // Clamp preferred_range to actual weapon reach
    unit->preferred_range = (int)(ug->preferred_range * unit->max_range);
    if (unit->preferred_range < 1) unit->preferred_range = 1;

    // Status reset + position (unchanged)
    unit->pin_markers = 0;
    unit->has_acted = false;
    unit->is_stunned = false;
    unit->is_immobilized = false;
    unit->actions_taken = 0;
    unit->x = x;
    unit->y = y;

    return unit;
}

// General Use Functions

const char* get_unit_type_name(UnitType type) {
    switch(type) {
        case UNIT_SWORDSMAN: return "UNIT_SWORDSMAN";
        case UNIT_ELITE_SWORDSMAN: return "UNIT_ELITE_SWORDSMAN";
        case UNIT_JAVELIN_THROWER: return "UNIT_JAVELIN_THROWER";
        case UNIT_HORSEMAN: return "UNIT_HORSEMAN";

        case UNIT_SPEARMAN: return "UNIT_SPEARMAN";
        case UNIT_ELITE_SPEARMAN: return "UNIT_ELITE_SPEARMAN";
        case UNIT_BOWMAN: return "UNIT_BOWMAN";
        case UNIT_CAVALRY: return "UNIT_CAVALRY";
        default: return "UNKNOWN";
    }
}

void add_weapon(Unit* unit, WeaponType type) {
    if (unit->weapon_count >= WEAPON_COUNT) {
        return;
    }

    // Create a weapon pointer to get type, ammo, and stats
    Weapon* w = &unit->weapons[unit->weapon_count];
    w->type = type;
    w->equipped = true;
    
    // Set stats based on weapon types
    switch (type) {
        case WEAPON_SWORD:
            w->damage_bonus = 1;
            w->range = 2;
            w->penetration_bonus = 0;
            break;
        case WEAPON_SPEAR:
            w->damage_bonus = 0;
            w->range = 4;
            w->penetration_bonus = 1;
            break;
        case WEAPON_JAVELIN:
            w->damage_bonus = 1;
            w->range = 16;
            w->penetration_bonus = 1;
            break;
        case WEAPON_BOW:
            w->damage_bonus = 0;
            w->range = 24;
            w->penetration_bonus = 0;
            break;
        default:
            w->damage_bonus = 0;
            w->range = 2;
            break;
    }
    unit->weapon_count++;
}

void clear_weapons(Unit* unit) {
    // Reset all weapons stats for a unit
    for (int i = 0; i < WEAPON_COUNT; i++) {
        unit->weapons[i].type = (WeaponType)i;
        unit->weapons[i].range = 0;
        unit->weapons[i].damage_bonus = 0;
        unit->weapons[i].equipped = false;
    }
    unit->weapon_count = 0;
}

void destroy_unit(Unit* unit) {
    if (unit) {
        free(unit);
    }
}