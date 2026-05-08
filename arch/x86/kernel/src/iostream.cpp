#include "../include/iostream.hpp"

namespace std {
    ostream cout;
    istream cin;

    // --- Вывод (ostream) ---

    void ostream::print_uint(uint64_t n, uint8_t base) {
        if (n == 0) { term.put_char('0'); return; }
        
        char buf[64];
        int i = 0;
        const char* chars = "0123456789ABCDEF";

        while (n > 0) {
            buf[i++] = chars[n % base];
            n /= base;
        }
        while (--i >= 0) term.put_char(buf[i]);
    }

    ostream& ostream::operator<<(const char* str) {
        term.print(str);
        return *this;
    }

    ostream& ostream::operator<<(char c) {
        term.put_char(c);
        return *this;
    }

    ostream& ostream::operator<<(uint64_t n) {
        if (current_fmt == hex) term.print_hex(n);
        else if (current_fmt == bin) print_uint(n, 2);
        else print_uint(n, 10);
        return *this;
    }

    ostream& ostream::operator<<(int n) {
        if (n < 0) { term.put_char('-'); n = -n; }
        return *this << (uint64_t)n;
    }

    // --- Ввод (istream) ---

    istream& istream::operator>>(char& c) {
        while (true) {
            char ch = kbd_pop();
            if (ch != 0) {
                c = ch;
                return *this;
            }
            __asm__ volatile("hlt");
        }
    }

    istream& istream::operator>>(char* buffer) {
        int i = 0;
        while (true) {
            char c;
            *this >> c; // Используем перегрузку для char

            if (c == '\n') {
                term.put_char('\n');
                buffer[i] = '\0';
                break;
            } else if (c == '\b') {
                if (i > 0) {
                    i--;
                    term.put_char('\b'); // Нужно будет научить терминал стирать символ
                }
            } else {
                term.put_char(c);
                buffer[i++] = c;
            }
        }
        return *this;
    }
}
