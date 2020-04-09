global enter_protected_mode
global enter_real_mode

section .text

[bits 16]
enter_protected_mode:
    cli
    pop     cx
    push    eax

    xor     ax, ax
    mov     ds, ax
    lgdt    [gdt_desc]

    mov     eax, cr0
    or      eax, 1
    mov     cr0, eax
    jmp     0x08:.clear_pipeline
[bits 32]
.clear_pipeline:
    mov     ax, 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax

    pop     eax
    and     ecx, 0x0000FFFF
    push    ecx
    ret

[bits 32]
enter_real_mode:
    cli
    pop     ecx
    push    eax

    jmp     0x18:.pm_16
[bits 16]
.pm_16:
    mov     ax, 0x20
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax

    mov     eax, cr0
    and     al, 0xFE
    mov     cr0, eax

    lidt    [idt_desc]

    jmp     0:.real_mode
.real_mode:
    mov     ax, 0
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax

    sti

    pop     eax
    push    cx
    ret

section .data
gdt_start:

gdt_null:           ; Selector 0x00
    dq 0

gdt_code:           ; Selector 0x08
    dw 0xFFFF       ; limit [0:16]
    dw 0            ; base  [0:16]
    db 0            ; base  [16:24]
    db 0b10011010   ; Privilege + Type
    db 0b11001111   ; Granularity + Attributes + limit [16:24]
    db 0            ; base [24:32]

gdt_data:           ; Selector 0x10
    dw 0xFFFF       ; limit [0:16]
    dw 0            ; base  [0:16]
    db 0            ; base  [16:24]
    db 0b10010010   ; Privilege + Type
    db 0b11001111   ; Granularity + Attributes + limit [16:24]
    db 0            ; base [24:32]

gdt_code_16:        ; Selector 0x18
    dw 0xFFFF       ; limit [0:16]
    dw 0            ; base  [0:16]
    db 0            ; base  [16:24]
    db 0b10011010
    db 0b00001111
    db 0            ; base  [24:32]

gdt_data_16:        ; Selector 0x20
    dw 0xFFFF       ; limit [0:16]
    dw 0            ; base  [0:16]
    db 0            ; base  [16:24]
    db 0b10010010
    db 0b00001111
    db 0            ; base  [24:32]

gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

idt_desc:
    dw 0x3FF
    dd 0
