#include "utils.h"
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int turn_count = 0;

static const char* SQUAD_COLORS[] = {
    COLOR_RED,     // squad 0
    COLOR_GREEN,   // squad 1
    COLOR_YELLOW,  // squad 2
    COLOR_BLUE,    // squad 3
    COLOR_MAGENTA, // squad 4
    COLOR_CYAN,    // squad 5
    COLOR_WHITE,   // squad 6
    COLOR_RED,     // squad 7 (reuse)
    COLOR_GREEN,   // squad 8
    COLOR_YELLOW   // squad 9
};
#define SQUAD_COLOR_COUNT (sizeof(SQUAD_COLORS)/sizeof(SQUAD_COLORS[0]))

const char* get_squad_color(int squad_id) {
    if (squad_id < 0 || squad_id >= SQUAD_COLOR_COUNT) return COLOR_RESET;
    return SQUAD_COLORS[squad_id];
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
    // Check bounds
    if (x < 0 || x >= field->width || y < 0 || y >= field->height)
        return false;
    // Check occupancy by other units
    for (int i = 0; i < field->unit_count; i++) {
        Unit* u = field->units[i];
        if (u == NULL || u == exclude || u->hp <= 0) continue;
        if (u->x == x && u->y == y)
            return false;
    }
    return true;
}

void clamp_unit_position(Unit* unit, Battlefield* field) {
    if (unit->x < 0) unit->x = 0;
    if (unit->x >= field->width) unit->x = field->width - 1;
    if (unit->y < 0) unit->y = 0;
    if (unit->y >= field->height) unit->y = field->height - 1;
}

