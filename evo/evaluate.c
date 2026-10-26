#include "evaluate.h"
#include "battlefield.h"
#include "spawn.h"
#include "strategy.h"
#include "composition.h"
#include "unit_genome.h"
#include "unit_utils.h"
#include "unit.h"
#include "queries.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void apply_genes(const float* german_weights, const float* roman_weights) {
    set_german_strategy(german_weights);
    set_roman_strategy(roman_weights);

    int german_idx = STRATEGY_GENE_COUNT;
    int roman_idx = STRATEGY_GENE_COUNT;
    for (unsigned int i = 0; i < UNIT_TYPE_COUNT; i++) {
        UnitType type = (UnitType)i;
        Faction f = get_faction_of_unit(type);
        if (f == FACTION_UNKNOWN) continue;

        if (f == FACTION_GERMAN) {
            unit_genomes[i].unit_aggression = german_weights[german_idx + 0];
            unit_genomes[i].preferred_range = german_weights[german_idx + 1];
            unit_genomes[i].spacing_x       = german_weights[german_idx + 2];
            unit_genomes[i].spacing_y       = german_weights[german_idx + 3];
            unit_genomes[i].local_force_ratio    = german_weights[german_idx + 4];
            german_idx += UNIT_TYPE_GENE_COUNT;
        } else {
            unit_genomes[i].unit_aggression = roman_weights[roman_idx + 0];
            unit_genomes[i].preferred_range = roman_weights[roman_idx + 1];
            unit_genomes[i].spacing_x       = roman_weights[roman_idx + 2];
            unit_genomes[i].spacing_y       = roman_weights[roman_idx + 3];
            unit_genomes[i].local_force_ratio    = roman_weights[roman_idx + 4];
            roman_idx += UNIT_TYPE_GENE_COUNT;
        }
    }

    // Read composition genes
    german_strategy.composition.count = 4;
    roman_strategy.composition.count  = 4;
    for (int i = 0; i < 4; i++) {
        german_strategy.composition.share[i] = german_weights[german_idx + i];
        roman_strategy.composition.share[i]  = roman_weights[roman_idx + i];
    }
}

static void print_comp_line(ForceComposition* comp, int n) {
    int total = 0;
    for (int i = 0; i < n; i++) {
        total += comp[i].count;
        if (comp[i].count > 0) {
            printf("%d%s ", comp[i].count, short_unit_name(comp[i].type));
        }
    }
    printf("(%d units)", total);
}

void print_generation_diagnostics(const float* german_weights, const float* roman_weights, int num_battles, const char* label)
{
    FactionStrategy old_german = german_strategy;
    FactionStrategy old_roman  = roman_strategy;
    UnitGenome old_genomes[UNIT_TYPE_COUNT];
    memcpy(old_genomes, unit_genomes, sizeof(UnitGenome) * UNIT_TYPE_COUNT);

    apply_genes(german_weights, roman_weights);

    ForceComposition german_comp[COMPOSITION_MAX_TYPES];
    ForceComposition roman_comp[COMPOSITION_MAX_TYPES];
    int german_comp_n = build_composition(&german_strategy.composition, FACTION_GERMAN, german_comp);
    int roman_comp_n  = build_composition(&roman_strategy.composition,  FACTION_ROMAN,  roman_comp);

    int german_wins = 0, roman_wins = 0, timeouts = 0;
    long total_german_dmg = 0, total_roman_dmg = 0;
    long total_turns = 0;
    long total_german_surv = 0, total_roman_surv = 0;

    for (int i = 0; i < num_battles; i++) {
        Battlefield field;
        initialize_battlefield(&field);
        spawn_german_forces(&field, german_comp, german_comp_n);
        spawn_roman_forces(&field, roman_comp, roman_comp_n);

        int winner = run_battle(&field, MAX_TURNS);
        if (winner == 0) german_wins++;
        else if (winner == 1) roman_wins++;
        else timeouts++;

        total_german_dmg += battle_stats.damage_taken_roman;
        total_roman_dmg  += battle_stats.damage_taken_german;
        total_turns      += turn_count;

        // Count survivors per faction
        int g_surv = 0, r_surv = 0;
        for (int u = 0; u < field.unit_count; u++) {
            Unit* unit = field.units[u];
            if (!unit || unit->hp <= 0) continue;
            if (unit->faction == FACTION_GERMAN) g_surv++;
            else                                 r_surv++;
        }
        total_german_surv += g_surv;
        total_roman_surv  += r_surv;

        cleanup_battlefield(&field);
    }

    german_strategy = old_german;
    roman_strategy = old_roman;
    memcpy(unit_genomes, old_genomes, sizeof(UnitGenome) * UNIT_TYPE_COUNT);

    long total_dmg = total_german_dmg + total_roman_dmg;
    float g_dmg_pct = (total_dmg > 0) ? 100.0f * total_german_dmg / total_dmg : 0.0f;
    float r_dmg_pct = (total_dmg > 0) ? 100.0f * total_roman_dmg  / total_dmg : 0.0f;

    printf("  [%s]\n", label);
    printf("    G: "); print_comp_line(german_comp, german_comp_n); printf("\n");
    printf("    R: "); print_comp_line(roman_comp,  roman_comp_n);  printf("\n");
    printf("    G %2dW / R %2dW / %2dTO  |  Dmg %3.0f/%3.0f  |  "
           "Turns %.1f  |  Surv %.1f/%.1f\n",
           german_wins, roman_wins, timeouts,
           g_dmg_pct, r_dmg_pct,
           (double)total_turns / num_battles,
           (double)total_german_surv / num_battles,
           (double)total_roman_surv / num_battles);
}

