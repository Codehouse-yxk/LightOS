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
#include "keyboard.h"

extern byte ReadPort(ushort port);

void PageFaultHandler()
{
    SetPrintPos(ERR_START_W, ERR_START_H);
    PrintString("Page Fault, Kill: ");
    PrintString(GetCurrentTaskName());

    KillTask();
}

void SegmentFaultHandler()
{
    SetPrintPos(ERR_START_W, ERR_START_H);
    PrintString("Segment Fault, Kill: ");
    PrintString(GetCurrentTaskName());

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
            TaskCallHandler(cmd, param1, param2);
            break;
        case 1:
            MutexCallHandler(cmd, param1, param2);
            break;
        case 2:
            KeyboardCallHandler(cmd, param1, param2);
        default:
            break;
    }
}

void KeyboardHandler()
{
    byte scanCode = ReadPort(0x60);
    PutScanCode(scanCode);
    NotifyGetKeyCode();
    SendEOI(MASTER_EOI_PORT);
}
