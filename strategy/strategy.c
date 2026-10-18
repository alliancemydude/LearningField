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
FactionStrategy german_strategy = {
    .strategic = {
        .aggression_bias       = 1.2f,
        .threat_weight         = 0.5f,
        .flank_percentage      = 0.3f,
        .flanking_bias         = 1.0f,
        .flank_depth           = 4.0f,
        .face_threat_weight    = 0.0f,
        .flank_targeting_weight = 0.0f,
        .cohesion_weight       = 0.0f
    },
    .deployment = {
        .deploy_horizontal_spread = 4.00f,
        .deploy_vertical_offset   = 1.00f
    }
};

FactionStrategy roman_strategy = {
    .strategic = {
        .aggression_bias       = 1.2f,
        .threat_weight         = 0.5f,
        .flank_percentage      = 0.3f,
        .flanking_bias         = 1.0f,
        .flank_depth           = 4.0f,
        .face_threat_weight    = 0.0f,
        .flank_targeting_weight = 0.0f,
        .cohesion_weight       = 0.0f
    },
    .deployment = {
        .deploy_horizontal_spread = 4.00f,
        .deploy_vertical_offset   = 1.00f
    }
};

//Role assignment

void assign_unit_roles(Battlefield* field, Faction faction) {
    FactionStrategy* strat = (faction == FACTION_GERMAN)
                             ? &german_strategy : &roman_strategy;
    float pct = strat->strategic.flank_percentage;

    Unit* list[MAX_UNITS];
    int n = 0;
    for (int i = 0; i < field->unit_count; i++) {
        Unit* u = field->units[i];
        if (!u || u->hp <= 0 || u->faction != faction) continue;
        list[n++] = u;
    }
    if (n == 0) return;

    // Ranged units get ROLE_RANGED first
    Unit* melee[MAX_UNITS];
    int m = 0;
    for (int i = 0; i < n; i++) {
        if (list[i]->is_ranged) {
            list[i]->role = ROLE_RANGED;
        } else {
            melee[m++] = list[i];
        }
    }
    if (m == 0) return;

    // Fastest melee units become flankers
    for (int i = 0; i < m - 1; i++) {
        for (int j = i + 1; j < m; j++) {
            if (melee[j]->movement > melee[i]->movement ||
               (melee[j]->movement == melee[i]->movement && melee[j]->id < melee[i]->id)) {
                Unit* tmp = melee[i]; melee[i] = melee[j]; melee[j] = tmp;
            }
        }
    }

    int flank_count = (int)(m * pct);
    for (int i = 0; i < m; i++) {
        melee[i]->role = (i < flank_count) ? ROLE_FLANK : ROLE_LINE;
    }
}

// Set genes
void set_german_strategy(const float weights[10]) {
    german_strategy.strategic.aggression_bias           = weights[0];
    german_strategy.strategic.threat_weight             = weights[1];
    german_strategy.strategic.flank_percentage          = weights[2];
    german_strategy.strategic.flanking_bias             = weights[3];
    german_strategy.deployment.deploy_horizontal_spread = weights[4];
    german_strategy.deployment.deploy_vertical_offset   = weights[5];
    german_strategy.strategic.flank_depth               = weights[6];
    german_strategy.strategic.face_threat_weight        = weights[7];
    german_strategy.strategic.flank_targeting_weight    = weights[8];
    german_strategy.strategic.cohesion_weight           = weights[9];
}

void set_roman_strategy(const float weights[10]) {
    roman_strategy.strategic.aggression_bias           = weights[0];
    roman_strategy.strategic.threat_weight             = weights[1];
    roman_strategy.strategic.flank_percentage          = weights[2];
    roman_strategy.strategic.flanking_bias             = weights[3];
    roman_strategy.deployment.deploy_horizontal_spread = weights[4];
    roman_strategy.deployment.deploy_vertical_offset   = weights[5];
    roman_strategy.strategic.flank_depth               = weights[6];
    roman_strategy.strategic.face_threat_weight        = weights[7];
    roman_strategy.strategic.flank_targeting_weight    = weights[8];
    roman_strategy.strategic.cohesion_weight           = weights[9];
}

