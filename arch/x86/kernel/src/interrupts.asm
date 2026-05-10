[BITS 64]
section .text

; Внешние C++ функции
extern isr0_handler
extern irq0_handler
extern irq1_handler

global isr_generic
extern generic_handler

; Помощник для сохранения контекста
%macro PUSH_ALL 0
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro POP_ALL 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro

; ISR 0: Divide by Zero
global isr0
isr0:
    PUSH_ALL
    mov rdi, rsp    ; Передаем указатель на стек как interrupt_frame
    call isr0_handler
    POP_ALL
    iretq

; IRQ 0: Timer
global irq32
irq32:
    PUSH_ALL
    mov rdi, rsp
    call irq0_handler
    POP_ALL
    iretq

; IRQ 1: Keyboard
global irq33
irq33:
    PUSH_ALL
    mov rdi, rsp
    call irq1_handler
    POP_ALL
    iretq


isr_generic:
    ; Мы не знаем, какой это номер, если не создадим 256 функций.
    ; Но для теста и для защиты системы от зависания сделаем так:
    PUSH_ALL
    mov rdi, rsp    ; 1-й аргумент: interrupt_frame*
    ; Номер прерывания мы пока не передаем (это сложно без макросов для 256 штук)
    call generic_handler
    POP_ALL
    iretq
