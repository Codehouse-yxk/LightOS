
/**
 * 内核任务模块，包含：任务初始化、任务调度等功能
 */

#include "task.h"
#include "utility.h"
#include "app.h"

#define MAX_TASK_NUM        4       //最大任务数量
#define MAX_RUNNING_TASK    2       //最大处于运行态任务数量
#define MAX_READY_TASK      (MAX_TASK_NUM - MAX_RUNNING_TASK)    //最大处于就绪态任务数量
#define MAX_TASK_BUFF_NUM   (MAX_TASK_NUM + 1)
#define PID_BASE            0x10    
#define MAX_TIME_SLICE      260    //最大时间片

static AppInfo* (*GetAppToRun)(uint index) = NULL;
static uint (*GetAppNum)() = NULL;

void (*const RunTask)(volatile Task *p) = NULL;
void (*const LoadTask)(volatile Task *p) = NULL;

static TSS gTss = {0};
volatile Task *gCTaskAddr = NULL;
static TaskNode gTaskBuffer[MAX_TASK_NUM] = {0};

static Queue gFreeTask = {0};     //空闲队列
static Queue gReadyTask = {0};    //就绪队列
static Queue gRunningTask = {0};  //运行队列
static Queue gWaittingTask = {0}; //等待队列

static uint gAppToRunIndex = 0;

static TaskNode gIdleTask = {0};    //默认任务

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
        "movw $0, %ax \n"
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

    StrCpy(p->name, name, sizeof(p->name)-1);
    p->tmain = entry;
    p->id = id;
    p->total = MAX_TIME_SLICE - pri;
    p->current = 0;

    SetDescValue(AddrOff(p->ldt, LDT_VIDEO_INDEX), 0xB8000, 0x07FFF, DA_DRWA + DA_32 + DA_DPL3); //设置ldt显存段
    SetDescValue(AddrOff(p->ldt, LDT_CODE32_INDEX), 0x00000, PageDirBase - 1, DA_C + DA_32 + DA_DPL3);   //设置ldt代码段
    SetDescValue(AddrOff(p->ldt, LDT_DATA32_INDEX), 0x00000, PageDirBase - 1, DA_DRW + DA_32 + DA_DPL3); //设置ldt数据段

    p->ldtSelector = GDT_TASK_LDT_SELECTOR;
    p->tssSelector = GDT_TASK_TSS_SELECTOR;
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

/* 根据业务app，创建任务，并将任务放入就绪队列 */
static void CreatTask()
{
    uint num = GetAppNum();

    //判断所有要执行的任务有没有全部创建，并且就绪队列有空位
    while ((gAppToRunIndex < num) && (Queue_Length(&gReadyTask) < MAX_READY_TASK))
    {
        TaskNode *tn = (TaskNode *)Queue_Remove(&gFreeTask);

        if (tn)
        {
            AppInfo* app = GetAppToRun(gAppToRunIndex);
            InitTask(&tn->task, gPid++, app->name, app->tmain, app->priority);
            Queue_Add(&gReadyTask, (QueueNode*)tn);
        }
        else
        {
            break;
        }
        gAppToRunIndex++;
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

void IdleTask()
{
    while(1);
    // int i = 0;
    // SetPrintPos(0, 14);
    // PrintString("Task Idle: ");
    // while (1)
    // {
    //     SetPrintPos(11, 14);
    //     PrintChar('A' + i);
    //     i = (i + 1) % 10;
    //     Delay(1);
    // }
}

void TaskModInit()
{
    int i = 0;

    byte* pStack = (byte*)(PageDirBase - (AppTaskSize * MAX_TASK_BUFF_NUM));

    for(i = 0; i < MAX_TASK_BUFF_NUM; i++)
    {
        TaskNode* tn = (void*)AddrOff(gTaskBuffer, i);
        tn->task.stack = (void*)AddrOff(pStack, i * AppTaskSize);
    }

    GetAppToRun = (void*)(*((uint*)GetAppToRunEntry));
    GetAppNum = (void*)(*((uint*)GetAppNumEntry));

    Queue_Init(&gFreeTask);
    Queue_Init(&gWaittingTask);
    Queue_Init(&gReadyTask);
    Queue_Init(&gRunningTask);

    for (i = 0; i < MAX_TASK_NUM; i++)
    {
        Queue_Add(&gFreeTask, (QueueNode *)AddrOff(gTaskBuffer, i));
    }

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

void Schedule()
{
    RunningToReady();   //判断运行态的任务是否需要切换到就绪态

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

void KillTask()
{
    QueueNode* node = Queue_Remove(&gRunningTask);

    Queue_Add(&gFreeTask, node);

    Schedule();
}