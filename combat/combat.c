#include "combat.h"
#include "utils.h"
#include "queries.h"
#include "battlefield.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

// Combat mechanics
int morale_check(Unit* unit) {

    int dice1 = roll(1, 6);
    int dice2 = roll(1, 6);

    // The roll must be less than morale - pins
    if (dice1 + dice2 < unit->morale - unit->pin_markers) {
        // remove a pin marker
        unit->pin_markers--;
        return 1;  // Morale check passed
    } else {
        return 0;  // Morale check failed
    }
}

int take_damage(Unit* unit, int kills) {
    int old_hp = unit->hp;

    unit->hp -= kills;
    if (unit->hp < 0) unit->hp = 0;

    if (unit->hp_per_soldier > 0) {
        unit->units_count = (unit->hp + unit->hp_per_soldier - 1) / unit->hp_per_soldier;
    } else {
        unit->units_count = (unit->hp > 0) ? 1 : 0;
    }

    int damage_dealt = old_hp - unit->hp;
    if (damage_dealt < 0) {
        damage_dealt = 0;
    }

    if (unit->faction == FACTION_GERMAN) {
        battle_stats.damage_taken_german += damage_dealt;
    } else {
        battle_stats.damage_taken_roman += damage_dealt;
    }

    if (unit->hp <= 0) {
        if (unit->faction == FACTION_GERMAN) {
            battle_stats.casualties_german++;
        } else {
            battle_stats.casualties_roman++;
        }
        battle_stats.casualties_by_type[unit->type]++;
    }

    return damage_dealt;
}

void add_pin_marker(Unit* unit) {
    unit->pin_markers += 1;
}


// Action executions
bool execute_attack(Unit* unit, Unit* target, bool advanced) {
    if (!unit || !target) return false;
    if (unit->hp <= 0) return false;

    bool attacked = false;

    // Find a usable weapon
    int distance_sq = get_distance_squared(unit, target);
    for (int i = 0; i < unit->weapon_count; i++) {
        Weapon* w = &unit->weapons[i];
        
        // Check that the range is valid
        if (distance_sq > w->range * w->range) {
            continue;
        }

        // This weapon is valid, load its stats
        int current_soldiers = unit->units_count;
        int attacks = current_soldiers * unit->attacks_per_unit;
        int damage_bonus = w->damage_bonus;
        int penetration_bonus = w->penetration_bonus;
        int hits = 0;

        // Get angle
        int angle = get_attack_angle(unit, target);
        int to_hit_bonus = 0;
        int damage_bonus_from_angle = 0;
        int cohesion_bonus = 0;
        if (angle == 1) to_hit_bonus = 1; // side
        if (angle == 2) { to_hit_bonus = 2; damage_bonus_from_angle = 1; }  // rear
        if (unit->cohesion_allies >= 3 &&
            roll(0, 99) < (int)(unit->effective_cohesion_weight * 100.0f)) {
            cohesion_bonus = 1;
        }
        to_hit_bonus += cohesion_bonus;

        // Roll attack for each shot
        for (int attack_number = 0; attack_number < attacks; attack_number++) {
            int attack_roll = roll(1, 6) + penetration_bonus + to_hit_bonus;
            if (advanced) {
                attack_roll -= 1;  // Penalty for advancing
            }

            if (attack_roll >= target->armor_class) {
                hits++;
                battle_stats.shots_hit++;
            }
        }

        battle_stats.shots_fired += attacks;

        if (hits == 0) {
            continue;
        }

        // Give the enemy a pin marker (if any hits were scored)
        if (hits > 0) {
            add_pin_marker(target);
        }

        // Deal damage
        int kills = 0;
        
        for (int h = 0; h < hits; h++) {
            int damage = roll(1, 6) + damage_bonus + damage_bonus_from_angle;
            if (damage >= target->armor_class) {
                kills++;
            }
        }

        if (kills > 0) {
            battle_stats.kills += kills;
            battle_stats.kills_by_type[target->type] += kills;
            if (to_hit_bonus == 1) {
                battle_stats.flank_kills += kills;
            } else if (to_hit_bonus == 2) {
                battle_stats.rear_kills += kills;
            }
            take_damage(target, kills);
        }

        // Exit the function after firing
        attacked = true;
        break;
    }

    // If no valid weapon is found:
    return attacked;
}

bool execute_fire(Unit* unit, Unit* target, bool advanced) {
    if (!unit || !target) return false;
    if (unit->hp <= 0) return false;
    bool fired = false;

    // Find a usable weapon
    int distance_sq = get_distance_squared(unit, target);
    for (int i = 0; i < unit->weapon_count; i++) {
        Weapon* w = &unit->weapons[i];
        
        // Check that the range is valid
        if (distance_sq > w->range * w->range) {
            continue;
        }

        // This weapon is valid, load its stats
        int current_soldiers = unit->units_count;
        int shots = current_soldiers * unit->attacks_per_unit;
        int damage_bonus = w->damage_bonus;
        int penetration_bonus = w->penetration_bonus;
        int hits = 0;

        // Get angle bonus
        int angle = get_attack_angle(unit, target);
        int to_hit_bonus = 0;
        int cohesion_bonus = 0;
        int damage_bonus_from_angle = 0;
        if (angle == 1) to_hit_bonus = 1; // side
        if (angle == 2) { to_hit_bonus = 2; damage_bonus_from_angle = 1; }  // rear
        if (unit->cohesion_allies >= 3 &&
            roll(0, 99) < (int)(unit->effective_cohesion_weight * 100.0f)) {
            cohesion_bonus = 1;
        }
        to_hit_bonus += cohesion_bonus;

        // Roll attack for each shot
        for (int shot = 0; shot < shots; shot++) {
            int attack_roll = roll(1, 6) + penetration_bonus + to_hit_bonus;
            if (advanced) {
                attack_roll -= 1;  // Penalty for advancing
            }

            if (attack_roll >= target->armor_class) {
                hits++;
                battle_stats.shots_hit++;
            }
        }

        battle_stats.shots_fired += shots;

        if (hits == 0) {
            continue;
        }
        

        // Give the enemy a pin marker (if any hits were scored)
        if (hits > 0) {
            add_pin_marker(target);
        }

        // Deal damage
        int kills = 0;
        for (int h = 0; h < hits; h++) {
            int damage = roll(1, 6) + damage_bonus + damage_bonus_from_angle;
            if (damage >= 4) {
                kills++;
            }
        }

        if (kills > 0) {
            battle_stats.kills += kills;
            battle_stats.kills_by_type[target->type] += kills;
            if (to_hit_bonus == 1) {
                battle_stats.flank_kills += kills;
            } else if (to_hit_bonus == 2) {
                battle_stats.rear_kills += kills;
            }
            take_damage(target, kills);
        }

        // Exit the function after firing
        fired = true;
        break;
    }

    // If no valid weapon is found:
    return fired;
}

bool execute_rally(Unit* unit) {
    bool did_something = false;

    // Rally the unit
    if (morale_check(unit) == 1) {
        did_something = true;
        unit->pin_markers = 0;
    }
    
    return did_something;
}

