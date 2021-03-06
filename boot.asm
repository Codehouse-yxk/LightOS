
%include "blfunc.asm"
%include "common.asm"

org BaseOfBoot

interface:
    BaseOfStack    equ    BaseOfBoot
    BaseOfTarget   equ    BaseOfLoader
    Target db  "LOADER     "
    TarLen equ ($-Target)

BLMain:
    mov ax, cs
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov sp, SPInitValue
	
	call LoadTarget
	
	cmp dx, 0
	jz output
	jmp BaseOfTarget
	
output:	
    mov bp, ErrStr
    mov cx, ErrLen
	call Print
	
	jmp $	

ErrStr db  "No LOADER"	
ErrLen equ ($-ErrStr)

Buffer:
	times 510-($-$$) db 0x00
	db 0x55, 0xaa
