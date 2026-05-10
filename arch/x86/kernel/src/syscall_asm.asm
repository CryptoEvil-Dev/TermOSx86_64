[BITS 64]
global isr_syscall
extern syscall_handler_regs

isr_syscall:
    ; Сохраняем все регистры (порядок важен для syscall_handler_regs)
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

    ; Передаём указатель на вершину стека (где лежат регистры)
    mov rdi, rsp
    call syscall_handler_regs

    ; Восстанавливаем регистры в обратном порядке
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

    iretq