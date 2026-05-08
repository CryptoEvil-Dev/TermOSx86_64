#include "../include/iostream.hpp"

namespace std {
    ostream cout;
    ostream cerr;
    istream cin;

    // --- Вывод (ostream) ---


    void write_to_stream(ostream* s, const char* str) {
    FILE* target = (s == &cerr) ? stderr : stdout;
    if (target && target->write) {
        target->write(str);
    }
}


    void ostream::print_uint(uint64_t n, uint8_t base) {
        if (n == 0) { *this << '0'; return; }
        
        char buf[65]; // 64 бита в двоичной системе + null
        int i = 0;
        const char* chars = "0123456789ABCDEF";

        while (n > 0) {
            buf[i++] = chars[n % base];
            n /= base;
        }
        
        // Переворачиваем и выводим через перегрузку char
        while (--i >= 0) {
            *this << buf[i];
        }
    }

    ostream& ostream::operator<<(const char* str) {
        write_to_stream(this, str);
        return *this;
    }

    ostream& ostream::operator<<(char c) {
        char buf[2] = {c, 0};
        write_to_stream(this, buf);
        return *this;
    }

    ostream& ostream::operator<<(uint64_t n) {
        if (current_fmt == hex) {
            *this << "0x";
            print_uint(n, 16);
        }
        else if (current_fmt == bin) print_uint(n, 2);
        else print_uint(n, 10);
        return *this;
    }

    ostream& ostream::operator<<(int n) {
        if (n < 0) { *this << '-'; n = -n; }
        return *this << (uint64_t)n;
    }

    // --- Ввод (istream) ---

    istream& istream::operator>>(char& c) {
        while (true) {
            if (stdin && stdin->read) {
                char ch = stdin->read();
                if(ch != 0) {
                    c = ch;
                    return *this;
                }
                __asm__ volatile("hlt");
            }
        }
    }

    istream& istream::operator>>(char* buffer) {
        int i = 0;
        while (true) {
            char c;
            *this >> c;

            if (c == '\n') {
                // *this >> c; // Это просто "эхо" в stdout через оператор ниже
                cout << '\n';
                buffer[i] = '\0';
                break;
            } else if (c == '\b') {
                if (i > 0) {
                    i--;
                    cout << '\b';
                }
            } else {
                cout << c;
                buffer[i++] = c;
            }
        }
        return *this;
    }
}
