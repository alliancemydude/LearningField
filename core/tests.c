#include "tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "battlefield.h"
#include "queries.h"
#include "unit.h"
#include "spawn.h"
#include "combat.h"
#include "movement.h"
#include "utils.h"
#include "unit_blueprint.h"

static int tests_run = 0;
static int tests_failed = 0;

#define ASSERT(cond, msg) do {                                              \
    if (!(cond)) {                                                          \
        printf("\n    FAIL: %s (%s:%d)", msg, __FILE__, __LINE__);          \
        return 1;                                                           \
    }                                                                       \
} while (0)

#define RUN(test) do {                                                      \
    printf("  %-40s", #test);                                               \
    tests_run++;                                                            \
    if (test() != 0) { tests_failed++; printf(" FAIL\n"); }                 \
    else printf(" ok\n");                                                   \
} while (0)

static int test_faction_of_unit(void) {
    ASSERT(get_faction_of_unit(UNIT_BARBARIAN)       == FACTION_GERMAN, "barbarian");
    ASSERT(get_faction_of_unit(UNIT_ELITE_BARBARIAN) == FACTION_GERMAN, "elite barbarian");
    ASSERT(get_faction_of_unit(UNIT_JAVELIN_THROWER)   == FACTION_GERMAN, "thrower");
    ASSERT(get_faction_of_unit(UNIT_GERMAN_CAVALRY)        == FACTION_GERMAN, "german cavalry");
    ASSERT(get_faction_of_unit(UNIT_LEGIONARE)        == FACTION_ROMAN, "legionare");
    ASSERT(get_faction_of_unit(UNIT_CENTURION)  == FACTION_ROMAN, "centurion");
    ASSERT(get_faction_of_unit(UNIT_ARCHER)          == FACTION_ROMAN, "archer");
    ASSERT(get_faction_of_unit(UNIT_ROMAN_CAVALRY)         == FACTION_ROMAN, "roman cavalry");
    return 0;
}

static int test_enemy_faction(void) {
    ASSERT(enemy_faction(FACTION_GERMAN)  == FACTION_ROMAN, "german->roman");
    ASSERT(enemy_faction(FACTION_ROMAN) == FACTION_GERMAN, "roman->german");
    return 0;
}

static int test_create_unit_matches_blueprint(void) {
    Unit* u = create_unit(UNIT_BARBARIAN, 1, 10, 10);
    ASSERT(u != NULL, "create returned NULL");

    const UnitBlueprint* bp = &unit_blueprints[UNIT_BARBARIAN];
    ASSERT(u->type == UNIT_BARBARIAN, "wrong type");
    ASSERT(u->faction == bp->faction, "faction mismatch");
    ASSERT(u->hp == bp->hp, "hp mismatch");
    ASSERT(u->max_hp == bp->hp, "max_hp mismatch");
    ASSERT(u->movement == bp->movement, "movement mismatch");
    ASSERT(u->half_movement == bp->movement / 2, "half_movement mismatch");
    ASSERT(u->armor_class == bp->armor_class, "armor mismatch");
    ASSERT(u->units_count == bp->units_count, "units_count mismatch");
    ASSERT(u->weapon_count == bp->weapon_count, "weapon_count mismatch");
    ASSERT(u->x == 10 && u->y == 10, "position mismatch");

    destroy_unit(u);
    return 0;
}

static int test_take_damage_invariants(void) {
    Unit* u = create_unit(UNIT_ELITE_BARBARIAN, 1, 0, 0);
    int initial_hp = u->hp;

    // Excessive damage never goes negative
    take_damage(u, initial_hp + 100);
    ASSERT(u->hp == 0, "hp not clamped to 0");
    ASSERT(u->units_count == 0, "units_count should be 0 at 0 hp");

    destroy_unit(u);
    return 0;
}

static int test_soldier_count_recompute(void) {
    // Elite Barbarian: hp = 2, hp_per_soldier = 2, units_count = 1
    // After 1 damage: hp = 1, units_count should still be 1.
    Unit* u = create_unit(UNIT_ELITE_BARBARIAN, 1, 0, 0);
    take_damage(u, 1);
    ASSERT(u->hp == 1, "hp wrong after 1 damage");
    ASSERT(u->units_count == 1, "soldier count should be 1");
    destroy_unit(u);

    // Barbarian: hp = 1, hp_per_soldier = 1, units_count = 1
    // After 1 damage: hp = 0, dead.
    Unit* v = create_unit(UNIT_BARBARIAN, 2, 0, 0);
    take_damage(v, 1);
    ASSERT(v->hp == 0, "barbarian should be dead");
    ASSERT(v->units_count == 0, "barbarian units_count should be 0");
    destroy_unit(v);

    return 0;
}

