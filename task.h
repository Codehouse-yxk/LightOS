
#ifndef TASK_H
#define TASK_H

#include "kernel.h"
#include "queue.h"

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

    void       (*tmain)();
    uint       id;
    ushort     current; //表示任务当前已执行的时间数
    ushort     total;   //表示任务每次执行的总时间数
    char       name[10]; 
    byte       stack[512];      //任务执行时使用的栈
} Task;

typedef struct
{
    QueueNode head;
    Task task;
} TaskNode;


/* 汇编中定义的RunTask，通过共享内存获取函数指针 */
void (*const RunTask)(volatile Task* p);

/* 汇编中定义的LoadTask，通过共享内存获取函数指针 */
void (*const LoadTask)(volatile Task* p);

/* 任务模块初始化 */
void TaskModInit();

/* 加载任务 */
void LaunchTask();

/* 任务调度 */
void Schedule();

/* 任务销毁 */
void KillTask();

#endif //TASK_H