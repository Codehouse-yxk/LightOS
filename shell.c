/*
 * @Author: yangxingkun
 * @Date: 2022-08-18 22:50:54
 * @FilePath: \LightOS\shell.c
 * @Description: Shell模块
 * @Github: https://github.com/Codehouse-yxk
 */

#include "shell.h"
#include "syscall.h"
#include "screen.h"
#include "utility.h"
#include "list.h"
#include "memory.h"
#include "rw_model.h"
#include "consumer_model.h"

#define BUFF_SIZE       64
#define TIP_STR         "LightOS $ "
#define KEY_ENTER       0x0D
#define KEY_BACKSPACE   0x08

typedef struct
{
    ListNode node;
    char* cmd;
    void (*run)();
} CmdRun;

static char gKBuff[BUFF_SIZE] = {0};    //存储用户输入的字符
static uint gKIndex = 0;                //记录新字符在缓冲区的位置，即输入字符的数量
static List gCmdList = {0};

static uint IsKeyDown(uint kc)
{
    return !!(kc & 0xFF000000);
}

static char GetChar(uint kc)
{
    return (char)kc;
}

static byte GetKeyCode(uint kc)
{
    return (byte)(kc >> 8);
}

static uint DoCmd(const char* cmd)
{
    uint ret = 0;

    ListNode* pos = NULL;

    List_ForEach(&gCmdList, pos)
    {
        CmdRun* task = (CmdRun*)pos;
        if(StrCmp(task->cmd, cmd, -1))
        {
            task->run();
            ret = 1;
            break;
        }
    }
    return ret;
}

static void Clear()
{
    int h = 0;
    int w = 0;
    SetPrintPos(ERR_START_W, ERR_START_H);
    for(h=ERR_START_H; h<SCREEN_HEIGHT; h++)
    {
        for(w=0; w<SCREEN_WIDTH; w++)
        {
            PrintChar(' ');
        }
    }
    SetPrintPos(CMD_START_W, CMD_START_H);
    PrintString(TIP_STR);
}

static void ClearInfoLine()
{
    int h = 0;
    int w = 0;
    SetPrintPos(CMD_START_W, CMD_START_H + 1);
    for(h=CMD_START_H+1; h <= CMD_START_H+2; h++)
    {
        for(w=0; w<SCREEN_WIDTH; w++)
        {
            PrintChar(' ');
        }
    }
    SetPrintPos(CMD_START_W, CMD_START_H + 1);
}

static UnKnow(const char* str)
{
    ClearInfoLine();
    PrintString("Unknow Cmd: ");
    PrintString(str);
}

static void ResetCmdLine()
{
    int w = 0;
    SetPrintPos(CMD_START_W, CMD_START_H);
    for(w=0; w<SCREEN_WIDTH; w++)
    {
        PrintChar(' ');
    }
    SetPrintPos(CMD_START_W, CMD_START_H);
    PrintString(TIP_STR);
}

static void EnterHandle()
{
    gKBuff[gKIndex++] = 0;
    ClearInfoLine();
    if((gKIndex > 1) && !DoCmd(gKBuff))
    {
        UnKnow(gKBuff);
    }
    gKIndex = 0;
    ResetCmdLine();
}

static void BackSpaceHandle()
{
    static uint tipsLen = 0;
    int w = GetPrintPosW();
    if(!tipsLen)
    {
        tipsLen = StrLen(TIP_STR);
    }

    if(w > tipsLen)
    {
        w--;
        SetPrintPos(w, CMD_START_H);
        PrintChar(' ');
        SetPrintPos(w, CMD_START_H);
        gKIndex--;
    }
}

static void KeyHandle(char c, byte vCode)
{
    if(gKIndex < BUFF_SIZE)
    {
        if(c)   //输入是ascii码
        {
            PrintChar(c);
            gKBuff[gKIndex++] = c;
        }
        else
        {
            switch(vCode)
            {
                case KEY_ENTER:
                    EnterHandle();
                    break;
                case KEY_BACKSPACE:
                    BackSpaceHandle();
                    break;
                default:
                    break;
            }
        }   
    }
    else
    {
        UnKnow("Input cmd too long");
        gKIndex = 0;
        ResetCmdLine();
    }
    
}

static void AddCmdEntry(const char* cmd, void (*run)())
{
    CmdRun* cr = (CmdRun*)Malloc(sizeof(CmdRun));
    if(cr && cmd && run)
    {
        int len = StrLen(cmd);
        cr->run = run;
        cr->cmd = Malloc(len);
        if(cr->cmd)
        {
            StrCpy(cr->cmd, cmd, -1);
        }
        List_Add(&gCmdList, (ListNode*)cr);
    }
    else
    {
        Free(cr);
    }
}

static void Shell()
{
    SetPrintPos(CMD_START_W, CMD_START_H);
    PrintString(TIP_STR);

    while (1)
    {
        uint code = ReadKey();
        if(IsKeyDown(code))
        {
            char data = GetChar(code);
            byte vdata = GetKeyCode(code);
            KeyHandle(data, vdata);
        }
    }
}

static Dowait()
{
    Delay(5);
    Wait("DeInit");
    ResetCmdLine();
}

static void RwModel()
{
    RunReadWriteModel();
    Dowait();
}

static void ConsumerModel()
{
    RunConsumerModel();
    Dowait();
}

static void ReadMem()
{
    uint ret = GetMemSize();
    PrintString("Pysical Memory Size: ");
    PrintIntHex(ret);

    PrintString(" Kb       ");
    PrintIntDec(ret >> 20);
    PrintString(" MB");
}

void ShellModInit()
{
    List_Init(&gCmdList);
    AddCmdEntry("clear", Clear);
    AddCmdEntry("rw_model", RwModel);
    AddCmdEntry("cs_model", ConsumerModel);
    AddCmdEntry("mem", ReadMem);
    RegApp("Shell", Shell, 255);
}