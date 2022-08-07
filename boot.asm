
; @Author: yangxingkun
; @Date: 2022-06-19 20:26:04
; @FilePath: \LightOS\boot.asm
; @Description: 引导程序
; @Github: https://github.com/Codehouse-yxk

%include "blfunc.asm"
%include "common.asm"

org BaseOfBoot

interface:
    BaseOfStack    equ    BaseOfBoot
    ;BaseOfTarget   equ    BaseOfLoader
    LoaderStr db  "LOADER     "
    LoaderLen equ ($-LoaderStr)

BLMain:
    mov ax, cs
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov sp, SPInitValue
	
	;参数入栈
	push word Buffer
	push word BaseOfLoader / 0x10
	push word BaseOfLoader
	push word LoaderLen
	push word LoaderStr

	call LoadTarget
	;add sp, 10	;清理栈
	
	cmp dx, 0
	jz output
	jmp BaseOfLoader
	
output:	
	mov ax, cs
	mov es, ax

    mov bp, ErrStr
    mov cx, ErrLen
	
	xor dx, dx     ;mov dx, 0
    mov ax, 0x1301
	mov bx, 0x0007
	int 0x10
	
	jmp $	

ErrStr db  "NLD"	
ErrLen equ ($-ErrStr)

Buffer:
	times 510-($-$$) db 0x00
	db 0x55, 0xaa
