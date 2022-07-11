#include "kernel.h"
#include "screen.h"
#include "global.h"

Task p = {0};

void (*const InitInterrupt)() = NULL;
void (*const EnableTimer)() = NULL;
void (*const SendEOI)(uint port) = NULL;
void TimeHandleEntry();

Task* gCTaskAddr = NULL;

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

void TimerHandler()
{
    static uint i = 0;
    static uint j = 0;
    if (i++ > 10)
    {
        j++;
        SetPrintPos(0, 18);
        PrintString("Timer cnt : ");
        PrintIntDec(j);
        i = 0;
    }
    
    SendEOI(MASTER_EOI_PORT);
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
    PrintIntHex(gGdtInfo.entry);
    PrintChar('\n');

    PrintString("Gdt Size: ");
    PrintIntDec(gGdtInfo.size);
    PrintChar('\n');

    PrintString("Idt Entry: ");
    PrintIntHex(gIdtInfo.entry);
    PrintChar('\n');

    PrintString("Idt Size: ");
    PrintIntDec(gIdtInfo.size);
    PrintChar('\n');

    // for (i = 0; i < gGdtInfo.size; i++)
    // {
    //     if (GetDescValue(gGdtInfo.entry + i, &base, &limit, &attr))
    //     {
    //         PrintIntHex(base);
    //         PrintString("       ");
    //         PrintIntHex(limit);
    //         PrintString("       ");
    //         PrintIntHex(attr);
    //         PrintString("\n");
    //     }
    //     else
    //     {
    //         PrintString("get descriptor fail\n");
    //     }
    // }

    PrintString("RunTask: ");
    PrintIntHex(RunTask);
    PrintChar('\n');

    p.rv.cs = LDT_CODE32_SELECTOR;
    p.rv.gs = LDT_VIDEO_SELECTOR;
    p.rv.ds = LDT_DATA32_SELECTOR;
    p.rv.es = LDT_DATA32_SELECTOR;
    p.rv.fs = LDT_DATA32_SELECTOR;
    p.rv.ss = LDT_DATA32_SELECTOR;

    p.rv.esp = (uint)p.stack + sizeof(p.stack); //任务私有栈
    p.rv.eip = (uint)TaskA;
    p.rv.eflags = 0x3202; //设置IOPL = 3（可以进行IO操作）， IF = 1（响应外部中断）；

    //设置内核栈
    p.tss.ss0 = GDT_DATA32_FLAT_SELECTOR; //设置高特权级下的栈
    p.tss.esp0 = (uint)&p.rv + sizeof(p.rv);
    p.tss.iomb = sizeof(p.tss);

    SetDescValue(p.ldt + LDT_VIDEO_INDEX, 0xB8000, 0x07FFF, DA_DRWA + DA_32 + DA_DPL3); //设置ldt显存段
    SetDescValue(p.ldt + LDT_CODE32_INDEX, 0x00000, 0xFFFFF, DA_C + DA_32 + DA_DPL3);   //设置ldt代码段
    SetDescValue(p.ldt + LDT_DATA32_INDEX, 0x00000, 0xFFFFF, DA_DRW + DA_32 + DA_DPL3); //设置ldt数据段

    p.ldtSelector = GDT_TASK_LDT_SELECTOR;
    p.tssSelector = GDT_TASK_TSS_SELECTOR;

    SetDescValue(&gGdtInfo.entry[GDT_TASK_LDT_INDEX], (uint)&p.ldt, sizeof(p.ldt) - 1, DA_LDT + DA_DPL0);    //在gdt中设置ldt
    SetDescValue(&gGdtInfo.entry[GDT_TASK_TSS_INDEX], (uint)&p.tss, sizeof(p.tss) - 1, DA_386TSS + DA_DPL0); //在gdt中设置tss

    SetIntHandler(gIdtInfo.entry + 0x20, (uint)TimeHandleEntry);

    InitInterrupt();

    EnableTimer();

    gCTaskAddr = &p;

    RunTask(gCTaskAddr);
}