static int test_is_tile_walkable_bounds(void) {
    Battlefield f;
    initialize_battlefield(&f);

    ASSERT(!is_tile_walkable(&f, -1, 0, NULL), "negative x");
    ASSERT(!is_tile_walkable(&f, 0, -1, NULL), "negative y");
    ASSERT(!is_tile_walkable(&f, f.width, 0, NULL), "past width");
    ASSERT(!is_tile_walkable(&f, 0, f.height, NULL), "past height");
    ASSERT( is_tile_walkable(&f, 0, 0, NULL), "corner");

    return 0;
}

static int test_spawn_no_overlaps(void) {
    Battlefield f;
    initialize_battlefield(&f);

    spawn_german_forces(&f, default_german_composition,
                          default_german_composition_count);
    spawn_roman_forces(&f, default_roman_composition,
                            default_roman_composition_count);

    for (int i = 0; i < f.unit_count; i++) {
        for (int j = i + 1; j < f.unit_count; j++) {
            Unit* a = f.units[i];
            Unit* b = f.units[j];
            if (!a || !b || a->hp <= 0 || b->hp <= 0) continue;
            ASSERT(!(a->x == b->x && a->y == b->y), "spawn overlap");
        }
    }

    cleanup_battlefield(&f);
    return 0;
}

static int test_battle_completes(void) {
    Battlefield f;
    initialize_battlefield(&f);

    spawn_german_forces(&f, default_german_composition,
                          default_german_composition_count);
    spawn_roman_forces(&f, default_roman_composition,
                            default_roman_composition_count);

    int winner = run_battle(&f, MAX_TURNS);
    ASSERT(winner == 0 || winner == 1 || winner == 2, "invalid winner code");

    for (int i = 0; i < f.unit_count; i++) {
        Unit* u = f.units[i];
        if (!u) continue;
        ASSERT(u->hp >= 0, "negative hp after battle");
        ASSERT(u->hp <= u->max_hp, "hp above max_hp");
        ASSERT(u->x >= 0 && u->x < f.width, "x out of bounds");
        ASSERT(u->y >= 0 && u->y < f.height, "y out of bounds");
    }

    cleanup_battlefield(&f);
    return 0;
}

static int test_distance_squared(void) {
    Unit a = { .x = 3, .y = 4 };
    Unit b = { .x = 0, .y = 0 };
    ASSERT(get_distance_squared(&a, &b) == 25, "3-4-5 triangle");

    Unit c = { .x = 5, .y = 5 };
    Unit d = { .x = 5, .y = 5 };
    ASSERT(get_distance_squared(&c, &d) == 0, "same tile");

    return 0;
}

static int test_movement_updates_grid(void) {
    Battlefield f;
    initialize_battlefield(&f);

    Unit* u = create_unit(UNIT_BARBARIAN, 1, 10, 10);
    f.units[f.unit_count++] = u;
    f.occupancy[10][10] = u;

    execute_move(u, &f, 15, 10, 10);

    ASSERT(f.occupancy[10][10] == NULL, "old tile not cleared");
    ASSERT(f.occupancy[u->y][u->x] == u, "new tile not set");

    destroy_unit(u);
    return 0;
}

int run_tests(void) {
    srand(42);   // fixed seed for reproducible failures

    tests_run = 0;
    tests_failed = 0;

    printf("\n=== RUNNING TESTS ===\n");
    RUN(test_faction_of_unit);
    RUN(test_enemy_faction);
    RUN(test_create_unit_matches_blueprint);
    RUN(test_take_damage_invariants);
    RUN(test_soldier_count_recompute);
    RUN(test_is_tile_walkable_bounds);
    RUN(test_spawn_no_overlaps);
    RUN(test_battle_completes);
    RUN(test_distance_squared);
    RUN(test_movement_updates_grid);

    printf("\n%d/%d passed.\n\n", tests_run - tests_failed, tests_run);
    return tests_failed == 0 ? 0 : 1;
}