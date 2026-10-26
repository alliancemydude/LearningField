#include "battlefield.h"
#include "unit.h"
#include "unit_utils.h"
#include "ai.h"
#include "strategy.h"
#include "evolution.h"
#include "utils.h"
#include "tests.h"
#include "calibration.h"
#include "spawn.h"
#include "battle_stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define STRATEGY_FILE "strategy.txt"

// Do not adjust
int german_wins = 0;
int roman_wins = 0;
int timeouts = 0;

// Basic adjustable values
int battles                     = 1000;
bool track_actions              = true;

static void load_settings(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("No settings file found, using defaults.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char* nl = strchr(line, '\n');
        if (nl) *nl = '\0';

        if (line[0] == '\0' || line[0] == '#') continue;

        char key[64];
        char val[192];
        if (sscanf(line, "%63s %191[^\n]", key, val) != 2) continue;

        if      (strcmp(key, "battles")        == 0) battles         = atoi(val);
        else if (strcmp(key, "pop_size")       == 0) pop_size        = atoi(val);
        else if (strcmp(key, "generations")    == 0) num_generations = atoi(val);
        else if (strcmp(key, "battles_min")    == 0) battles_min     = atoi(val);
        else if (strcmp(key, "battles_max")    == 0) battles_max     = atoi(val);
        else if (strcmp(key, "budget")         == 0) composition_budget = atoi(val);
    }
    fclose(f);
}

void start_match() {
    if (track_actions) {
        reset_stats();
    }

    // Loop through all battles
    for (int i = 0; i < battles; i++) {
        // Seed the battles
        srand((unsigned int)(time(NULL) + i * 1000 + clock()));

        // Generate a random battlefield
        Battlefield field;
        initialize_battlefield(&field);
        
        // Spawn forces for both sides
        spawn_german_forces(&field, default_german_composition, default_german_composition_count);
        spawn_roman_forces(&field, default_roman_composition, default_roman_composition_count);
        
        // Increment stats based on the winner
        int winner = run_battle(&field, MAX_TURNS);
        if (winner == 0) {
            german_wins++;
        } else if (winner == 1) {
            roman_wins++;
        } else {
            timeouts++;
        }

        // Free memory 
        cleanup_battlefield(&field);
    }
}

void print_results() {
    // Calculate percentage of wins for each side and timeouts
    float german_percent = (german_wins * 100.0f) / battles;
    float roman_percent = (roman_wins * 100.0f) / battles;
    float timeout_percent = (timeouts * 100.0f) / battles;

    printf("Results for %d battles:\n", battles);
    printf("\n");
    printf("Germany: %.1f%% (%d/%d)\n", german_percent, german_wins, battles);
    printf("Rome: %.1f%% (%d/%d)\n", roman_percent, roman_wins, battles);
    printf("Timeouts: %.1f%% (%d/%d)\n", timeout_percent, timeouts, battles);
    printf("\n");
}

void print_menu(void) {
    printf("\n");
    printf("=== Learning Field ===\n\n");
    printf("  Battles per run:    %d\n", battles);
    printf("  Loaded strategy:    %s\n", STRATEGY_FILE);
    printf("  Evolution settings: pop %d, %d generations\n\n", pop_size, num_generations);
    printf("  [1] Run %d battles (fast)         — watch the win rate\n", battles);
    printf("  [2] Run 1 battle with animation    — see the fight\n");
    printf("  [3] Evolve a new strategy          — improve the AI\n");
    printf("  [4] Calibrate unit point values    — balance units\n");
    printf("  [5] Run automated tests            — check for bugs\n");
    printf("  [6] Pairwise unit matrix           — see which units dominate\n");
    printf("  [q] Quit\n\n");
    printf("  Choice: ");
}

void print_loaded_composition(void) {
    printf("\n  German army: ");
    for (int i = 0; i < default_german_composition_count; i++) {
        if (default_german_composition[i].count > 0) {
            printf("%d %s ",
                   default_german_composition[i].count,
                   short_unit_name(default_german_composition[i].type));
        }
    }
    printf("\n  Roman army:  ");
    for (int i = 0; i < default_roman_composition_count; i++) {
        if (default_roman_composition[i].count > 0) {
            printf("%d %s ",
                   default_roman_composition[i].count,
                   short_unit_name(default_roman_composition[i].type));
        }
    }
    printf("\n\n");
}

int main(void) {
    load_settings("settings.txt");
    while (true) {
        srand(time(NULL));

        print_menu();

        char battle_type = getchar();
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        switch (battle_type) {
            case 'n': case '1': {
                clock_t start = clock();
                german_wins = 0;
                roman_wins = 0;
                timeouts = 0;
                load_strategy_from_file(STRATEGY_FILE);
                print_loaded_composition();
                start_match();
                clock_t end = clock();
                print_action_stats();
                print_results();
                printf("\nAll battles took %.1f seconds.\n", (float)(end - start) / CLOCKS_PER_SEC);
                break;
            }
            case 'f': case '2': {
                turn_based = true;
                print_battle_summary_flag = true;
                german_wins = roman_wins = timeouts = 0;
                load_strategy_from_file(STRATEGY_FILE);
                print_loaded_composition();
                start_match();
                print_results();
                turn_based = false;
                break;
            }
            case 'e': case '3': {
                clock_t start = clock();
                run_evolution();
                clock_t end = clock();
                printf("\nEvolution took %.1f seconds.\n", (float)(end - start) / CLOCKS_PER_SEC);
                break;
            }
            case 'c': case '4': {
                run_point_calibration(100);
                break;
            }
            case 'r': case '5': {
                run_tests();
                break;
            }
            case '6': case 'm': {
                run_pairwise_matrix(100);
                break;
            }
            case 'q': case 'Q':
                return 0;
            default:
                printf("  Unrecognized option '%c'. Try again.\n", battle_type);
                break;
        }
    }
    return 0;
}