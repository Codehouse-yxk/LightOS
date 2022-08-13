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
#include "rw_model.h"

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
    RegApp("Task A", TaskA, 255);
    RegApp("Task B", TaskB, 255);
    RegApp("Task C", TaskC, 255);
    // RegApp("Reader 3", Reader, 255);
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
    int i = 10;
    SetPrintPos(0, 15);
    PrintString("Task A: Running");
    while (i>0)
    {
        i--;
        Delay(1);
    }
    SetPrintPos(0, 15);
    PrintString("Task A: Exit");
}

void TaskB()
{
    int i = 20;
    SetPrintPos(0, 16);
    PrintString("Task B: Running");
    while (i>0)
    {
        i--;
        Delay(2);
    }
    SetPrintPos(0, 16);
    PrintString("Task B: Exit");
}

void TaskC()
{
    int i = 0;
    SetPrintPos(0, 17);
    PrintString("Task C: Waitting A Over");
    Wait("Task A");
    SetPrintPos(0, 17);
    PrintString("Task C: Waitting B Over");
    Wait("Task B");
    Delay(10);
    SetPrintPos(0, 18);
    PrintString("Task C: Exit");
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
