
;内核入口文件（汇编）

%include "common.asm"

global _start
global AppModInit

extern AppMain
extern GetAppToRun
extern GetAppNum

[section .text]
[bits 32]
_start:
AppModInit:     ;0xF000
    push ebp
    mov ebp, esp

    mov dword [GetAppToRunEntry], GetAppToRun
    mov dword [GetAppNumEntry], GetAppNum

    call AppMain

    leave
    
    ret
