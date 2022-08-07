#ifndef SYSCALL_H
#define SYSCALL_H

#include "type.h"

/**
 * @description: 结束任务
 */
void Exit();

/**
 * @description: 创建互斥锁
 * @return 互斥锁ID
 */
uint CreateMutex();

/**
 * @description: 进入临界区
 * @param 互斥锁ID
 */
void EnterCritical(uint mutex);

/**
 * @description: 退出临界区
 * @param 互斥锁ID
 */
void ExitCritical(uint mutex);

/**
 * @description: 销毁互斥锁
 * @param 互斥锁ID
 */
void DestroyMutex(uint mutex);


#endif //SYSCALL_H