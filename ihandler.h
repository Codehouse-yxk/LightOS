#ifndef IHANDLER_H
#define IHANDLER_H

#define DeclHandler(name) \
    extern void name##Entry();   \
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

/* 0x80中断服务程序，ax：功能码 */
void SysCallHandler(ushort ax);

#endif // IHANDLER_H