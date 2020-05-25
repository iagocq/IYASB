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

    pushad  ; save gp registers + eflags for caller access
    pushfd  ;

    call    enter_protected_mode
[bits 32]

    mov     eax, esp

    mov     ebp, [save_ebp]
    jmp     [save_ret]

save_ret:       dq 0
save_ebp:       dq 0
