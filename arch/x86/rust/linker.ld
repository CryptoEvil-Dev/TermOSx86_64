ENTRY(_start)

SECTIONS
{
    /* Секция 1: Первый загрузочный сектор BIOS */
    . = 0x7C00;
    .boot : AT(0x7C00)
    {
        *(.text)
    }

    /* Секция 2: Вторичный загрузчик (Смещения функций считаются от 0x7E00) */
    . = 0x7E00;
    
    .text_rust : AT(0x7E00)
    {
        _code_start = .;
        
        /* Гарантируем, что 32-битный входной мостик ляжет ровно на 0x7E00 */
        *target/primary.o(.bootpm)
        
        /* Следом идёт весь остальной код */
        *(.text .text.*)
        *(.rodata .rodata.*)
        *(.data .data.*)
        *(.bss .bss.*)
        
        . = ALIGN(4);
        _rust_end_marker = .;
    }

    _boot32_sectors = ABSOLUTE((_rust_end_marker - 0x7E00 + 511) / 512);

    /DISCARD/ :
    {
        *(.eh_frame)
        *(.note .note.*)
        *(.comment)
    }
}
