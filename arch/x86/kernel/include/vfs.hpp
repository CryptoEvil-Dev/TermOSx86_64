#pragma once
#include <stdint.h>

namespace std {
    // Тип функции для чтения/записи из абстрактного файла
    typedef void (*write_func)(const char* str);
    typedef char (*read_func)();

    void vga_write(const char* str);
    void vga_write_err(const char* str);

    struct FILE {
        write_func write;
        read_func read;
        const char* name;
    };

    // Три стандартных потока
    extern FILE* stdin;
    extern FILE* stdout;
    extern FILE* stderr;

    void vfs_init();
}