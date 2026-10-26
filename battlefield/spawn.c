#include <stdbool.h>
#include "spawn.h"
#include "battlefield.h"
#include "unit.h"
#include "strategy.h"
#include "utils.h"

// Forward declaration
static bool try_relocate(Battlefield* field, Unit* u);

void place_unit(Battlefield* field, Unit* u, int new_x, int new_y) {
    field->occupancy[u->y][u->x] = NULL;
    u->x = new_x;
    u->y = new_y;
    field->occupancy[u->y][u->x] = u;
}

void resolve_overlaps(Battlefield* field) {
    const int max_passes = 20;
    for (int pass = 0; pass < max_passes; pass++) {
        bool any_moved = false;
        for (int i = 0; i < field->unit_count; i++) {
            Unit* a = field->units[i];
            if (a == NULL || a->hp <= 0) continue;
            for (int j = i + 1; j < field->unit_count; j++) {
                Unit* b = field->units[j];
                if (b == NULL || b->hp <= 0) continue;
                if (a->x != b->x || a->y != b->y) continue;

                if (try_relocate(field, b)) {
                    any_moved = true;
                }
            }
        }
        if (!any_moved) return; // stable
    }
}

static bool try_relocate(Battlefield* field, Unit* u) {
    // 1. Try the 8 adjacent tiles first
    static const int dx[] = {0, 0, -1, 1, -1, -1, 1, 1};
    static const int dy[] = {-1, 1, 0, 0, -1, 1, -1, 1};
    for (int d = 0; d < 8; d++) {
        int nx = u->x + dx[d];
        int ny = u->y + dy[d];
        if (is_tile_walkable(field, nx, ny, u)) {
            field->occupancy[u->y][u->x] = NULL;
            u->x = nx;
            u->y = ny;
            field->occupancy[u->y][u->x] = u;
            return true;
        }
    }
    // 2. Expand outward, perimeter of increasing radii
    for (int radius = 2; radius <= 4; radius++) {
        for (int d = -radius; d <= radius; d++) {
            // Top and bottom edges of the ring
            for (int sign = -1; sign <= 1; sign += 2) {
                int nx = u->x + d;
                int ny = u->y + sign * radius;
                if (is_tile_walkable(field, nx, ny, u)) {
                    u->x = nx; u->y = ny; return true;
                }
                nx = u->x + sign * radius;
                ny = u->y + d;
                if (is_tile_walkable(field, nx, ny, u)) {
                    u->x = nx; u->y = ny; return true;
                }
            }
        }
    }
    // 3. Random fallback
    for (int attempt = 0; attempt < 30; attempt++) {
        int nx = roll(0, field->width - 1);
        int ny = roll(0, field->height - 1);
        if (is_tile_walkable(field, nx, ny, u)) {
            u->x = nx;
            u->y = ny;
            return true;
        }
    }
    return false; // map is full
}

void spawn_german_forces(Battlefield* field, ForceComposition forces[], int force_count) {
    int id = field->unit_count + 1;
    int y_min = field->height * 3 / 4;
    int y_max = field->height - 1;
    int x_min = 0;
    int x_max = field->width - 1;

    FactionStrategy* strat = &german_strategy;

    int total_units = 0;
    for (int f = 0; f < force_count; f++) {
        total_units += forces[f].count;
    }
    if (total_units == 0) return;

    float center_x = (float)field->width / 2.0f;
    if (center_x < 0.0f) center_x = 0.0f;
    if (center_x >= field->width) center_x = field->width - 1.0f;

    float horizontal_spread = strat->deployment.deploy_horizontal_spread / 10.0f;
    float vertical_offset = strat->deployment.deploy_vertical_offset / 10.0f;
    int y_center = y_max - (int)((y_max - y_min) * vertical_offset);
    if (y_center < y_min) y_center = y_min;
    if (y_center > y_max) y_center = y_max;

    int unit_index = 0;
    for (int f = 0; f < force_count; f++) {
        for (int i = 0; i < forces[f].count; i++) {
            if (field->unit_count >= MAX_UNITS) break;
            float fraction = (total_units == 1) ? 0.5f : (float)unit_index / (total_units - 1);
            float x_offset = (fraction - 0.5f) * 2.0f * horizontal_spread * (float)field->width * 0.4f;
            int desired_x = (int)(center_x + x_offset);
            float y_offset = (fraction - 0.5f) * (y_max - y_min) * 0.3f;
            int desired_y = y_center + (int)y_offset;

            if (desired_x < 0) desired_x = 0;
            if (desired_x >= field->width) desired_x = field->width - 1;
            if (desired_y < y_min) desired_y = y_min;
            if (desired_y > y_max) desired_y = y_max;

            int x = desired_x, y = desired_y;
            bool placed = false;

            if (is_tile_walkable(field, x, y, NULL)) {
                placed = true;
            }

            if (!placed) {
                for (int radius = 1; radius <= 3 && !placed; radius++) {
                    for (int dy = -radius; dy <= radius && !placed; dy++) {
                        for (int dx = -radius; dx <= radius && !placed; dx++) {
                            if (dx == 0 && dy == 0) continue;
                            int tx = desired_x + dx;
                            int ty = desired_y + dy;
                            if (tx < 0 || tx >= field->width || ty < y_min || ty > y_max) continue;
                            if (is_tile_walkable(field, tx, ty, NULL)) {
                                x = tx; y = ty; placed = true;
                            }
                        }
                    }
                }
            }

            if (!placed) {
                for (int attempt = 0; attempt < 100; attempt++) {
                    x = roll(x_min, x_max);
                    y = roll(y_min, y_max);
                    if (is_tile_walkable(field, x, y, NULL)) {
                        placed = true; break;
                    }
                }
            }

            if (!placed) {
                x = (unit_index * 3) % field->width;
                y = y_min + (unit_index % (y_max - y_min + 1));
                for (int yy = y_min; yy <= y_max && !placed; yy++) {
                    for (int xx = 0; xx < field->width && !placed; xx++) {
                        if (is_tile_walkable(field, xx, yy, NULL)) {
                            x = xx; y = yy; placed = true;
                        }
                    }
                }
            }

            Unit* unit = create_unit(forces[f].type, id++, x, y);
            if (unit) {
                field->units[field->unit_count++] = unit;
                field->occupancy[unit->y][unit->x] = unit;
            }
            unit_index++;
        }
    }
    resolve_overlaps(field);
}

