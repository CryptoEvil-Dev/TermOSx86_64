section .boot
global _start
[BITS 16]
; Зануляем регистры, устанавливаем стек с адреса загрузки, сохраняем номер диска
_start:
    mov [current_disk], dl

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

; Проверяем поддержку EBIOS, для загрузки с флешки с помощью LBA
    mov ah, 0x41
    mov bx, 0x55aa
    mov dl, [current_disk]
    int 0x13
    jc no_ebios
    cmp bx, 0xaa55
    jne no_ebios

; Читаем диск с помощью LBA используя структуру DAP
    mov ah, 0x42
    mov dl, [current_disk]
    mov si, dap
    int 0x13
    jc disk_error

; Открываем A20 Gate
    mov ax, 0x2401
    int 0x10
    jc a20_error
    
; Загружаем GDT
    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x01
    mov cr0, eax

    jmp CODE_SEG:init_pm



; Обработчики ошибок
a20_error:
    mov si, a20_error_str
    call prints
    jmp $

no_ebios:
    mov si, no_ebios_str
    call prints
    jmp $

disk_error:
    mov si, disk_error_str
    call prints
    jmp $

%include 'drivers/IO.asm'


current_disk db 0x00

dap:
    db 0x10         ; Размер пакета (всегда 16 байт для базового LBA)
    db 0x00         ; Зарезервировано (всегда 0)
    dw 0x03a       ; Сколько секторов читать (Обычно до 127)
    dw 0x7e00       ; Смещение буфера (куда грузим)
    dw 0x0000       ; Сегмент буфера
    dq 0x00000001   ; LBA адрес: номер сектора (0 - MBR, 1 - Следующий)

gdt_start:
    dq 0

gdt_code:
    dw 0xffff
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

gdt_data:
    dw 0xffff
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

gdt64_code:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 10011010b
    db 00100000b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Константы для селектора
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
CODE_SEG_64 equ gdt64_code - gdt_start


no_ebios_str db "[BOOT] PANIC: EBIOS IS NOT SUPPORTED!", 0
disk_error_str db "[BOOT] PANIC: DISK READ ERROR!", 0
a20_error_str db "[BOOT] PANIC: A20 GATE IS NOT OPENED!", 0
os_loaded db "[BOOT] SUCCESS", 0

times 510 - ($ - $$) db 0
dw 0xaa55

section .text

[BITS 32]

init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call setup_paging
    call enable_long_mode

    lgdt [gdt_descriptor]
    jmp CODE_SEG_64:init_64

setup_paging:
    mov edi, 0x1000  ; Указатель на начало памяти
    mov cr3, edi     ; 
    xor eax, eax     ; Значение, которым будем заполнять область (нулями)
    mov ecx, 4096    ; Количество повторений
    rep stosd        ; Зануляем память: Repeat Store String DWORD. Записать 4 байта - Смещение на 4 байта - Уменьшить ECX на 1

    mov dword [0x1000], 0x2003
    mov dword [0x2000], 0x3003
    mov dword [0x3000], 0x4003

    mov edi, 0x4000
    mov ebx, 0x00000003
    mov ecx, 512
.set_entry:
    mov [edi], ebx
    add ebx, 0x1000
    add edi, 8
    loop .set_entry
    ret

enable_long_mode:
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax    ; Включаем PAE (Phisical Address Extension)

    mov ecx, 0xC0000080 ; Загружаем адрес регистра EFER (Extended Feature Enable Register)
    rdmsr               ; Читаем состояние регистра MSR (EDX:EAX)
    or eax, 1 << 8      ; Выставляем 8-й бит (LME - Long Mode Enabled) в 1
    wrmsr               ; Сохраняем изменения

    mov eax, cr0        ; Включаем Pagging
    or eax, 1 << 31
    mov cr0, eax        ; Добро пожаловать в Long Mode
    ret


[BITS 64]
init_64:

    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    mov rsp, 0x90000 ; Безопасное место для стека
    sub rsp, 8

    extern _kmain
    call _kmain
    jmp $