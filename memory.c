/*
 * @Author: yangxingkun
 * @Date: 2022-08-06 14:59:00
 * @FilePath: \LightOS\memory.c
 * @Description: 内存管理模块
 * @Github: https://github.com/Codehouse-yxk
 */

#include "memory.h"


void MemModInit(byte* mem, uint size)
{
    //将需要管理的内存一分为二，一半用定长内存管理，一半用变长内存管理
    byte* fmem = mem;
    uint fsize = size / 2;
    byte* vmem = AddrOff(fmem, fsize);
    uint vsize = size - fsize;
    FMemInit(fmem, fsize);
    VMemInit(vmem, vsize);
}

void* Malloc(uint size)
{
    void* ret = NULL;
    //若申请内存小于32字节，则从定长内存申请
    if(size <= FM_ALLOC_SIZE)
    {
        ret = FMemAlloc();
    }
    // 若定长内存申请失败或者申请内存大于32字节，则从变长内存申请
    if( !ret )
    {
        ret = VMemAlloc(size);
    }
}

void Free(void* ptr)
{
    if( ptr )
    {
        if(!FMemFree(ptr))
        {
            VMemFree(ptr);
        }
    }
}
