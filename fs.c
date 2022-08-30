/*
 * @Author: yangxingkun
 * @Date: 2022-08-27 20:42:54
 * @FilePath: \LightOS\fs.c
 * @Description: 文件系统
 * @Github: https://github.com/Codehouse-yxk
 */

#include "fs.h"
#include "memory.h"

/**
 * @description: 读取扇区数据
 * @param 扇区号
 * @return 扇区数据
 */
static void* ReadSector(uint si)
{
    void* ret = NULL;

    if(si != SCT_END_FLAG)
    {
        ret = Malloc(SECT_SIZE);
        if(!(ret && HDRead(si, ret)))
        {
            Free(ret);
            ret = NULL;
        }
    }
    return ret;
}

/**
 * @description: 根据原始数据扇区号（即数据区的扇区）查找对应分配单元的位置
 * @param 原始扇区号
 * @return 分配单元信息
 */
static MapPos FindInMap(uint si)
{
    MapPos ret = {0};

    FSHeader* header = (si != SCT_END_FLAG) ? ReadSector(HEADER_SCT_IDX) : NULL;

    if(header)
    {
        uint offset = si - header->mapSize - FIXED_SCT_SIZE;    //计算原始数据扇区相对分配扇区偏移
        uint sctOff = offset / MAP_ITEM_CNT;                    //计算原始数据扇区对应的分配扇区的偏移【扇区偏移】
        uint idxOff = offset % MAP_ITEM_CNT;                    //计算原始数据扇区对应的分配扇区的偏移【扇区内偏移】
        uint* ps = ReadSector(sctOff + FIXED_SCT_SIZE);         //读取对应分配扇区

        if(ps)
        {
            ret.pSct = ps;
            ret.sctIdx = si;
            ret.sctOff = sctOff;
            ret.idxOff = idxOff;
        }
        Free(header);
    }

    return ret;
}

/**
 * @description: 获取目标扇区的后继扇区
 * @param 目标扇区
 * @return 扇区号
 */
static uint NextSector(uint si)
{
    uint ret = SCT_END_FLAG;

    FSHeader* header = (si != SCT_END_FLAG) ? ReadSector(HEADER_SCT_IDX) : NULL;

    if(header)
    {
        MapPos mp = FindInMap(si);       //查找目标数据扇区对应的分配单元信息
        if(mp.pSct)
        {
            uint* pInt = AddrOff(mp.pSct, mp.idxOff);

            if(*pInt != SCT_END_FLAG)
            {
                ret = *pInt + header->mapSize + FIXED_SCT_SIZE; //根据分配单元信息的数据计算链表中下一个扇区的绝对位置
            }

            Free(mp.pSct);
        }
        Free(header);
    }

    return ret;
}

/**
 * @description: 查找目标扇区所在链表的最后一个扇区
 * @param 目标扇区
 * @return 链表中的最后一个扇区
 */
static uint FindLast(uint sctBegin)
{
    uint ret = SCT_END_FLAG;
    uint next = sctBegin;

    while (next != SCT_END_FLAG)
    {
        ret = next;
        next = NextSector(next);
    }
    
    return ret;
}

/**
 * @description: 查找从sctBegin开始的扇区链表中，si扇区的前一个扇区
 * @param 扇区链表起始扇区
 * @param 目标扇区
 * @return 目标扇区前一个扇区
 */
static uint FindPre(uint sctBegin, uint si)
{
    uint ret = SCT_END_FLAG;
    uint next = sctBegin;

    while ((next != SCT_END_FLAG) && (next != si))
    {
        ret = next;
        next = NextSector(next);
    }

    if(next == SCT_END_FLAG)
    {
        ret = SCT_END_FLAG;
    }
    
    return ret;
}

/**
 * @description: 查找sctBegin扇区链表中的第idx个扇区
 * @param 扇区链表起始扇区
 * @param 索引
 * @return 链表中的第idx个扇区号
 */
static uint FindIndex(uint sctBegin, uint idx)
{
    uint ret = sctBegin;
    uint i = 0 ;

    while ((i < idx) && (ret != SCT_END_FLAG))
    {
        i++;
        ret = NextSector(ret);
    }

    return ret;    
}

/**
 * @description: 标记目标扇区不可用
 * @return 标记成功：1， 标记失败0
 */
static uint MarkSector(uint si)
{
    uint ret = (si == SCT_END_FLAG) ? 1 : 0;
    MapPos mp = FindInMap(si);
    if(!ret && mp.pSct)
    {
        uint *pInt = AddrOff(mp.pSct, mp.idxOff);

        *pInt = SCT_END_FLAG;

        ret = HDWrite(mp.sctOff + FIXED_SCT_SIZE, (byte*)mp.pSct);

        Free(mp.pSct);
    }

    return ret;
}

/**
 * @description: 申请一个扇区
 * @return 成功：申请到的扇区的绝对地址，失败：SCT_END_FLAG
 */
