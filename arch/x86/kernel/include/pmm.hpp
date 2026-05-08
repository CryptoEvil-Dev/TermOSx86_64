#pragma once
#include <stdint.h>

namespace Memory {
    // 1 страница = 4096 байт
    const uint64_t PAGE_SIZE = 4096;

    void pmm_init(uint64_t total_memory);
    void pmm_lock_pages(void* addr, uint64_t count);
    void* alloc_page();
    void free_page(void* addr);

    uint64_t get_free_memory();
}