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
    Unit* unit = malloc(sizeof(Unit));
    if (!unit) {
        return NULL;
    }

    // Reset orders
    unit->strategic_order.type = ORDER_NONE;
    unit->strategic_order.target_x = 0;
    unit->strategic_order.target_y = 0;
    
    // Reset identity
    unit->id = id;
    unit->type = type;
    unit->faction = bp->faction;
    unit->squad_id = -1;
    
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
    unit->is_armor = bp->is_armor;
    unit->units_count = bp->units_count;
    unit->shots_per_unit = bp->shots_per_unit;

    // Create a unit genome array
    UnitGenome* ug = &unit_genomes[type];

    // Unpack the array into usable variables
    unit->effective_aggression = ug->unit_aggression;
    unit->effective_explosive_threshold = ug->unit_explosive_threshold;
    unit->effective_ally_proximity = ug->unit_ally_proximity;
    unit->effective_retreat_hp_ratio = ug->unit_retreat_hp_ratio;
    unit->preferred_range = ug->preferred_range;
    unit->danger_range = ug->danger_range;
    unit->focus_target = NULL;
    
    // Create preferences based on variables
    unit->attack_order = bp->attack_order;
    unit->preferred_range_sq = unit->preferred_range * unit->preferred_range;
    unit->danger_range_sq = unit->danger_range * unit->danger_range;
    
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
        add_weapon(unit, bp->weapons[i].type, bp->weapons[i].ammo);
    }
    
    // Compute max range for this unit
    unit->max_range = get_weapon_range(unit);
    unit->max_range_sq = unit->max_range * unit->max_range;
    
    return unit;
}

// General Use Functions
Unit* create_unit_squad(UnitType type, int id, int x, int y) {
    return create_unit(type, id, x, y);
}

const char* get_unit_type_name(UnitType type) {
    switch(type) {
        case UNIT_CLONE_TROOPER: return "UNIT_CLONE_TROOPER";
        case UNIT_ELITE_CLONE: return "UNIT_ELITE_CLONE";
        case UNIT_CLONE_SNIPER: return "UNIT_CLONE_SNIPER";
        case UNIT_CLONE_OFFICER: return "UNIT_CLONE_OFFICER";

        case UNIT_LIGHT_CLONE_GUNNER: return "UNIT_LIGHT_CLONE_GUNNER";
        case UNIT_MEDIUM_CLONE_GUNNER: return "UNIT_MEDIUM_CLONE_GUNNER";
        case UNIT_HEAVY_CLONE_GUNNER: return "UNIT_HEAVY_CLONE_GUNNER";
        case UNIT_CLONE_ROCKET: return "UNIT_CLONE_ROCKET";

        case UNIT_SWAMP_SPEEDER: return "UNIT_SWAMP_SPEEDER";
        case UNIT_BARC_SPEEDER: return "UNIT_BARC_SPEEDER";
        case UNIT_SABER_TANK: return "UNIT_SABER_TANK";

        case UNIT_BATTLE_DROID: return "UNIT_BATTLE_DROID";
        case UNIT_SUPER_BATTLE_DROID: return "UNIT_SUPER_BATTLE_DROID";
        case UNIT_DROID_SNIPER: return "UNIT_DROID_SNIPER";
        case UNIT_DROID_OFFICER: return "UNIT_DROID_OFFICER";

        case UNIT_MEDIUM_DROID_GUNNER: return "UNIT_MEDIUM_DROID_GUNNER";
        case UNIT_HEAVY_DROID_GUNNER: return "UNIT_HEAVY_DROID_GUNNER";
        case UNIT_DROID_ROCKET: return "UNIT_DROID_ROCKET";

        case UNIT_SPIDER_DROID: return "UNIT_SPIDER_DROID";
        case UNIT_STAP: return "UNIT_STAP";
        case UNIT_AAT: return "UNIT_AAT";
        default: return "UNKNOWN";
    }
}

