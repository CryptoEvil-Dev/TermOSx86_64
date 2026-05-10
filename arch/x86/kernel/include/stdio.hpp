#pragma once

#include <stdint.h>

enum vga_color {
    BLACK = 0, BLUE = 1, GREEN = 2, CYAN = 3, RED = 4, MAGENTA = 5, BROWN = 6, LIGHT_GREY = 7,
    DARK_GREY = 8, LIGHT_BLUE = 9, LIGHT_GREEN = 10, LIGHT_CYAN = 11, LIGHT_RED = 12, 
    LIGHT_MAGENTA = 13, LIGHT_BROWN = 14, WHITE = 15,
};

class Terminal {
public:
    Terminal();
    void init();
    void put_char(char c);
    void print(const char* str);
    void print_hex(uint64_t n);
    void clear();
    void refresh();
    void set_color(vga_color fg, vga_color bg);
    // void update_cursor();

    vga_color get_bg();
    vga_color get_fg();

    int view_offset = 0;

private:
    void set_80x50();
    void scroll();
    void update_cursor();

    uint16_t* vidmem;
    int x, y;
    int max_x = 50;
    int max_y = 80;
    uint16_t screen_buffer[80 * 200];
    vga_color fg, bg;
    uint8_t current_color;
};

extern Terminal term;