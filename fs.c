/*
 * @Author: yangxingkun
 * @Date: 2022-08-27 20:42:54
 * @FilePath: \LightOS\fs.c
 * @Description: 文件系统
 * @Github: https://github.com/Codehouse-yxk
 */

#include "fs.h"
#include "memory.h"

static List gFDList = {0};

void FSModInit()
{
    HDModeInit();
    List_Init(&gFDList);
}

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

/**
 * @description: 将目标扇区插入扇区链表尾部
 * @param 扇区链表起始扇区
 * @param 目标扇区
 * @return 成功：1，失败：0
 */
static uint AddToLast(uint sctBegin, uint si)
{
    uint ret = 0;
    uint last = FindLast(sctBegin);

    if(last != SCT_END_FLAG)
    {
        MapPos lmp = FindInMap(last);
        MapPos smp = FindInMap(si);

        if(lmp.pSct && smp.pSct)
        {
            if(lmp.sctOff == smp.sctOff)    //两个扇区对应的管理单元处于同一扇区
            {
                uint* pInt = AddrOff(lmp.pSct, lmp.idxOff);
                *pInt = lmp.sctOff * MAP_ITEM_CNT + smp.idxOff; //将新增的扇区插入链表末尾
            
                pInt = AddrOff(lmp.pSct, smp.idxOff);
                *pInt = SCT_END_FLAG;       //尾部扇区写入结束标记

                ret = HDWrite(lmp.sctOff + FIXED_SCT_SIZE, (byte*)lmp.pSct);
            }
            else
            {
                uint* pInt = AddrOff(lmp.pSct, lmp.idxOff);
                *pInt = smp.sctOff * MAP_ITEM_CNT + smp.idxOff; //将新增的扇区插入链表末尾
            
                pInt = AddrOff(smp.pSct, smp.idxOff);
                *pInt = SCT_END_FLAG;       //尾部扇区写入结束标记

                ret = HDWrite(lmp.sctOff + FIXED_SCT_SIZE, (byte*)lmp.pSct)
                        && HDWrite(smp.sctOff + FIXED_SCT_SIZE, (byte*)smp.pSct);
            }
            Free(lmp.pSct);
            Free(smp.pSct);
        }
    }
    return ret;
}

/**
 * @description: 检查数据链表是否需要扩容
 * @param 文件信息
 * @return 已扩容：1，没有扩容：0
 */
static uint CheckStorage(FSRoot* fe)
{
    uint ret = 0;

    if(fe->lastBytes == SECT_SIZE)  //当数据链表最后一个扇区的空间用完时，需要申请新的扇区
    {
        uint si = AllocSector();

        //将申请到的空间插入数据链表的最后
        if(si != SCT_END_FLAG)
        {
            if(fe->sctBegin == SCT_END_FLAG)
            {
                fe->sctBegin = si;
                ret = 1;
            }
            else
            {
                ret = AddToLast(fe->sctBegin, si);
            }
        }

        if(ret)
        {
            fe->sctNum++;
            fe->lastBytes = 0;
        }
        else
        {
            FreeSector(si);
        }
    }
    return ret;
}

/**
 * @description: 创建新文件的FileEntry并写入根目录区
 * @param 文件名
 * @param 根目录数据起始扇区
 * @param 最后一个扇区已用的字节数
 * @return 新的FileEntry写入成功：1，写入失败：0
 */
static uint CreateFileEntry(const char* fileName, uint sctBegin, uint lastBytes)
{
    uint ret = 0;
    uint last = FindLast(sctBegin);
    FileEntry* feBase = NULL;

    if((last != SCT_END_FLAG) && (feBase = (FileEntry*)ReadSector(last)))
    {
        uint offSet = lastBytes / FE_SIZE;
        FileEntry* fe = AddrOff(feBase, offSet);

        //初始化新的FileEntry
        StrCpy(fe->name, fileName, sizeof(fe->name)-1);
        fe->type = 0;   //0表示文件类型
        fe->sctBegin = SCT_END_FLAG;
        fe->sctNum = 0;
        fe->inSctIdx = last;
        fe->inSctOff = offSet;
        fe->lastBytes = SECT_SIZE;
        
        ret = HDWrite(last, (byte*)feBase);

        Free(feBase);
    }

    return ret;
}

/**
 * @description: 在根目录创建文件
 * @param 文件名
 * @return 成功：1，失败：0
 */
