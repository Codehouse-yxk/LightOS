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

enum{
    NORMAL,     //普通互斥锁
    STRICT      //严格互斥锁
};


/**
 * @description: 结束任务
 */
void Exit();

/**
 * @description: 等待目标任务退出
 * @param 任务名
 */
void Wait(const char* name);

/**
 * @description: 注册任务
 * @param 任务名
 * @param 任务入口函数
 * @param 优先级【越小，获取CPU的概率越大】
 */
void RegApp(const char *name, void (*tmain)(), byte priority);

/**
 * @description: 创建互斥锁
 * @param: 互斥锁类型
 * @return 互斥锁ID
 */
uint CreateMutex(uint type);

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

/**
 * @description: 读取按键数据
 * @return 键码
 */
uint ReadKey();

/**
 * @description: 获取物理内存容量
 * @return 物理内存容量
 */
uint GetMemSize();


#endif //SYSCALL_H