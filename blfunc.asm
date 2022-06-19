
jmp short _start
nop

header:
    BS_OEMName     db "Light.OS"	;OEM字符个数必须是8
    BPB_BytsPerSec dw 512			;每个扇区的字节数
    BPB_SecPerClus db 1
    BPB_RsvdSecCnt dw 1
    BPB_NumFATs    db 2
    BPB_RootEntCnt dw 224
    BPB_TotSec16   dw 2880
    BPB_Media      db 0xF0
    BPB_FATSz16    dw 9
    BPB_SecPerTrk  dw 18			;柱面（磁道）扇区数
    BPB_NumHeads   dw 2
    BPB_HiddSec    dd 0
    BPB_TotSec32   dd 0
    BS_DrvNum      db 0
    BS_Reserved1   db 0
    BS_BootSig     db 0x29
    BS_VolID       dd 0
    BS_VolLab      db "Lt_OS_V0.01" ;卷标，字符数必须是11
    BS_FileSysType db "FAT12   "	;文件系统类型，字符数必须是8
    
const:
    RootEntryOffset  equ 19
    RootEntryLength  equ 14
    SPInitValue      equ BaseOfStack - EntryItemLength
    EntryItem        equ SPInitValue
    EntryItemLength  equ 32
    FatEntryOffset   equ 1
    FatEntryLength   equ 9
    
_start:
    jmp BLMain
    
;
; return:
;     dx --> (dx != 0) ? success : failure
LoadTarget:

	;①将目录项的内容读到内存中
	mov ax, RootEntryOffset
	mov cx, RootEntryLength
	mov bx, Buffer
	
	call ReadSector
	
	;②查找目标文件
	mov si, Target
	mov cx, TarLen
	mov dx, 0
	
	call FindEntry
	
	cmp dx, 0	;为0表示没找到目标文件
	jz finish
	
	;③拷贝目标文件的目录项信息到EntryItem
	mov si, bx
	mov di, EntryItem	;EntryItem：目录项文件信息的入口地址
	mov cx, EntryItemLength
	
	call MemCpy
	
	;④加载fat表
	mov ax, FatEntryLength	;计算fat表占用内存大小
	mov cx, [BPB_BytsPerSec]
	mul cx
	mov bx, BaseOfTarget
	sub bx, ax				;得到的bx就是fat表在内存中的起始地址
	
	mov ax, FatEntryOffset
	mov cx, FatEntryLength
	
	call ReadSector
	
	;⑤从fat表查找目标文件
	mov dx, [EntryItem + 0x1A]	;得到目标文件第一个扇区的地址
	mov si, BaseOfTarget / 0x10
	mov es, si
	mov si, 0
	
loading:
    mov ax, dx
    add ax, 31
    mov cx, 1
    push dx
    push bx
    mov bx, si
    call ReadSector
    pop bx
    pop cx
    call FatVec
    cmp dx, 0xFF7
    jnb finish
    add si, 512
    cmp si, 0
    jnz continue
    mov si, es
    add si, 0x1000
    mov es, si
    mov si, 0
continue:
    jmp loading
 
finish:   
    ret

; cx --> Fat表的下标(index)
; bx --> Fat表在内存中的起始位置
; return :
;	dx --> fat[index]表示对应fat表项的值,最后一个表项的值为若大于0xff7，表示文件结束。(此次实验中，dx的值为0xfff)
FatVec:
    push cx
    
    mov ax, cx
    shr ax, 1
    
    mov cx, 3
    mul cx
    mov cx, ax
    
    pop ax
    
    and ax, 1
    jz even
    jmp odd

even:    ; FatVec[j] = ( (Fat[i+1] & 0x0F) << 8 ) | Fat[i];
    mov dx, cx
    add dx, 1
    add dx, bx
    mov bp, dx
    mov dl, byte [bp]
    and dl, 0x0F
    shl dx, 8
    add cx, bx
    mov bp, cx
    or  dl, byte [bp]
    jmp return
    
odd:     ; FatVec[j+1] = (Fat[i+2] << 4) | ( (Fat[i+1] >> 4) & 0x0F );
    mov dx, cx
    add dx, 2
    add dx, bx
    mov bp, dx
    mov dl, byte [bp]
    mov dh, 0
    shl dx, 4
    add cx, 1
    add cx, bx
    mov bp, cx
    mov cl, byte [bp]
    shr cl, 4
    and cl, 0x0F
    mov ch, 0
    or  dx, cx

return: 
    ret

; ds:si --> source
; es:di --> destination
; cx    --> length
MemCpy:
    
    cmp si, di
    
    ja btoe
    
    add si, cx
    add di, cx
    dec si
    dec di
    
    jmp etob
    
btoe:
    cmp cx, 0
    jz done
    mov al, [si]
    mov byte [di], al
    inc si
    inc di
    dec cx
    jmp btoe
    
etob: 
    cmp cx, 0
    jz done
    mov al, [si]
    mov byte [di], al
    dec si
    dec di
    dec cx
    jmp etob

done:   
    ret

;查找目标文件
;es:bx --> 根目录区起始地址
;ds:si --> 指向目标字符串
;cx --> 目标字符串长度
;return:
;	(dx != 0) ? exit : noexit
;	如果存在 bx 保存的就是目标项的起始地址
FindEntry:
    push cx
    
    mov dx, [BPB_RootEntCnt]
    mov bp, sp
    
find:
    cmp dx, 0
    jz noexist
    mov di, bx
    mov cx, [bp]
    push si
    call MemCmp
    pop si
    cmp cx, 0
    jz exist
    add bx, 32
    dec dx
    jmp find

exist:
noexist: 
    pop cx
       
    ret

;内存比较函数（根据si的字符串找di）
;ds : si --> source
;es : di --> destination
;cx -->length
;return :
;	(cx == 0) ? equal : noequal
MemCmp:

compare:
    cmp cx, 0
    jz equal
    mov al, [si]
    cmp al, byte [di]
    jz goon
    jmp noequal
goon:
    inc si
    inc di
    dec cx
    jmp compare
    
equal: 
noequal:   

    ret

; es:bp --> string address
; cx    --> string length
Print:
    mov dx, 0
    mov ax, 0x1301
	mov bx, 0x0007
	int 0x10
    ret

; 重置软驱函数 no parameter
ResetFloppy:
    push ax
    mov ah, 0x00
    mov dl, [BS_DrvNum]	;驱动器号
    int 0x13			;中断
    pop ax
    ret

; 读取软驱数据函数：
; ax    --> logic sector number（逻辑扇区号）
; cx    --> number of sector（需要连续读取的扇区数）
; es:bx --> target address（需要将数据读取到的地址）
ReadSector:
    
    call ResetFloppy
    
    push bx
    push cx
    
	mov bl, [BPB_SecPerTrk]
    div bl
    mov cl, ah ;余数
    add cl, 1    ;+1得到扇区号
    mov ch, al ;商
    shr ch, 1    ;商右移一位，得到柱面号
    mov dh, al ;
    and dh, 1   ;得到磁头号
    mov dl, [BS_DrvNum]    ;驱动器号
    
    pop ax      ;将cx的值放入ax中，此时al中含有cx中需要读取的扇区个数
    pop bx
    
    mov ah, 0x02	;开始读取数据，0x02是读指令

read:    
    int 0x13
    jc read
    
    ret
