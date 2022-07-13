#ifndef IHANDLER_H
#define IHANDLER_H


#define DeclHandler(name)   void name##Entry();\
                            void name()

// 等价于：
// void TimerHandlerEntry();
// void TimerHandler();
DeclHandler(TimerHandler);

#endif //IHANDLER_H