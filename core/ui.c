#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
  #include <io.h>
#else
  #include <unistd.h>
  #include <sys/ioctl.h>
#endif

// -----------------------------------------------------------------------------
// State
// -----------------------------------------------------------------------------

static int  s_width       = 80;
static int  s_height      = 24;
static bool s_unicode     = false;
static bool s_light_theme = false;
static bool s_initialized = false;

// -----------------------------------------------------------------------------
// Palettes
// -----------------------------------------------------------------------------

static const char* palette_dark[UI_COLOR_COUNT] = {
    [UI_FRAME]   = "\033[90m",
    [UI_HEADER]  = "\033[96m",
    [UI_LABEL]   = "\033[37m",
    [UI_VALUE]   = "\033[97m",
    [UI_HINT]    = "\033[90m",
    [UI_SUCCESS] = "\033[92m",
    [UI_FAILURE] = "\033[31m",
    [UI_WARNING] = "\033[93m",
    [UI_GERMAN]  = "\033[94m",
    [UI_ROMAN]   = "\033[91m",
    [UI_ACCENT]  = "\033[95m",
    [UI_DIM]     = "\033[2m",
    [UI_RESET]   = "\033[0m",
};

static const char* palette_light[UI_COLOR_COUNT] = {
    [UI_FRAME]   = "\033[90m",
    [UI_HEADER]  = "\033[34m",
    [UI_LABEL]   = "\033[30m",
    [UI_VALUE]   = "\033[1;30m",
    [UI_HINT]    = "\033[90m",
    [UI_SUCCESS] = "\033[32m",
    [UI_FAILURE] = "\033[31m",
    [UI_WARNING] = "\033[33m",
    [UI_GERMAN]  = "\033[34m",
    [UI_ROMAN]   = "\033[31m",
    [UI_ACCENT]  = "\033[35m",
    [UI_DIM]     = "\033[2m",
    [UI_RESET]   = "\033[0m",
};

static const char** palette(void) {
    return s_light_theme ? palette_light : palette_dark;
}

// -----------------------------------------------------------------------------
// Detection
// -----------------------------------------------------------------------------

static bool detect_light_theme(void) {
    // $COLORFGBG format is "fg;bg" or "fg;default;bg". Background 7 or 15
    // means a light background.
    const char* v = getenv("COLORFGBG");
    if (!v) return false;
    const char* last = strrchr(v, ';');
    if (!last) return false;
    int bg = atoi(last + 1);
    return (bg == 7 || bg == 15);
}

static bool detect_unicode(void) {
    if (getenv("LF_ASCII_ONLY")) return false;

    const char* lang = getenv("LC_ALL");
    if (!lang || !*lang) lang = getenv("LC_CTYPE");
    if (!lang || !*lang) lang = getenv("LANG");
    if (!lang) return false;

    return strstr(lang, "UTF-8") || strstr(lang, "UTF8") ||
           strstr(lang, "utf-8") || strstr(lang, "utf8");
}

#ifdef _WIN32
static void enable_windows_ansi(void) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode)) {
        SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}

static void detect_size_windows(void) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE) return;
    if (GetConsoleScreenBufferInfo(h, &info)) {
        s_width  = info.srWindow.Right  - info.srWindow.Left + 1;
        s_height = info.srWindow.Bottom - info.srWindow.Top  + 1;
    }
}
#else
static void detect_size_posix(void) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        s_width  = w.ws_col;
        s_height = w.ws_row;
        return;
    }
    const char* cols = getenv("COLUMNS");
    const char* rows = getenv("LINES");
    if (cols && atoi(cols) > 0) s_width  = atoi(cols);
    if (rows && atoi(rows) > 0) s_height = atoi(rows);
}
#endif

void ui_init(void) {
    if (s_initialized) return;
    s_initialized = true;

#ifdef _WIN32
    enable_windows_ansi();
    detect_size_windows();
#else
    detect_size_posix();
#endif

    s_unicode = detect_unicode();

    const char* forced = getenv("LF_THEME");
    if (forced && strcmp(forced, "light") == 0) s_light_theme = true;
    else if (forced && strcmp(forced, "dark") == 0) s_light_theme = false;
    else s_light_theme = detect_light_theme();
}

