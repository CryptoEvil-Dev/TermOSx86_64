#include "../include/stdio.hpp"
#include "../include/io.hpp"

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
    fg = fg;
    bg = bg;
}

void Terminal::clear() {
    // Чистим ровно 2000 ячеек (80*25)
    for(int i = 0; i < 80 * 25; i++) {
        vidmem[i] = (uint16_t)' ' | (uint16_t)current_color << 8;
    }
    x = 0;
    y = 0;
    update_cursor();
}

void Terminal::put_char(char c) {
    if (c == '\n') {
        x = 0;
        y++;
    } else if (c == '\b') {
        if (x > 0) {
            x--;
            vidmem[y * 80 + x] = (uint16_t)' ' | (uint16_t)current_color << 8;
        } else if (y > 0) {
            y--;
            x = 79;
            vidmem[y * 80 + x] = (uint16_t)' ' | (uint16_t)current_color << 8;
        }
        update_cursor();
        return;
    } else {
        vidmem[y * 80 + x] = (uint16_t)c | (uint16_t)current_color << 8;
        x++;
    }

    // Перенос строки по достижению края
    if (x >= 80) {
        x = 0;
        y++;
    }

    // Скроллинг при достижении низа экрана (25-я строка)
    if (y >= 25) {
        // Копируем строки 1-24 на места 0-23
        for (int i = 0; i < 24 * 80; i++) {
            vidmem[i] = vidmem[i + 80];
        }
        // Забиваем последнюю строку пробелами
        for (int i = 24 * 80; i < 25 * 80; i++) {
            vidmem[i] = (uint16_t)' ' | (uint16_t)current_color << 8;
        }
        y = 24;
    }
    update_cursor();
}

void Terminal::refresh() {
    // Копируем кусок из screen_buffer в реальный vidmem (0xB8000)
    for(int i = 0; i < 80 * 25; i++) {
        vidmem[i] = screen_buffer[view_offset * 80 + i];
    }
}


void Terminal::scroll() {
    // Копируем 49 строк вверх
    for (int i = 0; i < 49 * 80; i++) {
        vidmem[i] = vidmem[i + 80];
    }
    // Очищаем 50-ю строку
    for (int i = 49 * 80; i < 50 * 80; i++) {
        vidmem[i] = (uint16_t)' ' | (uint16_t)current_color << 8;
    }
}


void Terminal::print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        put_char(str[i]);
    }
}

void Terminal::update_cursor() {
    uint16_t pos = y * 80 + x;
 
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
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
    // set_80x50();
}

vga_color Terminal::get_fg() {
    return fg;
}

vga_color Terminal::get_bg() {
    return bg;
}

void Terminal::set_80x50() {
    // 1. Загружаем шрифт 8x8 (вместо 8x16)
    // Это делается через BIOS прерывание 0x10, но в 64-бит мы сделаем через порты
    outb(0x3D4, 0x09);
    uint8_t max_scanline = inb(0x3D5) & 0xE0;
    outb(0x3D5, max_scanline | 0x07); // Высота символа 8 линий

    outb(0x3D4, 0x12);
    outb(0x3D5, 0x9F); // 400 линий вертикально

    // 2. Обнови пределы в Terminal
    // Теперь y может расти до 49!
}