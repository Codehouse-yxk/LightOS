#ifndef IHANDLER_H
#define IHANDLER_H

#define DeclHandler(name) \
    extern void name##Entry();   \
    void name()

// 等价于：
// extern void TimerHandlerEntry();
// void TimerHandler();
/* 时钟中断入口函数 + 时钟中断处理函数（中断服务程序） */
DeclHandler(TimerHandler);

#endif // IHANDLER_H