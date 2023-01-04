# Notes from making my toy OS

## Bootloader

### Writing my first letter

```assembly
[org 0x00]
[bits 16]

section code

.main:
;; es register needs a intermediate register to be written, 0xb800 is the first character
    mov eax, 0xb800
    mov es, eax

;; mov two bytes, first one is character printed and second one is color of background and text
    mov byte [es:0x00], 'I' ; 0xb800 + 0x00 = 0xb800
    mov byte [es:0x01], 0x1c; 3 means blue 0 means 0

;; jump to current address, creating a infinite loop
jmp $

;; need to make the file 512 bytes, we have 510 instead of 512 because we are butting 2 extra bytes for executable code
times 510 - ($-$$) db 0x00  ; pads the file with 0s, making it the right size

db 0x55
db 0xaa
```

Resources I read

* [colors in vga text mode](https://os.phil-opp.com/vga-text-mode/)

### clearing the screen

```assembly
[org 0x00]
[bits 16]

section code

.init: 
    mov eax, 0xb800
    mov es, eax
    mov eax, 0;             ; set eax to 0

.clear:
    mov byte [es:eax], 0 ; Move blank character to current text address
    inc eax             ; increment eax
    mov byte [es:eax], 0x1c ; Move the background color to the next address
    inc eax

    cmp eax, 2* 25* 80   ; check if we iterated through entire screen
    jl .clear           ; if less, loop backto .clear

.main:
    mov byte [es:0x00], 'I' 
    mov byte [es:0x01], 0x1c

jmp $

times 510 - ($-$$) db 0x00

db 0x55
db 0xaa
```

### Printing null terminated string

```assembly
[org 0x00]
[bits 16]

section code

.init: 
    mov eax, 0x07c0
    mov ds, eax
    mov eax, 0xb800
    mov es, eax
    mov eax, 0  ; initialize eax to 0
    mov ebx, 0  ; index of the character in the string that we are printing
    mov ecx, 0  ; Actual address of the character on the screen
    mov dl, 0   ; store actual value that we are printing to the screen


.clear:
    mov byte [es:eax], 0 ; Move blank character to current text address
    inc eax             ; increment eax
    mov byte [es:eax], 0x1c ; Move the background color to the next address
    inc eax

    cmp eax, 2* 25* 80   ; check if we iterated through entire screen
    jl .clear           ; if less, loop backto .clear

mov eax, text
call .print

.halt:
    jmp $

.print:
    mov dl, byte [eax + ebx]

    cmp dl, 0   ; detect null termination and jmp to end print
    je .end_print

    mov byte [es:ecx], dl
    inc ebx
    inc ecx
    inc ecx

    jmp .print

.end_print:
    ret


text: db 'Hello, World!', 0
text1: db 'ILL INI', 0

times 510 - ($-$$) db 0x00

db 0x55
db 0xaa
```

### Printing string at any spot of the screen

```assembly
[org 0x00]
[bits 16]

section code

.init: 
    mov eax, 0x07c0
    mov ds, eax
    mov eax, 0xb800
    mov es, eax
    mov eax, 0  ; initialize eax to 0
    mov ebx, 0  ; index of the character in the string that we are printing
    mov ecx, 0  ; Actual address of the character on the screen
    mov dl, 0   ; store actual value that we are printing to the screen


.clear:
    mov byte [es:eax], 0 ; Move blank character to current text address
    inc eax             ; increment eax
    mov byte [es:eax], 0x1c ; Move the background color to the next address
    inc eax

    cmp eax, 2* 25* 80   ; check if we iterated through entire screen
    jl .clear           ; if less, loop backto .clear

mov eax, text
mov ecx, 3 * 2 * 80 ; third line (80 char per line, 2 bytes per char)
push .halt ; push halt to stack so the program halts after .print rets
call .print

.halt:
    mov byte[es:0x00], 'L'  ; testing ret, if so, first character will be 'L'
    jmp $

.print:
    mov dl, byte [eax + ebx]

    cmp dl, 0   ; detect null termination and jmp to end print
    je .end_print

    mov byte [es:ecx], dl
    inc ebx
    inc ecx
    inc ecx

    jmp .print

.end_print:
    ret


text: db 'Hello, World!', 0
text1: db 'ILL INI', 0

times 510 - ($-$$) db 0x00

db 0x55
db 0xaa
```

### Protected 32-bit mode

```assembly
[org 0x7c00]
[bits 16]

section code

.init: 
    mov eax, 0xb800
    mov es, eax
    mov eax, 0  ; initialize eax to 0
    mov ebx, 0  ; index of the character in the string that we are printing
    mov ecx, 0  ; Actual address of the character on the screen
    mov dl, 0   ; store actual value that we are printing to the screen


.clear:
    mov byte [es:eax], 0 ; Move blank character to current text address
    inc eax             ; increment eax
    mov byte [es:eax], 0x1c ; Move the background color to the next address
    inc eax

    cmp eax, 2* 25* 80   ; check if we iterated through entire screen
    jl .clear           ; if less, loop backto .clear

mov eax, welcome
mov ecx, 0 * 2 * 80 ; print at first line
call .print

jmp .switch

.print:
    mov dl, byte [eax + ebx]

    cmp dl, 0   ; detect null termination and jmp to end print
    je .end_print

    mov byte [es:ecx], dl
    inc ebx
    inc ecx
    inc ecx

    jmp .print

.end_print:
    ret

.switch:
    cli ; turn off theinterrupts
    lgdt [gdt_descriptor]; load the GDT table

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax ; make the switch

    jmp protected_start

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

    jmp $

gdt_begin:
gdt_null_descriptor:
    dd 0x00
    dd 0x00
gdt_code_seg:
    dw 0xffff
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
```