
%include "blfunc.asm"
%include "common.asm"

org BaseOfLoader

interface:
    BaseOfStack    equ    BaseOfLoader
    BaseOfTarget   equ    BaseOfKernel
    Target db  "KERNEL     "
    TarLen equ ($-Target)

[section .gdt]
; GDT definition
;                                       Base,         Limit,        Attribute
GDT_ENTRY            :     Descriptor    0,            0,           0
CODE32_DESC          :     Descriptor    0,    Code32SegLen - 1,    DA_C + DA_32 + DA_DPL0
VIDEO_DESC           :     Descriptor    0xB8000,   0x07FFF,        DA_DRWA + DA_32 + DA_DPL0
CODE32_FLAT_DESC     :     Descriptor    0,         0xFFFFF,        DA_C + DA_32 + DA_DPL0
DATA32_FLAT_DESC     :     Descriptor    0,         0xFFFFF,        DA_DRW + DA_32 + DA_DPL0
TASK_LDT_DESC        :     Descriptor    0,            0,           0
TASK_TSS_DESC        :     Descriptor    0,            0,           0
; GDT end

GdtLen    equ   $ - GDT_ENTRY

GdtPtr:
          dw   GdtLen - 1
          dd   0
          
          
; GDT Selector
Code32Selector        equ (0x0001 << 3) + SA_TIG + SA_RPL0
VideoSelector         equ (0x0002 << 3) + SA_TIG + SA_RPL0
Code32FlatSelector    equ (0x0003 << 3) + SA_TIG + SA_RPL0
Data32FlatSelector    equ (0x0004 << 3) + SA_TIG + SA_RPL0
; end of [section .gdt]


[section .idt]
align 32
[bits 32]
IDT_ENTRY:
; IDT definition
;                        Selector,             Offset,       DCount,    Attribute
%rep 256
              Gate      Code32Selector,    DefaultHandler,   0,         DA_386IGate + DA_DPL0
%endrep

IdtLen    equ    $ - IDT_ENTRY

IdtPtr:
          dw    IdtLen - 1
          dd    0

; end of [section .idt]



[section .s16]
[bits 16]
BLMain:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, SPInitValue
	
    
    ;初始化全局段描述符表（GDT）中的描述符
    mov esi, CODE32_SEGMENT
    mov edi, CODE32_DESC
    
    call InitDescItem
    
    ;设置GdtPtr结构体,初始化GDT基地址
    mov eax, 0
    mov ax, ds
    shl eax, 4
    add eax, GDT_ENTRY
    mov dword [GdtPtr + 2], eax

    ; 初始化idt数据结构
    mov eax, 0
    mov ax, ds
    shl eax, 4
    add eax, IDT_ENTRY
    mov dword [IdtPtr + 2], eax
    
    call LoadTarget
	
	cmp dx, 0
	jz output

    call StoreGlobal

    ;①加载全局段描述符表
    lgdt [GdtPtr]
    
    ;②关中断
    ; set IOPL to 3
    cli 

    lidt [IdtPtr]
    
    pushf
    pop eax
    or eax, 0x3000
    push eax
    popf
    
    ;③打开A20地址线
	in al, 0x92			;从0x92端口读出数据到al
	or al, 00000010b	;将读到的数据第二位置1
	out 0x92, al		;将数据写入0x92端口
    
    ;④通知处理器进入保护模式
    mov eax, cr0
    or eax, 0x01
    mov cr0, eax
    
    ;⑤从16位实模式跳转到32位的实模式
    jmp dword Code32Selector : 0

output:	
    mov bp, ErrStr
    mov cx, ErrLen
	call Print
	
	jmp $

;根据代码段标签和段描述符标签初始化段描述符中段基址的函数
;esi --> 代码段标签
;edi --> 段描述符标签
InitDescItem:
    push eax

    mov eax, 0
    mov ax, cs
    shl eax, 4
    add eax, esi
    mov word [edi + 2], ax
    shr eax, 16
    mov byte [edi + 4], al
    mov byte [edi + 7], ah
    
    pop eax
    
    ret

;保存内核需要访问的数据信息到共享内存中
StoreGlobal:
    mov eax, dword [GdtPtr + 2]
    mov dword [GdtEntry], eax
    mov dword [GdtSize], GdtLen / 8

    mov eax, dword [IdtPtr + 2]
    mov dword [IdtEntry], eax
    mov dword [IdtSize], IdtLen / 8

    mov dword [RunTaskEntry], RunTask
    mov dword [InitInterruptEntry], InitInterrupt
    mov dword [EnableTimerEntry], EnableTimer
    mov dword [SendEOIEntry], SendEOI
    mov dword [LoadTaskEntry], LoadTask

    ret

