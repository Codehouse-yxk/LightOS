/*
 * @Author: yangxingkun
 * @Date: 2022-08-17 21:41:39
 * @FilePath: \LightOS\event.c
 * @Description: 事件模块
 * @Github: https://github.com/Codehouse-yxk
 */

#include "event.h"
#include "memory.h"

Event* CreateEvent(uint type, uint id, uint param1, uint param2)
{
    Event* ret = Malloc(sizeof(Event));

    ret->type = type;
    ret->id = id;
    ret->param1 = param1;
    ret->param2 = param2;

    return ret;
}

void DestroyEvent(Event* event)
{
    Free(event);
}