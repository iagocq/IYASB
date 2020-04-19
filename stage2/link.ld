STAGE2_ENTRY = 0x0500;
BSS = 0xa000;
ALLOC_MAX = 0x2000;

SECTIONS {
    . = STAGE2_ENTRY;

    .text : ALIGN(8) {
        *(.entry)
        *(.text)
    }

    .data : ALIGN(8) {
        *(.data)
    }
    .rodata : ALIGN(8) {
        *(.rodata)
    }

    . = BSS;
    .bss : ALIGN(8) {
        _bss_start_ = .;
        *(.bss)
        _bss_end_ = .;
    }

    . = ALIGN(8);

    _alloc_start_ = .;
    _alloc_end_ = . + ALLOC_MAX;
}
