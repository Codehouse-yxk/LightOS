/*
 * @Author: yangxingkun
 * @Date: 2022-07-13 08:11:02
 * @FilePath: \LightOS\interrupt.c
 * @Description: 中断主模块，包含：中断初始化、中断参数设置等
 * @Github: https://github.com/Codehouse-yxk
 */

#include "interrupt.h"
#include "ihandler.h"
#include "utility.h"

void (*const InitInterrupt)() = NULL;
void (*const EnableTimer)() = NULL;
void (*const SendEOI)(uint port) = NULL;

void IntModInit()
{
    //设置中断入口
    SetIntHandler(AddrOff(gIdtInfo.entry, 0x20), (uint)TimerHandlerEntry);
    SetIntHandler(AddrOff(gIdtInfo.entry, 0x80), (uint)SysCallHandlerEntry);
    SetIntHandler(AddrOff(gIdtInfo.entry, 0x0E), (uint)PageFaultHandlerEntry);
    SetIntHandler(AddrOff(gIdtInfo.entry, 0x0D), (uint)SegmentFaultHandlerEntry);

    InitInterrupt();

    // EnableTimer();   放到启动任务中去使能，否则会造成时序问题
}

int SetIntHandler(Gate *pGate, uint ifunc)
{
    int ret = 0;
    if (ret = (pGate != NULL))
    {
        pGate->offset1 = ifunc & 0xFFFF;
        pGate->selector = GDT_CODE32_FLAT_SELECTOR;
        pGate->dcount = 0;
        pGate->attr = DA_386IGate + DA_DPL3;        //todo:为什么int80和int20都用DPL3，int20不是从用户态进入中断的。
        pGate->offset2 = (ifunc >> 16) & 0xFFFF;
    }
    return ret;
}

int GetIntHandler(Gate *pGate, uint *pIfunc)
{
    int ret = 0;

    if (ret = (pGate && pIfunc))
    {
        *pIfunc = (pGate->offset2 << 16) | pGate->offset1;
    }

    return 0;
}