static uint AllocSector()
{
    uint ret = SCT_END_FLAG;

    FSHeader* header = ReadSector(HEADER_SCT_IDX);
    if(header && (header->freeBegin != SCT_END_FLAG))
    {
        MapPos mp = FindInMap(header->freeBegin);       //获取当前空闲链表首个扇区，并查找其对应的分配单元信息
        if(mp.pSct)
        {
            uint* pInt = AddrOff(mp.pSct, mp.idxOff);
            uint next = *pInt;
            uint flag = 1;
            ret = header->freeBegin;    //将申请到的扇区地址返回

            header->freeBegin = next + FIXED_SCT_SIZE + header->mapSize;    //计算下一个空闲扇区地址起始地址【绝对地址】
            header->freeNum--;          //空闲扇区减1

            *pInt = SCT_END_FLAG;       //标记当前扇区标记为不可用

            flag = HDWrite(HEADER_SCT_IDX, (byte*)header);      //将0扇区信息写回硬盘
            flag = flag && HDWrite(mp.sctOff + FIXED_SCT_SIZE, (byte*)mp.pSct); //将已修改的分配扇区写回硬盘

            if(!flag)
            {
                ret = SCT_END_FLAG; 
            }
            Free(mp.pSct);
        }
        Free(header);
    }

    return ret;
}

/**
 * @description: 回收一个扇区
 * @return 成功：1， 失败：SCT_END_FLAG
 */
static uint FreeSector(uint si)
{
    uint ret = SCT_END_FLAG;

    FSHeader* header = (si != SCT_END_FLAG) ? ReadSector(HEADER_SCT_IDX) : NULL;

    if(header)
    {
        MapPos mp = FindInMap(si);       //查找目标数据扇区对应的分配单元信息
        if(mp.pSct)
        {
            uint* pInt = AddrOff(mp.pSct, mp.idxOff);

            *pInt = header->freeBegin - FIXED_SCT_SIZE - header->mapSize;   //将当前扇区回收至管理单元空闲链表头部
            header->freeBegin = si;     //空闲链表头部就是回收的扇区绝对地址
            header->freeNum++;

            ret = HDWrite(HEADER_SCT_IDX, (byte*)header) 
                    && HDWrite(mp.sctOff + FIXED_SCT_SIZE, (byte*)mp.pSct); //将已修改的扇区写回硬盘

            Free(mp.pSct);
        }
        Free(header);
    }

    return ret;
} 

uint FSFormat()
{
    FSHeader* header = (FSHeader*)Malloc(SECT_SIZE);
    FSRoot* root = (FSRoot*)Malloc(SECT_SIZE);
    uint* p = (uint*)Malloc(MAP_ITEM_CNT * sizeof(uint));
    uint ret = 0;
    
    if(header && root && p)
    {
        uint i = 0;
        uint j = 0;
        uint current = 0;

        //①初始化0号扇区【文件系统信息区】，并写回硬盘
        StrCpy(header->magic, FS_MAGIC, sizeof(header->magic)-1);
        header->sctNum = GetHDSectors();
        header->mapSize = (header->sctNum - FIXED_SCT_SIZE) / 129 + !!((header->sctNum - FIXED_SCT_SIZE) % 129); //向上取整
        header->freeNum = header->sctNum - header->mapSize - FIXED_SCT_SIZE;
        header->freeBegin = FIXED_SCT_SIZE + header->mapSize;
        ret = HDWrite(HEADER_SCT_IDX, (byte*)header);       //将0扇区写回硬盘

        //②初始化1号扇区【根目录区】，并写回硬盘
        StrCpy(root->magic, ROOT_MAGIC, sizeof(root->magic)-1);
        root->sctNum = 0;   //初始化时根目录是空
        root->sctBegin = SCT_END_FLAG;
        root->lastBytes = SECT_SIZE;
        ret = ret && HDWrite(ROOT_SCT_IDX, (byte*)root);    //将根目录区写回硬盘

        //③构建扇区分配表中的空闲扇区链表，并写回硬盘
        for(i=0; ret && (i<header->mapSize) && (current<header->freeNum); i++)
        {
            for(j=0; j<MAP_ITEM_CNT; j++)
            {
                uint *pInt = AddrOff(p, j);
                if(current < header->freeNum)
                {
                    *pInt = current + 1;        //当前节点指向下一个数据区节点【将数据区对应的分配单元组织成链表】
                    if(current == header->freeNum - 1)
                    {
                        *pInt = SCT_END_FLAG;   //最后一个映射节点
                    }
                    current++;
                }
                else
                {
                    break;
                }
            }
            ret = HDWrite(i+FIXED_SCT_SIZE, (byte*)p);   //将扇区分配区（表）的数据写回硬盘
        }
        
        Free(p);
        Free(root);
        Free(header);
    }
    return ret;
}

uint FSIsFormatted()
{
    uint ret = 0;

    FSHeader* header = (FSHeader*)ReadSector(HEADER_SCT_IDX);
    FSRoot* root = (FSRoot*)ReadSector(ROOT_SCT_IDX);
    if(header && root)
    {
        ret = (StrCmp(header->magic, FS_MAGIC, -1))
                && (header->sctNum == GetHDSectors())
                && (StrCmp(root->magic, ROOT_MAGIC, -1));
        
        Free(header);
        Free(root);
    }
    return ret;
}






