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
    RegApp("Task A", TaskA, 250);
    RegApp("Task B", TaskB, 220);
    RegApp("Task C", TaskC, 220);
    RegApp("Task D", TaskD, 240);
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

void TaskA()
{
    int i = 0;
    SetPrintPos(0, 15);
    PrintString("Task A: ");
    while (i < 5)
    {
        //  强制修改内核内存空间，测试异常
        // if(i == 2)
        // {
        //     uint* p = (uint*)0x50000;
        //     *p = 10;
        // }
        SetPrintPos(10, 15);
        PrintChar('A' + i);
        i = (i + 1) % 26;
        Delay(2);
    }
    SetPrintPos(10, 15);
    PrintString("end Task A");
}

void TaskB()
{
    int i = 0;
    SetPrintPos(0, 16);
    PrintString("Task B: ");
    while (1)
    {
        SetPrintPos(10, 16);
        PrintChar('a' + i);
        i = (i + 1) % 10;
        Delay(2);
    }
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