int ui_width(void) {
    if (!s_initialized) ui_init();
    return s_width < UI_MIN_WIDTH ? UI_MIN_WIDTH : s_width;
}

int ui_height(void) {
    if (!s_initialized) ui_init();
    return s_height;
}

bool ui_supports_unicode(void) {
    if (!s_initialized) ui_init();
    return s_unicode;
}

void ui_set_theme(const char* theme) {
    if (!theme) return;
    if      (strcmp(theme, "light") == 0) s_light_theme = true;
    else if (strcmp(theme, "dark")  == 0) s_light_theme = false;
    else if (strcmp(theme, "auto")  == 0) s_light_theme = detect_light_theme();
}

const char* ui_color(UiColor c) {
    return palette()[c];
}

void ui_flush(void) {
    fflush(stdout);
}

// -----------------------------------------------------------------------------
// UTF-8 aware visible-length helpers
// -----------------------------------------------------------------------------

static int vis_len(const char* s) {
    int n = 0;
    while (*s) {
        unsigned char c = (unsigned char)*s;
        if (c == 0x1B) {
            while (*s && *s != 'm') s++;
            if (*s) s++;
            continue;
        }
        if (c < 0x80)               { s += 1; }
        else if ((c & 0xE0) == 0xC0) { s += 2; }
        else if ((c & 0xF0) == 0xE0) { s += 3; }
        else if ((c & 0xF8) == 0xF0) { s += 4; }
        else                          { s += 1; }
        n++;
    }
    return n;
}

static void put_vis_n(const char* s, int max_vis) {
    int seen = 0;
    while (*s && seen < max_vis) {
        unsigned char c = (unsigned char)*s;
        if (c == 0x1B) {
            while (*s && *s != 'm') putchar(*s++);
            if (*s) putchar(*s++);
        } else if (c < 0x80)               { putchar(*s++); seen++; }
        else if ((c & 0xE0) == 0xC0)       { putchar(*s++); putchar(*s++); seen++; }
        else if ((c & 0xF0) == 0xE0)       { putchar(*s++); putchar(*s++); putchar(*s++); seen++; }
        else if ((c & 0xF8) == 0xF0)       { putchar(*s++); putchar(*s++); putchar(*s++); putchar(*s++); seen++; }
        else                                { putchar(*s++); seen++; }
    }
}

// -----------------------------------------------------------------------------
// Box drawing
// -----------------------------------------------------------------------------

typedef struct {
    const char* tl; const char* tr;
    const char* bl; const char* br;
    const char* h;  const char* v;
    const char* lt; const char* rt;
} BoxChars;

static const BoxChars box_unicode = {
    "\u2554", "\u2557",
    "\u255A", "\u255D",
    "\u2550", "\u2551",
    "\u2560", "\u2563",
};

static const BoxChars box_ascii = {
    "+", "+", "+", "+", "=", "|", "+", "+",
};

static const BoxChars* box(void) {
    return s_unicode ? &box_unicode : &box_ascii;
}

static void repeat(const char* s, int n) {
    for (int i = 0; i < n; i++) fputs(s, stdout);
}

void ui_box_open(int width, const char* title) {
    if (width < 4) return;
    const BoxChars* b = box();
    const char* f = ui_color(UI_FRAME);
    const char* r = ui_color(UI_RESET);

    printf("%s%s", f, b->tl);
    repeat(b->h, width - 2);
    printf("%s%s\n", b->tr, r);

    if (title && *title) {
        int tlen = vis_len(title);
        int pad  = (width - 2 - tlen - 2) / 2;
        if (pad < 1) pad = 1;

        printf("%s%s%s ", f, b->v, r);
        repeat(" ", pad - 1);
        printf("%s%s%s", ui_color(UI_HEADER), title, r);
        int after = width - 2 - 1 - tlen - (pad - 1);
        if (after < 0) after = 0;
        repeat(" ", after);
        printf("%s%s%s\n", f, b->v, r);

        ui_box_sep(width);
    }
}