// Load strategies
static UnitType parse_unit_type(const char* name) {
    if (strcmp(name, "barbarian")      == 0) return UNIT_BARBARIAN;
    if (strcmp(name, "elite_barbarian")== 0) return UNIT_ELITE_BARBARIAN;
    if (strcmp(name, "javelin_thrower")== 0) return UNIT_JAVELIN_THROWER;
    if (strcmp(name, "german_cavalry") == 0) return UNIT_GERMAN_CAVALRY;
    if (strcmp(name, "legionare")       == 0) return UNIT_LEGIONARE;
    if (strcmp(name, "centurion") == 0) return UNIT_CENTURION;
    if (strcmp(name, "archer")         == 0) return UNIT_ARCHER;
    if (strcmp(name, "roman_cavalry")        == 0) return UNIT_ROMAN_CAVALRY;
    return UNIT_TYPE_COUNT;   // sentinel: not found
}

static void set_strategy_gene(FactionStrategy* strat, const char* gene, float value) {
    if      (strcmp(gene, "aggression_bias")          == 0) strat->strategic.aggression_bias = value;
    else if (strcmp(gene, "threat_weight")            == 0) strat->strategic.threat_weight = value;
    else if (strcmp(gene, "flank_percentage")         == 0) strat->strategic.flank_percentage = value;
    else if (strcmp(gene, "flanking_bias")            == 0) strat->strategic.flanking_bias = value;
    else if (strcmp(gene, "flank_depth")              == 0) strat->strategic.flank_depth = value;
    else if (strcmp(gene, "deploy_horizontal_spread") == 0) strat->deployment.deploy_horizontal_spread = value;
    else if (strcmp(gene, "deploy_vertical_offset")   == 0) strat->deployment.deploy_vertical_offset = value;
    else if (strcmp(gene, "face_threat_weight")       == 0) strat->strategic.face_threat_weight = value;
    else if (strcmp(gene, "flank_targeting_weight")   == 0) strat->strategic.flank_targeting_weight = value;
    else if (strcmp(gene, "cohesion_weight")          == 0) strat->strategic.cohesion_weight = value;
}

static void set_unit_gene(UnitType type, const char* gene, float value) {
    UnitGenome* ug = &unit_genomes[type];
    if      (strcmp(gene, "aggression")        == 0) ug->unit_aggression = value;
    else if (strcmp(gene, "preferred_range")   == 0) ug->preferred_range = value;
    else if (strcmp(gene, "spacing_x")         == 0) ug->spacing_x = value;
    else if (strcmp(gene, "spacing_y")         == 0) ug->spacing_y = value;
    else if (strcmp(gene, "local_force_ratio") == 0) ug->local_force_ratio = value;
}

// Find which slot in the faction's composition genome a given unit type occupies.
static int roster_slot(UnitType type, Faction f) {
    int slot = 0;
    for (int i = 0; i < UNIT_TYPE_COUNT; i++) {
        if (get_faction_of_unit((UnitType)i) == f) {
            if ((UnitType)i == type) return slot;
            slot++;
        }
    }
    return -1;
}

// Set or append a composition count for a faction.
static void set_composition_count(Faction f, UnitType type, int count) {
    ForceComposition* arr;
    int* n;
    if (f == FACTION_GERMAN) { arr = default_german_composition; n = &default_german_composition_count; }
    else                     { arr = default_roman_composition;  n = &default_roman_composition_count;  }

    for (int i = 0; i < *n; i++) {
        if (arr[i].type == type) { arr[i].count = count; return; }
    }
    if (*n < COMPOSITION_MAX_TYPES) {
        arr[*n].type = type;
        arr[*n].count = count;
        (*n)++;
    }
}