static uint CreateInRoot(const char* fileName)
{
    uint ret = 0;
    FSRoot* root = (FSRoot*)ReadSector(ROOT_SCT_IDX);

    if(root)
    {
        CheckStorage(root);     //检测是否有足够空间容纳新文件的FileEntry

        if(CreateFileEntry(fileName, root->sctBegin, root->lastBytes))
        {
            root->lastBytes += FE_SIZE;                 //创建FileEntry成功，更新最后一个扇区使用的字节数
            ret = HDWrite(ROOT_SCT_IDX, (byte*)root);   //将根目录（1扇区）信息扇区写回硬盘
        }
    }

    return ret;
}

uint FCreate(const char* fileName)
{
    uint ret = FExisted(fileName);

    if(ret != FS_EXISTED)
    {
        ret = CreateInRoot(fileName) ? FS_SUCCEED : FS_FAILED;
    }

    return ret;
}

/**
 * @description: 在扇区中查找目标文件
 * @param 文件名
 * @param 需要遍历的目录项数量
 * @return 文件目录项
 */
static FileEntry* FindInSector(const char* fileName, FileEntry*feBase, uint num)
{
    FileEntry* ret = NULL;
    int i = 0;

    for (i = 0; i < num; i++)
    {
        FileEntry* fe = AddrOff(feBase, i);
        if(StrCmp(fe->name, fileName, -1))
        {
            ret = (FileEntry*)Malloc(FE_SIZE);
            if(ret)
            {
                *ret = *fe;
            }
            break;
        }
    }
    
    return ret;
}

/**
 * @description: 在扇区查找文件项
 * @param 文件名
 * @param 根目录数据起始扇区
 * @param 根目录数据占用的扇区数量
 * @param 最后一个扇区所占用的字节数
 * @return 文件目录项
 */
static FileEntry* FindFileEntry(const char* fileName, uint sctBegin, uint sctNum, uint lastBytes)
{
    FileEntry* ret = NULL;
    uint next = sctBegin;
    uint i = 0;

    //①在前sctNum-1个扇区查找
    for(i=0; i<sctNum-1; i++)
    {
        FileEntry* feBase = (FileEntry*)ReadSector(next);
        if(feBase)
        {
            ret = FindInSector(fileName, feBase, FE_ITEM_CNT);
            Free(feBase);
        }
        
        if(ret) break;

        next = NextSector(next);    //若没有找到，继续查找下一个扇区
    }

    //②查找最后一个扇区
    if(!ret)
    {
        FileEntry* feBase = (FileEntry*)ReadSector(next);
        if(feBase)
        {
            ret = FindInSector(fileName, feBase, lastBytes / FE_SIZE);
            Free(feBase);
        }
    }

    return ret;
}

/**
 * @description: 在根目录查找目标文件
 * @param 文件名
 * @return 文件目录项
 */
static FileEntry* FindInRoot(const char* fileName)
{
    FileEntry* ret = NULL;

    FSRoot* root = (FSRoot*)ReadSector(ROOT_SCT_IDX);

    if(root && root->sctNum)
    {
        ret = FindFileEntry(fileName, root->sctBegin, root->sctNum, root->lastBytes);
        Free(root);
    }

    return ret;
}

uint FExisted(const char* fileName)
{
    uint ret = FS_NOEXIST;
    if(fileName)
    {
        FileEntry* fe = FindInRoot(fileName);
        ret = fe ? FS_EXISTED : FS_NOEXIST;
        Free(fe);
    }
    return ret;
}

/**
 * @description: 检测目标文件是否被打开
 * @param 文件名
 * @return 是：1，否：0
 */
static uint IsOpened(const char* name)
{
    uint ret = 0;
    ListNode* pos = NULL;
    List_ForEach(&gFDList, pos)
    {
        FileDesc* fd = (FileDesc*)pos;
        if(StrCmp(fd->fe.name, name, -1))
        {
            ret = 1;
            break;
        }
    }

    return ret;
}

/**
 * @description: 回收存储文件数据的扇区
 * @param 起始扇区
 * @return 回收的扇区总数
 */
static uint FreeFile(uint sctBegin)
{
    uint ret = 0;
    uint slider = sctBegin;

    while(slider != SCT_END_FLAG)
    {
        uint next = NextSector(slider);

        ret += FreeSector(slider);

        slider = next;
    }

    return ret;
}

/**
 * @description: 移动FileEntry，将src移到dst
 * @param dst
 * @param src
 */
static void MoveFileEntry(FileEntry* dst, FileEntry* src)
{
    if(dst && src)
    {
        uint inSctIdx = dst->inSctIdx;
        uint inSctOff = dst->inSctOff;

        *dst = *src;

        dst->inSctIdx = inSctIdx;
        dst->inSctOff = inSctOff;
    }
}

/**
 * @description: 调整最后一个扇区
 * @param 根目录信息
 * @return 已调整：1，没有调整：0
 */
