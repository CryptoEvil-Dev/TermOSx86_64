; ============================================================================
; Библиотека для ввода/вывода текстовой информации, с помощью прерываний BIOS.
; ============================================================================

global clear        ; void clear();
global printc       ; void printc(char bl);
global prints       ; void prints(char* si);

global getc         ; char getc() return char(al);
global gets         ; void gets(char* si);

global next_line    ; void next_line();


next_line:
    push ax
    mov ah, 0x0e
    mov al, 0x0a
    int 0x10
    mov al, 0x0d
    int 0x10
    pop ax
    ret

clear:
    push ax
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    pop ax
    ret

printc:
    push ax
    mov ah, 0x0e
    mov al, bl
    int 0x10
    pop ax
    ret

prints:
    push ax
    push si
    mov ah, 0x0e      ; Телетайпный вывод BIOS
.next_char:
    mov al, [si]      ; Берем символ
    cmp al, 0         ; Это конец строки (нуль-терминатор)?
    je .done          ; Если да, выходим из цикла
    int 0x10          ; Если нет, печатаем
    inc si            ; Переходим к следующему символу
    jmp .next_char    ; Повторяем
.done:
    pop si
    pop ax
    ret

getc:
    push bx
    mov ah, 0x00
    int 0x16
    mov ah, 0x0e
    mov bh, 0
    mov bl, 0x07
    int 0x10
    pop bx
    ret

gets:
    push ax
    push cx
    xor cx, cx
__input_string_loop:
    mov ah, 0
    int 0x16
    cmp al, 0x0d ; Enter
    je __input_string_enter
    cmp al, 0x08    ; Backspace
    je __input_string_backspace

    mov [si], al
    inc si
    inc cx

    mov ah, 0x0e
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    cmp cx, 255
    je __input_string_enter
    jmp __input_string_loop
__input_string_enter:
    mov ah, 0x0e
    mov al, 0x0d
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    mov al, 0xa
    int 0x10

    mov byte [si], 0
    pop cx
    pop ax
    ret
__input_string_backspace:
    cmp cx, 0
    je __input_string_loop
    ; 0x20 - Пробел
    mov ah, 0x0e
    mov al, 0x08
    int 0x10
    mov al, 0x20
    int 0x10
    mov al, 0x08
    int 0x10

    mov byte [si], 0
    dec si
    dec cx
    jmp __input_string_loop

; Скрытые функции

__out_string_next_char:
    mov al, [si]
    cmp al, 0
    call __out_string_if_zero
    int 0x10
    inc si
    jmp __out_string_next_char

__out_string_if_zero:
    ret
