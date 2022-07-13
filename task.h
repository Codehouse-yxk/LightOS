
#ifndef TASK_H
#define TASK_H

#include "kernel.h"

typedef struct {
    /* 段寄存器 */
    uint gs;
    uint fs;
    uint es;
    uint ds;

    /* 通用寄存器 */
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

    //处理器数据结构  LDT、选择子
    Descriptor ldt[3];
    ushort     ldtSelector;
    ushort     tssSelector;

    uint       id;
    char       name[8]; 
    byte       stack[512];      //任务执行时使用的栈
} Task;


extern void (*const RunTask)(volatile Task* p);

extern void (*const LoadTask)(volatile Task* p);

void TaskModInit();

void LaunchTask();

void Schedule();

#endif //TASK_H