static uint AdjustStorage(FSRoot* fe)
{
    uint ret = 0;

    if(!fe->lastBytes)
    {
        uint last = FindLast(fe->sctBegin);
        uint prev = FindPre(fe->sctBegin, last);

        if(FreeSector(last) && MarkSector(prev))    //释放最后一个扇区，并标记倒数第二个扇区成为最后一个扇区
        {
            fe->sctNum--;
            fe->lastBytes = SECT_SIZE;
            if(!fe->sctNum) //当扇区数为0，重置sctBegin
            {
                fe->sctBegin = SCT_END_FLAG;
            }
            ret = 1;
        }
    }

    return ret;
}

/**
 * @description: 抹除数据链表中最后n个字节
 * @param 根目录信息
 * @param 需要抹除的字节数
 * @return 真正抹除的字节数
 */
static uint EraseLast(FSRoot* fe, uint bytes)
{
    uint ret = 0;

    while (fe->sctNum && bytes>0)
    {
        if(bytes < fe->lastBytes)   //需要抹除的字节数小于最后一个扇区存储的字节数
        {
            fe->lastBytes -= bytes;
            ret += bytes;
            bytes = 0;
        }
        else
        {
            bytes -= fe->lastBytes;
            ret += fe->lastBytes;
            fe->lastBytes = 0;

            AdjustStorage(fe);      //调整最后一个扇区
        }
    }
    
    return ret;
}

/**
 * @description: 从根目录删除目标文件
 * @param 文件名
 * @return 成功：1， 失败：0
 */
static uint DeleteInRoot(const char* fileName)
{
    uint ret = 0;
    FSRoot* root = (FSRoot*)ReadSector(ROOT_SCT_IDX);
    FileEntry* fe = (FileEntry*)FindInRoot(fileName);

    if(root && fe)
    {
        uint lastSct = FindLast(root->sctBegin);        //获取保存FileEntry的最后一个扇区
        FileEntry* feTarget = ReadSector(fe->inSctIdx); //读取目标FileEntry所在的扇区
        FileEntry* feLast = (lastSct != SCT_END_FLAG) ? ReadSector(lastSct) : NULL;

        if(feTarget && feLast)
        {
            uint lastOff = root->lastBytes / FE_SIZE - 1;
            FileEntry* lastItem = AddrOff(feLast, lastOff);             //定位到最后一个扇区中的最后一个FileEntry
            FileEntry* targetItem = AddrOff(feTarget, fe->inSctOff);    //定位到目标文件FileEntry

            FreeFile(targetItem->sctBegin);
            MoveFileEntry(targetItem, lastItem);
            EraseLast(root, FE_SIZE);

            ret = HDWrite(ROOT_SCT_IDX, (byte*)root) 
                    && HDWrite(fe->inSctIdx, (byte*)feTarget);
        }

        Free(feTarget);
        Free(feLast);
    }

    Free(root);
    Free(fe);    

    return ret;
}

uint FDelete(const char* fileName)
{
    return fileName && !IsOpened(fileName) && (DeleteInRoot(fileName) ? FS_SUCCEED : FS_FAILED);
}

static uint FlushFileEntry(FileEntry* fe)
{
    uint ret = 0;

    FileEntry* feBase = ReadSector(fe->inSctIdx);
    FileEntry* feInSct = AddrOff(feBase, fe->inSctOff);

    if(feBase && feInSct)
    {
        *feInSct = *fe;
        ret = HDWrite(feInSct->inSctIdx, (byte*)feBase);
    }
    Free(feBase);
    return ret;
}

uint FRename(const char* oldName, const char* newName)
{
    uint ret = FS_FAILED;

    if(oldName && !IsOpened(oldName) && newName)
    {
        FileEntry* oldFe = FindInRoot(oldName);
        FileEntry* newFe = FindInRoot(newName);

        if(oldFe && !newFe)
        {
            StrCpy(oldFe->name, newName, sizeof(oldFe->name)-1);
            if(FlushFileEntry(oldFe))
            {
                ret = FS_SUCCEED;
            }
        }
        Free(oldFe);
        Free(newFe);
    }

    return ret;
}

uint FOpen(const char* fileName)
{
    FileDesc* ret = NULL;

    if(fileName && !IsOpened(fileName))
    {
        FileEntry* fe = FindInRoot(fileName);
        ret = (FileDesc*)Malloc(FD_SIZE);
        if(fe && ret)
        {
            ret->fe = *fe;
            ret->objIdx = SCT_END_FLAG;
            ret->offset = SECT_SIZE;
            ret->changed = 0;
             List_Add(&gFDList, (ListNode*)ret);
        }
        Free(fe);
    }

    return (uint)ret;
}

