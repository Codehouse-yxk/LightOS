
; @Author: yangxingkun
; @Date: 2022-06-19 20:26:04
; @FilePath: \LightOS\loader.asm
; @Description: 跳转保护模式、加载内核
; @Github: https://github.com/Codehouse-yxk

%include "blfunc.asm"
%include "common.asm"

org BaseOfLoader


BaseOfStack    equ    BaseOfLoader
KernalName db  "KERNEL     "
KernalLen equ ($-KernalName)
AppName db  "APP        "
AppLen equ ($-AppName)

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
PAGE_DIR_DESC        :     Descriptor  PageDirBase,   4095,         DA_DRW + DA_32
PAGE_TBL_DESC        :     Descriptor  PageTblBase,   1023,         DA_DRW + DA_LIMIT_4K + DA_32
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
PageDirSelector  	  equ (0x0007 << 3) + SA_TIG + SA_RPL0
PageTblSelector  	  equ (0x0008 << 3) + SA_TIG + SA_RPL0
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

    ;参数入栈-加载应用代码
    push word Buffer
    push word BaseOfApp / 0x10
    push word BaseOfApp
    push word AppLen
    push word AppName

    call LoadTarget
	add sp, 10  ;清理栈
	cmp dx, 0
	jz AppErr

    mov ax, cs
    mov es, ax
    
    ;参数入栈-加载内核代码
    push word Buffer
    push word BaseOfKernel / 0x10
    push word BaseOfKernel
    push word KernalLen
    push word KernalName

    call LoadTarget
	add sp, 10  ;清理栈
    cmp dx, 0
	jz KernelErr

    ;获取物理内存容量
    call GetMemSize

    ;保存共享数据
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

KernelErr:	
    mov bp, KernelErrStr
    mov cx, KernelErrLen
    jmp output
AppErr:
    mov bp, AppErrStr
    mov cx, AppErrLen
    jmp output
output:
    mov ax, cs
	mov es, ax
	xor dx, dx     ;mov dx, 0
    mov ax, 0x1301
	mov bx, 0x0007
	int 0x10
	
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

;获取物理内存容量
GetMemSize:
    mov dword [MemSize], 0

    xor eax, eax    ;清空CF标志位
    mov eax, 0xE801 ;获取物理内存大小，BIOS中断号

    int 0x15

    jc geterr       ;cf为1，表示获取失败ret

    shl eax, 10     ;15M以下内存容量【1kb -> 1b】
    shl ebx, 6      ;16M以上内存容量【64kb -> 1kb】
    shl ebx, 10     ;16M以上内存容量【1kb -> 1b】

    mov ecx, 1
    shl ecx, 20     ;修正内存容量，加上1MB的内存黑洞

    add dword [MemSize], eax
    add dword [MemSize], ebx
    add dword [MemSize], ecx

    jmp getok

geterr:
    mov dword [MemSize], 0
getok:
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
    
    lldt word [esp + 96]   ;p->ldtSelector
    ltr word [esp + 98]    ;p->tssSelector

    pop gs  ;p->rv.gs --> gs
    pop fs  ;p->rv.fs --> fs
    pop es  ;p->rv.es --> es
    pop ds  ;p->rv.ds --> ds

    popad   ;pop edi, esi, ebp, esp, ebx, edx, ecx, eax

    add esp, 4  ;mov esp, &(p->rv.eip)

    mov dx, MASTER_IMR_PORT
    
    in ax, dx

    %rep 5
    nop
    %endrep
    
    and ax, 0xFC    ;启动定时器中断、键盘中断
    out dx, al

    ;启动页表
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax

    iret    ;pop eip, cs, eflags, esp, ss

;void LoadTask(Task* p)
LoadTask:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]

    lldt word [eax + 96]

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

    call SetupPage

    jmp dword Code32FlatSelector : BaseOfKernel


SetupPage:
    push eax
    push ecx
    push edi
    push es
    
    mov ax, PageDirSelector
    mov es, ax
    mov ecx, 1024    ;  1K sub page tables
    mov edi, 0
    mov eax, PageTblBase | PG_P | PG_USU | PG_RWW
    
    cld
    
stdir:
    stosd
    add eax, 4096
    loop stdir
    
    mov ax, PageTblSelector
    mov es, ax
    mov ecx, 1024 * 1024   ; 1M pages
    mov edi, 0
    mov eax, PG_P | PG_USU | PG_RWW
    
    cld
    
sttbl:
    stosd
    add eax, 4096
    loop sttbl
    
    mov eax, PageDirBase
    mov cr3, eax
    
    pop es
    pop edi
    pop ecx
    pop eax
    
    ret  

;
;
DefaultHandlerFunc:
    iret
    
DefaultHandler    equ    DefaultHandlerFunc - $$

Code32SegLen    equ    $ - CODE32_SEGMENT

KernelErrStr db  "No KERNEL"	
KernelErrLen equ ($-KernelErrStr)

AppErrStr db  "No App"	
AppErrLen equ ($-AppErrStr)

Buffer db  0
