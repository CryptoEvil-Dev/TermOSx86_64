// ============================================================================
// iostream - Абстракция потоков ввода/вывода.
//      Библиотека поддерживает ввод istream и вывод через ostream.
// ============================================================================
#pragma once
#include "stdio.hpp"
#include "vfs.hpp"
#include "idt.hpp"

namespace std {
    enum fmt { dec, hex, bin };

    class ostream {
    private:
        fmt current_fmt = dec;
        void print_uint(uint64_t n, uint8_t base);

    public:
        // Манипуляторы
        ostream& operator<<(fmt f) { current_fmt = f; return *this; }
        
        // Строки и символы
        ostream& operator<<(const char* str);
        ostream& operator<<(char c);
        
        // Числа
        ostream& operator<<(int n);
        ostream& operator<<(uint64_t n);
        ostream& operator<<(uint32_t n) { return *this << (uint64_t)n; }

        // Указатели (всегда в HEX)
        ostream& operator<<(void* ptr) {
            fmt temp = current_fmt;
            current_fmt = hex;
            *this << (uint64_t)ptr;
            current_fmt = temp;
            return *this;
        }
    };

    class istream {
    public:
        // Чтение строки (до Enter)
        istream& operator>>(char* buffer);
        // Чтение одного символа
        istream& operator>>(char& c);
    };

    extern ostream cout;
    extern istream cin;
    extern ostream cerr;
    const char endl = '\n';

    void write_to_stream(ostream* s, const char* str);
}