
%include "common.asm"

global _start

extern KMain
extern ClearScreen
extern RunTask
extern gGdtInfo
extern gIdtInfo
extern InitInterrupt
extern EnableTimer
extern SendEOI

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

    mov eax, dword [IdtEntry]
    mov [gIdtInfo], eax
    mov eax, dword [IdtSize]
    mov [gIdtInfo + 4], eax

    mov eax, dword [RunTaskEntry]
    mov dword [RunTask], eax

    mov eax, dword [InitInterruptEntry]
    mov dword [InitInterrupt], eax

    mov eax, dword [EnableTimerEntry]
    mov dword [EnableTimer], eax

    mov eax, dword [SendEOIEntry]
    mov dword [SendEOI], eax

    leave

    ret
