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

uint StrLen(const char* str)
{
    uint ret = 0;
    while (str && str[ret])
    {
        ret++;
    }
    return ret;
}

uint StrCmp(const char* str1, const char* str2, uint n)
{
    uint ret = 1;
    if(!isEqual(str1, str2))
    {
        uint len1 = StrLen(str1);
        uint len2 = StrLen(str2);
        uint len = Min(Min(len1, len2), n);
        int i = 0;

        ret = isEqual(len1, len2);

        for(i=0; (i<len) && ret; i++)
        {
            ret = isEqual(str1[i], str2[i]);
        }
    }
    return ret;
}