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


static void Shell()
{
    SetPrintPos(0, 9);
    PrintString("LightOS >> ");

    while (1)
    {
        uint code = ReadKey();
        uint pressed = IsKeyDown(code);
        char data = GetChar(code);
        byte kdata = GetKeyCode(code);

         if(kdata==0x13 && pressed)
        {
            PrintString("Pause pressed\n");
        }
        else if(kdata==0x13)
        {
            PrintString("Pause release\n");
        }

        if(data && pressed)
        {
            PrintChar((char)code);
        }
        else if(kdata==0x0D && pressed) 
        {
            PrintChar('\n');
            PrintString("LightOS >> ");
        }
    }
    
}


void ShellModInit()
{
    RegApp("Shell", Shell, 255);
}