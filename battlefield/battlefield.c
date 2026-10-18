#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "battlefield.h"
#include "ai.h"
#include "strategy.h"
#include "utils.h"
#include "queries.h"

#ifdef _WIN32
  #include <windows.h>
  #define SLEEP_MS(x) Sleep(x)
#else
  #define SLEEP_MS(x) usleep((x) * 1000)
#endif

// Default force compositions
ForceComposition default_german_composition[COMPOSITION_MAX_TYPES] = {
    {UNIT_BARBARIAN,        84},
    {UNIT_ELITE_BARBARIAN,  8},
    {UNIT_JAVELIN_THROWER,  0},
    {UNIT_GERMAN_CAVALRY,   0}
};
int default_german_composition_count = 4;

ForceComposition default_roman_composition[COMPOSITION_MAX_TYPES] = {
    {UNIT_LEGIONARE,        46},
    {UNIT_CENTURION,        25},
    {UNIT_ARCHER,           1},
    {UNIT_ROMAN_CAVALRY,    0}
};
int default_roman_composition_count = 4;

bool is_unit_used_in_composition(UnitType type) {
    // Check German composition
    for (int i = 0; i < default_german_composition_count; i++) {
        if (default_german_composition[i].type == type && default_german_composition[i].count > 0) {
            return true;
        }
    }
    // Check Roman composition
    for (int i = 0; i < default_roman_composition_count; i++) {
        if (default_roman_composition[i].type == type && default_roman_composition[i].count > 0) {
            return true;
        }
    }
    return false;
}

// Action tracking:
ActionCounts action_counts = {0};
int total_actions_across_units = 0;
int units_that_acted = 0;
BattleStats battle_stats = {0};
char battle_action_log[4096] = {0};
bool turn_based = false;
bool print_battle_summary_flag = false;

// Initialization
static bool try_relocate(Battlefield* field, Unit* u) {
    // 1. Try the 8 adjacent tiles first
    static const int dx[] = {0, 0, -1, 1, -1, -1, 1, 1};
    static const int dy[] = {-1, 1, 0, 0, -1, 1, -1, 1};
    for (int d = 0; d < 8; d++) {
        int nx = u->x + dx[d];
        int ny = u->y + dy[d];
        if (is_tile_walkable(field, nx, ny, u)) {
            field->occupancy[u->y][u->x] = NULL;
            u->x = nx;
            u->y = ny;
            field->occupancy[u->y][u->x] = u;
            return true;
        }
    }
    // 2. Expand outward, perimeter of increasing radii
    for (int radius = 2; radius <= 4; radius++) {
        for (int d = -radius; d <= radius; d++) {
            // Top and bottom edges of the ring
            for (int sign = -1; sign <= 1; sign += 2) {
                int nx = u->x + d;
                int ny = u->y + sign * radius;
                if (is_tile_walkable(field, nx, ny, u)) {
                    u->x = nx; u->y = ny; return true;
                }
                nx = u->x + sign * radius;
                ny = u->y + d;
                if (is_tile_walkable(field, nx, ny, u)) {
                    u->x = nx; u->y = ny; return true;
                }
            }
        }
    }
    // 3. Random fallback
    for (int attempt = 0; attempt < 30; attempt++) {
        int nx = roll(0, field->width - 1);
        int ny = roll(0, field->height - 1);
        if (is_tile_walkable(field, nx, ny, u)) {
            u->x = nx;
            u->y = ny;
            return true;
        }
    }
    return false; // map is full
}

void place_unit(Battlefield* field, Unit* u, int new_x, int new_y) {
    field->occupancy[u->y][u->x] = NULL;
    u->x = new_x;
    u->y = new_y;
    field->occupancy[u->y][u->x] = u;
}

void resolve_overlaps(Battlefield* field) {
    const int max_passes = 20;
    for (int pass = 0; pass < max_passes; pass++) {
        bool any_moved = false;
        for (int i = 0; i < field->unit_count; i++) {
            Unit* a = field->units[i];
            if (a == NULL || a->hp <= 0) continue;
            for (int j = i + 1; j < field->unit_count; j++) {
                Unit* b = field->units[j];
                if (b == NULL || b->hp <= 0) continue;
                if (a->x != b->x || a->y != b->y) continue;

                if (try_relocate(field, b)) {
                    any_moved = true;
                }
            }
        }
        if (!any_moved) return; // stable
    }
}

