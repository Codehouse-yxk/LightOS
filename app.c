/*
 * @Author: yangxingkun
 * @Date: 2022-07-24 10:50:17
 * @FilePath: \LightOS\app.c
 * @Description: 应用层任务模块
 * @Github: https://github.com/Codehouse-yxk
 */

#include "app.h"
#include "type.h"
#include "utility.h"
#include "screen.h"
#include "memory.h"
#include "syscall.h"
#include "consumer_model.h"
#include "rw_model.h"

void TaskA();
void TaskB();

void AppMain()
{
    // RunConsumerModel();
    // ClearScreen();
    RunReadWriteModel();
}


