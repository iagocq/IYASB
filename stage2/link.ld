STAGE2_ENTRY = 0x0500;
BSS = 0xa000;

SECTIONS {
    . = STAGE2_ENTRY;

    .text : ALIGN(64) {
        *(.entry)
        *(.text)
    }

    .data : ALIGN(64) {
        *(.data)
    }
    .rodata : ALIGN(64) {
        *(.rodata)
    }

    . = BSS;
    .bss : {
        *(.bss)
    }
}
