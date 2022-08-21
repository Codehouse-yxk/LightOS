/*
 * @Author: yangxingkun
 * @Date: 2022-08-21 15:16:11
 * @FilePath: \LightOS\sysinfo.c
 * @Description: 
 * @Github: https://github.com/Codehouse-yxk
 */

#include "sysinfo.h"

uint gMemSize = 0;

static void GetMemSize(uint param1, uint param2)
{
    if(param1)
    {
        uint* ret = (uint*)param1;
        *ret = gMemSize;
    }
}

void SysInfoCallHandler(uint cmd, uint param1, uint param2)
{
    switch(cmd)
    {
        case 0:
            GetMemSize(param1, param2);
            break;
        default:
            break;
    }
}