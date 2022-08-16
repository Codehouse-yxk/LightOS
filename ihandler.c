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

extern volatile Task* gCTaskAddr;
extern byte ReadPort(ushort port);

void PageFaultHandler()
{
    SetPrintPos(0, 8);
    PrintString("Page Fault, Kill: ");
    PrintString((const char*)gCTaskAddr->name);

    KillTask();
}

void SegmentFaultHandler()
{
    SetPrintPos(0, 8);
    PrintString("Segment Fault, Kill: ");
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
            TaskCallHandler(cmd, param1, param2);
            break;
        case 1:
            MutexCallHandler(cmd, param1, param2);
            break;
        default:
            break;
    }
}

void KeyboardHandler()
{
    byte scanCode = ReadPort(0x60);
    PutScanCode(scanCode);
    
    /*  测试键盘按键
    uint code = 0;
    while(code = FeachKeyCode())
    {
        if(((byte)(code >> 8))==0x13 && (code & 0x1000000))
        {
            PrintIntHex(code);
            PrintString("Pause pressed\n");
        }
        else if((byte)(code >> 8)==0x13)
        {
            PrintIntHex(code);
            PrintString("Pause release\n");
        }

        if((char)code && (code & 0x1000000))
        {
            PrintChar((char)code);
        }
        else if(((byte)(code >> 8))==0x0D && (code & 0x1000000)) 
        {
            PrintChar('\n');
        }
    }
    */
    SendEOI(MASTER_EOI_PORT);
}
