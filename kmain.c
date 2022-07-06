#include "kernel.h"
#include "screen.h"
#include "global.h"

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

    PrintString("Gdt Entry: \n");

    for (i = 0; i < gGdtInfo.size; i++)
    {
        if (GetDescValue(gGdtInfo.entry + i, &base, &limit, &attr))
        {
            PrintIntHex(base);
            PrintString("       ");
            PrintIntHex(limit);
            PrintString("       ");
            PrintIntHex(attr);
            PrintString("\n");
        }
        else
        {
            PrintString("get descriptor fail\n");
        }

        
    }
}
