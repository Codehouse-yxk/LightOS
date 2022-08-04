#ifndef MEMORY_H
#define MEMORY_H

#include "type.h"


#define AddrIndex(a, b) (((uint)(a) - (uint)(b)) / sizeof(*a))      //计算a地址相对于b地址的偏移

#define FM_ALLOC_SIZE   32                 //一个内存分配单元大小
#define FM_NODE_SIZE    sizeof(FMemNode)   //一个管理单元节点大小

typedef byte(FMemUnit)[FM_ALLOC_SIZE]; //内存分配单元数据类型

typedef union _FMemNode FMemNode;

union _FMemNode
{
    FMemNode *next;
    FMemUnit *ptr;
}; //内存管理单元节点

typedef struct
{
    FMemNode *node;  //指向管理链表的头节点
    FMemNode *nbase; //指向内存管理单元起始地址
    FMemUnit *ubase; //指向内存分配单元起始地址
    uint max;        //记录当前有多少内存单元可用
} FMemList;


/**
 * @description: 初始化内存管理【从mem中构建管理单元和分配单元】
 * @param 内存起始地址
 * @param 内存总大小
 * @return 
 */
void FMemInit(byte* mem, uint size);

/**
 * @description: 申请一个内存单元
 * @return 成功：内存地址， 失败：NULL
 */
void* FMemAlloc();

/**
 * @description: 释放指定的内存空间
 * @param 需要释放的内存地址
 * @return 成功：1， 失败：0
 */
int FMemFree(void* ptr);

/**
 * @description 测试定长内存
 */
void fmem_test();





#endif // MEMORY_H