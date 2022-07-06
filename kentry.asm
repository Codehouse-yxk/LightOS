
%include "common.asm"

global _start

extern KMain
extern ClearScreen
extern gGdtInfo

[section .text]
[bits 32]
_start:
    mov ebp, 0

    call InitGlobal

    call ClearScreen
    
    call KMain
    
    jmp $

;将共享内存中的数据取出，并初始化全局变量
InitGlobal:
    push ebp
    mov ebp, esp

    mov eax, dword [GdtEntry]
    mov [gGdtInfo], eax
    mov eax, dword [GdtSize]
    mov [gGdtInfo + 4], eax

    leave

    ret
