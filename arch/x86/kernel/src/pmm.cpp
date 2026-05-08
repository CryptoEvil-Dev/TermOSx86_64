#include "../include/pmm.hpp"
#include "../include/iostream.hpp"

namespace Memory {
    uint8_t* bitmap;
    uint64_t max_pages;
    uint64_t free_memory;

    void set_bit(uint64_t page) {
        bitmap[page / 8] |= (1 << (page % 8));
    }
    void clear_bit(uint64_t page) {
        bitmap[page / 8] &= ~(1 << (page % 8));
    }
    bool get_bit(uint64_t page) {
        return bitmap[page / 8] & (1 << (page % 8));
    }

    void pmm_init(uint64_t total_memory) {
        max_pages = total_memory / PAGE_SIZE;
        bitmap = (uint8_t*)0x100000; // Карта на 1 Мб (безопасное место)
        free_memory = total_memory;

        // Изначально всё занято
        for(uint64_t i = 0; i < max_pages / 8; i++) bitmap[i] = 0xFF;
    }

    void pmm_lock_pages(void* addr, uint64_t count) {
        uint64_t start_page = (uint64_t)addr / PAGE_SIZE;
        for(uint64_t i = 0; i < count; i++) {
            if(!get_bit(start_page + i)) {
                set_bit(start_page + i);
                free_memory -= PAGE_SIZE;
            }
        }
    }

    void* alloc_page() {
        for(uint64_t i = 0; i < max_pages; i++) {
            if(!get_bit(i)) {
                set_bit(i);
                free_memory -= PAGE_SIZE;
                return (void*)(i * PAGE_SIZE);
            }
        }
        return nullptr; // Out of Memory
    }

    void free_page(void* addr) {
        uint64_t page = (uint64_t)addr / PAGE_SIZE;
        if(get_bit(page)) {
            clear_bit(page);
            free_memory += PAGE_SIZE;
        }
    }

    uint64_t get_free_memory() { return free_memory; }
}