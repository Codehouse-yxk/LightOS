
#ifndef SCREEN_H
#define SCREEN_H

#include "type.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

typedef enum {
    SCREEN_GRAY = 0x07,
    SCREEN_BLUE = 0x09,
    SCREEN_GREEN = 0x0A,
    SCREEN_RED = 0x0C,
    SCREEN_YELLOW = 0x0E,
    SCREEN_WHITE = 0x0F
} PrintColor;

/* 清空屏幕 */
void ClearScreen();

/* 设置光标坐标点 */
int SetPrintPos(byte w, byte h);

/* 设置字体颜色 */
void SetPrintColor(PrintColor c);

/* 打印字符 */
int PrintChar(const char c);

/* 打印字符串 */
int PrintString(const char *s);

/* 将数据按整型打印 */
int PrintIntDec(int n);

/* 清空屏幕 */
int PrintIntHex(uint n);

#endif // SCREEN_H