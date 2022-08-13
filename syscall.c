/*
 * @Author: yangxingkun
 * @Date: 2022-08-07 14:23:26
 * @FilePath: \LightOS\syscall.c
 * @Description: 系统调用接口（用户态）
 * @Github: https://github.com/Codehouse-yxk
 */

#include "syscall.h"
#include "screen.h"

void Exit()
{
    //触发0x80软中断，陷入内核态销毁任务
    // asm volatile(
    //     "movl $0, %eax \n"   //type:销毁任务
    //     "int $0x80 \n"
    // );
    SysCall(0, 0, 0, 0);
}

void Wait(const char* name)
{
    if(name)
    {
        SysCall(0, 1, name, 0);
    }
}

uint CreateMutex(uint type)
{
    volatile uint ret = 0;
    // asm volatile(
    //     "movl $1, %%eax \n"   //type:互斥锁
    //     "movl $0, %%ebx \n"   //cmd:创建互斥锁
    //     "movl %0, %%ecx \n"   //将ret参数通过ecx传递，此处时占位符
    //     "int $0x80 \n"
    //     :
    //     : "r"(&ret)           //输入参数ret
    //     : "eax", "ebx", "ecx", "edx"
    // );
    SysCall(1, 0, &ret, type);
    
    return ret;
}

void EnterCritical(uint mutex)
{
    // asm volatile(
    //     "movl $1, %%eax \n"   //type:互斥锁
    //     "movl $1, %%ebx \n"   //cmd:进入临界区
    //     "movl %0, %%ecx \n"   //将mutex参数通过ecx传递，此处时占位符
    //     "int $0x80 \n"
    //     :
    //     : "r"(mutex)         //输入参数mutex
    //     : "eax", "ebx", "ecx", "edx"
    // );

    volatile uint wait = 0;
    do {
        SysCall(1, 1, mutex, &wait);
    } while (wait);
    

}

void ExitCritical(uint mutex)
{
    // asm volatile(
    //     "movl $1, %%eax \n"   //type:互斥锁
    //     "movl $2, %%ebx \n"   //cmd:进入临界区
    //     "movl %0, %%ecx \n"   //将mutex参数通过ecx传递，此处时占位符
    //     "int $0x80 \n"
    //     :
    //     : "r"(mutex)         //输入参数mutex
    //     : "eax", "ebx", "ecx", "edx"
    // );
    SysCall(1, 2, mutex, 0);
}

uint DestroyMutex(uint mutex)
{
    volatile uint ret = 0;
    // asm volatile(
    //     "movl $1, %%eax \n"   //type:互斥锁
    //     "movl $3, %%ebx \n"   //cmd:进入临界区
    //     "movl %0, %%ecx \n"   //将mutex参数通过ecx传递，此处时占位符
    //     "int $0x80 \n"
    //     :
    //     : "r"(mutex)         //输入参数mutex
    //     : "eax", "ebx", "ecx", "edx"
    // );
    SysCall(1, 3, mutex, &ret);
    return ret;
}