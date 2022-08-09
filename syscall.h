#ifndef SYSCALL_H
#define SYSCALL_H

#include "type.h"


#define SysCall(type, cmd, param1, param2)  \
        asm volatile(                       \
            "movl $" #type ",   %%eax \n"   \
            "movl $" #cmd ",    %%ebx \n"   \
            "movl %0,           %%ecx \n"   \
            "movl %1,           %%edx \n"   \
            "int  $0x80               \n"   \
            :                               \
            : "r"(param1), "r"(param2)      \
            : "eax", "ebx", "ecx", "edx"    \
        )


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
 * @return 销毁成功：1，销毁失败：0
 */
uint DestroyMutex(uint mutex);


#endif //SYSCALL_H