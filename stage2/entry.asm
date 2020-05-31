; Stage 2 entry

[bits 16]

; Using any non-default section name to paste this section before any other
section .entry

extern centry
extern enter_protected_mode
extern _bss_start_
extern _bss_end_

global _start
global kernel_addr

_start:
    cli
    push    edx

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

    pop     edx
    mov     al, dl
    push    eax
    mov     [saved_disk], eax

    ; Just call the entry
    call    centry

    mov     edx, [saved_disk]
    jmp     [kernel_addr]

halt_forever:
    hlt
    jmp     halt_forever

kernel_addr: dd 0
saved_disk:  dd 0
