/*
 * @Author: yangxingkun
 * @Date: 2022-08-07 13:53:04
 * @FilePath: \LightOS\mutex.c
 * @Description: 互斥锁【内核】
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
            *pRet = (uint)SysCreateMutex(param2);    //将地址作为mutex的ID返回给应用
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

Mutex* SysCreateMutex(uint type)
{
    Mutex* ret = Malloc(sizeof(Mutex));

    if(ret)
    {
        ret->lock = 0;
        ret->type = type;
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
                List_DelNode(pos);
                Free(pos);
                ret = 1;
                break;
            }
        }
    }
    return ret;
}

static void SysNormalEnter(Mutex* mutex, uint* wait)
{
    if(mutex->lock)
    {
        *wait = 1;
        MtxSchedule(WAIT);
    }
    else
    {
        mutex->lock = 1;
        *wait = 0;
    }
}

static void SysStrictEnter(Mutex* mutex, uint* wait)
{
    if(mutex->lock)
    {
        if(isEqual(mutex->lock, gCTaskAddr))
        {
            *wait = 0;  //同一任务重复加锁，直接返回继续执行
        }
        else
        {
            *wait = 1;  //传给用户，当阻塞的任务重新被执行时，用户层再次进行mutex检测
            MtxSchedule(WAIT);
        }
    }
    else
    {
        mutex->lock = (uint)gCTaskAddr;
        *wait = 0;
    }
}

void SysEnterCritical(Mutex* mutex, uint* wait)
{
    if(mutex && IsMutexValid(mutex))
    {
        switch (mutex->type)
        {
        case NORMAL:
            SysNormalEnter(mutex, wait);    //普通互斥锁：只要锁没有用直接上锁【同一个任务不能重复上锁，会阻塞】
            break;
        case STRICT:
            SysStrictEnter(mutex, wait);    //严格互斥锁：需要判断是否是同一个任务上锁【同一个任务可以重复上锁，不会阻塞】
        default:
            break;
        }
    }
}

static void SysNormalExit(Mutex* mutex)
{
    mutex->lock = 0;
    MtxSchedule(NOTIFY);
}

static void SysStrictExit(Mutex* mutex)
{
    if(isEqual(mutex->lock, gCTaskAddr))
    {
        mutex->lock = 0;
        MtxSchedule(NOTIFY);
    }
    else    //非申请锁的任务操作锁，直接杀掉
    {
        KillTask();
    }
}

void SysExitCritical(Mutex* mutex)
{
    if(mutex && IsMutexValid(mutex))
    {
        switch (mutex->type)
        {
        case NORMAL:
            SysNormalExit(mutex);   //普通互斥锁直接突出
            break;
        case STRICT:
            SysStrictExit(mutex);   //严格互斥锁必须由上锁任务来解锁
        default:
            break;
        }
    }
}
