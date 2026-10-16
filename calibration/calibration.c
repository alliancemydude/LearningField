#include "calibration.h"
#include "battlefield.h"
#include "queries.h"
#include "utils.h"
#include "unit.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define REFERENCE_POINTS 200

// Mixed reference armies.
static ForceComposition mixed_roman[] = {
    { UNIT_LEGIONARE,        10},
    { UNIT_CENTURION,        5},
    { UNIT_ARCHER,           5},
    { UNIT_ROMAN_CAVALRY,    2},
};
static int mixed_roman_count = 4;

static ForceComposition mixed_german[] = {
    { UNIT_BARBARIAN,         10},
    { UNIT_ELITE_BARBARIAN,   5},
    { UNIT_JAVELIN_THROWER,   5},
    { UNIT_GERMAN_CAVALRY,    2},
};
static int mixed_german_count = 4;

static float win_rate_against_reference (UnitType test_type, Faction test_faction, int test_count,
    ForceComposition* ref, int ref_count, int num_battles) {
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
        if      (test_faction == FACTION_GERMAN && winner == 0) test_wins++;
        else if (test_faction == FACTION_ROMAN  && winner == 1) test_wins++;

        cleanup_battlefield(&field);
    }
    return (float)test_wins / (float)num_battles;
}

static float measure_mixed_value (ForceComposition* mixed, int mixed_count, UnitType meter_type, Faction meter_faction,
    float meter_unit_cost, int battles) {
    int lo = 1, hi = 40;
    while (hi - lo > 1) {
        int mid = (lo + hi) / 2;
        float wr = win_rate_against_reference(meter_type, meter_faction, mid, mixed, mixed_count, battles);
        printf("      %2d %s vs mixed -> %.1f%%\n", mid, get_unit_type_name(meter_type), wr * 100.0f);
        if (wr >= 0.5f) hi = mid;
        else            lo = mid;
    }
    return (float)hi * meter_unit_cost;
}

// Binomial standard error on a win rate.
static float win_rate_stderr(float wr, int n) {
    if (n <= 0) return 0.0f;
    return sqrtf(wr * (1.0f - wr) / (float)n);
}

static int find_crossover (UnitType test_type, Faction test_faction, ForceComposition* ref, int ref_count,
    int min_count, int max_count, int battles_per_step, float* out_fractional, int* out_integer, float* out_winrate) {
    int lo = min_count;
    int hi = max_count;

    while (hi - lo > 1) {
        int mid = (lo + hi) / 2;
        float wr = win_rate_against_reference(test_type, test_faction, mid, ref, ref_count, battles_per_step);
        float se = win_rate_stderr(wr, battles_per_step);

        printf("      N=%2d -> %.1f%% (±%.1f%%)\n", mid,
               wr * 100.0f, se * 200.0f);

        if (wr >= 0.5f) hi = mid;
        else            lo = mid;
    }

    // Interpolation: run one final high-precision batch at lo and hi.
    // 3x battles gives tighter estimates.
    int final_battles = battles_per_step * 3;

    float wr_lo = win_rate_against_reference(test_type, test_faction, lo, ref, ref_count, final_battles);
    float wr_hi = win_rate_against_reference(test_type, test_faction, hi, ref, ref_count, final_battles);

    printf("      Interpolation: N=%d -> %.1f%%, N=%d -> %.1f%%\n", lo, wr_lo * 100.0f, hi, wr_hi * 100.0f);

    // Linear interpolation. Assumes wr_lo < 0.5 <= wr_hi, which the
    // binary-search invariant guarantees.
    float fractional = (float)hi;
    if (wr_hi > wr_lo && wr_lo < 0.5f && wr_hi >= 0.5f) {
        fractional = (float)lo + (0.5f - wr_lo) / (wr_hi - wr_lo);
    }

    printf("      => fractional crossover = %.2f units\n", fractional);

    if (out_fractional) *out_fractional = fractional;
    if (out_integer)    *out_integer = hi;
    if (out_winrate)    *out_winrate = wr_hi;

    return hi;
}

static float calibrate_unit(UnitType type, Faction test_faction, ForceComposition* ref, int ref_count,
                            float reference_value, int battles_per_step, int* out_crossover) {
    printf("  %-20s (ref value %.0f):\n", get_unit_type_name(type), reference_value);

    float fractional = 0.0f;
    int   integer    = 0;

    find_crossover(type, test_faction, ref, ref_count, 1, 40, battles_per_step, &fractional, &integer, NULL);

    float cost = reference_value / fractional;

    printf("    => N=%.2f (integer %d), cost = %.2f points\n\n", fractional, integer, cost);

    if (out_crossover) *out_crossover = integer;
    return cost;
}

