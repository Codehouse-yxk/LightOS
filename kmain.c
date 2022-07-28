
/**
 *  内核入口（c）
 */

#include "screen.h"
#include "task.h"
#include "interrupt.h"

/**
 * 内核入口函数
 */
void KMain()
{
    void (*AppModInit)() = (void*)BaseOfApp;

    SetPrintColor(SCREEN_RED);

    PrintString("LightOS\n\n");

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

    AppModInit();

    TaskModInit();

    IntModInit();

    LaunchTask();

    SetPrintPos(0, 20);
    PrintString("main over");
}
