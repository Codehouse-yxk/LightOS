#ifndef FS_H
#define FS_H

#include "type.h"
#include "utility.h"
#include "harddisk.h"

#define FS_MAGIC            "LightFS-v1.0"
#define ROOT_MAGIC          "ROOT"
#define HEADER_SCT_IDX      0
#define ROOT_SCT_IDX        1
#define FIXED_SCT_SIZE      2
#define SCT_END_FLAG        ((uint)-1)
#define MAP_ITEM_CNT        (SECT_SIZE / sizeof(uint))


typedef struct			//存储于0扇区，记录文件系统概要信息
{
	byte forJmp[4];		//预留给jmp指令，0号扇区存储引导程序
	char magic[32];		//存放字符串，显示当前文件系统名称
	uint sctNum;		//当前硬盘扇区数量
	uint mapSize;		//扇区分配表的大小
	uint freeNum;		//维护空闲链表
	uint freeBegin;		//维护空闲链表
}FSHeader;


typedef struct			//存储于1扇区，记录根目录相关信息
{
	char magic[32];		//记录根目录名称

	//记录根目录涉及哪些扇区
	uint sctBegin;		//起始扇区
	uint sctNum;		//占用的扇区总数
	uint lastBytes;		//记录最后一个扇区用了多少字节
}FSRoot;

typedef struct
{
    uint* pSct;         //指向对应分配（管理）单元所在的扇区
    uint sctIdx;        //原始数据绝对扇区号
    uint sctOff;        //原始数据绝对扇区号对应的分配单元的位置【扇区偏移】
    uint idxOff;        //原始数据绝对扇区号对应的分配单元的位置【扇区内偏移】
} MapPos;

/**
 * @description: 格式化硬盘
 * @return 格式化成功：1，格式化失败：0
 */
uint FSFormat();

/**
 * @description: 判断硬盘是否已经格式化
 * @return 是：1， 否：0
 */
uint FSIsFormatted();



#endif //FS_H