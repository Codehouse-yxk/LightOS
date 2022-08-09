/*
 * @Author: yangxingkun
 * @Date: 2022-08-07 13:53:04
 * @FilePath: \LightOS\mutex.c
 * @Description: 互斥锁
 * @Github: https://github.com/Codehouse-yxk
 */

#include "mutex.h"
#include "screen.h"
#include "memory.h"
#include "task.h"

static List gMList = {0};

void MutexModInit()
{
    List_Init(&gMList);
}

void MutexCallHandler(uint cmd, uint param)
{
    switch (cmd)
    {
        case 0:
        {
            uint* pRet = (uint*)param;
            *pRet = (uint)SysCreateMutex();            //将地址作为mutex的ID返回给应用
            PrintString("mutex create id :");
            PrintIntHex(*pRet);
            PrintChar('\n');
            break;
        }
        case 1:
        {
            SysEnterCritical((Mutex*)param);
            break;
        }
        case 2:
        {
            SysExitCritical((Mutex*)param);
            break;
        }
        case 3:
        {
            SysDestroyMutex((Mutex*)param);
            break;
        }
        default:
            break;
    }
}

Mutex* SysCreateMutex()
{
    Mutex* ret = Malloc(sizeof(Mutex));

    if(ret)
    {
        ret->lock = 0;
        List_Add(&gMList, (ListNode*)ret);
    }

    return ret;
}

/**
 * @description: 判断互斥锁是否合法【互斥锁是否存在于管理链表中】
 * @param 互斥锁ID
 * @return 合法：1，不合法：0
 */
static uint IsMutexValid(Mutex* mutex)
{
    uint ret = 0;

    ListNode* pos = NULL;
    List_ForEach(&gMList, pos)
    {
        if(isEqual(pos, mutex))
        {
            ret = 1;
            break;
        }
    }

    return ret;
}

void SysDestroyMutex(Mutex* mutex)
{
    if(mutex)
    {
        ListNode* pos = NULL;

        List_ForEach(&gMList, pos)
        {
            if(isEqual(pos, mutex))
            {
                PrintString("destroy mutex :");
                PrintIntHex((uint)mutex);
                PrintChar('\n');
                List_DelNode(pos);
                Free(pos);
                break;
            }
        }
    }
}

void SysEnterCritical(Mutex* mutex)
{
    if(mutex && IsMutexValid(mutex))
    {
        if(mutex->lock)
        {
            PrintString("wait to get mutex, move task to waitting queue !\n");
            MtxSchedule(WAIT);
        }
        else
        {
            mutex->lock = 1;
            PrintString("get mutex, enter critical !\n");
        }
    }
}

void SysExitCritical(Mutex* mutex)
{
    if(mutex && IsMutexValid(mutex))
    {
        if(mutex->lock == 1)
        {
            mutex->lock = 0;
            PrintString("current task exit critical !\n");
            MtxSchedule(NOTIFY);
        }
    }
}
