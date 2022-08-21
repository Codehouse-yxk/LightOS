/*
 * @Author: yangxingkun
 * @Date: 2022-07-13 08:11:44
 * @FilePath: \LightOS\task.c
 * @Description: 内核任务模块，包含：任务初始化、任务调度等功能
 * @Github: https://github.com/Codehouse-yxk
 */


#include "task.h"
#include "utility.h"
#include "memory.h"
#include "screen.h"
#include "mutex.h"

#define MAX_TASK_NUM        16       //最大任务数量
#define MAX_RUNNING_TASK    8        //最大处于运行态任务数量
#define MAX_READY_TASK      (MAX_TASK_NUM - MAX_RUNNING_TASK)    //最大处于就绪态任务数量
#define MAX_TASK_BUFF_NUM   (MAX_TASK_NUM + 1)
#define PID_BASE            0x10    
#define MAX_TIME_SLICE      260      //最大时间片


void (*const RunTask)(volatile Task *p) = NULL;
void (*const LoadTask)(volatile Task *p) = NULL;

static TSS gTss = {0};
volatile Task *gCTaskAddr = NULL;
static TaskNode gTaskBuffer[MAX_TASK_NUM] = {0};

static Queue gAppToRun = {0};     //应用注册任务队列
static Queue gFreeTask = {0};     //空闲队列
static Queue gReadyTask = {0};    //就绪队列
static Queue gRunningTask = {0};  //运行队列

static TaskNode gIdleTask = {0};  //默认任务

static uint gPid = PID_BASE;

/* 所有任务入口函数 */
static void TaskEntry()
{
    if (gCTaskAddr)
    {
        gCTaskAddr->tmain();
    }

    //触发0x80软中断，陷入内核态销毁任务
    asm volatile(
        "movl $0, %eax \n"   //type:销毁任务
        "int $0x80 \n"
    );
}

static void InitTask(Task *p, uint id, const char* name, void (*entry)(), ushort pri)
{
    p->rv.cs = LDT_CODE32_SELECTOR;
    p->rv.gs = LDT_VIDEO_SELECTOR;
    p->rv.ds = LDT_DATA32_SELECTOR;
    p->rv.es = LDT_DATA32_SELECTOR;
    p->rv.fs = LDT_DATA32_SELECTOR;
    p->rv.ss = LDT_DATA32_SELECTOR;

    p->rv.esp = (uint)p->stack + AppTaskSize; //任务私有栈
    p->rv.eip = (uint)TaskEntry;
    p->rv.eflags = 0x3202; //设置IOPL = 3（可以进行IO操作）， IF = 1（响应外部中断）；

    if(name)
    {
        StrCpy(p->name, name, sizeof(p->name)-1);
    }
    else
    {
        *(p->name) = 0;
    }
    
    Queue_Init(&p->wait);
    p->tmain = entry;
    p->id = id;
    p->total = MAX_TIME_SLICE - pri;
    p->current = 0;
    p->event = NULL;

    SetDescValue(AddrOff(p->ldt, LDT_VIDEO_INDEX), 0xB8000, 0x07FFF, DA_DRWA + DA_32 + DA_DPL3); //设置ldt显存段
    SetDescValue(AddrOff(p->ldt, LDT_CODE32_INDEX), 0x00000, kernalHeapBase - 1, DA_C + DA_32 + DA_DPL3);   //设置ldt代码段
    SetDescValue(AddrOff(p->ldt, LDT_DATA32_INDEX), 0x00000, kernalHeapBase - 1, DA_DRW + DA_32 + DA_DPL3); //设置ldt数据段

    p->ldtSelector = GDT_TASK_LDT_SELECTOR;
    p->tssSelector = GDT_TASK_TSS_SELECTOR;
}

const char* GetCurrentTaskName()
{
    return (const char*)gCTaskAddr->name;
}

uint GetCurrentTaskId()
{
    return gCTaskAddr->id;
}

/**
 * @description: 根据任务名称查找目标任务
 * @param 目标任务名
 * @return 返回目标任务的地址
 */
static Task* FindTaskByName(const char* name)
{
    Task* ret = NULL;
    
    if(!StrCmp(name, "IdleTask", -1))
    {
        int i = 0;
        for(i=0; i<MAX_TASK_NUM; i++)
        {
            Task* tmp = &((TaskNode*)AddrOff(gTaskBuffer, i))->task;
            if(tmp->id && StrCmp(tmp->name, name, -1))
            {
                ret = tmp;
                break;
            }
        }
    }
    return ret;
}

