#ifndef KERNEL_H
#define KERNEL_H

#include "type.h"
#include "const.h"

/* GDT描述符内存结构 */
typedef struct {
    ushort limit1;          //段界限1
    ushort base1;           //段基址1
    byte   base2;           //段基址2
    byte   attr1;           //属性1
    byte   attr2_limit2;    //属性2+段界限2
    byte   base3;           //段基址3
} Descriptor;

/* 全局段描述符表 */
typedef struct {
    Descriptor* const entry;
    const int size;
} GdtInfo;

/* 门描述符结构 */
typedef struct {
    ushort offset1;
    ushort selector;
    byte   dcount;
    byte   attr;
    ushort offset2;
} Gate;

/* 中断描述符表 */
typedef struct {
    Gate* const entry;
    const int    size;
} IdtInfo;

typedef struct {
    uint gs;
    uint fs;
    uint es;
    uint ds;

    uint edi;
    uint esi;
    uint ebp;
    uint kesp;
    uint ebx;
    uint edx;
    uint ecx;
    uint eax;

    uint raddr;
    uint eip;
    uint cs;
    uint eflags;
    uint esp;
    uint ss;
} RegValue;

typedef struct
{
    uint   previous;
    uint   esp0;
    uint   ss0;         //0特权级栈，低特权级升高特权级时使用。
    uint   unused[22];
    ushort reserved;
    ushort iomb;
} TSS;

typedef struct
{
    RegValue   rv;          //各个寄存器值

    //处理器数据结构  LDT、TSS、选择子
    Descriptor ldt[3];
    TSS        tss;
    ushort     ldtSelector;
    ushort     tssSelector;

    uint       id;
    char       name[8]; 
    byte       stack[512];      //任务执行时使用的栈
} Task;


/* 设置段描述符的值（基址、界限、属性） */
int SetDescValue(Descriptor* pDesc, uint base, uint limit, ushort attr);

/* 获取段描述符的值（基址、界限、属性） */
int GetDescValue(Descriptor* pDesc, uint* pBase, uint* pLimit, ushort* pAttr);

/* 设置中断回调函数 */
int SetIntHandler(Gate* pGate, uint ifunc);

/* 获取中断回调函数 */
int GetIntHandler(Gate* pGate, uint* pIfunc);


#endif  //KERNEL_H