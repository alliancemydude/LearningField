#include "combat.h"
#include "utils.h"
#include "queries.h"
#include "battlefield.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

// Combat mechanics
int morale_check(Unit* unit, Battlefield* field) {
    // Determine if an officer is close enough to give a morale bonus
    int distance_from_officer = 9999;
    int rally_bonus = 0;  // Start with 0, add bonus if officer nearby
    
    if (unit->faction == FACTION_REPUBLIC) {
        distance_from_officer = get_distance_to_ally_type(unit, field, UNIT_CLONE_OFFICER);
    } else {
        distance_from_officer = get_distance_to_ally_type(unit, field, UNIT_DROID_OFFICER);
    }
    
    if (distance_from_officer <= 6) {
        rally_bonus = 4;
    }

    int dice1 = roll(1, 6);
    int dice2 = roll(1, 6);

    // The roll must be less than morale - pins + rally bonus
    if (dice1 + dice2 < unit->morale - unit->pin_markers + rally_bonus) {
        // remove a pin marker
        unit->pin_markers--;
        return 1;  // Morale check passed
    } else {
        return 0;  // Morale check failed
    }
}

bool has_explosives(Unit* unit) {
    // Determine if a unit has explosives
    for (int i = 0; i < unit->weapon_count; i++) {
        if (unit->weapons[i].type == WEAPON_GRENADE && unit->weapons[i].ammo > 0) {
            return true;
        }
        if (unit->weapons[i].type == WEAPON_ROCKET && unit->weapons[i].ammo > 0) {
            return true;
        }
    }
    return false;
}

int take_damage(Unit* unit, int kills, bool critical) {
    int old_hp = unit->hp;
    
    // If the unit is an elite and being shot by a sniper, remove 2hp 
    if (critical && (unit->type == UNIT_ELITE_CLONE || unit->type == UNIT_SUPER_BATTLE_DROID)) {
        // A sniper unit (the only one that can get criticals) can only shoot
        // once per round. Therefore at best it can only do 2 damage in a round
        unit->hp -= 2;
        unit->units_count -= 2;
    } else {
        // Remove HP according to kills
        unit->hp -= kills;

        // Remove unit soldiers according to kills
        if (unit->hp_per_soldier > 0) {
            unit->units_count -= kills / unit->hp_per_soldier;
        }
        if (unit->units_count < 0) unit->units_count = 0;
    }

    if (unit->hp < 0) {
        unit->hp = 0;
    }

    if (unit->units_count < 0) {
        unit->units_count = 0;
    }

    // Compute damage dealt
    int damage_dealt = old_hp - unit->hp;
    if (damage_dealt < 0) {
        damage_dealt = 0;
    }

    if (unit->faction == FACTION_REPUBLIC) {
        battle_stats.damage_dealt_rep += damage_dealt;
    } else {
        battle_stats.damage_dealt_sep += damage_dealt;
    }

    if (unit->hp <= 0) {
        if (unit->faction == FACTION_REPUBLIC) {
            battle_stats.casualties_rep++;
        } else {
            battle_stats.casualties_sep++;
        }
        battle_stats.casualties_by_type[unit->type]++;
    }

    return damage_dealt;
}

void add_pin_marker(Unit* unit) {
    unit->pin_markers += 1;
}


