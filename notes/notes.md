# Notes from making my toy OS
hello
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
    cli ; turn off interrupts
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

### C integration

#### boot.asm
```assembly
[org 0x7c00]
[bits 16]

section code

.switch:
    mov bx, 0x1000 ; location where the C code is loaded from hdd
    mov ah, 0x02
    mov al, 30 ; number of sectors to read from hdd
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

#### kernel_entry.asm

```assembly
[bits 32]
START:
[extern start]
    call start
    jmp $
```

#### main.c

```c
char* vram;

int start(){
    vram = (char*) 0xb8000;

    for(int i = 0; i < 2 * 25 * 80; i +=2){
        *(vram + i) = 0x00;
        *(vram + i + 1)  = 0x1c;
    }
    return 0;
}
```

#### makefile
```makefile
nasm boot/boot.asm -f bin -o boot/bin/boot.bin
nasm boot/kernel_entry.asm -f elf -o boot/bin/kernel_entry.bin

gcc -m32 -ffreestanding -c boot/main.c -o boot/bin/kernel.o
ld -m elf_i386 -o boot/bin/kernel.img -Ttext 0x1000 boot/bin/kernel_entry.bin boot/bin/kernel.o

objcopy -O binary -j .text boot/bin/kernel.img boot/bin/kernel.bin
cat boot/bin/boot.bin boot/bin/kernel.bin > os.img
```


### Switching from text mode to graphics mode
#### main.c
```c
#include <stdint.h>

//struct from osdev
typedef struct vbe_mode_info_structure {
	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			// number of bytes per horizontal line
	uint16_t width;			// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;			// unused...
	uint8_t y_char;			// ...
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;			// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;
 
	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;
 
	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
} VBEInfoBlock;

#define VBEInfoAddress 0x8000
VBEInfoBlock* gVBE;

int rgb(int r, int g, int b){
    r = (int)(r/3);
    g = (int)(g/2);
    b = (int)(b/3);

    return r<<11|g<<5|b;
}

int start(){
    gVBE = (VBEInfoBlock*) VBEInfoAddress;

    for (int i = 0; i < gVBE->width * gVBE->height; i++){
        *((uint32_t*)gVBE->framebuffer + i) = rgb(19,41,75);
    }

}
```
#### boot.asm

``` assembly
[org 0x7c00]
[bits 16]

section code

.switch:
    mov ax, 0x4f01  ;   querying the VBE mode info
    mov cx, 0x117   ;   16bit per pixel mode
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
    mov al, 1 ; number of sectors to read from hdd
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