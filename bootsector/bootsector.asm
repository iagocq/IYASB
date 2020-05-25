; ========= IYASB-bs =========
; a kinda small stage 2 loader
; ============================


[bits 16]

extern _stage2_entry_

%define NO_EXT_CODE             '1'
%define STAGE2_NOT_FOUND_CODE   '2'

%define O_BYTES_PER_LSECTOR     code_start+0x0b
%define O_LSECTORS_PER_CLUSTER  code_start+0x0d
%define O_RESERVED_LSECTORS     code_start+0x0e
%define O_NFATS                 code_start+0x10
%define O_LSECTORS_PER_FAT      code_start+0x24
%define O_ROOT_CLUSTER          code_start+0x2c

section .text

code_start:

jmp start
nop

; skip BPB
times 90-($$-$) db 0

start:
    ; disable hardware interrupts
    cli

    jmp     0:.clear_cs     ; clear segment registers
.clear_cs:                  ;
    xor     ax, ax          ;
    mov     ds, ax          ;
    mov     es, ax          ;
    mov     fs, ax          ;
    mov     gs, ax          ;
    mov     ss, ax          ;

    ; clear BSS
    mov     di, bss_start
    mov     cx, bss_end-bss_start
    rep stosb

    ; stack @ 0000:fff0
    mov     sp, 0xfff0

    ; save drive number
    mov     [drive_number], dl

    mov     WORD [ext_DAP.size], 0x1A
    mov     BYTE [rDAP.size], 0x10

    ; read drive parameters
    mov     ah, 0x48
    lea     si, [ext_DAP]
    int     0x13
    ; carry is set on error
    jc      no_ext_die

    ; calculate and cache useful values

    ; psectors_per_lsector_lg = log2(bytes_per_lsector / bytes_per_psector)
    mov     ax, [O_BYTES_PER_LSECTOR]
    mov     bx, WORD [ext_DAP.bps]
    mov     WORD [bytes_per_psector], bx
    div     bx
    xor     ah, ah
    bsr     ax, ax
    mov     [psectors_per_lsector_lg], al

    ; entries = bps / 0x20 <=> entries = bps >> 5
    shr     bx, 5
    mov     di, bx

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
    mov     [data_start], eax
    mov     ebx, eax

    xor     ebp, ebp
    inc     ebp

.find_loop:
    mov     si, read_buffer
    call    read_sectors

    mov     cx, di

    .entry_loop:
        cmp     BYTE [si], 0
        je      no_stage2_die

        pusha
        mov     di, stage2_filename

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

        add     si, 0x20
        loop    .entry_loop

    inc     ebx
    jmp     .find_loop


found_stage2:
    ; si currently points to the stage2 entry
    ; [si+0x14] = higher 2 bytes of start cluster
    ; [si+0x1a] = lower 2 bytes of start cluster

    ; ebp = nsectors
    ; si = current buffer

    ; edi = cluster number
    mov     di, WORD [si+0x14]
    shl     edi, 16
    mov     di, WORD [si+0x1a]

    ; si will be juggled around to hold the correct buffer in each occasion
    xor     esi, esi
    mov     si, _stage2_entry_

    ; while (edi < 0x0FFFFFF8)
    .read_loop:
        ; cluster_sec = data_start + (cluster - root_cluster) * psecs_per_cluster

        ; psecs_per_cluster
        movzx   eax, BYTE [O_LSECTORS_PER_CLUSTER]
        mov     cl, BYTE [psectors_per_lsector_lg]
        shl     ax, cl

        ; ax will be used to get the number of bytes read (* bytes_per_psector)
        ; ax will be used to pop later
        push    ax

        ; ebp should hold the psecs to read
        mov     ebp, eax

        ; * (cluster - root_cluster)
        push    edi

        sub     edi, [O_ROOT_CLUSTER]
        mul     edi

        pop     edi

        ; + data_start
        add     eax, [data_start]
        mov     ebx, eax

        call    read_sectors

        ; * bytes_per_psector (to increment si offset)
        pop     ax

        mul     WORD [bytes_per_psector]
        add     si, ax

        ; cluster_off = cluster << 2
        ; each entry is 4 bytes long
        mov     eax, edi
        shl     eax, 2

        ; cluster_next_off = edx = cluster_off % bytes_per_sec
        ; cluster_next_sec = eax = cluster_off / bytes_per_sec
        div     DWORD [bytes_per_psector]

        ; ebx = fat_start
        movzx   ebx, WORD [O_RESERVED_LSECTORS]
        shl     ebx, cl

        ; fat_sec = ebx = fat_start + cluster_next_sec
        add     ebx, eax

        ; read one sector
        xor     ebp, ebp
        inc     ebp

        push    si

        mov     si, read_buffer
        call    read_sectors

        ; cluster = [read_buffer+cluster_next_off]
        mov     edi, [esi+edx]
        pop     si

        cmp     edi, 0x0FFFFFF8
        jl      .read_loop
    mov     dl, [drive_number]
    jmp     0:_stage2_entry_

no_stage2_die:
    mov     al, STAGE2_NOT_FOUND_CODE
    jmp     die
no_ext_die:
    mov     al, NO_EXT_CODE

; AL = ASCII number of the single digit exit code
die:
    mov     ah, 0x4f
direct_die:
    push    WORD 0xb800
    pop     es
    mov     [es:0], ax

; die, but don't die on a busy loop
    cli
.loop_forever:
    hlt
    jmp     .loop_forever

; ebx = sector
; ebp = nsectors
; si = buffer
read_sectors:
    pushad

    mov     [rDAP.readn], ebp
    mov     [rDAP.b_offset], si
    mov     [rDAP.st_sector], ebx

    mov     dl, [drive_number]
    mov     ah, 0x42
    mov     si, rDAP
    int     0x13

    popad
    ret

stage2_filename: db 'STAGE2     '
; never overrun the partition table
times 440-($-$$) db 0
db 0xff
times 510-($-$$) db 0

; boot signature
dw 0xAA55

;==============================================================================
;==============================================================================

section .bss
bss_start:
read_buffer:    resb 4096

drive_number:   resb 1
entries:        resw 1

bytes_per_psector:          resd 1
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
bss_end:
