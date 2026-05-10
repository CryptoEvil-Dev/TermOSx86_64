#pragma once
#include <stdint.h>

using size_t = uint64_t;

namespace std {
    static inline int strcmp(const char* s1, const char* s2) {
        while (*s1 && (*s1 == *s2)) {
            s1++; s2++;
        }
        return *(const unsigned char*)s1 - *(const unsigned char*)s2;
    }

    static inline size_t strlen(const char* str) {
        size_t len = 0;
        while (str[len]) len++;
        return len;
    }

    static inline int find_char(const char* str, char c) {
        for (int i = 0; str[i]; i++) {
            if (str[i] == c) return i;
        }
        return -1;
    }

    static inline void strcpy(char* dest, const char* src) {
        while ((*dest++ = *src++));
    }

    static inline void* memset(void* dest, int ch, uint64_t count) {
        uint8_t* p = (uint8_t*)dest;
        while (count--) {
            *p++ = (uint8_t)ch;
        }
        return dest;
    }
}
