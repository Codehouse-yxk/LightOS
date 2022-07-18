
/**
 * 任务模块，包含：任务初始化、任务调度等功能
 */

#include "task.h"
#include "utility.h"

#define MAX_RUNNING_TASK 16

void (*const RunTask)(volatile Task *p) = NULL;

void (*const LoadTask)(volatile Task *p) = NULL;

static TSS gTss = {0};

volatile Task *gCTaskAddr = NULL;

static TaskNode gTaskBuffer[MAX_RUNNING_TASK] = {0};

static Queue gTaskQueue = {0};

/* 所有任务入口函数 */
static void TaskEntry()
{
    if(gCTaskAddr)
    {
        gCTaskAddr->tmain();
    }

    //触发0x80软中断，陷入内核态销毁任务
    asm volatile(
        "movw $0, %ax \n"
        "int $0x80 \n"
    );
    while(1);
}

void TaskA()
{
    int i = 0;
    SetPrintPos(0, 15);
    PrintString("Task A: ");
    while (i < 5)
    {
        SetPrintPos(10, 15);
        PrintChar('A' + i);
        i = (i + 1) % 26;
        Delay(2);
    }
    SetPrintPos(10, 15);
    PrintString("end Task A");
}

void TaskB()
{
    int i = 0;
    SetPrintPos(0, 16);
    PrintString("Task B: ");
    while (1)
    {
        SetPrintPos(10, 16);
        PrintChar('a' + i);
        i = (i + 1) % 10;
        Delay(2);
    }
}

void TaskC()
{
    int i = 0;
    SetPrintPos(0, 17);
    PrintString("Task C: ");
    while (1)
    {
        SetPrintPos(10, 17);
        PrintChar('m' + i);
        i = (i + 1) % 10;
        Delay(2);
    }
}

void TaskD()
{
    int i = 0;
    SetPrintPos(0, 18);
    PrintString("Task D: ");
    while (1)
    {
        SetPrintPos(10, 18);
        PrintChar('.' + i);
        i = (i + 1) % 10;
        Delay(2);
    }
}

static void InitTask(Task *p, void (*entry)())
{
    p->rv.cs = LDT_CODE32_SELECTOR;
    p->rv.gs = LDT_VIDEO_SELECTOR;
    p->rv.ds = LDT_DATA32_SELECTOR;
    p->rv.es = LDT_DATA32_SELECTOR;
    p->rv.fs = LDT_DATA32_SELECTOR;
    p->rv.ss = LDT_DATA32_SELECTOR;

    p->rv.esp = (uint)p->stack + sizeof(p->stack); //任务私有栈
    p->rv.eip = (uint)TaskEntry;
    p->rv.eflags = 0x3202; //设置IOPL = 3（可以进行IO操作）， IF = 1（响应外部中断）；
    
    p->tmain = entry;

    SetDescValue(AddrOff(p->ldt, LDT_VIDEO_INDEX), 0xB8000, 0x07FFF, DA_DRWA + DA_32 + DA_DPL3); //设置ldt显存段
    SetDescValue(AddrOff(p->ldt, LDT_CODE32_INDEX), 0x00000, 0xFFFFF, DA_C + DA_32 + DA_DPL3);   //设置ldt代码段
    SetDescValue(AddrOff(p->ldt, LDT_DATA32_INDEX), 0x00000, 0xFFFFF, DA_DRW + DA_32 + DA_DPL3); //设置ldt数据段

    p->ldtSelector = GDT_TASK_LDT_SELECTOR;
    p->tssSelector = GDT_TASK_TSS_SELECTOR;

    SetDescValue(AddrOff(gGdtInfo.entry, GDT_TASK_LDT_INDEX), (uint)&p->ldt, sizeof(p->ldt) - 1, DA_LDT + DA_DPL0); //在gdt中设置ldt
}

void TaskModInit()
{
    //Tss只需要设置一次
    SetDescValue(AddrOff(gGdtInfo.entry, GDT_TASK_TSS_INDEX), (uint)&gTss, sizeof(gTss) - 1, DA_386TSS + DA_DPL0);  //在gdt中设置tss

    InitTask(&((TaskNode*)AddrOff(gTaskBuffer, 0))->task, TaskA);
    InitTask(&((TaskNode*)AddrOff(gTaskBuffer, 1))->task, TaskB);
    InitTask(&((TaskNode*)AddrOff(gTaskBuffer, 2))->task, TaskC);
    InitTask(&((TaskNode*)AddrOff(gTaskBuffer, 3))->task, TaskD);

    Queue_Init(&gTaskQueue);
    Queue_Add(&gTaskQueue, &((TaskNode*)AddrOff(gTaskBuffer, 0))->head);
    Queue_Add(&gTaskQueue, &((TaskNode*)AddrOff(gTaskBuffer, 1))->head);
    Queue_Add(&gTaskQueue, &((TaskNode*)AddrOff(gTaskBuffer, 2))->head);
    Queue_Add(&gTaskQueue, &((TaskNode*)AddrOff(gTaskBuffer, 3))->head);
}

static void PrepareForRun(volatile Task *p)
{
    //设置内核栈
    gTss.ss0 = GDT_DATA32_FLAT_SELECTOR; //设置高特权级下的栈
    gTss.esp0 = (uint)&p->rv + sizeof(p->rv);
    gTss.iomb = sizeof(gTss);

    //每个任务有自己的ldt，任务切换时需要重新加载
    SetDescValue(AddrOff(gGdtInfo.entry, GDT_TASK_LDT_INDEX), (uint)&p->ldt, sizeof(p->ldt) - 1, DA_LDT + DA_DPL0);
}

void LaunchTask()
{
    gCTaskAddr = &((TaskNode*)AddrOff(gTaskBuffer, 0))->task;
    PrepareForRun(gCTaskAddr);
    RunTask(gCTaskAddr);        //启动第一个任务
}

void Schedule()
{
    Queue_Rotate(&gTaskQueue);
    QueueNode *node = Queue_Front(&gTaskQueue);
    if (node)
    {
        gCTaskAddr = &Queue_Node(node, TaskNode, head)->task;
        PrepareForRun(gCTaskAddr);
        LoadTask(gCTaskAddr); //只加载任务的ldt
    }
}

void KillTask()
{
    PrintString("   kill task");
}