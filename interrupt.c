

/**
 *  中断主模块，包含：中断初始化、中断参数设置等
*/

#include "interrupt.h"
#include "ihandler.h"
#include "utility.h"

void (*const InitInterrupt)() = NULL;
void (*const EnableTimer)() = NULL;
void (*const SendEOI)(uint port) = NULL;

void IntModInit()
{
    SetIntHandler(AddrOff(gIdtInfo.entry, 0x20), (uint)TimerHandlerEntry);

    InitInterrupt();

    EnableTimer();
}

int SetIntHandler(Gate *pGate, uint ifunc)
{
    int ret = 0;
    if (ret = (pGate != NULL))
    {
        pGate->offset1 = ifunc & 0xFFFF;
        pGate->selector = GDT_CODE32_FLAT_SELECTOR;
        pGate->dcount = 0;
        pGate->attr = DA_386IGate + DA_DPL0;
        pGate->offset2 = (ifunc >> 16) & 0xFFFF;
    }
    return ret;
}

int GetIntHandler(Gate *pGate, uint *pIfunc)
{
    int ret = 0;

    if(ret = (pGate && pIfunc))
    {
        *pIfunc = (pGate->offset2 << 16) | pGate->offset1;
    }

    return 0;
}

