/*
 * @Author: yangxingkun
 * @Date: 2022-08-05 21:57:53
 * @FilePath: \LightOS\vmemory.c
 * @Description: 变长内存管理
 * @Github: https://github.com/Codehouse-yxk
 */

#include "vmemory.h"
//测试
// #include <stdio.h>
// #include <time.h>
// #include <stdlib.h>

static List gVMemList = {0};

void VMemInit(byte* mem, uint size)
{
    List_Init((List*)&gVMemList);
    VMemHead* head = (VMemHead*)mem;
    head->used = 0;
    head->free = size - VM_HEAD_SIZE;
    head->ptr = AddrOff(head, 1);

    List_AddTail(&gVMemList, (ListNode*)head);
}

void* VMemAlloc(uint size)
{
    ListNode* pos = NULL;
    VMemHead* ret = NULL;

    uint alloc = size + VM_HEAD_SIZE;
    List_ForEach(&gVMemList, pos)
    {
        VMemHead* current = (VMemHead*)pos;
        if(current->free >= alloc)
        {
            byte* mem = (byte*)(((uint)current->ptr) + current->used + current->free - alloc);//由空闲区的最后向前分配
            ret = (VMemHead*)mem;
            ret->used = size;               //已使用内存大小就是申请的大小
            ret->free = 0;      
            ret->ptr = AddrOff(ret, 1);     //偏移VMemHead

            current->free -= alloc;         //将分配给新节点的内存减掉
            List_AddAfter((ListNode*)current, (ListNode*)ret);    //将新分配的内存作为节点加入当前节点的后面
            break;
        }
    }

    return ret ? ret->ptr : NULL;
}

int VMemFree(void* ptr)
{
    int ret = 0;

    if( ptr )
    {
        ListNode* pos = NULL;
        List_ForEach(&gVMemList, pos)
        {
            VMemHead* current = (VMemHead*)pos;

            if(isEqual(current->ptr, ptr))
            {
                VMemHead* pre = (VMemHead*)(current->head.prev);

                pre->free += current->used + current->free + VM_HEAD_SIZE;

                List_DelNode((ListNode*)current);

                ret = 1;

                break;
            }
        }
    }

    return ret;
}


// void vmem_test()
// {
//     static byte vmem[0x10000] = {0};
//     void* array[2000] = {0};
//     ListNode* pos = NULL;
//     int i = 0;
    
//     srand((unsigned)time(NULL));
    
//     VMemInit(vmem, sizeof(vmem));
    
//     List_ForEach(&gVMemList, pos)
//     {
//         VMemHead* current = (VMemHead*)pos;
        
//         printf("i = %d\n", i++);
//         printf("used: %d\n", current->used);
//         printf("free: %d\n", current->free);
//         printf("\n");
//     }
    
//     printf("Alloc Test:\n");
    
//     i = 0;
    
//     for(i=0; i<100000; i++)
//     {
//         int ii = i % 2000;
//         byte* p = VMemAlloc(1 + rand() % 400);
        
//         if( array[ii] )
//         {
//             VMemFree(array[ii]);
            
//             array[ii] = NULL; 
//         }
        
//         array[ii] = p;
        
//         if( i % 3 == 0 )
//         {
//             int index = rand() % 2000;
            
//             VMemFree(array[index]);
            
//             array[index] = NULL;
//         }
//     }
    
//     printf("\n");
    
//     printf("Free Test:\n");
    
//     for(i=0; i<2000; i++)
//     {
//         VMemFree(array[i]);
//     }
    
//     i = 0;
    
//     List_ForEach(&gVMemList, pos)
//     {
//         VMemHead* current = (VMemHead*)pos;
        
//         printf("i = %d\n", i++);
//         printf("used: %d\n", current->used);
//         printf("free: %d\n", current->free);
//         printf("\n");
//     }
// }