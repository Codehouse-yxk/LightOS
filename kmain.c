#include "kernel.h"
#include "screen.h"


/**
 * 内核入口函数
 */
void KMain()
{
    SetPrintColor(SCREEN_RED);

    PrintString("LightOS\n");

    SetPrintPos(10, 10);

    PrintIntDec(1232456);

    PrintChar('\n');

}
