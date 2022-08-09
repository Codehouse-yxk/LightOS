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
 * @param 参数1
 * @param 参数2
 */
void MutexCallHandler(uint cmd, uint param1, uint param2);

/**
 * @description: 创建互斥锁（内核）
 * @return 互斥锁ID
 */
Mutex* SysCreateMutex();

/**
 * @description: 销毁互斥锁（内核）
 * @param 互斥锁ID
 * @return 销毁成功：1，销毁失败：0
 */
uint SysDestroyMutex(Mutex* mutex);

/**
 * @description: 进入临界区（内核）
 * @param 互斥锁ID
 * @param 等待标志(需要传回用户层)
 */
void SysEnterCritical(Mutex* mutex, uint* wait);

/**
 * @description: 退出临界区（内核）
 * @param 互斥锁ID
 */
void SysExitCritical(Mutex* mutex);



#endif  //MUTEX_H