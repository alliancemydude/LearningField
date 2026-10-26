#include "calibration.h"
#include "battlefield.h"
#include "queries.h"
#include "utils.h"
#include "unit.h"
#include "unit_utils.h"
#include "spawn.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define REFERENCE_POINTS 1000

// Unit count per side for the matrix
#define PAIRWISE_COUNT 15

#define PAIRWISE_POINTS 1000

// Mixed reference armies.
static ForceComposition mixed_roman[4];
static int mixed_roman_count = 4;

static ForceComposition mixed_german[4];
static int mixed_german_count = 4;

static void init_reference_armies(void) {
    // Base proportions: 40 / 10 / 5 / 10 units
    const int base_counts[4] = { 40, 10, 5, 10 };
    const UnitType german_types[4] = {
        UNIT_BARBARIAN, UNIT_ELITE_BARBARIAN,
        UNIT_JAVELIN_THROWER, UNIT_GERMAN_CAVALRY
    };
    const UnitType roman_types[4] = {
        UNIT_LEGIONARE, UNIT_CENTURION,
        UNIT_ARCHER, UNIT_ROMAN_CAVALRY
    };

    // Cost of the base army at current point values
    int german_pts = 0;
    int roman_pts  = 0;
    for (int i = 0; i < 4; i++) {
        german_pts += base_counts[i] * unit_point_values[german_types[i]];
        roman_pts  += base_counts[i] * unit_point_values[roman_types[i]];
    }

    // Scale to fill the current budget
    float german_scale = (german_pts > 0) ? (float)composition_budget / german_pts : 1.0f;
    float roman_scale  = (roman_pts  > 0) ? (float)composition_budget / roman_pts  : 1.0f;

    for (int i = 0; i < 4; i++) {
        int g_count = (int)(base_counts[i] * german_scale);
        int r_count = (int)(base_counts[i] * roman_scale);
        if (g_count < 1) g_count = 1;
        if (r_count < 1) r_count = 1;

        mixed_german[i].type  = german_types[i];
        mixed_german[i].count = g_count;

        mixed_roman[i].type   = roman_types[i];
        mixed_roman[i].count  = r_count;
    }
    mixed_german_count = 4;
    mixed_roman_count  = 4;
}

static int run_pair(UnitType german_type, UnitType roman_type, int battles) {
    int german_cost = unit_point_values[german_type];
    int roman_cost  = unit_point_values[roman_type];
    if (german_cost <= 0 || roman_cost <= 0) return 0;

    int german_count = PAIRWISE_POINTS / german_cost;
    int roman_count  = PAIRWISE_POINTS / roman_cost;
    if (german_count < 1) german_count = 1;
    if (roman_count  < 1) roman_count  = 1;

    ForceComposition german_comp[1] = { { german_type, german_count } };
    ForceComposition roman_comp[1]  = { { roman_type,  roman_count  } };

    int german_wins = 0;
    for (int i = 0; i < battles; i++) {
        Battlefield field;
        initialize_battlefield(&field);
        spawn_german_forces(&field, german_comp, 1);
        spawn_roman_forces(&field, roman_comp, 1);

        int winner = run_battle(&field, MAX_TURNS);
        if (winner == 0) german_wins++;

        cleanup_battlefield(&field);
    }
    return german_wins;
}

void run_pairwise_matrix(int battles_per_pair) {
    // Gather rosters by faction
    UnitType german_roster[UNIT_TYPE_COUNT];
    UnitType roman_roster[UNIT_TYPE_COUNT];
    int gn = 0, rn = 0;

    for (int i = 0; i < UNIT_TYPE_COUNT; i++) {
        Faction f = get_faction_of_unit((UnitType)i);
        if (f == FACTION_GERMAN)      german_roster[gn++] = (UnitType)i;
        else if (f == FACTION_ROMAN)  roman_roster[rn++]  = (UnitType)i;
    }

    printf("\n=== PAIRWISE MATRIX ===\n");
    printf("Each cell: win rate of German (row) vs Roman (column)\n");
    printf("%dv%d, %d battles per cell\n\n", PAIRWISE_COUNT, PAIRWISE_COUNT, battles_per_pair);

    // Header row
    printf("%-12s", "");
    for (int j = 0; j < rn; j++) {
        int cnt = PAIRWISE_POINTS / unit_point_values[roman_roster[j]];
        printf("%6s(%2d)", short_unit_name(roman_roster[j]), cnt);
    }
    printf("\n");

    // Body
    for (int i = 0; i < gn; i++) {
        printf("%-12s", short_unit_name(german_roster[i]));
        for (int j = 0; j < rn; j++) {
            int wins = run_pair(german_roster[i], roman_roster[j], battles_per_pair);
            float wr = 100.0f * wins / battles_per_pair;
            printf("%9.1f%%", wr);
        }
        printf("\n");
    }
    printf("\n");
}

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
        if (test_faction == FACTION_GERMAN && winner == 0) {
            test_wins++;
        } else if (test_faction == FACTION_ROMAN  && winner == 1) {
            test_wins++;
        }

        cleanup_battlefield(&field);
    }
    return (float)test_wins / (float)num_battles;
}

