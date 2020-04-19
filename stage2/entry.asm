; Stage 2 entry

[bits 16]

; Using any non-default section name to paste this section before any other
section .entry

extern centry
extern enter_protected_mode
extern _bss_start_
extern _bss_end_

asm_entry:
    ; First enter protected mode, so code compiled by gcc can run
    call    enter_protected_mode

    [bits 32]

    ; clear bss
    xor     eax, eax

    mov     ecx, _bss_end_
    sub     ecx, _bss_start_
    shr     ecx, 2

    mov     edi, _bss_start_
    rep stosd

    ; Just jmp to the entry
    jmp     centry
