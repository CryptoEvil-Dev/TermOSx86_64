#include "../include/stdio.hpp"

Terminal term;

Terminal::Terminal() {
    vidmem = (uint16_t*)0xb8000;
    x = 0;
    y = 0;
    set_color(WHITE, BLACK);
    clear();
}

void Terminal::set_color(vga_color fg, vga_color bg) {
    current_color = fg | (bg << 4);
}

void Terminal::clear() {
    for(int i = 0; i < 80 * 25; i++) {
        vidmem[i] = (uint16_t)' ' | (uint16_t)current_color << 8;
    }
    x = 0;
    y = 0;
}

void Terminal::put_char(char c) {
    if (c == '\n') {
        x = 0;
        y++;
    } else {
        const int index = y * 80 + x;
        vidmem[index] = (uint16_t)c | (uint16_t)current_color << 8;
        x++;
    }

    if (x >= 80) {
        x = 0;
        y++;
    }

    if (y >= 25) {
        scroll();
        y = 24;
    }
}

void Terminal::scroll() {
    for (int i = 0; i < 24 * 80; i++) {
        vidmem[i] = vidmem[i + 80];
    }
    for (int i = 24 * 80; i < 25 * 80; i++) {
        vidmem[i] = (uint16_t)' ' | (uint16_t)current_color << 8;
    }
}

void Terminal::print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        put_char(str[i]);
    }
}

void Terminal::update_cursor() {
    // Позже здесь будет код общения с портами 0x3D4/0x3D5
}

void Terminal::print_hex(uint64_t n) {
    const char* hex_chars = "0123456789ABCDEF";
    print("0x");
    for (int i = 15; i >= 0; i--) {
        // ОЧЕНЬ ВАЖНО: сначала сдвиг, потом маска, и всё это в uint64_t
        uint8_t nibble = (uint8_t)((n >> (i * 4)) & 0x0F);
        put_char(hex_chars[nibble]);
    }
}

void Terminal::init() {
    vidmem = (uint16_t*)0xb8000;
    x = 0; y = 0;
    set_color(WHITE, BLACK);
    clear();
}