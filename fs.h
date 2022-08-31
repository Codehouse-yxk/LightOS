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
#define SCT_END_FLAG        ((uint)-1)					//扇区无法使用标记【末尾标记】
#define FE_SIZE				(sizeof(FileEntry))
#define FE_ITEM_CNT			(SECT_SIZE / FE_SIZE)		//一个扇区可以存放的FileEntry数量
#define MAP_ITEM_CNT        (SECT_SIZE / sizeof(uint))	//一个扇区可以存放的管理单元的数量


enum
{
	FS_FAILED,	//文件创建失败
	FS_SUCCEED,	//文件创建成功
	FS_EXISTED,	//文件已经存在
	FS_NOEXIST	//文件不存在
};

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

typedef struct
{
	char name[32];		//文件名
	uint sctBegin;		//文件占用的起始扇区
	uint sctNum;		//文件占用的扇区数
	uint lastBytes;		//文件占用最后一个扇区已使用的字节数
	uint type;			//文件类型
	uint inSctIdx;		//当前FileEntry目录项所在扇区号
	uint inSctOff;		//当前FileEntry目录项所在扇区内偏移
	uint reserved[2];
} FileEntry;

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

/**
 * @description: 在根目录创建指定文件
 * @param 需要创建文件的文件名
 * @return 创建成功：1， 创建失败：0
 */
uint FCreate(const char* fileName);

/**
 * @description: 检测根目录是否含有目标文件
 * @param 文件名
 * @return 是：1，否：0
 */
uint FExisted(const char* fileName);



#endif //FS_H