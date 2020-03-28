; ========= IYASB-bs =========
; a kinda small stage 2 loader
; ============================


[bits 16]

%define NO_EXT_CODE             '1'
%define STAGE2_NOT_FOUND_CODE   '2'

%define O_BYTES_PER_LSECTOR     code_start+0x0b
%define O_LSECTORS_PER_CLUSTER  code_start+0x0d
%define O_RESERVED_LSECTORS     code_start+0x0e
%define O_NFATS                 code_start+0x10
%define O_LSECTORS_PER_FAT      code_start+0x24

section .text

code_start:

jmp start
nop

; skip BPB
times 90-($$-$) db 0

start:
    cli                     ; disable hardware interrupts

    jmp     0:.clear_cs     ; clear segment registers
.clear_cs:                  ;
    xor     ax, ax          ;
    mov     ds, ax          ;
    mov     es, ax          ;
    mov     fs, ax          ;
    mov     gs, ax          ;
    mov     ss, ax          ;

    mov     sp, 0xffff      ; stack @ 0000:ffff

    mov     [drive_number], dl  ; save drive number

    mov     WORD [ext_DAP.size], 0x1A
    mov     BYTE [rDAP.size], 0x10
    mov     BYTE [rDAP.readn], 1
    mov     WORD [rDAP.b_offset], read_buffer
    mov     WORD [rDAP.b_segment], ax

    mov     ah, 0x48        ; read drive parameters
    lea     si, [ext_DAP]
    int     0x13
    jc      no_ext_die      ; carry is set on error

    ; calculate and cache useful values

    ; psectors_per_lsector_lg = log2(bytes_per_lsector / bytes_per_psector)
    mov     ax, [O_BYTES_PER_LSECTOR]
    mov     bx, [ext_DAP.bps]
    div     bx
    xor     ah, ah
    bsr     ax, ax
    mov     [psectors_per_lsector_lg], al

    ; entries = bps / 0x20 <=> entries = bps >> 5
    shr     bx, 5
    mov     [entries], bl

    ; data_start = ((nfats * lsectors_per_fat) + reserved_lsectors)
    ;              << psectors_per_lsector_lg

    ; nfats * lsectors_per_fat
    movzx   eax, BYTE [O_NFATS]
    mul     DWORD [O_LSECTORS_PER_FAT]

    ; + reserved_lsectors
    add     ax, WORD [O_RESERVED_LSECTORS]

    ; << psectors_per_lsector_lg
    mov     cl, [psectors_per_lsector_lg]
    shl     eax, cl

    ; save data start
    mov     [rDAP.st_sector], eax
    mov     [data_start], eax

    mov     dl, [drive_number]
.find_loop:
    call    read_sectors
    lea     si, [stage2_filename]

    movzx   eax, WORD [rDAP.b_offset]
    mov     cx, [entries]

    .entry_loop:
        cmp     BYTE [eax], 0
        je      no_stage_2_die
        lea     di, [eax]

        pusha

        ; 11 characters to compare
        mov     cx, 11

        ; compare filename
        cld
        .l_equal:
            lodsb               ; al = [si++]
            scasb               ; [di++] == al
            loopz   .l_equal    ; loop while [si] == [di] (ZF is set) and cx > 0
            jnz     .not_equal
        test    cx, cx

        .not_equal:
        popa
        jz      found_stage2

        add     ax, 0x20
        loop    .entry_loop

    inc     DWORD [rDAP.st_sector]
    jmp     .find_loop


found_stage2:
    jmp     $

no_stage_2_die:
    mov     al, STAGE2_NOT_FOUND_CODE
    jmp     die
no_ext_die:
    mov     al, NO_EXT_CODE

; AL = ASCII number of the single digit exit code
die:
    mov     bx, 0xb800
    mov     es, bx
    mov     ah, 0x4f
    mov     [es:0], ax

; die, but don't die on a busy loop
    cli
.loop_forever:
    hlt
    jmp     die.loop_forever

read_sectors:
    mov     ah, 0x42
    lea     si, [rDAP]
    int     0x13
    ret

; hardcoded because yeah

; never overrun the partition table
stage2_filename:    db 'STAGE2     '
times 440-($-$$) db 0
times 510-($-$$) db 0

; boot signature
dw 0xAA55

;==============================================================================
;==============================================================================

section .bss
read_buffer:    resb 4096

drive_number:   resb 1
entries:        resw 1

data_start:     resd 1
psectors_per_lsector_lg:    resb 1

; the read DAP used for int 13h extensions calls
rDAP:
    .size:      resb 1
    .unused:    resb 1
    .readn:     resb 2

    ; segment and offset pointing to the buffer
    .b_offset:  resb 2
    .b_segment: resb 2

    ; start sector (0 based)
    .st_sector: resb 8

; the extensions DAP used to check int 13h extensions support
ext_DAP:
    .size:      resb 2
    .inf_flags: resb 2

    .phy_cil:   resb 4
    .phy_heads: resb 4
    .phy_spt:   resb 4

    .nsectors:  resb 8
    .bps:       resb 2

    .edd_ptr:   resb 4