void ui_box_sep(int width) {
    if (width < 4) return;
    const BoxChars* b = box();
    printf("%s%s", ui_color(UI_FRAME), b->lt);
    repeat(b->h, width - 2);
    printf("%s%s\n", b->rt, ui_color(UI_RESET));
}

void ui_box_close(int width) {
    if (width < 4) return;
    const BoxChars* b = box();
    printf("%s%s", ui_color(UI_FRAME), b->bl);
    repeat(b->h, width - 2);
    printf("%s%s\n", b->br, ui_color(UI_RESET));
}

void ui_box_line(int width, const char* text) {
    ui_box_line_color(width, text, UI_LABEL);
}

void ui_box_line_color(int width, const char* text, UiColor c) {
    if (width < 4) return;
    const BoxChars* b = box();
    const char* f = ui_color(UI_FRAME);
    const char* r = ui_color(UI_RESET);

    int interior = width - 2;
    int tlen = text ? vis_len(text) : 0;
    if (tlen > interior - 2) tlen = interior - 2;

    printf("%s%s%s ", f, b->v, r);
    if (text) {
        printf("%s", ui_color(c));
        put_vis_n(text, tlen);
        printf("%s", r);
    }
    int pad = interior - 1 - tlen;
    if (pad > 0) repeat(" ", pad);
    printf("%s%s%s\n", f, b->v, r);
}

// -----------------------------------------------------------------------------
// Text helpers
// -----------------------------------------------------------------------------

void ui_hr(UiRule rule, int width) {
    const char* ch = (rule == UI_RULE_HEAVY)
        ? (s_unicode ? "\u2550" : "=")
        : (s_unicode ? "\u2500" : "-");
    printf("%s", ui_color(UI_FRAME));
    repeat(ch, width);
    printf("%s\n", ui_color(UI_RESET));
}

void ui_kv(const char* label, const char* value, int width) {
    int indent = 4;
    int llen = label  ? vis_len(label)  : 0;
    int vlen = value  ? vis_len(value)  : 0;
    int avail = width - indent - llen - vlen - 2;

    printf("    %s%s%s ", ui_color(UI_LABEL), label ? label : "", ui_color(UI_RESET));
    if (avail >= 3) {
        printf("%s", ui_color(UI_HINT));
        repeat(s_unicode ? "\u00B7" : ".", avail);
        printf("%s ", ui_color(UI_RESET));
    } else {
        putchar(' ');
    }
    printf("%s%s%s\n", ui_color(UI_VALUE), value ? value : "", ui_color(UI_RESET));
}

void ui_kv_int(const char* label, int value, int width) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", value);
    ui_kv(label, buf, width);
}

void ui_kv_float(const char* label, float value, int precision, int width) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*f", precision, value);
    ui_kv(label, buf, width);
}

void ui_center(const char* text, int width) {
    ui_center_color(text, width, UI_LABEL);
}

void ui_center_color(const char* text, int width, UiColor c) {
    int tlen = text ? vis_len(text) : 0;
    int pad  = (width - tlen) / 2;
    if (pad < 0) pad = 0;
    repeat(" ", pad);
    if (text) {
        printf("%s%s%s\n", ui_color(c), text, ui_color(UI_RESET));
    } else {
        putchar('\n');
    }
}

void ui_progress(float fraction, int bar_width, UiColor c) {
    if (fraction < 0.0f) fraction = 0.0f;
    if (fraction > 1.0f) fraction = 1.0f;
    int filled = (int)(fraction * bar_width + 0.5f);

    const char* full  = s_unicode ? "\u2588" : "#";
    const char* empty = s_unicode ? "\u2591" : "-";

    printf("%s", ui_color(c));
    repeat(full, filled);
    printf("%s", ui_color(UI_HINT));
    repeat(empty, bar_width - filled);
    printf("%s", ui_color(UI_RESET));
}

// -----------------------------------------------------------------------------
// Screen control
// -----------------------------------------------------------------------------

void ui_clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void ui_cursor_up_clear(int n) {
    for (int i = 0; i < n; i++) {
        printf("\033[1A\033[2K");
    }
    fflush(stdout);
}