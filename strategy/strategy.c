#include "strategy.h"
#include "battlefield.h"
#include "unit.h"
#include "queries.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

// Set the current weights
FactionStrategy republic_strategy = {
    .strategic = {
        .aggression_bias       = 1.2f,
        .threat_weight         = 0.5f,
        .flank_percentage      = 0.3f,
        .flanking_bias         = 1.0f,
        .flank_depth           = 4.0f
    },
    .deployment = {
        .deploy_horizontal_spread = 4.00f,
        .deploy_vertical_offset   = 1.00f
    }
};

FactionStrategy separatist_strategy = {
    .strategic = {
        .aggression_bias       = 1.2f,
        .threat_weight         = 0.5f,
        .flank_percentage      = 0.3f,
        .flanking_bias         = 1.0f,
        .flank_depth           = 4.0f
    },
    .deployment = {
        .deploy_horizontal_spread = 4.00f,
        .deploy_vertical_offset   = 1.00f
    }
};

//Role assignment

void assign_unit_roles(Battlefield* field, Faction faction) {
    FactionStrategy* strat = (faction == FACTION_REPUBLIC) ? &republic_strategy : &separatist_strategy;
    float pct = strat->strategic.flank_percentage;

    Unit* list[MAX_UNITS];
    int n = 0;
    for (int i = 0; i < field->unit_count; i++) {
        Unit* u = field->units[i];
        if (!u || u->hp <= 0 || u->faction != faction) continue;
        list[n++] = u;
    }
    if (n == 0) return;

    // Fastest units become flankers, ties broken by id
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (list[j]->movement > list[i]->movement ||
               (list[j]->movement == list[i]->movement && list[j]->id < list[i]->id)) {
                Unit* tmp = list[i]; list[i] = list[j]; list[j] = tmp;
            }
        }
    }

    int flank_count = (int)(n * pct);
    for (int i = 0; i < n; i++) {
        list[i]->role = (i < flank_count) ? ROLE_FLANK : ROLE_LINE;
    }
}

// Set genes
void set_republic_strategy(const float weights[7]) {
    republic_strategy.strategic.aggression_bias       = weights[0];
    republic_strategy.strategic.threat_weight         = weights[1];
    republic_strategy.strategic.flank_percentage      = weights[2];
    republic_strategy.strategic.flanking_bias         = weights[3];
    republic_strategy.strategic.flank_depth           = weights[4];

    republic_strategy.deployment.deploy_horizontal_spread = weights[5];
    republic_strategy.deployment.deploy_vertical_offset   = weights[6];
}

void set_separatist_strategy(const float weights[7]) {
    separatist_strategy.strategic.aggression_bias       = weights[0];
    separatist_strategy.strategic.threat_weight         = weights[1];
    separatist_strategy.strategic.flank_percentage      = weights[2];
    separatist_strategy.strategic.flanking_bias         = weights[3];
    separatist_strategy.strategic.flank_depth           = weights[4];

    separatist_strategy.deployment.deploy_horizontal_spread = weights[5];
    separatist_strategy.deployment.deploy_vertical_offset   = weights[6];
}

// Load strategies
void load_strategy_from_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Error: Could not open strategy file '%s'\n", filename);
        return;
    }

    float rep_weights[7] = {0};
    float sep_weights[7] = {0};
    int rep_count = 0, sep_count = 0;
    int current_faction = -1;
    char line[256];

    while (fgets(line, sizeof(line), f)) {
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';

        if (line[0] == '#' || line[0] == '\0') continue;

        if (strstr(line, "[REPUBLIC]"))  { current_faction = 0; continue; }
        if (strstr(line, "[SEPARATIST]")){ current_faction = 1; continue; }

        char key[64];
        float value;
        if (sscanf(line, "%s %f", key, &value) == 2) {
            if (current_faction == 0 && rep_count < 8) {
                rep_weights[rep_count++] = value;
            } else if (current_faction == 1 && sep_count < 8) {
                sep_weights[sep_count++] = value;
            }
        }
    }
    fclose(f);

    while (rep_count < 8) rep_weights[rep_count++] = 0.6f;
    while (sep_count < 8) sep_weights[sep_count++] = 0.6f;

    set_republic_strategy(rep_weights);
    set_separatist_strategy(sep_weights);
    printf("Loaded strategies from %s\n", filename);
}

// Fitness evaluation
PairFitness evaluate_matchup(const float* rep_weights, const float* sep_weights, int num_battles) {
    FactionStrategy old_rep = republic_strategy;
    FactionStrategy old_sep = separatist_strategy;

    set_republic_strategy(rep_weights);
    set_separatist_strategy(sep_weights);

    int rep_wins = 0, sep_wins = 0, timeouts = 0;

    for (int i = 0; i < num_battles; i++) {
        Battlefield field;
        initialize_battlefield(&field);

        spawn_republic_forces(&field, default_republic_composition, default_republic_composition_count);
        spawn_separatist_forces(&field, default_separatist_composition, default_separatist_composition_count);

        int winner = run_battle(&field, MAX_TURNS);
        if (winner == 0) rep_wins++;
        else if (winner == 1) sep_wins++;
        else if (winner == 2) timeouts++;

        cleanup_battlefield(&field);
    }

    republic_strategy = old_rep;
    separatist_strategy = old_sep;

    float penalty = 2.0f;
    PairFitness out;
    out.rep_fitness = (float)rep_wins / num_battles - penalty * (float)timeouts / num_battles;
    out.sep_fitness = (float)sep_wins / num_battles - penalty * (float)timeouts / num_battles;
    return out;
}