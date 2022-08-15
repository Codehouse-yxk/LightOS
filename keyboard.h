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
 * @description: 键盘中断触发接口函数
 * @param {byte} code
 * @return {*}
 */
void PutScanCode(byte code);

/**
 * @description: 
 * @return {*}
 */
uint FeachKeyCode();



#endif //KEYBOARD_H