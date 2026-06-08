#include "include/iostream.hpp"
#include "include/stdio.hpp"
#include "include/string.hpp"
#include "include/idt.hpp"
#include "include/pic.hpp"
#include "include/io.hpp"
#include "include/pmm.hpp"
#include "include/ata.hpp"
#include "include/fat32.hpp"

#include "include/tsh.hpp"

extern "C" void _kmain() {
    std::vfs_init();
    Interrupts::init();
    __asm__ volatile("sti");

    Memory::pmm_init(128 * 1024 * 1024); // 128 Мб

    // Запираем первые 2 Мб (Там BIOS, стек, ядро и таблицы страниц)
    Memory::pmm_lock_pages((void*)0, 512);

    std::cout << "TermOS 64-bit Kernel Started!" << std::endl;
    std::cout << "PMM Initialized. Free memory: " << std::dec << Memory::get_free_memory() / 1024 / 1024 << " MB" << std::endl;

    Storage::disk_mgr.init();
    
    Storage::fat32_init(1);

    // Чистим мусор в контроллере клавиатуры
    // while(inb(0x64) & 1) inb(0x60);

    std::cout << "TermOS Shell v0.1" << std::endl;
    std::cout << "Kernel base: " << std::hex << (void*)0x7E00 << std::dec << std::endl;


    Shell::run();
    int raw[128] = {0};
}
