[bits 32]

global call_interrupt

extern enter_real_mode
extern enter_protected_mode

call_interrupt:
    pop     eax     ; return address

    mov     [save_ebp], ebp
    mov     [save_ret], eax

    call    enter_real_mode

[bits 16]
    pop     eax     ; int number
    mov     [int_number], al
    jmp     0:.clear_pipeline

.clear_pipeline:
    popad

    db      0xCD    ; int instruction
int_number: db 0    ; int number
    cli

    ; don't destroy the stack for the caller
    sub     esp, 8*4+4

    mov     [save_esp], esp
    mov     esp, registers.end
    pushad  ; save gp registers + eflags for caller access
    pushfd  ;
    mov     esp, [save_esp]

    call    enter_protected_mode
[bits 32]

    mov     eax, registers

    mov     ebp, [save_ebp]
    jmp     [save_ret]

save_ret:       dd 0
save_ebp:       dd 0

save_esp:       dd 0xd1d2d3d4


registers:      dd 0xe1e2e3e4   ; eflags
                times 8 dd 0xf1f2f3f4 ; gp registers
.end:
