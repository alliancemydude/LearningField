#include "composition.h"
#include "queries.h"
#include "unit_utils.h"
#include <stdio.h>

int composition_budget = 1000;

int build_composition(const CompositionGenome* genes, Faction faction, ForceComposition* out) {
    // Enumerate the faction's roster in enum order.
    UnitType roster[COMPOSITION_MAX_TYPES];
    int roster_n = 0;
    for (int i = 0; i < UNIT_TYPE_COUNT && roster_n < COMPOSITION_MAX_TYPES; i++) {
        if (get_faction_of_unit((UnitType)i) == faction) {
            roster[roster_n++] = (UnitType)i;
        }
    }
    if (roster_n == 0) {
        return 0;
    }

    // Sum of shares for normalization.
    float total = 0.0f;
    for (int i = 0; i < roster_n; i++) {
        if (genes->share[i] > 0.0f) total += genes->share[i];
    }

    // Allocate each type's point budget.
    int spent = 0;
    for (int i = 0; i < roster_n; i++) {
        float share = (total > 0.0f) ? genes->share[i] : 1.0f / (float)roster_n;
        if (share < 0.0f) {
            share = 0.0f;
        }

        float points_for_type = composition_budget * (share / total);
        int cost = get_unit_point_value(roster[i]);
        if (cost <= 0) {
            cost = 1;
        }

        int count = (int)(points_for_type / cost);
        if (count < 0) {
            count = 0;
        }

        out[i].type  = roster[i];
        out[i].count = count;
        spent += count * cost;
    }

    // Spend any rounding leftover on the cheapest unit type.
    int cheapest_idx  = 0;
    int cheapest_cost = get_unit_point_value(roster[0]);
    for (int i = 1; i < roster_n; i++) {
        int c = get_unit_point_value(roster[i]);
        if (c > 0 && c < cheapest_cost) {
            cheapest_cost = c; cheapest_idx = i;
        }
    }
    while (cheapest_cost > 0 && spent + cheapest_cost <= composition_budget) {
        out[cheapest_idx].count++;
        spent += cheapest_cost;
    }

    return roster_n;
}