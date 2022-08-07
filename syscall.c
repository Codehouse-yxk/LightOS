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
    asm volatile(
        "movl $0, %eax \n"   //type:销毁任务
        "int $0x80 \n"
    );
}

uint CreateMutex()
{
    volatile uint ret = 0;
    asm volatile(
        "movl $1, %%eax \n"   //type:互斥锁
        "movl $0, %%ebx \n"   //cmd:创建互斥锁
        "movl %0, %%ecx \n"   //将ret参数通过ecx传递，此处时占位符
        "int $0x80 \n"
        :
        : "r"(&ret)           //输入参数ret
        : "eax", "ebx", "ecx", "edx"
    );
    
    return ret;
}

void EnterCritical(uint mutex)
{
    asm volatile(
        "movl $1, %%eax \n"   //type:互斥锁
        "movl $1, %%ebx \n"   //cmd:进入临界区
        "movl %0, %%ecx \n"   //将mutex参数通过ecx传递，此处时占位符
        "int $0x80 \n"
        :
        : "r"(mutex)         //输入参数mutex
        : "eax", "ebx", "ecx", "edx"
    );
}

void ExitCritical(uint mutex)
{
    asm volatile(
        "movl $1, %%eax \n"   //type:互斥锁
        "movl $2, %%ebx \n"   //cmd:进入临界区
        "movl %0, %%ecx \n"   //将mutex参数通过ecx传递，此处时占位符
        "int $0x80 \n"
        :
        : "r"(mutex)         //输入参数mutex
        : "eax", "ebx", "ecx", "edx"
    );
}

void DestroyMutex(uint mutex)
{
    asm volatile(
        "movl $1, %%eax \n"   //type:互斥锁
        "movl $3, %%ebx \n"   //cmd:进入临界区
        "movl %0, %%ecx \n"   //将mutex参数通过ecx传递，此处时占位符
        "int $0x80 \n"
        :
        : "r"(mutex)         //输入参数mutex
        : "eax", "ebx", "ecx", "edx"
    );
}