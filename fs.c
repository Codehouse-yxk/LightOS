/*
 * @Author: yangxingkun
 * @Date: 2022-08-27 20:42:54
 * @FilePath: \LightOS\fs.c
 * @Description: 文件系统
 * @Github: https://github.com/Codehouse-yxk
 */

#include "fs.h"
#include "memory.h"


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
                    *pInt = current + 1;        //当前节点指向下一个节点【将数据区对应的分配单元组织成链表】
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






