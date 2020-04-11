; Stage 2 entry

[bits 16]

; Using any non-default section name to paste this section before any other
section .entry

extern centry
extern enter_protected_mode

asm_entry:
    ; First enter protected mode, so code compiled by gcc can run
    call    enter_protected_mode

    [bits 32]
    ; Just jmp to the entry
    jmp     centry
