
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
    
    call LoadTarget
	
	cmp dx, 0
	jz output

    ;①加载全局段描述符表
    lgdt [GdtPtr]
    
    ;②关中断
    ; set IOPL to 3
    cli 
    
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

Code32SegLen    equ    $ - CODE32_SEGMENT

ErrStr db  "No KERNEL"	
ErrLen equ ($-ErrStr)

Buffer db  0
