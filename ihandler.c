/**
 *  中断子模块，包括中断服务函数定义
*/

#include "interrupt.h"
#include "ihandler.h"
#include "task.h"


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
