#ifndef VMEMORY_H
#define VMEMORY_H

#include "type.h"
#include "list.h"

#define     VM_HEAD_SIZE    sizeof(VMemHead)


typedef struct
{
    ListNode head;  //链表头
    uint used;      //已使用的内存
    uint free;      //空闲内存
    byte* ptr;      //指向已用区域起始地址
}VMemHead;



/**
 * @description: 初始化内存管理模块
 * @param 目标空间
 * @param 所需大小
 * @return 
 */
void VMemInit(byte* mem, uint size);

/**
 * @description: 申请内存单元
 * @param: 申请内存大小
 * @return 成功：内存地址， 失败：NULL
 */
void* VMemAlloc(uint size);

/**
 * @description: 释放指定的内存空间
 * @param 需要释放的内存地址
 * @return 成功：1， 失败：0
 */
int VMemFree(void* ptr);

/**
 * @description 测试定长内存
 */
// void vmem_test();

#endif //VMEMORY_H