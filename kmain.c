#include "kernel.h"
#include "screen.h"
#include "global.h"

Task taskA = {0};
Task taskB = {0};
TSS gTss = {0};

typedef void (*Entry)();

void (*const InitInterrupt)() = NULL;
void (*const EnableTimer)() = NULL;
void (*const SendEOI)(uint port) = NULL;

extern void TimeHandleEntry();

volatile Task *gCTaskAddr = NULL;

void Delay(int n)
{
    while (n-- > 0)
    {

        int i = 0;
        int j = 0;
        for (i = 0; i < 1000; i++)
        {
            for (j = 0; j < 1000; j++)
            {
                asm volatile("nop\n");
            }
        }
    }
}

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

void ChangeTask()
{
    gCTaskAddr = (gCTaskAddr == &taskA) ? &taskB : &taskA;
    
    gTss.ss0 = GDT_DATA32_FLAT_SELECTOR;
    gTss.esp0 = (uint)&gCTaskAddr->rv.gs + sizeof(RegValue);    //指定初始内核栈
    
    //每个任务有自己的ldt，任务切换时需要重新加载
    SetDescValue(&gGdtInfo.entry[GDT_TASK_LDT_INDEX], (uint)&gCTaskAddr->ldt, sizeof(gCTaskAddr->ldt)-1, DA_LDT + DA_DPL0);
    
    LoadTask(gCTaskAddr);   //只加载任务的ldt
}

void TimerHandler()
{
    static uint i = 0;
    i = (i + 1) % 5;
    
    if( i == 0 )
    {
        ChangeTask();
    }

    SendEOI(MASTER_EOI_PORT);
}

void InitTask(Task *p, Entry entry)
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

    SetDescValue(p->ldt + LDT_VIDEO_INDEX, 0xB8000, 0x07FFF, DA_DRWA + DA_32 + DA_DPL3); //设置ldt显存段
    SetDescValue(p->ldt + LDT_CODE32_INDEX, 0x00000, 0xFFFFF, DA_C + DA_32 + DA_DPL3);   //设置ldt代码段
    SetDescValue(p->ldt + LDT_DATA32_INDEX, 0x00000, 0xFFFFF, DA_DRW + DA_32 + DA_DPL3); //设置ldt数据段

    p->ldtSelector = GDT_TASK_LDT_SELECTOR;
    p->tssSelector = GDT_TASK_TSS_SELECTOR;

    SetDescValue(&gGdtInfo.entry[GDT_TASK_LDT_INDEX], (uint)&p->ldt, sizeof(p->ldt) - 1, DA_LDT + DA_DPL0); //在gdt中设置ldt
    SetDescValue(&gGdtInfo.entry[GDT_TASK_TSS_INDEX], (uint)&gTss, sizeof(gTss) - 1, DA_386TSS + DA_DPL0);  //在gdt中设置tss
}

/**
 * 内核入口函数
 */
void KMain()
{
    SetPrintColor(SCREEN_RED);

    PrintString("LightOS\n\n");

    uint base = 0x12345;
    uint limit = 0xABCD;
    ushort attr = 0x4098;
    int i = 0;
    uint temp = 0;

    PrintString("Gdt Entry: ");
    PrintIntHex((uint)gGdtInfo.entry);
    PrintChar('\n');

    PrintString("Gdt Size: ");
    PrintIntDec(gGdtInfo.size);
    PrintChar('\n');

    PrintString("Idt Entry: ");
    PrintIntHex((uint)gIdtInfo.entry);
    PrintChar('\n');

    PrintString("Idt Size: ");
    PrintIntDec(gIdtInfo.size);
    PrintChar('\n');

    PrintString("RunTask: ");
    PrintIntHex((uint)RunTask);
    PrintChar('\n');

    InitTask(&taskA, TaskA);

    InitTask(&taskB, TaskB);

    SetIntHandler(gIdtInfo.entry + 0x20, (uint)TimeHandleEntry);

    InitInterrupt();

    EnableTimer();

    gCTaskAddr = &taskB;

    RunTask(gCTaskAddr);
}
