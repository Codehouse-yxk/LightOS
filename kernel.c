
#include "kernel.h"

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

int SetIntHandler(Gate *pGate, uint ifunc)
{
    int ret = 0;
    if (ret = (ifunc != NULL))
    {
        pGate->offset1 = ifunc & 0xFFFF;
        pGate->selector = GDT_CODE32_FLAT_SELECTOR;
        pGate->dcount = 0;
        pGate->attr = DA_386IGate + DA_DPL0;
        pGate->offset2 = (ifunc >> 16) & 0xFFFF;
    }
    return ret;
}

int GetIntHandler(Gate *pGate, uint *pIfunc)
{
    int ret = 0;

    if(ret = (pGate && pIfunc))
    {
        *pIfunc = (pGate->offset2 << 16) | pGate->offset1;
    }

    return 0;
}