/*
 * @Author: yangxingkun
 * @Date: 2022-07-24 10:50:17
 * @FilePath: \LightOS\app.c
 * @Description: 应用层任务模块
 * @Github: https://github.com/Codehouse-yxk
 */


#include "app.h"
#include "type.h"
#include "utility.h"
#include "screen.h"
#include "memory.h"
#include "syscall.h"
#include "consumer_model.h"

#define MAX_APP_NUM 16
static AppInfo gAppToRun[MAX_APP_NUM] = {0}; //记录哪些应用程序需要被系统创建任务（进程）来执行
static uint gAppNum = 0;

void TaskA();
void TaskB();
void TaskC();
void TaskD();

static void RegApp(const char *name, void (*tmain)(), byte priority)
{
    if (gAppNum < MAX_APP_NUM)
    {
        AppInfo *app = AddrOff(gAppToRun, gAppNum);
        app->name = name;
        app->tmain = tmain;
        app->priority = priority;
        gAppNum++;
    }
}

void AppMain()
{
    RegApp("Producer A", ProducerA, 50);
    RegApp("Producer B", ProducerB, 100);
    RegApp("Consumer A", ConsumerA, 100);
    RegApp("Consumer B", ConsumerB, 50);
}

AppInfo *GetAppToRun(uint index)
{
    AppInfo *ret = NULL;
    if (index < MAX_APP_NUM)
    {
        ret = AddrOff(gAppToRun, index);
    }
    return ret;
}

uint GetAppNum()
{
    return gAppNum;
}

static uint mutex = 0;
static int num = 0;

void TaskA()
{
    int i = 0;
    SetPrintPos(0, 15);
    PrintString("Task A: ");

    mutex = CreateMutex(STRICT);

    EnterCritical(mutex);

    for(i=0; i<30; i++)
    {
        SetPrintPos(12, 15);
        PrintIntDec(num);
        num++;
        Delay(2);
    }

    ExitCritical(mutex);

    uint ret = DestroyMutex(mutex);
    if(ret)
    {
        PrintString("   destroy mutex success\n");
    }else
    {
        PrintString("   destroy mutex fail\n");
    }
}

void TaskB()
{
    int i = 0;
    SetPrintPos(0, 16);
    PrintString("Task B: ");

    ExitCritical(mutex);

    uint ret = DestroyMutex(mutex);
    if(ret)
    {
        PrintString("   destroy mutex success\n");
    }else
    {
        PrintString("   destroy mutex fail\n");
    }

    // EnterCritical(mutex);
    // while (1)
    // {
    //     SetPrintPos(12, 16);
    //     PrintIntDec(num);
    //     num+=2;
    //     Delay(5);
    // }
    // ExitCritical(mutex);
}

void TaskC()
{
    int i = 0;
    SetPrintPos(0, 17);
    PrintString("Task C: ");
    while (1)
    {
        SetPrintPos(10, 17);
        PrintChar('m' + i);
        i = (i + 1) % 10;
        Delay(2);
    }
}

void TaskD()
{
    int i = 0;
    SetPrintPos(0, 18);
    PrintString("Task D: ");
    while (1)
    {
        SetPrintPos(10, 18);
        PrintChar('.' + i);
        i = (i + 1) % 10;
        Delay(2);
    }
}
