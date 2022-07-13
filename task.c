
/**
 * 任务模块，包含：任务初始化、任务调度等功能
 */

#include "task.h"
#include "utility.h"

void (*const RunTask)(volatile Task *p) = NULL;

void (*const LoadTask)(volatile Task *p) = NULL;

Task taskA = {0};
Task taskB = {0};
TSS gTss = {0};
volatile Task *gCTaskAddr = NULL;

void TaskA()
{
    int i = 0;
    SetPrintPos(0, 15);
    PrintString("Task A: ");
    while (1)
    {
        SetPrintPos(10, 15);
        PrintChar('A' + i);
        i = (i + 1) % 26;
        Delay(2);
    }
}

void TaskB()
{
    int i = 0;
    SetPrintPos(0, 16);
    PrintString("Task B: ");
    while (1)
    {
        SetPrintPos(10, 16);
        PrintIntDec(i);
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
    p->rv.eip = (uint)entry;
    p->rv.eflags = 0x3202; //设置IOPL = 3（可以进行IO操作）， IF = 1（响应外部中断）；

    //设置内核栈
    gTss.ss0 = GDT_DATA32_FLAT_SELECTOR; //设置高特权级下的栈
    gTss.esp0 = (uint)&p->rv + sizeof(p->rv);
    gTss.iomb = sizeof(gTss);

    SetDescValue(AddrOff(p->ldt, LDT_VIDEO_INDEX), 0xB8000, 0x07FFF, DA_DRWA + DA_32 + DA_DPL3); //设置ldt显存段
    SetDescValue(AddrOff(p->ldt, LDT_CODE32_INDEX), 0x00000, 0xFFFFF, DA_C + DA_32 + DA_DPL3);   //设置ldt代码段
    SetDescValue(AddrOff(p->ldt, LDT_DATA32_INDEX), 0x00000, 0xFFFFF, DA_DRW + DA_32 + DA_DPL3); //设置ldt数据段

    p->ldtSelector = GDT_TASK_LDT_SELECTOR;
    p->tssSelector = GDT_TASK_TSS_SELECTOR;

    SetDescValue(AddrOff(gGdtInfo.entry, GDT_TASK_LDT_INDEX), (uint)&p->ldt, sizeof(p->ldt) - 1, DA_LDT + DA_DPL0); //在gdt中设置ldt
    SetDescValue(AddrOff(gGdtInfo.entry, GDT_TASK_TSS_INDEX), (uint)&gTss, sizeof(gTss) - 1, DA_386TSS + DA_DPL0);  //在gdt中设置tss
}

void TaskModInit()
{
    InitTask(&taskA, TaskA);

    InitTask(&taskB, TaskB);
}

void LaunchTask()
{
    gCTaskAddr = &taskB;

    RunTask(gCTaskAddr);
}

void Schedule()
{
    gCTaskAddr = (gCTaskAddr == &taskA) ? &taskB : &taskA;

    if (gCTaskAddr)
    {
        gTss.ss0 = GDT_DATA32_FLAT_SELECTOR;
        gTss.esp0 = (uint)&gCTaskAddr->rv.gs + sizeof(RegValue); //指定初始内核栈

        //每个任务有自己的ldt，任务切换时需要重新加载
        SetDescValue(AddrOff(gGdtInfo.entry, GDT_TASK_LDT_INDEX), (uint)&gCTaskAddr->ldt, sizeof(gCTaskAddr->ldt) - 1, DA_LDT + DA_DPL0);

        LoadTask(gCTaskAddr); //只加载任务的ldt
    }
}