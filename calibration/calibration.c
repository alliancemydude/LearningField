#include "calibration.h"
#include "battlefield.h"
#include "queries.h"
#include "utils.h"
#include "unit.h"
#include <stdio.h>
#include <stdlib.h>

static float win_rate_against_reference(
    UnitType test_type, Faction test_faction, int test_count,
    ForceComposition* ref, int ref_count,
    int num_battles)
{
    ForceComposition test_comp[1] = { { test_type, test_count } };
    int test_wins = 0;

    for (int i = 0; i < num_battles; i++) {
        Battlefield field;
        initialize_battlefield(&field);

        if (test_faction == FACTION_GERMAN) {
            spawn_german_forces(&field, test_comp, 1);
            spawn_roman_forces(&field, ref, ref_count);
        } else {
            spawn_german_forces(&field, ref, ref_count);
            spawn_roman_forces(&field, test_comp, 1);
        }

        int winner = run_battle(&field, MAX_TURNS);
        if      (test_faction == FACTION_GERMAN   && winner == 0) test_wins++;
        else if (test_faction == FACTION_ROMAN && winner == 1) test_wins++;

        cleanup_battlefield(&field);
    }
    return (float)test_wins / (float)num_battles;
}

// Binary search for the count that gives ~50% win rate.
static int find_crossover(
    UnitType test_type, Faction test_faction,
    ForceComposition* ref, int ref_count,
    int min_count, int max_count, int battles_per_step,
    int verbose)
{
    int lo = min_count;
    int hi = max_count;

    while (hi - lo > 1) {
        int mid = (lo + hi) / 2;
        float wr = win_rate_against_reference(test_type, test_faction, mid,
                                              ref, ref_count,
                                              battles_per_step);

        if (verbose) {
            printf("      N=%2d -> %.1f%%\n", mid, wr * 100.0f);
        }

        if (wr >= 0.5f) hi = mid;
        else            lo = mid;
    }
    return hi;
}

static void calibrate_unit(UnitType type, Faction test_faction, ForceComposition* ref, int ref_count,
                           int reference_points, int battles_per_step)
{
    printf("  %-20s (vs %2d %s):\n", get_unit_type_name(type), ref[0].count, get_unit_type_name(ref[0].type));

    int crossover = find_crossover(type, test_faction, ref, ref_count, 1, 40, battles_per_step, 1);

    if (crossover <= 0) {
        printf("    FAILED\n\n");
        return;
    }

    float cost = (float)reference_points / (float)crossover;
    printf("    => crossover N=%d, cost = %.2f points\n\n", crossover, cost);
}

void run_point_calibration(int battles_per_step)
{
    printf("\n=== POINT CALIBRATION ===\n");
    printf("Battles per binary-search step: %d\n", battles_per_step);
    printf("Search range: 1 to 40 units\n\n");

    // Reference armies, both worth 200 "reference points"
    ForceComposition german_reference[1] = { { UNIT_SWORDSMAN, 20 } };
    ForceComposition roman_reference[1] = { { UNIT_SPEARMAN, 20 } };
    int reference_points = 200;

    printf("German reference:    %2d %s = %d pts\n", german_reference[0].count,
           get_unit_type_name(german_reference[0].type), reference_points);
    printf("Roman reference:  %2d %s = %d pts\n\n", roman_reference[0].count,
           get_unit_type_name(roman_reference[0].type), reference_points);

    printf("--- Roman units vs German reference ---\n");
for (int i = 0; i < UNIT_TYPE_COUNT; i++) {
    UnitType type = (UnitType)i;
    if (get_faction_of_unit(type) != FACTION_ROMAN) continue;
    calibrate_unit(type, FACTION_ROMAN, german_reference, 1, reference_points, battles_per_step);
}

printf("--- German units vs Roman reference ---\n");
for (int i = 0; i < UNIT_TYPE_COUNT; i++) {
    UnitType type = (UnitType)i;
    if (get_faction_of_unit(type) != FACTION_GERMAN) continue;
    if (type == UNIT_SWORDSMAN) continue;

    calibrate_unit(type, FACTION_GERMAN, roman_reference, 1, reference_points, battles_per_step);
}

    printf("=== Calibration complete ===\n");
    printf("Costs are in reference points where 20 refs = 200.\n");
    printf("Keep in mind that specialist units (ranged, cavalry) are underpriced by this\n");
    printf("method. Add a correction factor of roughly 1.3-1.5x to those.\n");
}