void initialize_battlefield(Battlefield* field) {
    field->width = MAX_COLS;
    field->height = MAX_ROWS;
    field->unit_count = 0;

    for (int i = 0; i < MAX_UNITS; i++) {
        field->units[i] = NULL;
    }

    // Also clear the occupancy grid
    for (int y = 0; y < MAX_ROWS; y++) {
        for (int x = 0; x < MAX_COLS; x++) {
            field->occupancy[y][x] = NULL;
        }
    }
}

void spawn_german_forces(Battlefield* field, ForceComposition forces[], int force_count) {
    int id = field->unit_count + 1;
    int y_min = field->height * 3 / 4;
    int y_max = field->height - 1;
    int x_min = 0;
    int x_max = field->width - 1;

    FactionStrategy* strat = &german_strategy;

    int total_units = 0;
    for (int f = 0; f < force_count; f++) {
        total_units += forces[f].count;
    }
    if (total_units == 0) return;

    float center_x = (float)field->width / 2.0f;
    if (center_x < 0.0f) center_x = 0.0f;
    if (center_x >= field->width) center_x = field->width - 1.0f;

    float horizontal_spread = strat->deployment.deploy_horizontal_spread / 10.0f;
    float vertical_offset = strat->deployment.deploy_vertical_offset / 10.0f;
    int y_center = y_max - (int)((y_max - y_min) * vertical_offset);
    if (y_center < y_min) y_center = y_min;
    if (y_center > y_max) y_center = y_max;

    int unit_index = 0;
    for (int f = 0; f < force_count; f++) {
        for (int i = 0; i < forces[f].count; i++) {
            if (field->unit_count >= MAX_UNITS) break;
            float fraction = (total_units == 1) ? 0.5f : (float)unit_index / (total_units - 1);
            float x_offset = (fraction - 0.5f) * 2.0f * horizontal_spread * (float)field->width * 0.4f;
            int desired_x = (int)(center_x + x_offset);
            float y_offset = (fraction - 0.5f) * (y_max - y_min) * 0.3f;
            int desired_y = y_center + (int)y_offset;

            if (desired_x < 0) desired_x = 0;
            if (desired_x >= field->width) desired_x = field->width - 1;
            if (desired_y < y_min) desired_y = y_min;
            if (desired_y > y_max) desired_y = y_max;

            int x = desired_x, y = desired_y;
            bool placed = false;

            if (is_tile_walkable(field, x, y, NULL)) {
                placed = true;
            }

            if (!placed) {
                for (int radius = 1; radius <= 3 && !placed; radius++) {
                    for (int dy = -radius; dy <= radius && !placed; dy++) {
                        for (int dx = -radius; dx <= radius && !placed; dx++) {
                            if (dx == 0 && dy == 0) continue;
                            int tx = desired_x + dx;
                            int ty = desired_y + dy;
                            if (tx < 0 || tx >= field->width || ty < y_min || ty > y_max) continue;
                            if (is_tile_walkable(field, tx, ty, NULL)) {
                                x = tx; y = ty; placed = true;
                            }
                        }
                    }
                }
            }

            if (!placed) {
                for (int attempt = 0; attempt < 100; attempt++) {
                    x = roll(x_min, x_max);
                    y = roll(y_min, y_max);
                    if (is_tile_walkable(field, x, y, NULL)) {
                        placed = true; break;
                    }
                }
            }

            if (!placed) {
                x = (unit_index * 3) % field->width;
                y = y_min + (unit_index % (y_max - y_min + 1));
                for (int yy = y_min; yy <= y_max && !placed; yy++) {
                    for (int xx = 0; xx < field->width && !placed; xx++) {
                        if (is_tile_walkable(field, xx, yy, NULL)) {
                            x = xx; y = yy; placed = true;
                        }
                    }
                }
            }

            Unit* unit = create_unit(forces[f].type, id++, x, y);
            if (unit) {
                field->units[field->unit_count++] = unit;
                field->occupancy[unit->y][unit->x] = unit;
            }
            unit_index++;
        }
    }
    resolve_overlaps(field);
}

