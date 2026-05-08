#include "include/stdio.hpp"
#include "include/idt.hpp"
#include "include/io.hpp"

void pic_remap() {
    // ICW 1: Начало инициализации
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // ICW 2: Сдвигаем векторы (Master на 32, Slave на 40)
    outb(0x21, 0x20);   // 0x20 = 32
    outb(0xA1, 0x28);   // 0x28 = 40

    // ICW 3: Соединяем контроллеры
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // ICW 4: Режим 8086
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Маскируем всё кроме клавиатуры (IRQ1)
    outb(0x21, 0xFD);
    outb(0xA1, 0xFF);

    outb(0x21, 0xfc); // 11111100b - разрешены IRQ0 и IRQ1
}

extern "C" void _kmain() {
    term.init(); // Оживляем глобальный терминал
    term.print("TermOS 64-bit Kernel Started!\n");

    pic_remap();
    idt_init();

    // Чистим мусор в контроллере клавиатуры
    while(inb(0x64) & 1) inb(0x60);

    __asm__ volatile("sti"); // Разрешаем прерывания

    term.print("Interrupts enabled. Type something...\n");

    // ВАЖНО: Вечный цикл, чтобы ядро продолжало жить!
    while(1) {
        __asm__ volatile("hlt"); // Спим до следующего прерывания
    }
}