[section .gfunc]
[bits 32]

;void RunTask(Task* p)
;param : Task* p
RunTask:
    push ebp
    mov ebp, esp

    mov esp, [ebp + 8]      ;mov esp, &(p->rv.gs)
    
    lldt word [esp + 200]   ;p->ldtSelector
    ltr word [esp + 202]    ;p->tssSelector

    pop gs  ;p->rv.gs --> gs
    pop fs  ;p->rv.fs --> fs
    pop es  ;p->rv.es --> es
    pop ds  ;p->rv.ds --> ds

    popad   ;pop edi, esi, ebp, esp, ebx, edx, ecx, eax

    add esp, 4  ;mov esp, &(p->rv.eip)

    iret    ;pop eip, cs, eflags, esp, ss

;void LoadTask(Task* p)
LoadTask:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]

    lldt word [eax + 200]

    leave
    
    ret

[section .sfunc]
[bits 32]
;
;
Delay:
    %rep 5
    nop
    %endrep
    ret
    
;
;
Init8259A:
    push ax
    
    ; master
    ; ICW1
    mov al, 00010001B
    out MASTER_ICW1_PORT, al
    
    call Delay
    
    ; ICW2
    mov al, 0x20
    out MASTER_ICW2_PORT, al
    
    call Delay
    
    ; ICW3
    mov al, 00000100B
    out MASTER_ICW3_PORT, al
    
    call Delay
    
    ; ICW4
    mov al, 00010001B
    out MASTER_ICW4_PORT, al
    
    call Delay
    
    ; slave
    ; ICW1
    mov al, 00010001B
    out SLAVE_ICW1_PORT, al
    
    call Delay
    
    ; ICW2
    mov al, 0x28
    out SLAVE_ICW2_PORT, al
    
    call Delay
    
    ; ICW3
    mov al, 00000010B
    out SLAVE_ICW3_PORT, al
    
    call Delay
    
    ; ICW4
    mov al, 00000001B
    out SLAVE_ICW4_PORT, al
    
    call Delay
    
    pop ax
    
    ret
    
; al --> IMR register value
; dx --> 8259A port
WriteIMR:
    out dx, al
    call Delay
    ret
    
; dx --> 8259A
; return:
;     ax --> IMR register value
ReadIMR:
    in ax, dx
    call Delay
    ret

;
; dx --> 8259A port
WriteEOI:
    push ax
    
    mov al, 0x20
    out dx, al
    
    call Delay
    
    pop ax
    
    ret

;初始化中断
InitInterrupt:
    push ebp
    mov ebp, esp

    push ax
    push dx
    
    call Init8259A
    
    sti
    
    mov ax, 0xFF
    mov dx, MASTER_IMR_PORT
    
    call WriteIMR
    
    mov ax, 0xFF
    mov dx, SLAVE_IMR_PORT
    
    call WriteIMR
    
    pop dx
    pop ax
    
    leave 
    ret

;
;使能定时器
EnableTimer:
    push ebp
    mov ebp, esp
    
    push ax
    push dx
    
    mov dx, MASTER_IMR_PORT
    
    call ReadIMR
    
    and ax, 0xFE
    
    call WriteIMR
    
    pop dx
    pop ax
    
    leave
    ret  

;往目标端口发送中断结束标志
;void SendEOI(uint port)
SendEOI:
    push ebp
    mov ebp, esp

    mov edx, [ebp + 8]
    mov al, 0x20        ;中断结束标志：0x20
    out dx, al

    call Delay

    leave
    ret
    
[section .s32]
[bits 32]
CODE32_SEGMENT:

    mov ax, VideoSelector
    mov gs, ax

    mov ax, Data32FlatSelector
    mov ds, ax
    mov es, ax
    mov fs, ax

    mov ax, Data32FlatSelector
    mov ss, ax
    mov esp, BaseOfLoader

    jmp dword Code32FlatSelector : BaseOfTarget

;
;
DefaultHandlerFunc:
    iret
    
DefaultHandler    equ    DefaultHandlerFunc - $$

Code32SegLen    equ    $ - CODE32_SEGMENT

ErrStr db  "No KERNEL"	
ErrLen equ ($-ErrStr)

Buffer db  0
