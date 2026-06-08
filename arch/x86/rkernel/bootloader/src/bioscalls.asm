global prints

prints:
    push ax
    push si
    mov ah, 0x0e      ; Телетайпный вывод BIOS
.next_char:
    lodsb             ; AL = [SI], и сразу SI = SI + 1 (размер: всего 1 байт!)
    test al, al       ; Проверяем, равен ли AL нулю (быстрее и компактнее, чем cmp al, 0)
    jz .done          ; Если ноль — выходим
    int 0x10          ; Печатаем символ
    jmp .next_char    ; Повторяем
.done:
    pop si
    pop ax
    ret