#include "syscall.hpp"   // Syscall numbers
#include <stdint.h>

// Обёртки для системных вызовов (inline)
static inline uint64_t syscall0(uint64_t no) {
    uint64_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(no) : "rcx", "r11", "memory");
    return ret;
}

static inline uint64_t syscall1(uint64_t no, uint64_t arg1) {
    uint64_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(no), "D"(arg1) : "rcx", "r11", "memory");
    return ret;
}

// Функция-точка входа (должна быть extern "C", чтобы линкер нашёл)
extern "C" void editor_main() {
    // Приветствие
    syscall1(0, (uint64_t)"--- Simple Text Editor ---\n");
    syscall1(0, (uint64_t)"Enter text (ESC to exit):\n");

    const int MAX_LEN = 4096;
    char buffer[MAX_LEN];
    int pos = 0;

    while (1) {
        char c = (char)syscall0(1);   // syscall getchar (1)

        if (c == 27) break;           // ESC - выход
        else if (c == '\b') {
            if (pos > 0) {
                pos--;
                // Стереть символ на экране: backspace, пробел, backspace
                syscall1(0, (uint64_t)"\b \b");
            }
        } else if (c == '\n') {
            syscall1(0, (uint64_t)"\n");
            buffer[pos++] = '\n';
        } else if (c >= ' ' && c <= '~') {
            // Печатаемый символ
            char str[2] = {c, '\0'};
            syscall1(0, (uint64_t)str);
            buffer[pos++] = c;
        }
        // Игнорируем непечатные символы (кроме спецкодов)
    }

    syscall1(0, (uint64_t)"\nExiting editor...\n");
    syscall0(6);   // syscall exit
}