// Reset both compositions so a fresh load starts from zero.
static void reset_loaded_compositions(void) {
    default_german_composition_count = 0;
    default_roman_composition_count  = 0;
}

void load_strategy_from_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Error: Could not open strategy file '%s'\n", filename);
        return;
    }

    reset_loaded_compositions();

    FactionStrategy* current_strat = NULL;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        // Strip trailing newline / carriage return
        char* nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        nl = strchr(line, '\r');
        if (nl) *nl = '\0';

        // Skip blanks and comments
        if (line[0] == '\0' || line[0] == '#') continue;

        // Section headers
        if (strstr(line, "[GERMAN]"))   { current_strat = &german_strategy;  continue; }
        if (strstr(line, "[ROMAN]")) { current_strat = &roman_strategy; continue; }
        if (!current_strat) continue;

        // Split "prefix.gene value"
        char key[64];
        float value;
        if (sscanf(line, "%63s %f", key, &value) != 2) continue;

        char* dot = strchr(key, '.');
        if (!dot) continue;
        *dot = '\0';
        const char* prefix = key;
        const char* gene   = dot + 1;

        if (strcmp(prefix, "strategy") == 0) {
            set_strategy_gene(current_strat, gene, value);
        } else if (strcmp(prefix, "composition") == 0) {
            Faction f = (current_strat == &german_strategy) ? FACTION_GERMAN : FACTION_ROMAN;
            size_t len = strlen(gene);

            if (len > 6 && strcmp(gene + len - 6, "_count") == 0) {
                char unit_name[64];
                snprintf(unit_name, sizeof(unit_name), "%.*s", (int)(len - 6), gene);
                UnitType type = parse_unit_type(unit_name);
                if (type != UNIT_TYPE_COUNT) {
                    set_composition_count(f, type, (int)value);
                }
            } else if (len > 6 && strcmp(gene + len - 6, "_share") == 0) {
                char unit_name[64];
                snprintf(unit_name, sizeof(unit_name), "%.*s", (int)(len - 6), gene);
                UnitType type = parse_unit_type(unit_name);
                if (type != UNIT_TYPE_COUNT) {
                    int slot = roster_slot(type, f);
                    if (slot >= 0 && slot < COMPOSITION_MAX_TYPES) {
                        current_strat->composition.share[slot] = value;
                    }
                }
            }
        } else {
            UnitType type = parse_unit_type(prefix);
            if (type != UNIT_TYPE_COUNT) {
                set_unit_gene(type, gene, value);
            }
        }
    }
    fclose(f);
    printf("Loaded genomes from %s\n", filename);
}

// Fitness evaluation
PairFitness evaluate_matchup(const float* german_weights, const float* roman_weights, int num_battles) {
    FactionStrategy old_german = german_strategy;
    FactionStrategy old_roman = roman_strategy;

    set_german_strategy(german_weights);
    set_roman_strategy(roman_weights);

    int german_wins = 0, roman_wins = 0, timeouts = 0;

    for (int i = 0; i < num_battles; i++) {
        Battlefield field;
        initialize_battlefield(&field);

        spawn_german_forces(&field, default_german_composition, default_german_composition_count);
        spawn_roman_forces(&field, default_roman_composition, default_roman_composition_count);

        int winner = run_battle(&field, MAX_TURNS);
        if (winner == 0) german_wins++;
        else if (winner == 1) roman_wins++;
        else if (winner == 2) timeouts++;

        cleanup_battlefield(&field);
    }

    german_strategy = old_german;
    roman_strategy = old_roman;

    float penalty = 2.0f;
    PairFitness out;
    out.german_fitness = (float)german_wins / num_battles - penalty * (float)timeouts / num_battles;
    out.roman_fitness = (float)roman_wins / num_battles - penalty * (float)timeouts / num_battles;
    return out;
}