
#ifndef APP_H
#define APP_H

#include "type.h"

/* 任务信息结构体 */
typedef struct{
    const char* name;
    void (*tmain)();
    byte priority;  //数值越小，优先级越高（可获取时间片越久），最大值：255
}AppInfo;

/**
 * @description: 应用层第一个任务
 */
void AppMain();


#endif  //APP_H