void spawn_roman_forces(Battlefield* field, ForceComposition forces[], int force_count) {
    int id = field->unit_count + 1;
    int y_min = 0;
    int y_max = field->height / 4 - 1;
    int x_min = 0;
    int x_max = field->width - 1;

    FactionStrategy* strat = &roman_strategy;

    int total_units = 0;
    for (int f = 0; f < force_count; f++) {
        total_units += forces[f].count;
    }
    if (total_units == 0) return;

    float center_x = (float)field->width / 2.0f;
    if (center_x < 0.0f) center_x = 0.0f;
    if (center_x >= field->width) center_x = field->width - 1.0f;

    float horizontal_spread = strat->deployment.deploy_horizontal_spread / 10.0f;
    float vertical_offset = strat->deployment.deploy_vertical_offset / 10.0f;
    int y_center = y_min + (int)((y_max - y_min) * vertical_offset);
    if (y_center < y_min) y_center = y_min;
    if (y_center > y_max) y_center = y_max;

    int unit_index = 0;
    for (int f = 0; f < force_count; f++) {
        for (int i = 0; i < forces[f].count; i++) {
            if (field->unit_count >= MAX_UNITS) break;
            float fraction = (total_units == 1) ? 0.5f : (float)unit_index / (total_units - 1);
            float x_offset = (fraction - 0.5f) * 2.0f * horizontal_spread * (float)field->width * 0.4f;
            int desired_x = (int)(center_x + x_offset);
            float y_offset = (fraction - 0.5f) * (y_max - y_min) * 0.3f;
            int desired_y = y_center + (int)y_offset;

            if (desired_x < 0) desired_x = 0;
            if (desired_x >= field->width) desired_x = field->width - 1;
            if (desired_y < y_min) desired_y = y_min;
            if (desired_y > y_max) desired_y = y_max;

            int x = desired_x, y = desired_y;
            bool placed = false;

            if (is_tile_walkable(field, x, y, NULL)) {
                placed = true;
            }

            if (!placed) {
                for (int radius = 1; radius <= 3 && !placed; radius++) {
                    for (int dy = -radius; dy <= radius && !placed; dy++) {
                        for (int dx = -radius; dx <= radius && !placed; dx++) {
                            if (dx == 0 && dy == 0) continue;
                            int tx = desired_x + dx;
                            int ty = desired_y + dy;
                            if (tx < 0 || tx >= field->width || ty < y_min || ty > y_max) continue;
                            if (is_tile_walkable(field, tx, ty, NULL)) {
                                x = tx; y = ty; placed = true;
                            }
                        }
                    }
                }
            }

            if (!placed) {
                for (int attempt = 0; attempt < 100; attempt++) {
                    x = roll(x_min, x_max);
                    y = roll(y_min, y_max);
                    if (is_tile_walkable(field, x, y, NULL)) {
                        placed = true; break;
                    }
                }
            }

            if (!placed) {
                x = (unit_index * 3) % field->width;
                y = y_min + (unit_index % (y_max - y_min + 1));
                for (int yy = y_min; yy <= y_max && !placed; yy++) {
                    for (int xx = 0; xx < field->width && !placed; xx++) {
                        if (is_tile_walkable(field, xx, yy, NULL)) {
                            x = xx; y = yy; placed = true;
                        }
                    }
                }
            }

            Unit* unit = create_unit(forces[f].type, id++, x, y);
            if (unit) {
                field->units[field->unit_count++] = unit;
                field->occupancy[unit->y][unit->x] = unit;
            }
            unit_index++;
        }
    }
    resolve_overlaps(field);
}


