#include <stdio.h>
#include "battle_stats.h"
#include "battlefield.h"
#include "unit.h"
#include "utils.h"

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