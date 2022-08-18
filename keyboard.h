#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "type.h"
#include "event.h"
#include "queue.h"

#define KB_BUFF_SIZE    8
typedef struct
{
    byte ascii1;   // no shift code
    byte ascii2;   // shift code
    byte scode;    // scan code
    byte kcode;    // key code
} KeyCode;

typedef struct
{
    uint head;
    uint tail;
    uint count;
    uint max;
    uint buff[KB_BUFF_SIZE];
}keyCodeBuff;

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
 * @description: 按键通知
 */
void NotifyGetKeyCode();

/**
 * @description: 键盘数据系统调用处理函数
 * @param 键盘指令
 * @param 参数1
 * @param 参数2
 */
void KeyboardCallHandler(uint cmd, uint param1, uint param2);



#endif //KEYBOARD_H