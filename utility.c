/*
 * @Author: yangxingkun
 * @Date: 2022-07-13 08:11:29
 * @FilePath: \LightOS\utility.c
 * @Description: 通用模块，包含：延迟函数等
 * @Github: https://github.com/Codehouse-yxk
 */

#include "utility.h"

void Delay(int n)
{
    while (n-- > 0)
    {

        int i = 0;
        int j = 0;
        for (i = 0; i < 1000; i++)
        {
            for (j = 0; j < 1000; j++)
            {
                asm volatile("nop\n");
            }
        }
    }
}

char *StrCpy(char *dst, const char *src, int n)
{
    char *ret = dst;
    int i = 0;
    for (i = 0; (src[i])&&(i<n); i++)
    {
        dst[i] = src[i];
    }
    dst[i] = 0;
    return ret;
}