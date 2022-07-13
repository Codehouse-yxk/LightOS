#ifndef UTILITY_H
#define UTILITY_H

#define AddrOff(a, i)   ((void *)((uint)a + i * sizeof(*a)))

void Delay(int n);

#endif //UTILITY_H