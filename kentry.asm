
%include "common.asm"

global _start
global TimeHandleEntry

extern gGdtInfo
extern gIdtInfo
extern gCTaskAddr

extern TimerHandler
extern KMain
extern ClearScreen
extern RunTask
extern InitInterrupt
extern EnableTimer
extern SendEOI

;进入中断（保存上下文）
%macro BeginISR 0
    sub esp, 4  ;跳过p.rv.raddr

    pushad      ;push eax, ecx, edx, ebx, esp, ebp, esi, edi

    push ds		;p->rv.ds --> ds
    push es		;p->rv.es --> es
    push fs		;p->rv.fs --> fs
    push gs		;p->rv.gs --> gs

    ;中断后，ds es使用的段选择子和ss一致，均为 GDT_DATA32_FLAT_SELECTOR
    mov dx, ss
    mov ds, dx
    mov es, dx

    mov esp, BaseOfLoader   ;esp指向内核栈
%endmacro

;退出中断（恢复上下文）
%macro EndISR 0
    mov esp, [gCTaskAddr]

    pop gs      ;gs --> p->rv.gs
    pop fs      ;fs --> p->rv.fs
    pop es      ;es --> p->rv.es
    pop ds      ;ds --> p->rv.ds

    popad       ;pop edi, esi, ebp, esp, ebx, edx, ecx, eax

    add esp, 4  ;mov esp, &(p->rv.eip)

    iret        ;pop eip, cs, eflags, esp, ss
%endmacro


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

TimeHandleEntry:
    BeginISR
    call TimerHandler
    EndISR

    

    
