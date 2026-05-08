#pragma once
#include <stdint.h>

struct idt_entry_t {
    uint16_t isr_low; uint16_t kernel_cs; uint8_t ist; uint8_t attributes;
    uint16_t isr_mid; uint32_t isr_high; uint32_t reserved;
} __attribute__((packed));

struct idtr_t {
    uint16_t limit; uint64_t base;
} __attribute__((packed));

struct interrupt_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
    uint64_t rip, cs, rflags, rsp, ss; 
};

// Ассемблерные заглушки из interrupts.asm
extern "C" {
    void isr0();  void irq32(); void irq33();
}

// Глобальные состояния
extern uint64_t ticks;

// Интерфейс IDT
void idt_init();
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);

// Интерфейс клавиатуры (для iostream)
void kbd_init();
char kbd_pop(); 
