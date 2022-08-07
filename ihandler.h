#ifndef IHANDLER_H
#define IHANDLER_H

#define DeclHandler(name)     \
        void name##Entry();   \
        void name()


/* 页错误中断入口函数 */
extern void PageFaultHandlerEntry();

/* 页错误中断服务程序 */
void PageFaultHandler();

/* 段越界中断入口函数 */
extern void SegmentFaultHandlerEntry();

/* 段越界中断服务程序 */
void SegmentFaultHandler();

/* 时钟中断入口函数 */
extern void TimerHandlerEntry();

/* 时钟中断服务程序 */
void TimerHandler();

/* 系统调用（0x80中断）入口函数 */
extern void SysCallHandlerEntry();

/* ，ax：功能码 */
/**
 * @description: 0x80中断服务程序
 * @param 中断功能号
 * @param 子功能号
 * @param 参数1
 * @param 参数2
 */
void SysCallHandler(uint type, uint cmd, uint param1, uint param2);

#endif // IHANDLER_H