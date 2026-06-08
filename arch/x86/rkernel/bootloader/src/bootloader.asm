section .boot
global _start
[BITS 16]

_start:
    mov [current_disk], dl
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    mov ah, 0x41
    mov bx, 0x55aa
    int 0x13
    jc no_ebios
    cmp bx, 0xaa55
    jne no_ebios

    mov ah, 0x42
    mov si, dap
    int 0x13
    jc disk_error

    mov ax, 0x2401
    int 0x10
    jc a20_error

    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x01
    mov cr0, eax

    jmp CODE_SEG:init_pm

a20_error:
    mov si, a20_error_str
    call prints
    jmp $

disk_error:
    mov si, disk_error_str
    call prints
    jmp $



%include 'bioscalls.asm'

dap:
    db 0x10
    db 0x00
    dw 0x03a
    dw 0x7e00
    dw 0x0000
    dq 0x00000001

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

gdt_end




current_disk: db 0x00

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
CODE64_SEG equ gdt64_code - gdt_start

no_ebios_str db "[BOOT] PANIC: EBIOS IS NOT SUPPORTED!", 0
disk_error_str db "[BOOT] PANIC: DISK READ ERROR!", 0
a20_error_str db "[BOOT] PANIC: A20 GATE IS NOT OPENED!", 0

times 510 - ($ - $$) db 0
dw 0xaa55