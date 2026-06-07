// ============================================================================
// PIC - Programmable Interrupt Controller.
//      Библиотека для работы с Intel PIC 8259A контроллером прерываний.
//      Позволяет контроллеру работать в каскажном режиме (Master-Slave) 
// ============================================================================

#pragma once
#include <stdint.h>
#include "io.hpp"

namespace PIC {
    const uint8_t MASTER_COMMAND = 0x20;    // Регистр команд Master (ICW1, OCW2, OCW3)
    const uint8_t MASTER_DATA    = 0x21;    // Регистр данных Master (ICW2-ICW4, маска IMR)
    const uint8_t SLAVE_COMMAND  = 0xA0;    // Регистр команд Slave
    const uint8_t SLAVE_DATA     = 0xA1;    // Регистр данных Slave

    const uint8_t EOI = 0x20;               // End of Interrupt

    // Инициализация (Remap)
    void init(uint8_t offset_master, uint8_t offset_slave);
    
    // Подтверждение прерывания
    void send_eoi(uint8_t irq);

    // Управление масками (разрешить/запретить конкретный IRQ)
    void mask(uint8_t irq);
    void unmask(uint8_t irq);

    // Настройка PIC 8259A в режим Master-Slave в ICW1-ICW4
    void remap();
}