void add_weapon(Unit* unit, WeaponType type, int ammo) {
    if (unit->weapon_count >= WEAPON_COUNT) {
        return;
    }

    // Create a weapon pointer to get type, ammo, and stats
    Weapon* w = &unit->weapons[unit->weapon_count];
    w->type = type;
    w->ammo = ammo;
    w->equipped = true;
    w->weapon_shots = 1;
    w->explosion_radius = 0;
    
    // Set stats based on weapon types
    switch (type) {
        // Small arms
        case WEAPON_BLASTER:
            w->weapon_shots = 1;
            w->damage_bonus = 0;
            w->penetration_bonus = 0;
            w->range = 24;
            w->explosion_radius = 0;
            break;
        case WEAPON_CARBINE:
            w->weapon_shots = 1;
            w->damage_bonus = 0;
            w->penetration_bonus = 0;
            w->range = 18;
            w->explosion_radius = 0;
            break;
        case WEAPON_SNIPER:
            w->weapon_shots = 1;
            w->damage_bonus = 0;
            w->penetration_bonus = 1;
            w->range = 36;
            w->explosion_radius = 0;
            break;

        // Explosives
        case WEAPON_GRENADE:
            w->weapon_shots = 1;
            w->damage_bonus = 1;
            w->penetration_bonus = 0;
            w->range = 12;
            w->explosion_radius = 2;
            break;
        case WEAPON_ROCKET:
            w->weapon_shots = 1;
            w->damage_bonus = 0;
            w->penetration_bonus = 1;
            w->range = 16;
            w->explosion_radius = 1;
            break;

        // Machine guns
        case WEAPON_SUB_MG:
            w->weapon_shots = 2;
            w->damage_bonus = 0;
            w->penetration_bonus = 0;
            w->range = 18;
            w->explosion_radius = 0;
            break;
        case WEAPON_LIGHT_MG:
            w->weapon_shots = 4;
            w->damage_bonus = 0;
            w->penetration_bonus = 0;
            w->range = 24;
            w->explosion_radius = 0;
            break;
        case WEAPON_MEDIUM_MG:
            w->weapon_shots = 6;
            w->damage_bonus = 0;
            w->penetration_bonus = 0;
            w->range = 36;
            w->explosion_radius = 0;
            break;
        case WEAPON_HEAVY_MG:
            w->weapon_shots = 6;
            w->damage_bonus = 1;
            w->penetration_bonus = 1;
            w->range = 36;
            w->explosion_radius = 0;
            break;

        // Anti-Tank weapons
        case WEAPON_LIGHT_AT:
            w->weapon_shots = 1;
            w->damage_bonus = 0;
            w->penetration_bonus = 4;
            w->range = 48;
            w->explosion_radius = 1;
            break;
        case WEAPON_MEDIUM_AT:
            w->weapon_shots = 1;
            w->damage_bonus = 0;
            w->penetration_bonus = 5;
            w->range = 60;
            w->explosion_radius = 1;
            break;
        case WEAPON_BLASTER_CANNON:
            w->weapon_shots = 2;
            w->damage_bonus = 0;
            w->penetration_bonus = 2;
            w->range = 48;
            w->explosion_radius = 1;
            break;
        case WEAPON_DUAL_BLASTER_CANNON:
            w->weapon_shots = 4;
            w->damage_bonus = 0;
            w->penetration_bonus = 2;
            w->range = 48;
            w->explosion_radius = 1;
            break;
        case WEAPON_MISSILE_POD:
            w->weapon_shots = 1;
            w->damage_bonus = 0;
            w->penetration_bonus = 5;
            w->range = 24;
            w->explosion_radius = 1;
            break;
        case WEAPON_BEAM_CANNON:
            w->weapon_shots = 2;
            w->damage_bonus = 0;
            w->penetration_bonus = 3;
            w->range = 72;
            w->explosion_radius = 1;
            break;
        default:
            w->weapon_shots = 1;
            w->damage_bonus = 0;
            w->penetration_bonus = 0;
            w->range = 12;
            w->explosion_radius = 0;
            break;
    }
    unit->weapon_count++;
}

void clear_weapons(Unit* unit) {
    // Reset all weapons stats for a unit
    for (int i = 0; i < WEAPON_COUNT; i++) {
        unit->weapons[i].type = (WeaponType)i;
        unit->weapons[i].ammo = 0;
        unit->weapons[i].penetration_bonus = 0;
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