static float measure_mixed_value(ForceComposition* mixed, int mixed_count, UnitType meter_type, Faction meter_faction,
                                 float meter_unit_cost, int battles) {
    // Search wide enough to reach the full budget
    int max_units = 2 * composition_budget / (int)meter_unit_cost + 10;
    int low = 1, high = max_units;

    while (high - low > 1) {
        int mid = (low + high) / 2;
        float wr = win_rate_against_reference(meter_type, meter_faction, mid, mixed, mixed_count, battles);
        printf("      %2d %s vs mixed -> %.1f%%\n", mid,
               get_unit_type_name(meter_type), wr * 100.0f);
        if (wr >= 0.5f) high = mid;
        else            low = mid;
    }

    if (high == max_units) {
        printf("      WARNING: search saturated at N=%d\n", max_units);
    }
    return (float)high * meter_unit_cost;
}

// Binomial standard error on a win rate.
static float win_rate_stderr(float wr, int n) {
    if (n <= 0) {
        return 0.0f;
    }
    return sqrtf(wr * (1.0f - wr) / (float)n);
}

static int find_crossover (UnitType test_type, Faction test_faction, ForceComposition* ref, int ref_count,
    int min_count, int max_count, int battles_per_step, float* out_fractional, int* out_integer, float* out_winrate) {
    
        int low = min_count;
    int high = max_count;

    while (high - low > 1) {
        int mid = (low + high) / 2;
        float winrate = win_rate_against_reference(test_type, test_faction, mid, ref, ref_count, battles_per_step);
        float se = win_rate_stderr(winrate, battles_per_step);

        printf("      N=%2d -> %.1f%% (±%.1f%%)\n", mid, winrate * 100.0f, se * 200.0f);

        if (winrate >= 0.5f) {
            high = mid;
        } else {
            low = mid;
        }
    }

    // Interpolation: run one final high-precision batch at low and hi.
    // 3x battles gives tighter estimates.
    int final_battles = battles_per_step * 3;

    float winrate_low = win_rate_against_reference(test_type, test_faction, low, ref, ref_count, final_battles);
    float winrate_high = win_rate_against_reference(test_type, test_faction, high, ref, ref_count, final_battles);

    printf("      Interpolation: N=%d -> %.1f%%, N=%d -> %.1f%%\n", low, winrate_low * 100.0f, high, winrate_high * 100.0f);

    // Linear interpolation. Assumes winrate_low < 0.5 <= winrate_high, which the
    // binary-search invariant guarantees.
    float fractional = (float)high;
    if (winrate_high > winrate_low && winrate_low < 0.5f && winrate_high >= 0.5f) {
        fractional = (float)low + (0.5f - winrate_low) / (winrate_high - winrate_low);
    }

    if (high == max_count && winrate_high < 0.5f) {
        printf("      WARNING: crossover saturated at N=%d (winrate %.1f%%) — "
            "result unreliable, increase max_count\n",
            max_count, winrate_high * 100.0f);
    }

    printf("      => fractional crossover = %.2f units\n", fractional);

    if (out_fractional) *out_fractional = fractional;
    if (out_integer)    *out_integer = high;
    if (out_winrate)    *out_winrate = winrate_high;

    return high;
}

static float calibrate_unit(UnitType type, Faction test_faction, ForceComposition* ref, int ref_count,
                            float reference_value, int battles_per_step, int* out_crossover) {
    printf("  %-20s (ref value %.0f):\n", get_unit_type_name(type), reference_value);

    // Each unit's crossover is roughly reference_value / cost.
    int max_units = 2 * (int)(reference_value / 5.0f) + 10;

    float fractional = 0.0f;
    int   integer    = 0;

    find_crossover(type, test_faction, ref, ref_count, 1, max_units, battles_per_step, &fractional, &integer, NULL);

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

    init_reference_armies();

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