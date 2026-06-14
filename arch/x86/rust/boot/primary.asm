extern _rust_end_marker ; Линковщик сам передаст сюда адрес конца 32-битного загрузчика

[BITS 16]
global _start

section .text
_start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    mov [BOOT_INFO + 11], dl ; Сохраняем номер загрузочного диска

    ; 1. Сбор карты памяти (E820)
    ; Адрес буфера для карты памяти: BOOT_INFO + 8
    mov di, BOOT_INFO + 14
    xor ebx, ebx
    xor bp, bp

.e820_loop:
    mov edx, 0x534D4150         ; Магическое число 'SMAP'
    mov eax, 0xE820
    mov ecx, 24                 ; Просим BIOS записать 24 байта (размер одной записи)
    int 0x15
    jc .e820_done               ; Если флаг переноса (CF) установлен - сбор окончен

    cmp eax, 0x534D4150         ; BIOS должен вернуть 'SMAP' в EAX
    jne .e820_done

    add di, 24                  ; Сдвигаем буфер на одну запись вперёд
    add bp, 1                   ; Увеличиваем счётчик на один (add работает быстрее чем inc)
    test ebx, ebx               ; Если ebx стал 0, то записей больше нет
    jnz .e820_loop

.e820_done:
    mov [BOOT_INFO + 12], bp

    mov ax, 0x4F01
    mov cx, 0x411E
    mov di, 0x2000
    int 0x10
    cmp ax, 0x004F
    jne .no_graphics

    mov eax, [0x2000 + 40]
    mov [BOOT_INFO], eax

    mov ax, [0x2000 + 18]
    mov [BOOT_INFO + 4], ax

    mov ax, [0x2000 + 20]
    mov [BOOT_INFO + 6], ax

    mov al, [0x2000 + 25]
    mov [BOOT_INFO + 8], al

    mov ax, [0x2000 + 16]
    mov [BOOT_INFO + 9], ax

    mov ax, 0x4F02
    mov bx, 0x411E
    int 0x10


; Тут мы идём дальше. Не забывая положить адресу структуры например в EBX
.no_graphics:
    ; Динамический расчёт и чтение кода Rust через DAP
    mov eax, _rust_end_marker   ; Загружаем адрес конца загрузчика
    sub eax, 0x7E00             ; Отнимаем адрес начала, получаем размер загрузчика
    shr eax, 9                  ; Сдвигаем регистр вправо на 9 бит = деление на 512
    add eax, 1                  ; Округляем в большую сторону (+1 сектор про запас)

    ; Записываем результат в DAP
    mov [dap.sectors_to_read], ax

    ; Восстанавливаем номер диска в структуру DAP
    mov dl, [BOOT_INFO + 11]
    mov [dap.drive_num], dl

    ; Вызываем расширенное чтение BIOS
    mov ah, 0x42
    mov si, dap         ; DS:SI указывает на структуру DAP
    int 0x13
    jc .disk_error      ; Ошибка чтения диска -> Зависаем

; 4. Проверка магической сигнатуры
    mov ebx, [_rust_end_marker]
    cmp ebx, 0x0BADB002             ; Проверяем, на месте ли число
    jne  .signature_error           ; Если магия не совпала -> Зависаем


; 5. Переход в 32 бита
; Включаем линию A20
    in al, 0x92
    or al, 0x02
    out 0x92, al

    cli
    lgdt [gdt_descriptor]

; Включем Protected Mode
    mov eax, cr0
    or eax, 0x01
    mov cr0, eax

    jmp 0x08:init_pm

.signature_error:
.disk_error:
    jmp $


section .text
align 4
dap:
    db 0x10
    db 0x00
.sectors_to_read:
    dw 0x0000
.buffer_offset:
    dw 0x7E00
.buffer_segment:
    dw 0x0000
.lba_start_low:
    dd 0x00000001
.lba_start_high:
    dd 0x00000000
.drive_num:
    db 0x00

align 4
gdt_start:
    dq 0x0000000000000000
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A
    db 0xCF
    db 0x00
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92
    db 0xCF
    db 0x00
gdt_end:

align 4
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start


BOOT_INFO equ 0x0500    ; Адрес по котороуму будем хранить структуру карты памяти и параметры графического режима
DISK_PARAMS equ 0x1000   ; Адрес буфера под параметры загрузочного диска

times 510 - ($ - $$) db 0
dw 0xAA55

section .bootpm
[BITS 32]

global init_pm
extern _kboot

init_pm:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000
    mov ebp, esp

    and esp, 0xFFFFFFF0

    call _kboot

.hang:
    jmp $