#include "../include/idt.hpp"
#include "../include/pic.hpp"
#include "../include/io.hpp"
#include "../include/task.hpp"
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
static bool ctrl_pressed = false;   // <-- добавлено

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

// Внешние asm-функции
extern "C" void isr_generic();
extern "C" void isr_syscall();
extern "C" void isr_yield();
extern "C" void irq32();
extern "C" void irq33();

// Обработчик клавиатуры (без аргументов, EOI отправляется здесь)
extern "C" void irq1_handler() {
    uint8_t scancode = inb(0x60);

    // Обработка Ctrl
    if (scancode == 0x1D) {
        ctrl_pressed = true;
    } else if (scancode == 0x9D) {
        ctrl_pressed = false;
    }

    // Обработка Shift
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = true;
    } else if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = false;
    }

    // Прокрутка (дополнительные клавиши)
    if (scancode == 0x49) {
        term.view_offset -= 1;
        term.refresh();
    } else if (scancode == 0x51) {
        term.view_offset += 1;
        term.refresh();
    }
    // Обычные клавиши (нажатие)
    else if (!(scancode & 0x80)) {
        // Проверка Ctrl+C
        if (ctrl_pressed && scancode == 0x2E) {   // scancode C
            Tasking::Task* fg = Tasking::get_foreground_task();
            if (fg) {
                Tasking::kill_task(fg);
            }
        } else {
            char c = shift_pressed ? kbd_us_shift[scancode] : kbd_us[scancode];
            if (c > 0) {
                kbd_push(c);
            }
        }
    }

    // Отправляем EOI для IRQ1
    PIC::send_eoi(1);
}

// Универсальный обработчик исключений
extern "C" void generic_handler(interrupt_frame* frame) {
    std::cerr << "\n[IDT] Received unhandled interrupt/exception!" << std::endl;
    // Чтобы не повесить систему, отправляем EOI ведущему и ведомому контроллерам
    PIC::send_eoi(0);
    PIC::send_eoi(8);
}

// Установка дескриптора IDT
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

        // По умолчанию все векторы указывают на isr_generic
        for (int i = 0; i < 256; i++) {
            idt_set_descriptor(i, (void*)isr_generic, 0x8E);
        }

        // Переопределяем нужные векторы
        idt_set_descriptor(32, (void*)irq32, 0x8E);          // IRQ0 (таймер)
        idt_set_descriptor(33, (void*)irq33, 0x8E);          // IRQ1 (клавиатура)
        idt_set_descriptor(0x80, (void*)isr_syscall, 0x8E | 0x60); // системный вызов
        idt_set_descriptor(0x81, (void*)isr_yield, 0x8E);     // yield

        idtr.base = (uint64_t)&idt;
        idtr.limit = sizeof(idt) - 1;
        __asm__ volatile("lidt %0" : : "m"(idtr));

        PIC::unmask(0);
        PIC::unmask(1);
    }
}