/**
 *  中断子模块，包括中断服务函数定义
*/

#include "interrupt.h"
#include "ihandler.h"
#include "task.h"
#include "screen.h"

extern volatile Task* gCTaskAddr;

void PageFaultHandler()
{
    SetPrintPos(0, 8);
    PrintString("Page Fault, Kill :");
    PrintString(gCTaskAddr->name);

    KillTask();
}

void SegmentFaultHandler()
{
    SetPrintPos(0, 8);
    PrintString("Segment Fault, Kill :");
    PrintString(gCTaskAddr->name);

    KillTask();
}

void TimerHandler()
{
    static uint i = 0;
    i = (i + 1) % 5;
    
    if( i == 0 )
    {
        Schedule();
    }

    SendEOI(MASTER_EOI_PORT);
}

void SysCallHandler(ushort ax)
{
    if(ax == 0)
    {
        KillTask();
    }
}
