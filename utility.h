#ifndef UTILITY_H
#define UTILITY_H

#define AddrOff(a, i)   ((void *)((uint)a + i * sizeof(*a)))

#define isEqual(a, b)           \
({                              \
    unsigned ta = (unsigned)a;  \
    unsigned tb = (unsigned)b;  \
    !(ta - tb);                 \
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

/* 延时函数 */
void Delay(int n);

/* 字符串拷贝函数 */
char* StrCpy(char* dst, const char* src, int n);

#endif //UTILITY_H