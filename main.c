#include "battlefield.h"
#include "unit.h"
#include "ai.h"
#include "strategy.h"
#include "evolution.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define STRATEGY_FILE "strategy.txt"

// Do not adjust
int clone_wins = 0;
int droid_wins = 0;
int timeouts = 0;

// Basic adjustable values
int battles                     = 10000;
bool track_actions              = true;

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
        spawn_republic_forces(&field, default_republic_composition, default_republic_composition_count);
        spawn_separatist_forces(&field, default_separatist_composition, default_separatist_composition_count);
        
        // Increment stats based on the winner
        int winner = run_battle(&field, MAX_TURNS);
        if (winner == 0) {
            clone_wins++;
        } else if (winner == 1) {
            droid_wins++;
        } else {
            timeouts++;
        }

        // Free memory 
        cleanup_battlefield(&field);
    }
}

void print_results() {
    // Calculate percentage of wins for each side and timeouts
    float clone_percent = (clone_wins * 100.0f) / battles;
    float droid_percent = (droid_wins * 100.0f) / battles;
    float timeout_percent = (timeouts * 100.0f) / battles;

    printf("Results for %d battles:\n", battles);
    printf("\n");
    printf("Clones: %.1f%% (%d/%d)\n", clone_percent, clone_wins, battles);
    printf("Droids: %.1f%% (%d/%d)\n", droid_percent, droid_wins, battles);
    printf("Timeouts: %.1f%% (%d/%d)\n", timeout_percent, timeouts, battles);
    printf("\n");
}

int main() {
    while (true) {
        srand(time(NULL));

        printf("Choose which way to run the program.\n");
        printf("1. Normal combat (n)\n");
        printf("2. Turn-by-turn battle (t)\n");
        printf("3. Verbose analysis (v)\n");
        printf("4. Strategic evolution (e)\n");

        char battle_type = getchar();

        if (battle_type == 'n') {
            clock_t start = clock();

            load_strategy_from_file(STRATEGY_FILE);
            start_match();

            clock_t end = clock();
            print_action_stats();
            print_results();
            float time_taken = ((float)(end - start)) / CLOCKS_PER_SEC;
            printf("\nAll battles took %.1f seconds.\n", time_taken);

        } else if (battle_type == 't') {
            turn_based = true;

            load_strategy_from_file(STRATEGY_FILE);
            start_match();

            print_results();
        } else if (battle_type == 'v') {
            turn_based = true;
            print_battle_summary_flag = true;

            load_strategy_from_file(STRATEGY_FILE);
            start_match();

            print_results();
        } else if (battle_type == 'e') {
            clock_t start = clock();

            printf("Select evolution mode:\n");
            printf("1. Tactical\n");
            printf("2. Strategic\n");
            printf("3. Deployment\n");
            printf("4. All\n");
            printf("5. Unit (co‑evolves both factions)\n");
            printf("Enter choice (1-5): ");

            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }

            int choice = getchar() - '0';

            EvolutionMode mode;
            switch (choice) {
                case 1: mode = EVOLVE_TACTICAL; break;
                case 2: mode = EVOLVE_STRATEGIC; break;
                case 3: mode = EVOLVE_DEPLOYMENT; break;
                case 5: mode = EVOLVE_UNIT; break;
                default: mode = EVOLVE_ALL; break;
            }

            run_evolution(mode);

            clock_t end = clock();
            float time_taken = ((float)(end - start)) / CLOCKS_PER_SEC;
            printf("\nEvolution took %.1f seconds.\n", time_taken);
        } else {
            printf("Listen man just hit the right button\n");
        }

        printf("\n");
        printf("\n");
        printf("\n");
    }
    return 0;
}