void print_screen(Battlefield* field) {
    // Clear screen and move cursor home
    printf("\033[2J\033[H");

    // turn count and faction summary
    int rep_count = 0, sep_count = 0;
    float rep_hp = 0.0f, sep_hp = 0.0f;
    for (int i = 0; i < field->unit_count; i++) {
        Unit* u = field->units[i];
        if (u->hp <= 0) continue;
        if (u->faction == FACTION_REPUBLIC) {
            rep_count++;
            rep_hp += u->hp;
        } else {
            sep_count++;
            sep_hp += u->hp;
        }
    }
    printf("Turn %d | Republic: %d units (HP: %.1f) | Separatist: %d units (HP: %.1f)\n",
           turn_count, rep_count, rep_hp, sep_count, sep_hp);
    printf("Legend: T=Clone Trooper, E=Elite, O=Officer, S=Sniper, L=Light MG, H=Heavy MG, "
           "R=Rocket, W=Swamp Speeder, B=BARC/Saber/STAP/AAT, P=Spider Droid\n");
    printf("Green = Republic, Red = Separatist, . open\n\n");

    // Print column headers
    printf("   ");
    for (int x = 0; x < field->width; x++) {
        printf("%2d ", x % 10);
    }
    printf("\n");

    for (int y = 0; y < field->height; y++) {
        printf("%2d ", y);
        for (int x = 0; x < field->width; x++) {
            Unit* unit_on_tile = NULL;
            bool is_dead = false;

            for (int i = 0; i < field->unit_count; i++) {
                Unit* u = field->units[i];
                if (u == NULL) continue;
                if (u->x == x && u->y == y) {
                    if (u->hp <= 0) {
                        is_dead = true;
                    } else {
                        unit_on_tile = u;
                    }
                    break;
                }
            }

            if (unit_on_tile != NULL) {
                const char* symbol = get_unit_symbol(unit_on_tile);
                int sid = unit_on_tile->squad_id;
                int soldier_count = unit_on_tile->units_count;
                if (soldier_count < 0) soldier_count = 0;
                if (soldier_count > 9) soldier_count = 9;
                const char* color = get_squad_color(sid % 10);
                char buf[3];
                snprintf(buf, sizeof(buf), "%s%d", symbol, soldier_count);
                printf("%s%2s%s ", color, buf, COLOR_RESET);
            } else if (is_dead) {
                printf(" $ ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

void print_screen_to_file(Battlefield* field, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Error: Could not open %s for writing.\n", filename);
        return;
    }

    // Print header with map dimensions and turn number
    fprintf(f, "Battlefield: %d x %d  |  Turn: %d\n", field->width, field->height, turn_count);
    fprintf(f, "Legend: [R]epublic (green), [S]eparatist (red), . open\n\n");

    // Print column headers (only if width is manageable)
    fprintf(f, "   ");
    for (int x = 0; x < field->width; x++) {
        fprintf(f, "%2d ", x);
    }
    fprintf(f, "\n");

    for (int y = 0; y < field->height; y++) {
        fprintf(f, "%2d ", y);
        for (int x = 0; x < field->width; x++) {
            Unit* unit_on_tile = NULL;
            bool is_dead = false;

            for (int i = 0; i < field->unit_count; i++) {
                Unit* u = field->units[i];
                if (u == NULL) continue;
                if (u->x == x && u->y == y) {
                    if (u->hp <= 0) {
                        is_dead = true;
                    } else {
                        unit_on_tile = u;
                    }
                    break;
                }
            }

            if (unit_on_tile != NULL) {
                const char* symbol = get_unit_symbol(unit_on_tile);
                const char* color = (unit_on_tile->faction == FACTION_REPUBLIC) ? COLOR_GREEN : COLOR_RED;
                fprintf(f, "%s %s %s", color, symbol, COLOR_RESET);
            } else {
                fprintf(f, " $ ");
            }
        }
        fprintf(f, "\n");
    }

    fprintf(f, "\n");
    fclose(f);
}

void print_screen_summary(Battlefield* field) {
    int rep_count = 0, sep_count = 0;
    float rep_hp = 0.0f, sep_hp = 0.0f;
    for (int i = 0; i < field->unit_count; i++) {
        Unit* u = field->units[i];
        if (u->hp <= 0) continue;
        if (u->faction == FACTION_REPUBLIC) {
            rep_count++;
            rep_hp += u->hp;
        } else {
            sep_count++;
            sep_hp += u->hp;
        }
    }
    printf("Turn %d | Republic: %d units (HP: %.1f) | Separatist: %d units (HP: %.1f)\n",
           turn_count, rep_count, rep_hp, sep_count, sep_hp);
}

const char* get_unit_symbol(Unit* unit) {
    if (unit == NULL) return "?";
    
    switch (unit->type) {
        // Republic (lowercase)
        case UNIT_CLONE_TROOPER:       return "T";
        case UNIT_ELITE_CLONE:         return "E";
        case UNIT_CLONE_SNIPER:        return "S";
        case UNIT_CLONE_OFFICER:       return "O";
        case UNIT_LIGHT_CLONE_GUNNER:  return "L";
        case UNIT_MEDIUM_CLONE_GUNNER: return "M";
        case UNIT_HEAVY_CLONE_GUNNER:  return "H";
        case UNIT_CLONE_ROCKET:        return "R";
        case UNIT_SWAMP_SPEEDER:       return "W";
        case UNIT_BARC_SPEEDER:        return "B";
        case UNIT_SABER_TANK:          return "B";
        
        // Separatist (uppercase)
        case UNIT_BATTLE_DROID:        return "B";
        case UNIT_SUPER_BATTLE_DROID:  return "S";
        case UNIT_DROID_SNIPER:        return "S";
        case UNIT_DROID_OFFICER:       return "O";
        case UNIT_MEDIUM_DROID_GUNNER: return "M";
        case UNIT_HEAVY_DROID_GUNNER:  return "H";
        case UNIT_DROID_ROCKET:        return "R";
        case UNIT_SPIDER_DROID:        return "P";
        case UNIT_STAP:                return "T";
        case UNIT_AAT:                 return "T";
        
        default: return "???";
    }
}