/**
 * @description: 检测文件描述符是否合法
 * @param 文件描述符
 * @return 合法：1， 不合法：0
 */
static uint IsFDValid(FileDesc* fd)
{
    uint ret = 0;
    ListNode* pos = NULL;

    List_ForEach(&gFDList, pos)
    {
        if(isEqual(fd, pos))
        {
            ret = 1;
            break;
        }
    }

    return ret;
}

/**
 * @description: 将文件缓冲区的数据刷新到硬盘
 * @param 文件描述符
 * @return 成功：1， 失败：0
 */
static uint FlushCache(FileDesc* fd)
{
    uint ret = 0;

    if(fd->changed)
    {
        uint sctIdx = FindIndex(fd->fe.sctBegin, fd->objIdx);   //根据文件占用的扇区链表找到当前正在修改的扇区

        if((sctIdx != SCT_END_FLAG) && (ret = HDWrite(sctIdx, fd->cache)))
        {
            fd->changed = 0;
        }
    }
    else    //数据没有发生改变，则不需要写入硬盘
    {
        ret = 1; 
    }

    return ret;
}

static uint ToFlush(FileDesc* fd)
{
    return FlushCache(fd) && FlushFileEntry(&fd->fe);
}

void FClose(uint fd)
{
    FileDesc* pfd = (FileDesc*)fd;

    if(IsFDValid(pfd))      //检测文件描述符是否合法
    {
        ToFlush(pfd);       //将文件缓冲区的数据刷新到硬盘
        List_DelNode((ListNode*)pfd);   //删除该文件描述符
        Free(pfd);
    }
}

static uint ReadToCache(FileDesc* fd, uint objIdx)
{
    uint ret = 0;

    if(objIdx < fd->fe.sctNum)
    {
        uint sctIdx = FindIndex(fd->fe.sctBegin, objIdx);
        
        ToFlush(fd);        //先将旧缓冲区中的数据刷新到硬盘
        
        if((sctIdx != SCT_END_FLAG) && (ret = HDRead(sctIdx, fd->cache)))   //将新扇区数据读到缓冲区
        {
            fd->objIdx = objIdx;
            fd->offset = 0;
            fd->changed = 0;
        }
    }
    return ret;
}

/**
 * @description: 准备缓冲区
 * @param 文件描述符
 * @param 数据链表中扇区位置
 * @return 成功：1，失败：0
 */
static uint PrepareCache(FileDesc* fd, uint objIdx)
{
    CheckStorage((FSRoot*)&fd->fe);
    return ReadToCache(fd, objIdx);
}

/**
 * @description: 拷贝数据到缓冲区
 * @param 文件描述符
 * @param 数据buff
 * @param 需要写入的数据长度
 * @return 成功：拷贝的数据长度，失败：-1
 */
static int CopyToCache(FileDesc* fd, byte* buff, uint len)
{
    int ret = -1;

    if(fd->objIdx != SCT_END_FLAG)
    {
        uint n = SECT_SIZE - fd->offset;
        byte* p = AddrOff(fd->cache, fd->offset);
        n = Min(n, len);

        MemCpy(p, buff, n);
        
        fd->offset += n;
        fd->changed = 1;

        //判断是否是最后一个数据扇区，如果数据长度变大，需要更新读写指针偏移
        if(((fd->fe.sctNum - 1) == fd->objIdx) && (fd->fe.lastBytes < fd->offset))
        {
            fd->fe.lastBytes = fd->offset;
        }
        ret = n;
    }

    return ret;
}

/**
 * @description: 写数据
 * @param 文件描述符
 * @param 数据buff
 * @param 数据长度
 * @return 成功：写入的数据长度，失败：-1
 */
static int ToWrite(FileDesc* fd, byte* buff, uint len)
{
    int ret = 1;
    int i = 0;
    int n = 0;

    while((i<len) && ret)
    {
        byte* p = AddrOff(buff, i);

        if(fd->offset == SECT_SIZE) //缓冲区已满，需要准备下一个缓冲区
        {
            ret = PrepareCache(fd, fd->objIdx + 1);
        }

        if(ret)
        {
            n = CopyToCache(fd, p, len - i);
            if(n < 0)   
            {
                i = -1;
                break;
            }
            i += n;
        }
    }

    ret = i;

    return ret;
}

int FWrite(uint fd, byte* buff, uint len)
{
    int ret = -1;

    if(IsFDValid((FileDesc*)fd) && buff && len)
    {
        ret = ToWrite((FileDesc*)fd, buff, len);
    }

    return ret;
}




