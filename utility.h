#ifndef UTILITY_H
#define UTILITY_H

#include "type.h"

#define AddrOff(a, i)   ((void *)((uint)(a) + (i) * sizeof(*(a))))

#define isEqual(a, b)               \
({                                  \
    unsigned ta = (unsigned)(a);    \
    unsigned tb = (unsigned)(b);    \
    !(ta - tb);                     \
})

#define OffsetOf(type, member)  ((unsigned)&(((type*)0)->member))

/* 
    ptr：member成员指针 
    type：目标结构体类型
    member：结构体中的目标成员
*/
#define ContainerOf(ptr, type, member)                  \
({                                                      \
    const typeof(((type*)0)->member)* __mptr = (ptr);    \
    (type*)((char*)__mptr - OffsetOf(type, member));     \
})

#define Min(a, b)   (((a) > (b)) ? (b) : (a))
#define Max(a, b)   (((a) > (b)) ? (a) : (b))

/* 计算数组元素的个数 */
#define ArrayNum(a)  (sizeof(a)/sizeof(*(a)))


/* 延时函数 */
void Delay(int n);

/* 字符串拷贝函数 */
char* StrCpy(char* dst, const char* src, uint n);

/**
 * @description: 获取字符串长度
 * @param 目标字符串
 * @return 长度
 */
uint StrLen(const char* str);

/**
 * @description: 比较两个字符串是否相等
 * @param 第一个字符串
 * @param 第二个字符串
 * @param 需要比较的长度
 * @return 相等：1，不相等：0
 */
uint StrCmp(const char* str1, const char* str2, uint n);

/**
 * @description: 内存数据拷贝
 * @param 目的地址
 * @param 源地址
 * @param 拷贝长度
 * @return 目的地址
 */
byte* MemCpy(byte* dst, const byte* src, uint n);

/**
 * @description: 内存数据设置
 * @param 目的地址
 * @param 设置的长度
 * @param 设置的数据
 * @return 目的地址
 */
byte* MemSet(byte* dst, uint n, byte val);

#endif //UTILITY_H