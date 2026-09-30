#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#include "unit.h"
#include "battlefield.h"


// Colors!
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define BOLD          "\033[1m"

extern int turn_count;

// Random number generator:
int roll(int min, int max);

// Distance functions:
int get_distance(Unit* a, Unit* b);

static inline int get_distance_squared(Unit* a, Unit* b) {
    if (a == NULL || b == NULL) return 9999 * 9999;
    int dx = a->x - b->x;
    int dy = a->y - b->y;
    return dx*dx + dy*dy;
}

// Position utilities:
const char* get_squad_color(int squad_id, Faction faction);

bool is_tile_walkable(Battlefield* field, int x, int y, Unit* exclude);

void clamp_unit_position(Unit* unit, Battlefield* field);

const char* get_unit_symbol(Unit* unit);

void print_screen(Battlefield* field);

void print_screen_to_file(Battlefield* field, const char* filename);

void print_screen_summary(Battlefield* field);

#endif