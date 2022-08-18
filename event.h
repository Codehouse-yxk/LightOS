#ifndef EVENT_H
#define EVENT_H

#include "type.h"

/* 事件类型定义 */
enum{
    NoneEvent,
    MutexEvent,     //等待互斥锁而产生的事件
    KeyboardEvent,  //等待按键而产生的事件
    TaskEvent       //等待某个任务基数而产生的事件
};

typedef struct{
    uint type;      //事件类型
    uint id;        //事件标识信息
    uint param1;    //参数1
    uint param2;    //参数2
}Event;

/**
 * @description: 创建事件
 * @param 事件类型
 * @param 事件标识信息
 * @param 参数1
 * @param 参数2
 * @return 事件对象
 */
Event* CreateEvent(uint type, uint id, uint param1, uint param2);

/**
 * @description: 销毁事件
 * @param 事件对象
 */
void DestroyEvent(Event* event);




#endif  //EVENT_H