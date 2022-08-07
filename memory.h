#ifndef MEMORY_H
#define MEMORY_H

#include "type.h"
#include "fmemory.h"
#include "vmemory.h"

/**
 * @description: 内存管理模块初始化
 * @param 需要管理的内存起始地址
 * @param 内存大小
 */
void MemModInit(byte* mem, uint size);

/**
 * @description: 申请内存
 * @param 需要申请的内存大小
 * @return 成功：申请到的内存地址   失败：NULL
 */

void* Malloc(uint size);

/**
 * @description: 释放内存
 */
void Free(void* ptr);



#endif  //MEMORY_H