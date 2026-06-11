ENTRY(_start)

SECTIONS {
    . = 0x7c00;

    .boot:
    {
        *(.text)
        *(.rodata)

        . = 0x7c00 + 510;
        SHORT(0xaa55)
    }
}