void spawn_roman_forces(Battlefield* field, ForceComposition forces[], int force_count) {
    int id = field->unit_count + 1;
    int y_min = 0;
    int y_max = field->height / 4 - 1;
    int x_min = 0;
    int x_max = field->width - 1;

    FactionStrategy* strat = &roman_strategy;

    int total_units = 0;
    for (int f = 0; f < force_count; f++) {
        total_units += forces[f].count;
    }
    if (total_units == 0) return;

    float center_x = (float)field->width / 2.0f;
    if (center_x < 0.0f) center_x = 0.0f;
    if (center_x >= field->width) center_x = field->width - 1.0f;

    float horizontal_spread = strat->deployment.deploy_horizontal_spread / 10.0f;
    float vertical_offset = strat->deployment.deploy_vertical_offset / 10.0f;
    int y_center = y_min + (int)((y_max - y_min) * vertical_offset);
    if (y_center < y_min) y_center = y_min;
    if (y_center > y_max) y_center = y_max;

    int unit_index = 0;
    for (int f = 0; f < force_count; f++) {
        for (int i = 0; i < forces[f].count; i++) {
            if (field->unit_count >= MAX_UNITS) break;
            float fraction = (total_units == 1) ? 0.5f : (float)unit_index / (total_units - 1);
            float x_offset = (fraction - 0.5f) * 2.0f * horizontal_spread * (float)field->width * 0.4f;
            int desired_x = (int)(center_x + x_offset);
            float y_offset = (fraction - 0.5f) * (y_max - y_min) * 0.3f;
            int desired_y = y_center + (int)y_offset;

            if (desired_x < 0) desired_x = 0;
            if (desired_x >= field->width) desired_x = field->width - 1;
            if (desired_y < y_min) desired_y = y_min;
            if (desired_y > y_max) desired_y = y_max;

            int x = desired_x, y = desired_y;
            bool placed = false;

            if (is_tile_walkable(field, x, y, NULL)) {
                placed = true;
            }

            if (!placed) {
                for (int radius = 1; radius <= 3 && !placed; radius++) {
                    for (int dy = -radius; dy <= radius && !placed; dy++) {
                        for (int dx = -radius; dx <= radius && !placed; dx++) {
                            if (dx == 0 && dy == 0) continue;
                            int tx = desired_x + dx;
                            int ty = desired_y + dy;
                            if (tx < 0 || tx >= field->width || ty < y_min || ty > y_max) continue;
                            if (is_tile_walkable(field, tx, ty, NULL)) {
                                x = tx; y = ty; placed = true;
                            }
                        }
                    }
                }
            }

            if (!placed) {
                for (int attempt = 0; attempt < 100; attempt++) {
                    x = roll(x_min, x_max);
                    y = roll(y_min, y_max);
                    if (is_tile_walkable(field, x, y, NULL)) {
                        placed = true; break;
                    }
                }
            }

            if (!placed) {
                x = (unit_index * 3) % field->width;
                y = y_min + (unit_index % (y_max - y_min + 1));
                for (int yy = y_min; yy <= y_max && !placed; yy++) {
                    for (int xx = 0; xx < field->width && !placed; xx++) {
                        if (is_tile_walkable(field, xx, yy, NULL)) {
                            x = xx; y = yy; placed = true;
                        }
                    }
                }
            }

            Unit* unit = create_unit(forces[f].type, id++, x, y);
            if (unit) {
                field->units[field->unit_count++] = unit;
                field->occupancy[unit->y][unit->x] = unit;
            }
            unit_index++;
        }
    }
    resolve_overlaps(field);
}