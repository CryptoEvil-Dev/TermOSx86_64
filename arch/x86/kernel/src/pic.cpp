#include "../include/pic.hpp"

void PIC::init(uint8_t offset_master, uint8_t offset_slave) {
    outb(MASTER_COMMAND, 0x11); // ICW1: Инициализация
    outb(SLAVE_COMMAND,  0x11);

    outb(MASTER_DATA, offset_master); // ICW2: Смещение векторов
    outb(SLAVE_DATA,  offset_slave);

    outb(MASTER_DATA, 0x04); // ICW3: Master видит Slave на IRQ2
    outb(SLAVE_DATA,  0x02); // Slave знает свой ID

    outb(MASTER_DATA, 0x01); // ICW4: Режим 8086
    outb(SLAVE_DATA,  0x01);

    // По умолчанию маскируем всё
    outb(MASTER_DATA, 0xFF);
    outb(SLAVE_DATA,  0xFF);
}

void PIC::send_eoi(uint8_t irq) {
    if (irq >= 8) outb(SLAVE_COMMAND, EOI);
    outb(MASTER_COMMAND, EOI);
}

void PIC::unmask(uint8_t irq) {
    uint16_t port = (irq < 8) ? MASTER_DATA : SLAVE_DATA;
    uint8_t value = inb(port) & ~(1 << (irq % 8));
    outb(port, value);
}

void PIC::remap() {
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