// Verification: run same-cost armies, check they land ~50/50

static void verify_cost(UnitType type, Faction test_faction, ForceComposition* ref, int ref_count,
                        int test_count, float test_cost, float ref_cost, int battles) {
    printf("  %-20s: %2d @ %.1f pts vs %2d ref @ %.1f pts ... ", 
        get_unit_type_name(type), test_count, test_cost, ref[0].count, ref_cost);

    float wr = win_rate_against_reference(type, test_faction, test_count, ref, ref_count, battles);
    float se = win_rate_stderr(wr, battles);

    printf("%.1f%% (±%.1f%%)\n", wr * 100.0f, se * 200.0f);
}

void run_point_calibration(int battles_per_step) {
    printf("\n=== POINT CALIBRATION v3 (mixed references) ===\n");
    printf("Battles per binary-search step: %d\n\n", battles_per_step);

    // Mixed armies need their own value measured first
    printf("--- Phase 0: Valuing mixed reference armies ---\n\n");

    printf("German meter (barbarian) vs Roman mixed army:\n");
    float roman_mixed_value = measure_mixed_value(mixed_roman, mixed_roman_count,
        UNIT_BARBARIAN, FACTION_GERMAN, 10.0f, battles_per_step);
    printf("  => Roman mixed army is worth %.1f points\n\n", roman_mixed_value);

    printf("Roman meter (legionare) vs German mixed army:\n");
    float german_mixed_value = measure_mixed_value(mixed_german, mixed_german_count,
        UNIT_LEGIONARE, FACTION_ROMAN, 10.0f, battles_per_step);
    printf("  => German mixed army is worth %.1f points\n\n", german_mixed_value);

    // Phase 1: calibrate each unit against the appropriate mixed reference
    float costs[UNIT_TYPE_COUNT]      = {0};
    int   crossovers[UNIT_TYPE_COUNT] = {0};

    printf("--- Phase 1: Calibration against mixed references ---\n\n");

    printf("German units (vs Roman mixed army, value %.1f):\n", roman_mixed_value);
    for (int i = 0; i < UNIT_TYPE_COUNT; i++) {
        UnitType type = (UnitType)i;
        if (get_faction_of_unit(type) != FACTION_GERMAN) {
            continue;
        }
        if (type == UNIT_BARBARIAN) {
            costs[type] = 10.0f; crossovers[type] = 20; continue;
        }
        costs[type] = calibrate_unit(type, FACTION_GERMAN, mixed_roman, mixed_roman_count,
                                     roman_mixed_value, battles_per_step, &crossovers[type]);
    }

    printf("Roman units (vs German mixed army, value %.1f):\n", german_mixed_value);
    for (int i = 0; i < UNIT_TYPE_COUNT; i++) {
        UnitType type = (UnitType)i;
        if (get_faction_of_unit(type) != FACTION_ROMAN) {
            continue;
        }
        if (type == UNIT_LEGIONARE) {
            costs[type] = 10.0f; crossovers[type] = 20; continue;
        }
        costs[type] = calibrate_unit(type, FACTION_ROMAN, mixed_german, mixed_german_count,
                                     german_mixed_value, battles_per_step, &crossovers[type]);
    }

    // Phase 2: verify at the discovered crossover count
    printf("--- Phase 2: Verification ---\n\n");

    printf("German units vs Roman mixed army:\n");
    for (int i = 0; i < UNIT_TYPE_COUNT; i++) {
        UnitType type = (UnitType)i;
        if (get_faction_of_unit(type) != FACTION_GERMAN) {
            continue;
        }
        if (type == UNIT_BARBARIAN) {
            continue;
        }
        verify_cost(type, FACTION_GERMAN, mixed_roman, mixed_roman_count,
                    crossovers[type], costs[type], 0.0f, battles_per_step * 2);
    }

    printf("\nRoman units vs German mixed army:\n");
    for (int i = 0; i < UNIT_TYPE_COUNT; i++) {
        UnitType type = (UnitType)i;
        if (get_faction_of_unit(type) != FACTION_ROMAN) {
            continue;
        }
        if (type == UNIT_LEGIONARE) {
            continue;
        }
        verify_cost(type, FACTION_ROMAN, mixed_german, mixed_german_count,
                    crossovers[type], costs[type], 0.0f, battles_per_step * 2);
    }

    // Summary
    printf("\n=== FINAL COSTS (mixed-army calibration) ===\n");
    for (int i = 0; i < UNIT_TYPE_COUNT; i++) {
        UnitType type = (UnitType)i;
        if (costs[type] <= 0) {
            continue;
        }
        printf("  %-20s  %.1f points  (N=%d)\n", get_unit_type_name(type), costs[type], crossovers[type]);
    }
    printf("\n");
}