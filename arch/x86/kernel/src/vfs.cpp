#include "../include/vfs.hpp"
#include "../include/stdio.hpp"
#include "../include/idt.hpp"

namespace std {
    void vga_write(const char* str) { term.print(str); }
    void vga_write_err(const char* str) {
        term.set_color(RED, BLACK);
        term.print(str);
        term.set_color(WHITE, BLACK);
    }
    char kbd_read() { return kbd_pop(); }

    // Используем статические объекты внутри функций (Singleton-style)
    // Это гарантирует инициализацию при первом вызове
    FILE& get_stdout_obj() { static FILE f = { vga_write, nullptr, "stdout" }; return f; }
    FILE& get_stdin_obj()  { static FILE f = { nullptr, kbd_read, "stdin" }; return f; }
    FILE& get_stderr_obj() { static FILE f = { vga_write_err, nullptr, "stderr" }; return f; }

    // Переопределяем указатели как макросы или через функции в hpp
    FILE* stdout = nullptr;
    FILE* stdin  = nullptr;
    FILE* stderr = nullptr;

    void vfs_init() {
        term.init();
        stdout = &get_stdout_obj();
        stdin  = &get_stdin_obj();
        stderr = &get_stderr_obj();
    }
}