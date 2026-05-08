[BITS 64]
section .text

; Макрос для аппаратных прерываний (IRQ)
%macro irq_stub 2
global isr%1
isr%1:
    push qword 0    ; Фиктивный код ошибки
    push qword %1   ; Номер прерывания
    
    ; Сохраняем регистры
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi

    mov rdi, rsp    ; Указатель на frame
    extern %2       ; Имя функции из C++
    call %2

    ; Восстанавливаем регистры
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
    add rsp, 16
    iretq
%endmacro

; Генерируем заглушки для наших нужд
irq_stub 0, IRQNULL
irq_stub 32, IRQ0_SystemClock_Handler
irq_stub 33, IRQ1_PS2Keyboard_Handler
irq_stub 14, IRQ14_PageFault
; Сюда можно добавить irq_stub 14, PageFault_Handler для исключений