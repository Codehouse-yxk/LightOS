
#ifndef TASK_H
#define TASK_H

#include "kernel.h"
#include "queue.h"
#include "app.h"
#include "event.h"

typedef struct {
    /* 段寄存器 */
    uint gs;
    uint fs;
    uint es;
    uint ds;

    /* 通用寄存器 */
    uint edi;
    uint esi;
    uint ebp;
    uint kesp;
    uint ebx;
    uint edx;
    uint ecx;
    uint eax;

    uint raddr;

    uint eip;
    uint cs;
    uint eflags;
    uint esp;
    uint ss;
} RegValue;

typedef struct
{
    uint   previous;
    uint   esp0;
    uint   ss0;         //0特权级栈，低特权级升高特权级时使用。
    uint   unused[22];
    ushort reserved;
    ushort iomb;
} TSS;

typedef struct
{
    RegValue   rv;          //各个寄存器值

    //处理器数据结构  LDT、选择子
    Descriptor ldt[3];
    ushort     ldtSelector;
    ushort     tssSelector;

    void       (*tmain)();
    uint       id;
    ushort     current; //表示任务当前已执行的时间数
    ushort     total;   //表示任务每次执行的总时间数
    char       name[16]; 
    Queue      wait;
    byte*      stack;   //任务执行时使用的栈
    Event*     event;   //当前任务等待的事件
} Task;

typedef struct
{
    QueueNode head;
    Task task;
} TaskNode;

typedef struct
{
    QueueNode head;
    AppInfo app;
} AppNode;

/* Mutex——cmd */
enum{
    NOTIFY,
    WAIT
};

/* Task——cmd */
enum{
    KILLCMD,    //杀死任务
    WAITCMD,    //等待任务
    REGAPPCMD   //注册任务
};


/* 汇编中定义的RunTask，通过共享内存获取函数指针 */
void (*const RunTask)(volatile Task* p);

/* 汇编中定义的LoadTask，通过共享内存获取函数指针 */
void (*const LoadTask)(volatile Task* p);

/**
 * @description: 任务相关功能
 * @param 功能号
 * @param 参数1
 * @param 参数2
 */
void TaskCallHandler( uint cmd, uint param1, uint param2);

/**
 * @description: 任务模块初始化
 */
void TaskModInit();

/**
 * @description: 加载任务
 */
void LaunchTask();

/**
 * @description: 任务调度
 */
void Schedule();

/**
 * @description: 事件调度入口
 * @param 调度类型
 * @param 调度信息
 */
void EventSchedule(uint action, Event* event);

/**
 * @description: 任务销毁
 */
void KillTask();

/**
 * @description: 等待目标任务结束
 * @param 目标任务名
 */
void WaitTask(const char* name);

/**
 * @description: 获取当前任务名称
 * @return 任务名
 */
const char* GetCurrentTaskName();

/**
 * @description: 获取当前任务ID
 * @return 任务ID
 */
uint GetCurrentTaskId();

#endif //TASK_H