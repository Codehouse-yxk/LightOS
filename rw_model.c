/*
 * @Author: yangxingkun
 * @Date: 2022-08-11 21:23:47
 * @FilePath: \LightOS\rw_model.c
 * @Description: 读写者模型
 * @Github: https://github.com/Codehouse-yxk
 */
#include "rw_model.h"
#include "syscall.h"
#include "screen.h"
#include "utility.h"


static uint gWriteMutex = 0;
static uint gReadMutex = 0;
static char data = 'A';
static uint gReaderCnt = 0;
static uint stopFlag = 0;

static void DataReset()
{
    data = 'A';
    gReaderCnt = 0;
    stopFlag = 0;
}

static void Writer()
{
    int cnt = 0;
    SetPrintPos(0, 15);
    PrintString("Writer: ");

    while(1)
    {   
        EnterCritical(gWriteMutex);

        SetPrintPos(10+cnt, 15);
        PrintChar(data);
        if(data >= 'Z'){
            ExitCritical(gWriteMutex);
            break;
        }
        data++;
        cnt++;

        ExitCritical(gWriteMutex);

        Delay(2);
    }
    stopFlag = 1;
    SetPrintPos(0, 21);
    PrintString("Writer  exit");
}

static void Reader()
{
    static int cnt = 0;
    static int hFlag = 0;
    SetPrintPos(0, 18);
    PrintString("Reader: ");

    while(!stopFlag)
    {
        EnterCritical(gReadMutex);

        if(gReaderCnt == 0)
        {
            EnterCritical(gWriteMutex);
        }

        gReaderCnt++;
        ExitCritical(gReadMutex);

        //读数据
        if(cnt > 50)
        {
            hFlag++;
            cnt = 0;
        }
        SetPrintPos(10+cnt, 18+hFlag);
        PrintChar(data);
        cnt++;

        EnterCritical(gReadMutex);
        gReaderCnt--;
        if(gReaderCnt == 0)
        {
            ExitCritical(gWriteMutex);
        }
        ExitCritical(gReadMutex);

        Delay(2);
    }
    SetPrintPos(0, 22);
    PrintString("Reader  exit");
}

static void Initialize()
{
    SetPrintPos(0,12);
    PrintString("read_write model task initialize");
    DataReset();
    gWriteMutex = CreateMutex(NORMAL);
    gReadMutex = CreateMutex(STRICT);
}

static void DeInit()
{
    Wait("Writer");
    Wait("ReaderA");
    Wait("ReaderB");
    Wait("ReaderC");

    DestroyMutex(gWriteMutex);
    DestroyMutex(gReadMutex);

    SetPrintPos(0,13);
    PrintString("read_write model task finish");
}

void RunReadWriteModel()
{
    Initialize();

    RegApp("Writer", Writer, 255);
    RegApp("ReaderA", Reader, 255);
    RegApp("ReaderB", Reader, 255);
    RegApp("ReaderC", Reader, 255);

    RegApp("DeInit", DeInit, 255);
}

