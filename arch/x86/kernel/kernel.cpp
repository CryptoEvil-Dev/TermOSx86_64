#include "include/iostream.hpp"
#include "include/stdio.hpp"
#include "include/idt.hpp"
#include "include/pic.hpp"
#include "include/io.hpp"

extern "C" void _kmain() {
    term.init(); // Оживляем глобальный терминал
    PIC::remap();
    idt_init();
    __asm__ volatile("sti");

    std::cout << "TermOS 64-bit Kernel Started!" << std::endl;


    // Чистим мусор в контроллере клавиатуры
    // while(inb(0x64) & 1) inb(0x60);

    std::cout << "TermOS Shell v0.1" << std::endl;
    std::cout << "Kernel base: " << std::hex << (void*)0x7E00 << std::dec << std::endl;

    while(1) {
        std::cout << "> ";
        char cmd[64];
        std::cin >> cmd;
        
        std::cout << "You typed: " << cmd << std::endl;
        std::cout << "Ticks since boot: " << ticks << std::endl;
    }
}