PairFitness evaluate_all(const float* german_weights, const float* roman_weights, int num_battles) {
    // Stash current state so we can restore after the batch.
    FactionStrategy old_german = german_strategy;
    FactionStrategy old_roman = roman_strategy;
    UnitGenome old_genomes[UNIT_TYPE_COUNT];
    memcpy(old_genomes, unit_genomes, sizeof(UnitGenome) * UNIT_TYPE_COUNT);

    // Apply strategy genes
    apply_genes(german_weights, roman_weights);

    // Build force compositions from the genes.
    ForceComposition german_comp[COMPOSITION_MAX_TYPES];
    ForceComposition roman_comp[COMPOSITION_MAX_TYPES];
    int german_comp_n = build_composition(&german_strategy.composition, FACTION_GERMAN, german_comp);
    int roman_comp_n  = build_composition(&roman_strategy.composition,  FACTION_ROMAN,  roman_comp);

    // Weights (TUNE THESE IF NEEDED)
    const float W_WIN           = 1.0f;
    const float W_DAMAGE_SHARE  = 0.3f;
    const float W_TURN_PENALTY  = 0.15f;
    const float W_TIMEOUT       = 0.5f;
    const float W_NO_ENGAGE     = 0.05f;

    float german_score = 0.0f;
    float roman_score = 0.0f;
    int   timeouts  = 0;

    for (int i = 0; i < num_battles; i++) {
        Battlefield field;
        initialize_battlefield(&field);
        spawn_german_forces(&field, german_comp, german_comp_n);
        spawn_roman_forces(&field, roman_comp, roman_comp_n);

        int winner = run_battle(&field, MAX_TURNS);

        // Outcome
        if (winner == 0) {
            german_score += W_WIN;
        } else if (winner == 1) {
            roman_score += W_WIN;
        } else {
            timeouts++;
        }

        // Damage share
        int german_dmg = battle_stats.damage_taken_roman;
        int roman_dmg = battle_stats.damage_taken_german;
        int total_dmg = german_dmg + roman_dmg;

        if (total_dmg > 0) {
            german_score += W_DAMAGE_SHARE * ((float)german_dmg / total_dmg);
            roman_score += W_DAMAGE_SHARE * ((float)roman_dmg / total_dmg);
        } else {
            // Nobody engaged at all
            german_score -= W_NO_ENGAGE;
            roman_score -= W_NO_ENGAGE;
        }

        // Turn efficiency penalty (both sides)
        float turn_ratio = (float)turn_count / MAX_TURNS;
        german_score -= W_TURN_PENALTY * turn_ratio;
        roman_score -= W_TURN_PENALTY * turn_ratio;

        cleanup_battlefield(&field);
    }

    // Average over battles
    german_score /= num_battles;
    roman_score /= num_battles;

    // Timeout penalty applied to both sides
    float timeout_rate = (float)timeouts / num_battles;
    german_score -= W_TIMEOUT * timeout_rate;
    roman_score -= W_TIMEOUT * timeout_rate;

    // Restore
    german_strategy = old_german;
    roman_strategy = old_roman;
    memcpy(unit_genomes, old_genomes, sizeof(UnitGenome) * UNIT_TYPE_COUNT);

    PairFitness out;
    out.german_fitness = german_score;
    out.roman_fitness = roman_score;
    return out;
}