#include "../include/idt.hpp"
#include "../include/io.hpp"
#include "stdio.hpp"

// ОПРЕДЕЛЕНИЯ (ВЫДЕЛЕНИЕ памяти)
idt_entry_t idt[256] = { {0x1234, 0x18, 0, 0x8E, 0, 0, 0} };
idtr_t idtr;

// Список всех заглушек для компилятора
extern "C" {
    void isr0();
    void isr32();
    void isr33();
}

uint64_t ticks = 0;

// IRQ 0 - System Clock
extern "C" void IRQ0_SystemClock_Handler(interrupt_frame* frame) {
    ticks++;
    // Например выводить "Tick" каждые 100 тиков (Примерно раз в секунду)
    if(ticks % 100 == 0) {
        term.print("Tick");
    }

    outb(0x20, 0x20); // EOI
}

// IRQ 1 - PS2 Keyboard
extern "C" void IRQ1_PS2Keyboard_Handler(interrupt_frame* frame) {
    uint8_t scancode = inb(0x60);

    if(!(scancode & 0x80)) {
        term.print("KBD Scnacode: ");
        term.print_hex(scancode);
        term.print("\n");
    }

    outb(0x20, 0x20);
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    uint64_t addr = (uint64_t)isr; // Берем чистый 64-битный адрес
    term.print_hex(addr); 

    idt[vector].isr_low    = (uint16_t)(addr & 0xFFFF);
    idt[vector].kernel_cs  = 0x18;
    idt[vector].ist        = 0;
    idt[vector].attributes = flags;
    idt[vector].isr_mid    = (uint16_t)((addr >> 16) & 0xFFFF);
    idt[vector].isr_high   = (uint32_t)((addr >> 32) & 0xFFFFFFFF);
    idt[vector].reserved   = 0;
}

// Эту функцию мы вызовем в _kmain
void idt_init() {
    // Явно обнулим всю таблицу перед заполнением, чтобы IST был точно 0
    for(int i = 0; i < 256; i++) {
        idt_set_descriptor(i, (void*)isr0, 0x8E); // Забьем всё заглушкой
    }
    
    // А теперь ставим нужный нам breakpoint
    idt_set_descriptor(32, (void*)isr32, 0x8E);
    idt_set_descriptor(33, (void*)isr33, 0x8E);

    idtr.base = (uint64_t)&idt;
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * 256 - 1;

    __asm__ volatile("lidt %0" : : "m"(idtr));
}

void timer_init(uint8_t hz) {
    uint32_t divisor = 1193182 / hz;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)(divisor >> 8) & 0xFF);
}

extern "C" void exception_handler(interrupt_frame* frame) {
    if (frame->interrupt_number == 32 || frame->interrupt_number == 0x20) {
        outb(0x20, 0x20); // Отправляем EOI Master PIC
        return; // ПРОСТО ВЫХОДИМ, чтобы ядро работало дальше
    }

    // 2. Обработка клавиатуры
    if (frame->interrupt_number == 33 || frame->interrupt_number == 0x21) {
        uint8_t scancode = inb(0x60);
        term.print("KBD: ");
        term.print_hex(scancode);
        term.print("\n");
        
        outb(0x20, 0x20); // EOI
        return;
    }

    term.set_color(WHITE, RED);
    term.clear();
    term.print("!!!! KERNEL PANIC !!!!\n");

    term.print("Exception: ");
    term.print_hex(frame->interrupt_number);
    term.print("\nError Code: ");
    term.print_hex(frame->error_code);
    term.print("\nRIP: ");
    term.print_hex(frame->rip);
    
    while(1) { __asm__ volatile("hlt"); }
}