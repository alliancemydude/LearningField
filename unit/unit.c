#include "unit.h"
#include "unit_blueprint.h"
#include "battlefield.h"
#include "unit_utils.h"
#include "queries.h"
#include <stdlib.h>

Unit* create_unit(UnitType type, int id, int x, int y) {
    // Create an empty unit type. Populate all basic stats
    if (type < 0 || type >= UNIT_TYPE_COUNT) {
        return NULL;
    }
    
    // Create a blueprint array for this unit
    const UnitBlueprint* bp = &unit_blueprints[type];

    // Create the unit object
    Unit* unit = calloc(1, sizeof(Unit));
    if (!unit) {
        return NULL;
    }
    
    // Reset identity
    unit->id = id;
    unit->type = type;
    unit->faction = bp->faction;
    
    // Give stats according to blueprints
    unit->hp = bp->hp;
    unit->max_hp = bp->hp;
    unit->hp_per_soldier = bp->hp_per_soldier;
    unit->movement = bp->movement;
    unit->half_movement = bp->movement / 2;
    unit->morale = bp->morale;
    unit->rally_bonus = bp->rally_bonus;
    unit->armor_class = bp->armor_class;
    unit->point_value = bp->point_value;
    unit->is_ranged = bp->is_ranged;
    unit->units_count = bp->units_count;
    unit->attacks_per_unit = bp->attacks_per_unit;

    // Create a unit genome array
    UnitGenome* ug = &unit_genomes[type];

    // Compute max range for this unit
    unit->max_range = get_weapon_range(unit);
    unit->max_range_sq = unit->max_range * unit->max_range;

    // Unpack the array into usable variables
    unit->effective_aggression = ug->unit_aggression;
    unit->effective_ally_proximity = ug->unit_ally_proximity;
    unit->preferred_range = ug->preferred_range;
    if (unit->preferred_range > unit->max_range) {
        unit->preferred_range = unit->max_range;
    }
    
    // Create preferences based on variables
    unit->attack_order = bp->attack_order;
    unit->preferred_range_sq = unit->preferred_range * unit->preferred_range;
    
    // Reset unit status
    unit->pin_markers = 0;
    unit->has_acted = false;
    unit->is_stunned = false;
    unit->is_immobilized = false;
    unit->actions_taken = 0;
    
    // Give the unit its position
    unit->x = x;
    unit->y = y;
    unit->squad_moved = false;
    
    // Give the unit weapons according to its specifications
    clear_weapons(unit);
    for (int i = 0; i < bp->weapon_count; i++) {
        add_weapon(unit, bp->weapons[i].type);
    }
    
    return unit;
}

// General Use Functions
Unit* create_unit_squad(UnitType type, int id, int x, int y) {
    return create_unit(type, id, x, y);
}

const char* get_unit_type_name(UnitType type) {
    switch(type) {
        case UNIT_SWORDSMAN: return "UNIT_SWORDSMAN";
        case UNIT_ELITE_SWORDSMAN: return "UNIT_ELITE_SWORDSMAN";
        case UNIT_LONGBOWMAN: return "UNIT_LONGBOWMAN";
        case UNIT_HORSEMAN: return "UNIT_HORSEMAN";

        case UNIT_SPEARMAN: return "UNIT_SPEARMAN";
        case UNIT_ELITE_SPEARMAN: return "UNIT_ELITE_SPEARMAN";
        case UNIT_SHORTBOWMAN: return "UNIT_SHORTBOWMAN";
        case UNIT_CAMELMAN: return "UNIT_CAMELMAN";
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
    w->explosion_radius = 0;
    
    // Set stats based on weapon types
    switch (type) {
        case WEAPON_SWORD:
            w->damage_bonus = 1;
            w->range = 2;
            w->explosion_radius = 0;
            break;
        case WEAPON_SPEAR:
            w->damage_bonus = 0;
            w->range = 4;
            w->explosion_radius = 0;
            break;
        case WEAPON_LONGBOW:
            w->damage_bonus = 0;
            w->range = 24;
            w->explosion_radius = 0;
            break;
        case WEAPON_SHORTBOW:
            w->damage_bonus = 1;
            w->range = 16;
            w->explosion_radius = 0;
            break;
        default:
            w->damage_bonus = 0;
            w->range = 2;
            w->explosion_radius = 0;
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