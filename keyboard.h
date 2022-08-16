#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "type.h"

typedef struct
{
    byte ascii1;   // no shift code
    byte ascii2;   // shift code
    byte scode;    // scan code
    byte kcode;    // key code
} KeyCode;

enum
{
    KeyRelease = 0,
    KeyPress = 0x1000000
};

/**
 * @description: 键盘驱动模块初始化
 */
void KeyboardModInit();

/**
 * @description: 将扫描码构建出键码并保存到缓冲区
 * @param 键盘扫描码
 */
void PutScanCode(byte code);

/**
 * @description: 获取缓冲区键码
 * @return 键码
 */
uint FeachKeyCode();



#endif //KEYBOARD_H