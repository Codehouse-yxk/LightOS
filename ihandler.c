/*
 * @Author: yangxingkun
 * @Date: 2022-07-13 08:37:12
 * @FilePath: \LightOS\ihandler.c
 * @Description: 中断子模块，包括中断服务函数定义
 * @Github: https://github.com/Codehouse-yxk
 */

#include "interrupt.h"
#include "ihandler.h"
#include "task.h"
#include "screen.h"
#include "mutex.h"

extern volatile Task* gCTaskAddr;

void PageFaultHandler()
{
    SetPrintPos(0, 8);
    PrintString("Page Fault, Kill :");
    PrintString((const char*)gCTaskAddr->name);

    KillTask();
}

void SegmentFaultHandler()
{
    SetPrintPos(0, 8);
    PrintString("Segment Fault, Kill :");
    PrintString((const char*)gCTaskAddr->name);

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

void SysCallHandler(uint type, uint cmd, uint param1, uint param2)
{
    switch (type)
    {
        case 0:
            KillTask();
            break;
        case 1:
            MutexCallHandler(cmd, param1);
            break;
        default:
            break;
    }
}
