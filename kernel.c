/*
 * @Author: yangxingkun
 * @Date: 2022-07-05 00:20:04
 * @FilePath: \LightOS\kernel.c
 * @Description: 内核gdt、idt、page等操作
 * @Github: https://github.com/Codehouse-yxk
 */


#include "kernel.h"

GdtInfo gGdtInfo = {0};

IdtInfo gIdtInfo = {0};

int SetDescValue(Descriptor *pDesc, uint base, uint limit, ushort attr)
{
    int ret = 0;

    if (ret = (!!pDesc))
    {
        pDesc->limit1 = limit & 0xFFFF;
        pDesc->base1 = base & 0xFFFF;
        pDesc->base2 = (base >> 16) & 0xFF;
        pDesc->attr1 = attr & 0xFF;
        pDesc->attr2_limit2 = ((limit >> 16) & 0xF) | ((attr >> 8) & 0xF0);
        pDesc->base3 = (base >> 24) & 0xFF;
    }

    return ret;
}

int GetDescValue(Descriptor *pDesc, uint *pBase, uint *pLimit, ushort *pAttr)
{
    int ret = 0;

    if (ret = (pDesc && pBase && pLimit && pAttr))
    {
        *pBase = (pDesc->base3 << 24) | (pDesc->base2 << 16) | pDesc->base1;
        *pLimit = ((pDesc->attr2_limit2 & 0xF) << 16) | pDesc->limit1;
        *pAttr = ((pDesc->attr2_limit2 & 0xF0) << 8) | pDesc->attr1;
    }

    return ret;
}

void ConfigPageTable()
{
    uint* Tblbase = (void*)PageTblBase;
    uint index = BaseOfApp / 0x1000 - 1;
    int i = 0;
    for(i=0; i<=index; i++)
    {
        uint* addr = Tblbase + i;
        uint value = *addr;
        value = value & 0xFFFFFFFD;
        *addr = value;
    }
}