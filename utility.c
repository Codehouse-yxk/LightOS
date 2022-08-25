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

char *StrCpy(char *dst, const char *src, uint n)
{
    char *ret = dst;
    uint dstA = (uint)dst;
    uint srcA = (uint)src;
    int i = 0;

    //防止地址覆盖
    if(dstA < srcA)
    {
        for (i = 0; (src[i])&&(i<n); i++)
        {
            dst[i] = src[i];
        }
        dst[i] = 0;
    }

    if(dstA > srcA)
    {
        uint len = StrLen(src);
        len = Min(len, n);
        dst[len] = 0;
        for (i=len-1; i>=0; i--)
        {
            dst[i] = src[i];
        }
    }

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

byte* MemCpy(byte* dst, const byte* src, uint n)
{
    byte* ret = dst;
    uint dstA = (uint)dst;
    uint srcA = (uint)src;
    int i = 0;

    //防止地址覆盖
    if(dstA < srcA) 
    {
        for(i=0; i<n; i++)
        {
            dst[i] = src[i];
        }
    }

    if(dstA > srcA)
    {
        for(i=n-1; i>=0; i--)
        {
            dst[i] = src[i];
        }
    }

    return ret;
}

byte* MemSet(byte* dst, uint n, byte val)
{
    byte* ret = dst;
    int i = 0;

    while(i < n)
    {
        dst[i++] = val;
    }

    return ret;
}