// Start of Battle
int run_battle(Battlefield* field, int max_turns) {
    turn_count = 0;
    battle_action_log[0] = '\0';

    memset(&battle_stats, 0, sizeof(BattleStats));

    battle_stats.winner = 2;

    for (int i = 0; i < UNIT_TYPE_COUNT; i++) {
        battle_stats.kills_by_type[i] = 0;
        battle_stats.casualties_by_type[i] = 0;
    }

    while (turn_count < max_turns) {
        int german_count = 0, roman_count = 0;
        for (int i = 0; i < field->unit_count; i++) {
            if (field->units[i] == NULL || field->units[i]->hp <= 0) continue;
            if (field->units[i]->faction == FACTION_GERMAN) german_count++;
            else roman_count++;
        }
        if (german_count == 0) {
            battle_stats.winner = 1;
            return 1;
        }
        if (roman_count == 0) {
            battle_stats.winner = 0;
            return 0;
        }

        // Reset if units have acted, moved, or rallied, and their pathing
        for (int i = 0; i < field->unit_count; i++) {
            Unit* u = field->units[i];
            if (!u || u->hp <= 0) continue;
            u->has_acted = false;
            u->moved_this_turn = false;
            u->rallied_this_turn = false;
            u->path_len = 0;
        }

        // Populate local enemies and allies
        for (int i = 0; i < field->unit_count; i++) {
            Unit* u = field->units[i];
            if (!u || u->hp <= 0) continue;
            u->local_allies    = count_faction_in_radius(u, field, u->faction, 5);
            u->local_enemies   = count_faction_in_radius(u, field, enemy_faction(u->faction), 5);
            u->cohesion_allies = count_same_facing_allies(u, field, 2);   // NEW
        }

        // Get closest enemy distances
        compute_closest_enemy_distances(field);

        // MOVE PHASE
        bool german_turn = (roll(1, 2) == 1);
        int acted_this_turn = 0;
        int total_active = german_count + roman_count;
        bool toggled_once = false;  // track if we already tried both sides
        int safety = 0; // infinite loop prevention

        if (turn_based) {
            print_screen(field);
            printf("-- start of turn %d, press any key to begin move phase --\n", turn_count);
            getchar();
        }

        while (acted_this_turn < total_active && safety++ < total_active * 2) {
            Faction current_faction = german_turn ? FACTION_GERMAN : FACTION_ROMAN;
            Unit* acting_unit = select_closest_unit(field, current_faction);

            if (!acting_unit) {
                // If we've already toggled and still no unit, all units have acted
                if (toggled_once) break;
                german_turn = !german_turn;
                toggled_once = true;
                continue;
            }
            toggled_once = false;  // reset toggled flag when we find a unit

            acting_unit->has_acted = true;
            unit_move_phase(acting_unit, field);
            acted_this_turn++;
            german_turn = !german_turn;
        }

        if (turn_based) {
            // Find the longest path any unit took this phase
            int max_path = 0;
            for (int i = 0; i < field->unit_count; i++) {
                Unit* u = field->units[i];
                if (!u || u->hp <= 0) continue;
                if (u->path_len > max_path) max_path = u->path_len;
            }

            // Play back one tile per frame
            for (int frame = 0; frame < max_path; frame++) {
                for (int i = 0; i < field->unit_count; i++) {
                    Unit* u = field->units[i];
                    if (!u || u->hp <= 0) continue;
                    if (u->path_len == 0) continue;

                    int step = (frame < u->path_len) ? frame : (u->path_len - 1);
                    u->x = u->path_x[step];
                    u->y = u->path_y[step];
                }
                print_screen(field);
                SLEEP_MS(80);
            }

            // Final resting state
            print_screen(field);
            printf("-- end of move phase, press any key for attack phase --\n");
            getchar();
        }

        // Reset has_acted for the attack phase
        for (int i = 0; i < field->unit_count; i++) {
            if (field->units[i] && field->units[i]->hp > 0) {
                field->units[i]->has_acted = false;
            }
        }
        compute_closest_enemy_distances(field);

        // ATTACK PHASE
        german_turn = (roll(1, 2) == 1);
        acted_this_turn = 0;
        toggled_once = false;
        safety = 0;

        while (acted_this_turn < total_active && safety++ < total_active * 2) {
            Faction current = german_turn ? FACTION_GERMAN : FACTION_ROMAN;
            Unit* acting = select_closest_unit(field, current);

            if (!acting) {
                if (toggled_once) break;
                german_turn = !german_turn;
                toggled_once = true;
                continue;
            }
            toggled_once = false;

            acting->has_acted = true;
            unit_attack_phase(acting, field);
            acted_this_turn++;
            german_turn = !german_turn;
        }

        if (turn_based) {
            print_screen(field);
            printf("-- end of turn, press any key to continue --\n");
            getchar();
        }

        compact_dead_units(field);
        turn_count++;
    }

    battle_stats.winner = 2;
    if (turn_based) print_battle_summary_and_wait();
    return 2;
}