// Action executions
bool execute_attack(Unit* unit, Unit* target, bool advanced, Battlefield* field) {
    if (target == NULL) {
        return false;
    }

    if (unit->hp <= 0) return false;

    bool attacked = false;

    if (unit == NULL || target == NULL || field == NULL) return false;

    // Find a usable weapon
    for (int i = 0; i < unit->weapon_count; i++) {
        Weapon* w = &unit->weapons[i];
        int distance_sq = get_distance_squared(unit, target);

        // Check that the range is valid
        if (distance_sq > w->range * w->range) {
            continue;
        }

        // This weapon is valid, load its stats
        int current_soldiers = unit->units_count;
        int attacks = current_soldiers * unit->attacks_per_unit;
        int penetration = w->penetration_bonus;
        int damage_bonus = w->damage_bonus;
        int hits = 0;

        // Roll attack for each shot
        if (!target->is_armor) { // if the target is not armor, roll normally
            for (int attack_number = 0; attack_number < attacks; attack_number++) {
                int attack_roll = roll(1, 6) + damage_bonus;
                if (advanced) {
                    attack_roll -= 1;  // Penalty for advancing
                }

                if (attack_roll >= target->armor_class) {
                    hits++;
                    battle_stats.shots_hit++;
                }
            }
        } else { // the target is armor, which uses penetration
            for (int attack_number = 0; attack_number < attacks; attack_number++) {
                int attack_roll = roll(1, 6) + penetration;
                if (advanced) {
                    attack_roll -= 1;  // Penalty for advancing
                }

                if (attack_roll >= target->armor_class) {
                    hits++;
                    battle_stats.shots_hit++;
                }
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
        bool critical = (w->type == WEAPON_SNIPER);
        
        if (target->is_armor && !critical) { // Armored target (vehicle)
            for (int h = 0; h < hits; h++) {
                int damage = roll(1, 6);

                if (damage == 1) {
                    // Crew stunned, extra pin marker
                    add_pin_marker(target);
                } else if (damage == 2) {
                    // Vehicle immobilized, extra pin
                    target->is_immobilized = true;
                    add_pin_marker(target);
                } else if (damage == 3) {
                    // Vehicle on fire, extra pin + morale check
                    add_pin_marker(target);
                    if (morale_check(target, field) == 0) {
                        kills++;
                    }
                } else {
                    // Vehicle destroyed
                    kills++;
                }
            }
        } else if (critical) {
            // Snipers always kill on a hit
            kills = hits;
        } else { // Soft target (infantry)
            for (int h = 0; h < hits; h++) {
                int damage = roll(1, 6);
                if (damage >= 4) {
                    kills++;
                }
            }
        }

        if (kills > 0) {
            battle_stats.kills += kills;
            battle_stats.kills_by_type[target->type] += kills;
            take_damage(target, kills, critical);
        }

        // Exit the function after firing
        attacked = true;
        break;
    }

    // If no valid weapon is found:
    return attacked;
}

bool execute_fire(Unit* unit, Unit* target, bool advanced, Battlefield* field) {
    if (target == NULL) {
        return false;
    }

    if (unit->hp <= 0) return false;

    bool fired = false;

    if (unit == NULL || target == NULL || field == NULL) return false;

    // Find a usable weapon
    for (int i = 0; i < unit->weapon_count; i++) {
        Weapon* w = &unit->weapons[i];
        int distance_sq = get_distance_squared(unit, target);

        // Check that the range is valid
        if (distance_sq > w->range * w->range) {
            continue;
        }

        // This weapon is valid, load its stats
        int current_soldiers = unit->units_count;
        int shots = current_soldiers * unit->attacks_per_unit * w->weapon_shots;
        int penetration = w->penetration_bonus;
        int damage_bonus = w->damage_bonus;
        int hits = 0;

        // Roll attack for each shot
        if (!target->is_armor) { // if the target is not armor, roll normally
            for (int shot = 0; shot < shots; shot++) {
                int attack_roll = roll(1, 6) + damage_bonus;
                if (advanced) {
                    attack_roll -= 1;  // Penalty for advancing
                }

                if (attack_roll >= target->armor_class) {
                    hits++;
                    battle_stats.shots_hit++;
                }
            }
        } else { // the target is armor, which uses penetration
            for (int shot = 0; shot < shots; shot++) {
                int attack_roll = roll(1, 6) + penetration;
                if (advanced) {
                    attack_roll -= 1;  // Penalty for advancing
                }

                if (attack_roll >= target->armor_class) {
                    hits++;
                    battle_stats.shots_hit++;
                }
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

        // Deal area-of-effect damage
        if (w->explosion_radius > 0) {
        // For each hit, apply splash to nearby enemies (excluding primary)
            for (int h = 0; h < hits; h++) {
                Unit* nearby[MAX_UNITS];
                int count = get_enemies_in_range(target, field, w->explosion_radius, nearby);
                for (int j = 0; j < count; j++) {
                    Unit* enemy = nearby[j];
                    if (enemy == target) continue;   // primary already takes direct damage

                    if (enemy->is_armor) continue;

                    int kills = 0;
                    if (enemy->is_armor) {
                        int damage = roll(1, 6);
                        if (damage == 1) add_pin_marker(enemy);
                        else if (damage == 2) { enemy->is_immobilized = true; add_pin_marker(enemy); }
                        else if (damage == 3) { add_pin_marker(enemy); if (morale_check(enemy, field) == 0) kills = 1; }
                        else kills = 1;
                    } else {
                        int damage_roll = roll(1, 6) + damage_bonus;   // splash uses weapon's damage bonus
                        if (damage_roll >= 4) kills = 1;
                    }

                    if (kills > 0) take_damage(enemy, kills, false);

                    // Pin markers from blast
                    add_pin_marker(enemy);
                    add_pin_marker(enemy);   // extra shock
                }
            }
        }

        // Deal damage
        int kills = 0;
        bool critical = (w->type == WEAPON_SNIPER);
        
        if (target->is_armor && !critical) { // Armored target (vehicle)
            for (int h = 0; h < hits; h++) {
                int damage = roll(1, 6);

                if (damage == 1) {
                    // Crew stunned, extra pin marker
                    add_pin_marker(target);
                } else if (damage == 2) {
                    // Vehicle immobilized, extra pin
                    target->is_immobilized = true;
                    add_pin_marker(target);
                } else if (damage == 3) {
                    // Vehicle on fire, extra pin + morale check
                    add_pin_marker(target);
                    if (morale_check(target, field) == 0) {
                        kills++;
                    }
                } else {
                    // Vehicle destroyed
                    kills++;
                }
            }
        } else if (critical) {
            // Snipers always kill on a hit
            kills = hits;
        } else { // Soft target (infantry)
            for (int h = 0; h < hits; h++) {
                int damage = roll(1, 6);
                if (damage >= 4) {
                    kills++;
                }
            }
        }

        if (kills > 0) {
            battle_stats.kills += kills;
            battle_stats.kills_by_type[target->type] += kills;
            take_damage(target, kills, critical);
        }

        // Exit the function after firing
        fired = true;
        break;
    }

    // If no valid weapon is found:
    return fired;
}

bool execute_explosive(Unit* unit, Unit* target, bool advanced, Battlefield* field) {
    if (target == NULL) {
        return false;
    }

    // Find grenade
    Weapon* explosive = NULL;
    for (int i = 0; i < unit->weapon_count; i++) {
        Weapon* w = &unit->weapons[i];
        if (w->ammo <= 0) {
            continue;
        }
        if (w->type == WEAPON_GRENADE || w->type == WEAPON_ROCKET) {
            explosive = w;
            break;
        }
    }

    if (!explosive) {
        return false;
    }

    // Check range
    int distance = get_distance(unit, target);
    if (advanced) {
        distance -= (unit->movement / 2);
        if (distance < 0) {
            distance = 0;
        }
    }

    int effective_armor = target->armor_class;

    int hit_roll = roll(1, 6);
    if (target->is_armor) {
        hit_roll += explosive->penetration_bonus;
    } else {
        hit_roll += explosive->damage_bonus;
    }
    if (advanced) {
        hit_roll -= 1;
    }

    battle_stats.explosives_used++;
    battle_stats.shots_hit++;

    bool hits_primary = (hit_roll >= effective_armor);

    // If it hits, find all enemies in the explosion radius
    int explosion_radius = explosive->explosion_radius;
    Unit* nearby[MAX_UNITS];
    int enemy_count = get_enemies_in_range(target, field, explosion_radius, nearby);

    // Get all targets
    int total_targets = 0;
    Unit* all_targets[MAX_UNITS];
    all_targets[total_targets++] = target;
    for (int i = 0; i < enemy_count && total_targets < MAX_UNITS; i++) {
        bool dup = false;
        for (int j = 0; j < total_targets; j++) {
            if (all_targets[j] == nearby[i]) {
                dup = true;
                break;
            }
        }
        if (!dup) {
            all_targets[total_targets++] = nearby[i];
        }
    }

    // Apply damage to all targets if the primary target was hit
    if (hits_primary) {
        for (int t = 0; t < total_targets; t++) {
            Unit* enemy = all_targets[t];

            // Armor units are not damaged by AOE (except primary)
            if (enemy->is_armor && enemy != target) {
                continue;
            }

            // Apply damage
            int kills = 0;
            if (enemy->is_armor && enemy == target) {
                int damage = roll(1, 6);
                if (damage == 1) {
                    add_pin_marker(enemy);
                } else if (damage == 2) {
                    enemy->is_immobilized = true;
                    add_pin_marker(enemy);
                } else if (damage == 3) {
                    add_pin_marker(enemy);
                    if (morale_check(enemy, field) == 0) {
                        kills = 1;
                    }
                } else {
                    kills = 1;
                }
            } else {
                int damage = roll(1, 6) + explosive->damage_bonus;
                if (damage >= 4) {
                    kills = 1;
                }
            }

            if (kills > 0) {
                battle_stats.kills += kills;
                battle_stats.kills_by_type[target->type] += kills;
                take_damage(enemy, kills, false);
            }

            // Pin markers from explosive blast
            add_pin_marker(enemy);
        }
    }

    // Consume the explosive
    explosive->ammo--;

    return true;
}

bool execute_rally(Unit* unit, Battlefield* field) {
    // Regular troops can only rally themselves, officers can rally troops as an action
    bool is_officer = (unit->type == UNIT_CLONE_OFFICER || unit->type == UNIT_DROID_OFFICER);
    bool did_something = false;

    // If the unit is an officer & does not have pin markers, it is rallying an ally
    if (is_officer && unit->pin_markers == 0) {
        // rally the nearest pinned ally
        Unit* to_rally = find_closest_pinned_ally(unit, field);

        // Make sure a pinned ally can be found
        if (to_rally == NULL) {
            return did_something;
        }
        
        // Roll a morale check for the unit, if successful the unit loses all pin markers
        if (morale_check(to_rally, field) == 1) {
            did_something = true;
            to_rally->pin_markers = 0;
        }
    } else {
        // Rally the unit
        if (morale_check(unit, field) == 1) {
            did_something = true;
            unit->pin_markers = 0;
        }
    }
    return did_something;
}

