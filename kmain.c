/*
 * @Author: yangxingkun
 * @Date: 2022-06-19 20:26:04
 * @FilePath: \LightOS\kmain.c
 * @Description: 内核入口（c）
 * @Github: https://github.com/Codehouse-yxk
 */

#include "screen.h"
#include "task.h"
#include "interrupt.h"
#include "memory.h"
#include "mutex.h"
#include "keyboard.h"
#include "sysinfo.h"

/**
 * 内核入口函数
 */
void KMain()
{
    void (*AppModInit)() = (void*)BaseOfApp;

    SetPrintColor(SCREEN_RED);

    PrintString("LightOS\n");

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

    PrintString("Number Of Hard Disk: ");
    PrintIntDec(*((byte*)0x475));   //BIOS中0x475地址上保存的是硬盘数量
    PrintChar('\n');

    MemModInit((byte*)kernalHeapBase, HeapSize);

    KeyboardModInit();

    MutexModInit();

    AppModInit();

    TaskModInit();

    IntModInit();

    ConfigPageTable();

    LaunchTask();

    SetPrintPos(0, 20);
    PrintString("main over");
}