// Cleanup
void compact_dead_units(Battlefield* field) {
    int write = 0;
    for (int read = 0; read < field->unit_count; read++) {
        Unit* u = field->units[read];
        if (u == NULL) continue;

        if (u->hp <= 0) {
            if (u->x >= 0 && u->x < field->width &&
                u->y >= 0 && u->y < field->height) {
                field->occupancy[u->y][u->x] = NULL;
            }
            free(u);
        } else {
            field->units[write++] = u;
        }
    }
    field->unit_count = write;
}

void cleanup_battlefield(Battlefield* field) {
    // Free all units (no memory leaks pleaseeeee)
    for (int i = 0; i < field->unit_count; i++) {
        free(field->units[i]);
        field->units[i] = NULL;
    }
    field->unit_count = 0;
}

void reset_stats() {
    total_actions_across_units = 0;
    units_that_acted = 0;
    
    // Reset action_counts
    action_counts.fire = 0;
    action_counts.attack = 0;
    action_counts.rally = 0;
    action_counts.dash = 0;
    action_counts.total = 0;
}

void print_action_stats() {
    if (action_counts.total == 0) {
        printf("No actions tracked.\n");
        return;
    }
    
    printf("\n=== ACTION ECONOMY ===\n");
    printf("Total actions tracked: %d\n", action_counts.total);
    printf("\n");
    
    // Categories
    int aggressive = action_counts.fire + action_counts.attack;
    int movement = action_counts.dash;
    int defensive = action_counts.rally;
    
    printf("Category breakdown:\n");
    printf("  Aggressive: %.1f%% (%d)\n", 
           (aggressive * 100.0f) / action_counts.total, aggressive);
    printf("  Movement:   %.1f%% (%d)\n", 
           (movement * 100.0f) / action_counts.total, movement);
    printf("  Defensive:  %.1f%% (%d)\n", 
           (defensive * 100.0f) / action_counts.total, defensive);
    printf("\n");
    
    printf("Detailed breakdown:\n");
    printf("  Fire:                 %.1f%%\n", 
           (action_counts.fire * 100.0f) / action_counts.total);
    printf("  Attack:               %.1f%%\n", 
           (action_counts.attack * 100.0f) / action_counts.total);
    printf("  Dash:                 %.1f%%\n", 
           (action_counts.dash * 100.0f) / action_counts.total);
    printf("  Rally:                %.1f%%\n", 
           (action_counts.rally * 100.0f) / action_counts.total);
    printf("\n");
}

void print_battle_summary() {
    printf("\n=== BATTLE SUMMARY ===\n");
    printf("Winner: %s\n",
       (battle_stats.winner == 0) ? "German" :
       (battle_stats.winner == 1) ? "Roman"  : "Timeout");
    printf("Turns: %d\n", turn_count);
    printf("Shots fired: %d\n", battle_stats.shots_fired);
    printf("Shots hit: %d\n", battle_stats.shots_hit);
    printf("Kills: %d\n", battle_stats.kills);
    printf("  Flank kills: %d, Rear kills: %d\n", battle_stats.flank_kills, battle_stats.rear_kills);
    int german_dealt = battle_stats.damage_taken_roman;
    int roman_dealt = battle_stats.damage_taken_german;
    printf("Damage dealt: German %d, Roman %d\n", german_dealt, roman_dealt);
    printf("Casualties: German %d, Roman %d\n", battle_stats.casualties_german, battle_stats.casualties_roman);
    // Print kills by type
    for (int i = 0; i < UNIT_TYPE_COUNT; i++) {
        printf("  %s: %d kills\n", get_unit_type_name((UnitType)i), battle_stats.kills_by_type[i]);
    }
    printf("\n");
}

void print_battle_summary_and_wait(void) {
    if (turn_based) {
        print_battle_summary();
        printf("Press any key to continue... ");
        char press_next = getchar();
        if (press_next) printf("\n");
    }
}