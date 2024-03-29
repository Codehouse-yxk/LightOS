
; @Author: yangxingkun
; @Date: 2022-06-19 20:26:04
; @FilePath: \LightOS\kentry.asm
; @Description: 内核入口文件（汇编）
; @Github: https://github.com/Codehouse-yxk

%include "common.asm"

global _start
global PageFaultHandlerEntry
global SegmentFaultHandlerEntry
global TimerHandlerEntry
global SysCallHandlerEntry
global KeyboardHandlerEntry
global ReadPort
global WritePort
global ReadPortW
global WritePortW

extern gGdtInfo
extern gIdtInfo
extern gCTaskAddr

extern PageFaultHandler
extern SegmentFaultHandler
extern TimerHandler
extern SysCallHandler
extern KeyboardHandler
extern KMain
extern ClearScreen
extern RunTask
extern InitInterrupt
extern SendEOI
extern LoadTask
extern gMemSize

; 进入异常
%macro BeginFSR 0
    cli
    
   ; sub esp, 4  ;跳过p.rv.raddr

    pushad      ;push eax, ecx, edx, ebx, esp, ebp, esi, edi

    push ds		;p(实时)->rv.ds --> p(任务)->rv.ds
    push es		;p(实时)->rv.es --> p(任务)->rv.es
    push fs		;p(实时)->rv.fs --> p(任务)->rv.fs
    push gs		;p(实时)->rv.gs --> p(任务)->rv.gs

    ;中断后，ds es使用的段选择子和ss一致，均为 GDT_DATA32_FLAT_SELECTOR
    mov si, ss
    mov ds, si
    mov es, si

    mov esp, BaseOfLoader   ;esp指向内核栈
%endmacro

;进入中断（保存上下文）
;因为将任务RegValue成员当作内核栈使用，所以才有以下处理方式：
;将当前实时上下文保存到RegValue结构中（任务上下文寄存器实时数据-->RegValue）
%macro BeginISR 0
    cli

    sub esp, 4  ;跳过p.rv.raddr

    pushad      ;push eax, ecx, edx, ebx, esp, ebp, esi, edi

    push ds		;p(实时)->rv.ds --> p(任务)->rv.ds
    push es		;p(实时)->rv.es --> p(任务)->rv.es
    push fs		;p(实时)->rv.fs --> p(任务)->rv.fs
    push gs		;p(实时)->rv.gs --> p(任务)->rv.gs

    ;中断后，ds es使用的段选择子和ss一致，均为 GDT_DATA32_FLAT_SELECTOR
    mov si, ss
    mov ds, si
    mov es, si

    mov esp, BaseOfLoader   ;esp指向内核栈
%endmacro

;退出中断（恢复上下文）
;从RegValue结构中恢复上下文（RegValue（进入中断时保存的上下文）-->任务的RegValue【gCTaskAddr】）
%macro EndISR 0
    mov esp, [gCTaskAddr]

	pop gs      ;p(任务)->rv.gs --> p(实时)->rv.gs
    pop fs      ;p(任务)->rv.fs --> p(实时)->rv.fs
    pop es      ;p(任务)->rv.es --> p(实时)->rv.es
    pop ds      ;p(任务)->rv.ds --> p(实时)->rv.ds

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

    mov eax, dword [SendEOIEntry]
    mov dword [SendEOI], eax

    mov eax, dword [LoadTaskEntry]
    mov dword [LoadTask], eax

    mov eax, dword [MemSize]
    mov dword [gMemSize], eax

    leave

    ret

; 读取端口数据
; param: 目标端口号
; return: 读到的数据[一个字节]
; byte ReadPort(ushort port);
ReadPort:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov dx, [ebp + 8]
    in al, dx

    nop
    nop
    nop

    leave

    ret

; 往端口写数据
; param1: 目标端口
; param2：写入的数据[一个字节]
WritePort:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov dx, [ebp + 8]
    mov al, [ebp + 12]
    out dx, al

    nop
    nop
    nop

    leave

    ret

; 读取端口数据(单位：字)
; param1: 目标端口号
; param2: 读取缓冲区
; param3: 需要读取2n个字节数据
; void ReadPortW(ushort port, ushort* buf, uint n);
ReadPortW:
    push ebp
    mov ebp, esp

    mov edx, [ebp + 8]      ;port
    mov edi, [ebp + 12]     ;buf
    mov ecx, [ebp + 16]     ;n

    cld
    rep insw

    nop
    nop
    nop

    leave

    ret

; 往端口写数据(单位：字)
; param1: 目标端口号
; param2: 需要写的数据
; param3: 需要读取2n个字节数据
; void WritePortW(ushort port, ushort* buf, uint n)
WritePortW:
    push ebp
    mov ebp, esp

    mov edx, [ebp + 8]      ;port
    mov esi, [ebp + 12]     ;buf
    mov ecx, [ebp + 16]     ;n

    cld
    rep outsw

    nop
    nop
    nop

    leave

    ret


PageFaultHandlerEntry:
    BeginFSR
    call PageFaultHandler
    EndISR

SegmentFaultHandlerEntry:
    BeginFSR
    call SegmentFaultHandler
    EndISR

TimerHandlerEntry:
    BeginISR
    call TimerHandler
    EndISR

SysCallHandlerEntry:
    BeginISR
    push edx
    push ecx
    push ebx
    push eax
    call SysCallHandler
    pop eax
    pop ebx
    pop ecx
    pop edx
    EndISR

KeyboardHandlerEntry:
    BeginISR
    call KeyboardHandler
    EndISR
    

    
