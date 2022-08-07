#ifndef MUTEX_H
#define MUTEX_H

#include "type.h"
#include "list.h"

typedef struct
{
    ListNode head;
    uint lock;
} Mutex;

/**
 * @description: 互斥锁模块初始化
 */
void MutexModInit();

/**
 * @description: 互斥锁系统调用处理函数
 * @param 互斥锁操作指令
 * @param 参数
 */
void MutexCallHandler(uint cmd, uint param);

/**
 * @description: 创建互斥锁（内核）
 * @return 互斥锁ID
 */
Mutex* SysCreateMutex();

/**
 * @description: 销毁互斥锁（内核）
 * @param 互斥锁ID
 */
void SysDestroyMutex(Mutex* mutex);

/**
 * @description: 进入临界区（内核）
 * @param 互斥锁ID
 */
void SysEnterCritical(Mutex* mutex);

/**
 * @description: 退出临界区（内核）
 * @param 互斥锁ID
 */
void SysExitCritical(Mutex* mutex);



#endif  //MUTEX_H