
/**
 *  应用层任务模块
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

static void RegApp(const char *name, void (*tmain)())
{
    if (gAppNum < MAX_APP_NUM)
    {
        AppInfo *app = AddrOff(gAppToRun, gAppNum);
        app->name = name;
        app->tmain = tmain;
        gAppNum++;
    }
}

void AppModInit()
{
    RegApp("Task A", TaskA);
    RegApp("Task B", TaskB);
    RegApp("Task C", TaskC);
    RegApp("Task D", TaskD);
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
    while (i < 5)
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
    while (i < 5)
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
    while (i < 5)
    {
        SetPrintPos(10, 18);
        PrintChar('.' + i);
        i = (i + 1) % 10;
        Delay(2);
    }
}