static void PrepareForRun(volatile Task *p)
{
    p->current++;

    //设置内核栈
    gTss.ss0 = GDT_DATA32_FLAT_SELECTOR; //设置高特权级下的栈
    gTss.esp0 = (uint)&p->rv + sizeof(p->rv);
    gTss.iomb = sizeof(gTss);

    //每个任务有自己的ldt，任务切换时需要重新加载
    SetDescValue(AddrOff(gGdtInfo.entry, GDT_TASK_LDT_INDEX), (uint)&p->ldt, sizeof(p->ldt) - 1, DA_LDT + DA_DPL0);
}

static AppInfoToRun(const char *name, void (*tmain)(), byte priority)
{
    AppNode* an = (AppNode*)Malloc(sizeof(AppNode));
    if(an)
    {
        char* s = name ? Malloc(StrLen(name) + 1) : NULL;
        an->app.name = s ? StrCpy(s, name, -1) : NULL;
        an->app.tmain = tmain;
        an->app.priority = priority;

        Queue_Add(&gAppToRun, (QueueNode*)an);
    }
}

static void AppMainToRun()
{
    AppInfoToRun("AppMain", (void*)(*((uint*)AppMainEntry)), 200);
}

/* 根据业务app，创建任务，并将任务放入就绪队列 */
static void CreatTask()
{
    //判断所有要执行的任务有没有全部创建，并且就绪队列有空位
    while ((Queue_Length(&gAppToRun) > 0) && (Queue_Length(&gReadyTask) < MAX_READY_TASK))
    {
        TaskNode *tn = (TaskNode *)Queue_Remove(&gFreeTask);

        if (tn)
        {
            AppNode* an = (AppNode*)Queue_Remove(&gAppToRun);
            InitTask(&tn->task, gPid++, an->app.name, an->app.tmain, an->app.priority);
            Queue_Add(&gReadyTask, (QueueNode*)tn);
            Free((void*)an->app.name);
            Free(an);
        }
        else
        {
            break;
        }
    }
}

/* 检测运行中的任务，如果没有业务任务，需要运行内核默认gIdleTask */
static CheckRunningTask()
{
    int len = Queue_Length(&gRunningTask);
    if(len == 0)
    {
        Queue_Add(&gRunningTask, (QueueNode*)&gIdleTask);
    }
    else if(len > 1)
    {
        //如果有任务，此时不能有默认的gIdleTask，所以将默认的gIdleTask从运行队列中删掉
        if(isEqual(Queue_Front(&gRunningTask), (QueueNode*)&gIdleTask))
        {
            Queue_Remove(&gRunningTask);
        }
    }
}

/* 准备任务，并将任务切换到运行态 */
static void ReadyToRunning()
{
    QueueNode* node = NULL;

    if(Queue_Length(&gReadyTask) < MAX_READY_TASK)
    {
        CreatTask();
    }

    while((Queue_Length(&gReadyTask)>0) && (Queue_Length(&gRunningTask)<MAX_RUNNING_TASK))
    {
        node = Queue_Remove(&gReadyTask);

        ((TaskNode*)node)->task.current = 0;

        Queue_Add(&gRunningTask, node);
    }
}

/* 将运行队列任务切换到就绪队列 */
static void RunningToReady()
{
    if(Queue_Length(&gRunningTask) > 0)
    {
        TaskNode* tn = (TaskNode*)Queue_Front(&gRunningTask);

        if(!isEqual(tn, (QueueNode*)&gIdleTask))
        {
            if(tn->task.current == tn->task.total)
            {
                Queue_Remove(&gRunningTask);

                Queue_Add(&gReadyTask, (QueueNode*)tn);
            }
        }
    }
}

static void RunningToWaitting(Queue* wait)
{
    if(Queue_Length(&gRunningTask) > 0)
    {
        TaskNode* tn = (TaskNode*)Queue_Front(&gRunningTask);

        if(!isEqual(tn, (QueueNode*)&gIdleTask))
        {
            Queue_Remove(&gRunningTask);
            Queue_Add(wait, (QueueNode*)tn);
        }
    }
}

static void WaittingToReady(Queue* wait)
{
    while(Queue_Length(wait) > 0)
    {
        TaskNode* tn = (TaskNode*)Queue_Front(wait);

        DestroyEvent(tn->task.event);
        tn->task.event = NULL;

        Queue_Remove(wait);

        Queue_Add(&gReadyTask, (QueueNode*)tn);
    }
}

void IdleTask()
{
    while(1);
}

