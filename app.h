
#ifndef APP_H
#define APP_H

#include "type.h"

/* 任务信息结构体 */
typedef struct{
    const char* name;
    void (*tmain)();
    byte priority;  //数值越小，优先级越高（可获取时间片越久），最大值：255
}AppInfo;

/* 应用层任务模块初始化 */
// void AppModInt();

/* 获取指定任务执行 */
// AppInfo* GetAppToRun(uint index);

/* 获取任务数量 */
// uint GetAppNum();



#endif  //APP_H