#include "../include/idt.hpp"
#include "../include/pic.hpp"
#include "../include/io.hpp"
#include "stdio.hpp"
#include "iostream.hpp"

// Выделение памяти
idt_entry_t idt[256];
idtr_t idtr;
uint64_t ticks = 0;

// Кольцевой буфер клавиатуры
static char kbd_buffer[256];
static uint32_t kbd_head = 0;
static uint32_t kbd_tail = 0;

static bool shift_pressed = false;

static const char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

static const char kbd_us_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

extern "C" void isr_generic();

// --- Обработчики (ISR/IRQ) ---

extern "C" void irq0_handler(interrupt_frame* frame) {
    ticks++;
    PIC::send_eoi(0);
}

extern "C" void irq1_handler(interrupt_frame* frame) {
    uint8_t scancode = inb(0x60);

    // Обработка нажатия/отпускания Shift
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = true;
    } else if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = false;
    }

    if(scancode == 0x49) {
        term.view_offset -= 1;
        term.refresh();
    }
    if(scancode == 0x51) {
        term.view_offset += 1;
        term.refresh();
    }
    // Обработка обычных клавиш
    else if (!(scancode & 0x80)) { 
        // ВЫБИРАЕМ ТАБЛИЦУ
        char c = shift_pressed ? kbd_us_shift[scancode] : kbd_us[scancode];
        
        if (c > 0) kbd_push(c);
    }

    PIC::send_eoi(1);
}

extern "C" void generic_handler(interrupt_frame* frame) {
    // Так как мы не знаем точный номер без таблицы переходов, 
    // просто пишем общее предупреждение
    std::cerr << "\n[IDT] Received unhandled interrupt/exception!" << std::endl;
    
    // Если это аппаратное прерывание (IRQ), нужно отправить EOI, 
    // иначе другие прерывания (клавиатура/таймер) перестанут работать.
    // На всякий случай отправляем в оба контроллера.
    PIC::send_eoi(0); // Master
    PIC::send_eoi(8); // Slave
}


// --- Системные функции ---

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    uint64_t addr = (uint64_t)isr;
    idt[vector].isr_low    = (uint16_t)(addr & 0xFFFF);
    idt[vector].kernel_cs  = 0x18;
    idt[vector].ist        = 0;
    idt[vector].attributes = flags;
    idt[vector].isr_mid    = (uint16_t)((addr >> 16) & 0xFFFF);
    idt[vector].isr_high   = (uint32_t)((addr >> 32) & 0xFFFFFFFF);
    idt[vector].reserved   = 0;
}

void kbd_push(char c) {
    uint32_t next = (kbd_head + 1) % 256;
    if (next != kbd_tail) {
        kbd_buffer[kbd_head] = c;
        kbd_head = next;
    }
}

char kbd_pop() {
    if (kbd_head == kbd_tail) return 0;
    char c = kbd_buffer[kbd_tail];
    kbd_tail = (kbd_tail + 1) % 256;
    return c;
}


namespace Interrupts {
    void init() {
        PIC::init(0x20, 0x28);

        // Паникуем по умолчанию
        for(int i = 0; i < 256; i++) {
            idt_set_descriptor(i, (void*)isr_generic, 0x8E);
        }

        // Регистрируем конкретные IRQ
        idt_set_descriptor(0,  (void*)isr0,  0x8E); 
        idt_set_descriptor(32, (void*)irq32, 0x8E); 
        idt_set_descriptor(33, (void*)irq33, 0x8E); 

        idtr.base = (uint64_t)&idt;
        idtr.limit = sizeof(idt) - 1;
        __asm__ volatile("lidt %0" : : "m"(idtr));

        PIC::unmask(0);
        PIC::unmask(1);
    }
}

// Паникуем исключительно в std::err
extern "C" void isr0_handler(interrupt_frame* frame) {
    std::cerr << "\n--- KERNEL PANIC ---" << std::endl;
    std::cerr << "Exception: " << std::dec << (int)frame->rip << " (Divide by Zero)" << std::endl;
    std::cerr << "RSP: " << std::hex << (void*)frame->rsp << std::endl;
    while(1) __asm__ volatile("hlt");
}