#ifndef UI_H
#define UI_H

#include <stdbool.h>

// -----------------------------------------------------------------------------
// Terminal information
// -----------------------------------------------------------------------------

// Detect terminal size, unicode capability, and theme. Safe to call multiple
// times; only the first call does work.
void ui_init(void);

// Current terminal width in columns. Never returns less than UI_MIN_WIDTH.
int ui_width(void);

// Current terminal height in rows.
int ui_height(void);

// True if the terminal supports UTF-8 box-drawing characters.
bool ui_supports_unicode(void);

// -----------------------------------------------------------------------------
// Semantic colors
// -----------------------------------------------------------------------------

typedef enum {
    UI_FRAME,     // box borders, structural characters
    UI_HEADER,    // section titles, banner text
    UI_LABEL,     // field labels
    UI_VALUE,     // field values
    UI_HINT,      // dim text, key hints
    UI_SUCCESS,   // wins, positive outcomes
    UI_FAILURE,   // losses, negative outcomes
    UI_WARNING,   // timeouts, edge cases
    UI_GERMAN,    // German faction
    UI_ROMAN,     // Roman faction
    UI_ACCENT,    // emphasis on secondary info
    UI_DIM,       // dimmed text (attribute, not color)
    UI_RESET,
    UI_COLOR_COUNT
} UiColor;

// Returns the ANSI escape sequence for a semantic color.
const char* ui_color(UiColor c);

// Force a theme: "dark", "light", or "auto". Overrides $COLORFGBG detection.
// Can be called any time; takes effect on the next write.
void ui_set_theme(const char* theme);

// -----------------------------------------------------------------------------
// Boxes
// -----------------------------------------------------------------------------

#define UI_MIN_WIDTH   40
#define UI_PANEL_WIDTH 76    // total width including borders

void ui_box_open(int width, const char* title);
void ui_box_sep(int width);
void ui_box_close(int width);
void ui_box_line(int width, const char* text);
void ui_box_line_color(int width, const char* text, UiColor c);

// -----------------------------------------------------------------------------
// Text helpers
// -----------------------------------------------------------------------------

typedef enum { UI_RULE_LIGHT, UI_RULE_HEAVY } UiRule;

// Horizontal rule of the given width.
void ui_hr(UiRule rule, int width);

// Label + dotted leader + right-aligned value, total width matches `width`.
void ui_kv(const char* label, const char* value, int width);
void ui_kv_int(const char* label, int value, int width);
void ui_kv_float(const char* label, float value, int precision, int width);

// Centered text within `width` columns.
void ui_center(const char* text, int width);
void ui_center_color(const char* text, int width, UiColor c);

// A horizontal progress bar of `bar_width` visible cells, filled to `fraction`.
void ui_progress(float fraction, int bar_width, UiColor c);

// -----------------------------------------------------------------------------
// Screen control
// -----------------------------------------------------------------------------

// Clear the whole screen and move cursor to home.
void ui_clear_screen(void);

// Move cursor up N lines, clearing each as we go. Leaves cursor at the start
// of the now-blank block, ready for a redraw.
void ui_cursor_up_clear(int n);

// Flush stdout. Call after drawing a live panel.
void ui_flush(void);

#endif