#include "rw_model.h"
#include "syscall.h"
#include "screen.h"
#include "utility.h"


static uint gWriteMutex = 0;
static uint gReadMutex = 0;
static char data = 'A';
static uint gReaderCnt = 0;
static uint stopFlag = 0;

void Writer()
{
    int cnt = 0;
    gWriteMutex = CreateMutex(NORMAL);
    gReadMutex = CreateMutex(STRICT);

    SetPrintPos(0, 15);
    PrintString("Writer: ");

    while(data < 'Z')
    {   
        EnterCritical(gWriteMutex);

        SetPrintPos(10+cnt, 15);
        data++;
        PrintChar(data);
        cnt++;
        
        ExitCritical(gWriteMutex);

        Delay(2);
    }
    stopFlag = 1;
    SetPrintPos(0, 21);
    PrintString("Writer  exit");
}

void Reader()
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

        Delay(1);
    }
    SetPrintPos(0, 22);
    PrintString("Reader  exit");
}

