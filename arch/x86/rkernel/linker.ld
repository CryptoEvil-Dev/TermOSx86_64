ENTRY(_start)

SECTIONS {
    . = 1M; /* Загружаем ядро по адресу 1 МБ */

    .text : {
        KEEP(*(.boot_text)) /* Сначала идет ассемблерный код инициализации */
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data .data.*)
    }

    .bss : {
        *(.bss .bss.*)
    }
}
