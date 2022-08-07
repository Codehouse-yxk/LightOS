/*
 * @Author: yangxingkun
 * @Date: 2022-08-03 22:11:01
 * @FilePath: \LightOS\fmemory.c
 * @Description: 定长内存管理
 * @Github: https://github.com/Codehouse-yxk
 */

#include "fmemory.h"
#include "utility.h"

//测试
// #include <stdio.h>
// #include <time.h>
// #include <stdlib.h>

static FMemList gFMemList = {0};

void FMemInit(byte* mem, uint size)
{
    
    int i = 0;
    uint max = size / (FM_NODE_SIZE + FM_ALLOC_SIZE);

    gFMemList.max = max;
    gFMemList.nbase = (FMemNode*)mem;
    gFMemList.ubase = (FMemUnit*)((uint)mem + max * FM_NODE_SIZE);
    gFMemList.node = (FMemNode*)mem;

    FMemNode* p = gFMemList.node;

    //构建管理单元链表
    for(i = 0; i < max-1; i++)
    {
        FMemNode* current = (FMemNode*)AddrOff(p, i);
        FMemNode* next = (FMemNode*)AddrOff(p, i + 1);
        current->next = next;
    }

    ((FMemNode*)AddrOff(p, i))->next = NULL;
}

void* FMemAlloc()
{
    void * ret = NULL;

    if( gFMemList.node )
    {
        FMemNode* alloc = gFMemList.node;               //获取一个管理单元

        uint index = AddrIndex(alloc, gFMemList.nbase); //根据管理单元得到分配单元的偏移

        ret = AddrOff(gFMemList.ubase, index);          //根据偏移得到准备分配的内存地址

        gFMemList.node = alloc->next;                   //更新当前链表头部指向下一个管理单元

        alloc->ptr = ret;   //做标记，用于回收内存
    }
    return ret;
}

int FMemFree(void* ptr)
{
    int ret = 0;

    if( ptr )
    {
        uint index = AddrIndex((FMemUnit*)ptr, gFMemList.ubase);  //根据需要释放的分配单元得到偏移
        FMemNode* node = AddrOff(gFMemList.nbase, index);   //根据偏移得到分配单元对应的管理单元
        if((index < gFMemList.max) && isEqual(ptr, node->ptr))  
        {
            node->next = gFMemList.node;    //将内存归还到链表头部
            gFMemList.node = node;          //更新当前链表头
            ret = 1;
        }
    }

    return ret;
}

// void fmem_test()
// {
//     static byte fmem[0x10000] = {0};
//     static void* array[2000] = {0};
//     int i = 0;
    
//     FMemNode* pos = NULL;
    
//     FMemInit(fmem, sizeof(fmem));
    
//     pos = gFMemList.node;
    
//     while( pos )
//     {
//         i++;
//         pos = pos->next;
//     }
    
//     printf("i = %d\n", i++);
    
//     for(i=0; i<100000; i++)
//     {
//         int ii = i % 2000;
//         byte* p = FMemAlloc();
        
//         if( array[ii] )
//         {
//             FMemFree(array[ii]);
            
//             array[ii] = NULL; 
//         }
        
//         array[ii] = p;
        
//         if( i % 3 == 0 )
//         {
//             int index = rand() % 2000;
            
//             FMemFree(array[index]);
            
//             array[index] = NULL;
//         }
//     }
    
//     for(i=0; i<2000; i++)
//     {
//         FMemFree(array[i]);
//     }
    
//     i = 0;
    
//     pos = gFMemList.node;
    
//     while( pos )
//     {
//         i++;
//         pos = pos->next;
//     }
    
//     printf("i = %d\n", i++);
// }
