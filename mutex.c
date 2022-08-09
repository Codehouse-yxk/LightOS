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


extern volatile Task* gCTaskAddr;

static List gMList = {0};

void MutexModInit()
{
    List_Init(&gMList);
}

void MutexCallHandler(uint cmd, uint param1, uint param2)
{
    switch (cmd)
    {
        case 0:
        {
            uint* pRet = (uint*)param1;
            *pRet = (uint)SysCreateMutex();            //将地址作为mutex的ID返回给应用
            PrintString("mutex create id :");
            PrintIntHex(*pRet);
            PrintChar('\n');
            break;
        }
        case 1:
        {
            uint* pRet = (uint*)param2;
            SysEnterCritical((Mutex*)param1, pRet);
            break;
        }
        case 2:
        {
            SysExitCritical((Mutex*)param1);
            break;
        }
        case 3:
        {
            uint* pRet = (uint*)param2;
            *pRet = SysDestroyMutex((Mutex*)param1);
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

uint SysDestroyMutex(Mutex* mutex)
{
    uint ret = 0;
    if(mutex)
    {
        ListNode* pos = NULL;

        List_ForEach(&gMList, pos)
        {
            if(isEqual(pos, mutex) && isEqual(mutex->lock, 0))
            {
                PrintString("destroy mutex :");
                PrintIntHex((uint)mutex);
                PrintChar('\n');
                List_DelNode(pos);
                Free(pos);
                ret = 1;
                break;
            }
        }
    }
    return ret;
}

void SysEnterCritical(Mutex* mutex, uint* wait)
{
    if(mutex && IsMutexValid(mutex))
    {
        if(mutex->lock)
        {
            if(isEqual(mutex->lock, gCTaskAddr))
            {
                PrintString("the same task get mutex again!\n");
                *wait = 0;  //同一任务重复加锁，直接返回继续执行
            }
            else
            {
                PrintString("wait to get mutex, move task to waitting queue !\n");
                *wait = 1;          //传给用户，当阻塞的任务重新被执行时，用户层再次进行mutex检测
                MtxSchedule(WAIT);
            }
        }
        else
        {
            mutex->lock = (uint)gCTaskAddr;
            *wait = 0;
            PrintString("get mutex, enter critical !   ");
            PrintIntHex((uint)mutex->lock);
            PrintChar('\n');
        }
    }
}

void SysExitCritical(Mutex* mutex)
{
    if(mutex && IsMutexValid(mutex))
    {
        if(isEqual(mutex->lock, gCTaskAddr))
        {
            mutex->lock = 0;
            PrintString("current task exit critical !\n");
            MtxSchedule(NOTIFY);
        }
        else    //非申请锁的任务操作锁，直接杀掉
        {
            PrintString("Task: "); 
            PrintIntHex((uint)gCTaskAddr);
            PrintString("  ");
            PrintIntHex((uint)mutex->lock);
            PrintString("  ");
            PrintString(gCTaskAddr->name);
            PrintString("   OOPS\n");
            KillTask();
        }
    }
}
