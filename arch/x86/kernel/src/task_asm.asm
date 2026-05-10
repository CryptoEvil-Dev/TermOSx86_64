[BITS 64]
global do_switch
; void do_switch(uint64_t** old_stack_ptr, uint64_t* new_stack);
; rdi = old_stack_ptr (адрес, куда сохранить текущий rsp)
; rsi = new_stack (значение нового rsp)
do_switch:
    ; Сохраняем текущий rsp по адресу из rdi
    mov [rdi], rsp

    ; Переключаем стек
    mov rsp, rsi

    ; Восстанавливаем регистры в порядке, обратном сохранению:
    ; (на вершине стека лежит r15, затем r14, ..., rax)
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

    ; Выполняем iretq
    iretq