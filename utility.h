#ifndef UTILITY_H
#define UTILITY_H

#define AddrOff(a, i)   ((void *)((uint)a + i * sizeof(*a)))

/* 延时函数 */
void Delay(int n);

#endif //UTILITY_H