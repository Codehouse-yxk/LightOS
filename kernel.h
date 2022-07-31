#ifndef KERNEL_H
#define KERNEL_H

#include "type.h"
#include "const.h"

/* GDT描述符内存结构 */
typedef struct {
    ushort limit1;          //段界限1
    ushort base1;           //段基址1
    byte   base2;           //段基址2
    byte   attr1;           //属性1
    byte   attr2_limit2;    //属性2+段界限2
    byte   base3;           //段基址3
} Descriptor;

/* 全局段描述符表 */
typedef struct {
    Descriptor* const entry;
    const int size;
} GdtInfo;

/* 门描述符结构 */
typedef struct {
    ushort offset1;
    ushort selector;
    byte   dcount;
    byte   attr;
    ushort offset2;
} Gate;

/* 中断描述符表 */
typedef struct {
    Gate* const entry;
    const int    size;
} IdtInfo;

GdtInfo gGdtInfo;

IdtInfo gIdtInfo;


/* 设置段描述符的值（基址、界限、属性） */
int SetDescValue(Descriptor* pDesc, uint base, uint limit, ushort attr);

/* 获取段描述符的值（基址、界限、属性） */
int GetDescValue(Descriptor* pDesc, uint* pBase, uint* pLimit, ushort* pAttr);

/* 配置页表属性 */
void ConfigPageTable();

#endif  //KERNEL_H