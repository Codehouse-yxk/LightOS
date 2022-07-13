#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "const.h"
#include "type.h"
#include "kernel.h"

/* 初始化中断，汇编中定义的InitInterrupt，通过共享内存获取函数指针 */
void (*const InitInterrupt)();

/* 使能中断，汇编中定义的EnableTimer，通过共享内存获取函数指针 */
void (*const EnableTimer)();

/* 退出中断，汇编中定义的SendEOI，通过共享内存获取函数指针 */
void (*const SendEOI)(uint port);

/* 中断模块初始化 */
void IntModInit();

/* 设置中断回调函数 */
int SetIntHandler(Gate *pGate, uint ifunc);

/* 获取中断回调函数 */
int GetIntHandler(Gate *pGate, uint *pIfunc);

#endif // INTERRUPT_H