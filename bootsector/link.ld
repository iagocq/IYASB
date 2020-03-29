BS_ENTRY = 0x7c00;
BSS = 0xa000;
STAGE2_ENTRY = 0x500;

SECTIONS {
    _stage2_entry_ = STAGE2_ENTRY;
    . = BS_ENTRY;

    .text : {
        *(.text)
    }

    . = BSS;
    .bss : {
        *(.bss)
    }
}