void TaskModInit()
{
    int i = 0;

    byte* pStack = (byte*)(AppHeapBase - (AppTaskSize * MAX_TASK_BUFF_NUM));

    for(i = 0; i < MAX_TASK_BUFF_NUM; i++)
    {
        TaskNode* tn = (void*)AddrOff(gTaskBuffer, i);
        tn->task.stack = (void*)AddrOff(pStack, i * AppTaskSize);
    }
    Queue_Init(&gAppToRun);
    Queue_Init(&gFreeTask);
    Queue_Init(&gReadyTask);
    Queue_Init(&gRunningTask);

    for (i = 0; i < MAX_TASK_NUM; i++)
    {
        Queue_Add(&gFreeTask, (QueueNode *)AddrOff(gTaskBuffer, i));
    }

    AppMainToRun(); //创建第一个应用层任务

    // Tss共用，只需要设置一次
    SetDescValue(AddrOff(gGdtInfo.entry, GDT_TASK_TSS_INDEX), (uint)&gTss, sizeof(gTss) - 1, DA_386TSS + DA_DPL0); //在gdt中设置tss

    InitTask(&gIdleTask.task, gPid++, "IdleTask", IdleTask, 255);

    ReadyToRunning();

    CheckRunningTask();
}

void LaunchTask()
{
    gCTaskAddr = &((TaskNode*)Queue_Front(&gRunningTask))->task;
    PrepareForRun(gCTaskAddr);
    RunTask(gCTaskAddr); //启动第一个任务
}

static void ScheduleNext()
{
    ReadyToRunning();   //就绪态任务切换到运行态

    CheckRunningTask();

    Queue_Rotate(&gRunningTask);

    QueueNode *node = Queue_Front(&gRunningTask);
    if (node)
    {
        gCTaskAddr = &Queue_Node(node, TaskNode, head)->task;
        PrepareForRun(gCTaskAddr);
        LoadTask(gCTaskAddr); //只加载任务的ldt
    }
}

static void WaitEvent(Queue* wait, Event* event)
{   
    gCTaskAddr->event = event;

    RunningToWaitting(wait);

    ScheduleNext();
}

void Schedule()
{
    RunningToReady();   //判断运行态的任务是否需要切换到就绪态
    ScheduleNext();
}

static void MutexSchedule(uint action, Event* event)
{
    Mutex* mutex = (Mutex*)event->id;
    if(action == NOTIFY)
    {
        WaittingToReady(&mutex->wait);
    }
    else if(action == WAIT)
    {
        WaitEvent(&mutex->wait, event);
    }
}

static void KeyboardSchedule(uint action, Event* event)
{
    Queue* wait = (Queue*)event->id;

    if(action == NOTIFY)
    {
        uint kc = event->param1;
        ListNode* pos = NULL;

        List_ForEach((List*)wait, pos)
        {
            //将键盘产生的键码赋值给每个等待任务的param1，将数据传给用户
            TaskNode* tn = (TaskNode*)pos;
            uint* ret = (uint *)(tn->task.event->param1);
            *ret = kc;
        }
        WaittingToReady(wait);
    }
    else if(action == WAIT)
    {
        WaitEvent(wait, event);
    }
}

static void TaskSchedule(uint action, Event* event)
{
    Task* task = (Task*)event->id;
    if(action == NOTIFY)
    {
        WaittingToReady(&task->wait);
    }
    else if(action == WAIT)
    {
        WaitEvent(&task->wait, event);
    }
}

void EventSchedule(uint action, Event* event)
{
    switch(event->type)
    {
        case MutexEvent:
            MutexSchedule(action, event);
            break;
        case KeyboardEvent:
            KeyboardSchedule(action, event);
            break;
        case TaskEvent:
            TaskSchedule(action, event);
            break;
        default:
            break;
    }
}

void WaitTask(const char* name)
{
    Task* task = FindTaskByName(name);
    if(task)
    {
        Event* event = CreateEvent(TaskEvent, (uint)task, 0, 0);
        if(event)
        {
            EventSchedule(WAIT, event);
        }
    }
}

void KillTask()
{
    QueueNode* node = Queue_Remove(&gRunningTask);

    Task* task = &((TaskNode*)node)->task;
    Event event = {TaskEvent, (uint)task, 0, 0};
    EventSchedule(NOTIFY, &event);
    
    //任务结束，id必须设置为0，因为wait时需要用id进行判断，如果对应id任务已经销毁，则不需要阻塞等待。
    task->id = 0;

    Queue_Add(&gFreeTask, node);

    Schedule();
}

void TaskCallHandler(uint cmd, uint param1, uint param2)
{
    switch(cmd)
    {
        case KILLCMD:
        {
            KillTask();
            break;
        }
        case WAITCMD:
        {
            WaitTask((const char*)param1);
            break;
        }
        case REGAPPCMD:
        {
            AppInfo* info = (AppInfo*)param1;
            AppInfoToRun(info->name, info->tmain, info->priority);
            break;
        }
        default:
            break;
    }
}