[org 0x7c00]
[bits 16]

section code

.switch:
    mov ax, 0x4f01  ;   get the VBE mode info from addresss 0x4f01
    mov cx, 0x117   ;   set graphics mode as 16bit per pixel mode
    mov bx, 0x0800  ;   offset for the VBE info structure
    mov es, bx
    mov di, 0x00
    int 0x10


    ; make the switch to graphics mode
    mov ax, 0x4f02
    mov bx, 0x117
    int 0x10

    xor ax, ax
    mov ds, ax
    mov es, ax

    mov bx, 0x1000 ; location where the C code is loaded from hdd
    mov ah, 0x02
    mov al, 43 ; number of sectors to read from hdd, 512 bytes per sector
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02
    int 0x13

    cli ; turn off theinterrupts
    lgdt [gdt_descriptor]; load the GDT table

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax ; make the switch

    jmp code_seg:protected_start

welcome: db 'Welcome to myOS', 0

[bits 32]
protected_start:
    mov ax, data_seg
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; update the stack pointer
    mov ebp, 0x90000
    mov esp, ebp

    call 0x1000
    jmp $

gdt_begin:
gdt_null_descriptor:
    dd 0x00
    dd 0x00
gdt_code_seg:
    dw 0xeeee
    dw 0x00
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00
gdt_data_seg:
    dw 0xffff
    dw 0x00
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_begin - 1
    dd gdt_begin

code_seg equ gdt_code_seg - gdt_begin
data_seg equ gdt_data_seg - gdt_begin

times 510 - ($-$$) db 0x00

db 0x55
db 0xaa