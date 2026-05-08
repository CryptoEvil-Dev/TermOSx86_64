OUTPUT_FORMAT("binary")
ENTRY(_start)

SECTIONS
{
    . = 0x7c00;
    .boot : { build/boot.o(.boot) }

    /* Код ядра */
    . = 0x7e00;
    .text : {
        build/boot.o(.text) /* 32/64-бит части загрузчика */
        *(.text*)
    }

    /* Данные должны быть ВНУТРИ бинарника */
    .rodata : { *(.rodata*) }
    .data : { *(.data*) }

    /* BSS в конце. ВАЖНО: Мы инициализировали idt, 
       так что она будет в .data и попадет в файл! */
    .bss : { 
        *(COMMON)
        *(.bss*) 
    }
}
