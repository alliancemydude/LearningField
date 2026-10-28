#include "utils.h"
#include "ui.h"
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int turn_count = 0;

const char* get_unit_color(Unit* unit) {
    if (unit == NULL) return COLOR_WHITE;
    return (unit->faction == FACTION_GERMAN) ? COLOR_BLUE : COLOR_RED;
}

int roll(int min, int max) {
    return min + (rand() % (max - min + 1));
}

int get_distance(Unit* a, Unit* b) {
    if (a == NULL || b == NULL) return 9999;
    int dx = a->x - b->x;
    int dy = a->y - b->y;
    return sqrt(dx*dx + dy*dy);
}

bool is_tile_walkable(Battlefield* field, int x, int y, Unit* exclude) {
    if (x < 0 || x >= field->width || y < 0 || y >= field->height) return false;
    Unit* occupant = field->occupancy[y][x];
    return (occupant == NULL || occupant == exclude || occupant->hp <= 0);
}

void clamp_unit_position(Unit* unit, Battlefield* field) {
    if (unit->x < 0) unit->x = 0;
    if (unit->x >= field->width) unit->x = field->width - 1;
    if (unit->y < 0) unit->y = 0;
    if (unit->y >= field->height) unit->y = field->height - 1;
}

void print_screen(Battlefield* field) {
    ui_clear_screen();

    int term_w = ui_width();
    bool show_unit_type = (term_w >= 150);
    int cell_w = show_unit_type ? 3 : 2;

    // Natural width of the grid including the "%2d  " row-label prefix.
    int grid_w = 4 + field->width * cell_w;
    int rule_w = grid_w;

    // Counts for the header
    int german_count = 0, roman_count = 0;
    float german_hp = 0.0f, roman_hp = 0.0f;
    for (int i = 0; i < field->unit_count; i++) {
        Unit* u = field->units[i];
        if (!u || u->hp <= 0) continue;
        if (u->faction == FACTION_GERMAN) {
            german_count++; german_hp += u->hp;
        } else {
            roman_count++;  roman_hp += u->hp;
        }
    }

    // ── Header ─────────────────────────────────────────────────────────────
    ui_hr(UI_RULE_HEAVY, rule_w);
    printf("  %sTURN %d%s   %sGerman %d%s   %sRoman %d%s\n",
           ui_color(UI_HEADER), turn_count, ui_color(UI_RESET),
           ui_color(UI_GERMAN), german_count, ui_color(UI_RESET),
           ui_color(UI_ROMAN),  roman_count,  ui_color(UI_RESET));
    ui_hr(UI_RULE_HEAVY, rule_w);
    printf("\n");

    // ── Column numbers ────────────────────────────────────────────────────
    printf("    ");
    for (int x = 0; x < field->width; x++) {
        if (show_unit_type) printf("%2d ", x % 10);
        else                printf("%d ",  x % 10);
    }
    printf("\n");

    // ── Grid ──────────────────────────────────────────────────────────────
    for (int y = 0; y < field->height; y++) {
        printf("%2d  ", y);
        for (int x = 0; x < field->width; x++) {
            Unit* u = field->occupancy[y][x];
            if (u && u->hp > 0) {
                UiColor col = (u->faction == FACTION_GERMAN) ? UI_GERMAN : UI_ROMAN;
                const char* fc = "^";
                switch (u->facing) {
                    case FACE_EAST:  fc = ">"; break;
                    case FACE_SOUTH: fc = "v"; break;
                    case FACE_WEST:  fc = "<"; break;
                }
                if (show_unit_type) {
                    printf("%s%s%s%s ", ui_color(col), get_unit_symbol(u), fc,
                                       ui_color(UI_RESET));
                } else {
                    printf("%s%s%s ", ui_color(col), fc, ui_color(UI_RESET));
                }
            } else {
                printf(show_unit_type ? " . " : ". ");
            }
        }
        printf("\n");
    }
    printf("\n");

    // ── Legend ────────────────────────────────────────────────────────────
    ui_hr(UI_RULE_LIGHT, rule_w);
    if (show_unit_type) {
        printf("  %sLEGEND%s  B Barbarian  E Elite  J Javelin  C Cavalry  "
               "L Legionare  A Archer  H Cavalry\n",
               ui_color(UI_HINT), ui_color(UI_RESET));
    } else {
        printf("  %sArrows show facing:  ^ north   > east   v south   < west%s\n",
               ui_color(UI_HINT), ui_color(UI_RESET));
    }
    ui_hr(UI_RULE_LIGHT, rule_w);
}

void print_screen_summary(Battlefield* field) {
    int german_count = 0, roman_count = 0;
    float german_hp = 0.0f, roman_hp = 0.0f;
    for (int i = 0; i < field->unit_count; i++) {
        Unit* u = field->units[i];
        if (!u || u->hp <= 0) continue;
        if (u->faction == FACTION_GERMAN) {
            german_count++; german_hp += u->hp;
        } else {
            roman_count++;  roman_hp += u->hp;
        }
    }
    printf("  %sTURN %d%s   %sGerman %d%s (HP %.1f)   %sRoman %d%s (HP %.1f)\n",
           ui_color(UI_HEADER), turn_count, ui_color(UI_RESET),
           ui_color(UI_GERMAN), german_count, ui_color(UI_RESET), german_hp,
           ui_color(UI_ROMAN),  roman_count,  ui_color(UI_RESET), roman_hp);
}

const char* get_unit_symbol(Unit* unit) {
    if (unit == NULL) return "?";
    
    switch (unit->type) {
        case UNIT_BARBARIAN:        return "B";
        case UNIT_ELITE_BARBARIAN:  return "E";
        case UNIT_JAVELIN_THROWER:  return "J";
        case UNIT_GERMAN_CAVALRY:         return "C";
        case UNIT_LEGIONARE:         return "L";
        case UNIT_CENTURION:   return "C";
        case UNIT_ARCHER:           return "A";
        case UNIT_ROMAN_CAVALRY:          return "H";
        
        default: return "???";
    }
}