#include "utils.h"
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int turn_count = 0;

const char* get_unit_color(Unit* unit) {
    if (unit == NULL) return COLOR_WHITE;
    return (unit->faction == FACTION_REPUBLIC) ? COLOR_BLUE : COLOR_RED;
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
    printf("Legend: S=Swordsman, E=Elite, H=Horseman, B=Bowman P = Spearman,\n");
    printf("Blue = Republic, Red = Separatist, . open\n\n");

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
                int soldier_count = unit_on_tile->units_count;
                if (soldier_count < 0) soldier_count = 0;
                if (soldier_count > 9) soldier_count = 9;
                const char* color = get_unit_color(unit_on_tile);
                char buf[3];
                snprintf(buf, sizeof(buf), "%s%d", symbol, soldier_count);
                printf("%s%2s%s ", color, buf, COLOR_RESET);
            } else if (is_dead) {
                printf(" $ ");
            } else {
                printf(" . ");   // empty tile
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
                const char* color = get_unit_color(unit_on_tile);
                fprintf(f, "%s %s %s", color, symbol, COLOR_RESET);
            } else if (is_dead) {
                // Unit is dead
                fprintf(f, " $ ");
            } else {
                fprintf(f, " . ");
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
        case UNIT_SWORDSMAN:        return "S";
        case UNIT_ELITE_SWORDSMAN:  return "E";
        case UNIT_HORSEMAN:         return "H";
        case UNIT_LONGBOWMAN:       return "B";
        case UNIT_SPEARMAN:         return "P";
        case UNIT_ELITE_SPEARMAN:   return "E";
        case UNIT_SHORTBOWMAN:      return "B";
        case UNIT_CAMELMAN:         return "H";
        
        default: return "???";
    }
}