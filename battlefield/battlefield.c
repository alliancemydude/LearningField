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
#include "battle_stats.h"

#ifdef _WIN32
  #include <windows.h>
  #define SLEEP_MS(x) Sleep(x)
#else
  #define SLEEP_MS(x) usleep((x) * 1000)
#endif

// Default force compositions
ForceComposition default_german_composition[COMPOSITION_MAX_TYPES] = {
    {UNIT_BARBARIAN,        50},
    {UNIT_ELITE_BARBARIAN,  10},
    {UNIT_JAVELIN_THROWER,  10},
    {UNIT_GERMAN_CAVALRY,   15}
};
int default_german_composition_count = 4;

ForceComposition default_roman_composition[COMPOSITION_MAX_TYPES] = {
    {UNIT_LEGIONARE,        50},
    {UNIT_CENTURION,        10},
    {UNIT_ARCHER,           10},
    {UNIT_ROMAN_CAVALRY,    15}
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

