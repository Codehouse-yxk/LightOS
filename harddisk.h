#ifndef HARD_DISK_H
#define HARD_DISK_H

#include "type.h"

#define SECT_SIZE   512

/**
 * @description: 硬盘模块初始化
 */
void HDModeInit();

/**
 * @description: 获取硬盘扇区数
 * @return 扇区数
 */
uint GetHDSectors();

/**
 * @description: 往硬盘写入数据
 * @param 扇区编号
 * @param 要写入的数据
 * @return 写入成功：1， 写入失败：0
 */
uint HDWrite(uint si, byte* buf);

/**
 * @description: 从硬盘读取数据
 * @param 扇区编号
 * @param 读到的数据
 * @return 读取成功：1， 读取失败：0
 */
uint HDRead(uint si, byte* buf);


#endif  //HARD_DISK_H