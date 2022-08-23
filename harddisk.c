/*
 * @Author: yangxingkun
 * @Date: 2022-08-22 08:11:11
 * @FilePath: \LightOS\harddisk.c
 * @Description: 硬盘驱动模块
 * @Github: https://github.com/Codehouse-yxk
 */

#include "harddisk.h"
#include "memory.h"

#define ATA_IDENTIFY    0xEC    //获取硬盘信息寄存器指令
#define ATA_READ        0x20    //读取扇区指令
#define ATA_WRITE       0x30    //写扇区指令

//控制块寄存器
#define REG_DEV_CTRL  0x3F6     

//命令块寄存器
#define REG_DATA      0x1F0
#define REG_FEATURES  0x1F1
#define REG_ERROR     0x1F1
#define REG_NSECTOR   0x1F2
#define REG_LBA_LOW   0x1F3
#define REG_LBA_MID   0x1F4
#define REG_LBA_HIGH  0x1F5
#define REG_DEVICE    0x1F6
#define REG_STATUS    0x1F7
#define REG_COMMAND   0x1F7

//状态寄存器Mask
#define	STATUS_BSY  0x80
#define	STATUS_DRDY 0x40
#define	STATUS_DFSE 0x20
#define	STATUS_DSC  0x10
#define	STATUS_DRQ  0x08
#define	STATUS_CORR 0x04
#define	STATUS_IDX  0x02
#define	STATUS_ERR  0x01

extern byte ReadPort(ushort port);
extern void WritePort(ushort port, byte value);
extern void WritePortW(ushort port, ushort* buf, uint n);
extern void ReadPortW(ushort port, ushort* buf, uint n);

typedef struct
{
    byte LBA_low;
    byte LBA_mid;
    byte LBA_high;
    byte device_reg;
    byte command;
} HDRegValue;

/**
 * @description: 硬盘设备是否繁忙
 * @return 1：是，0：否
 */
static uint IsDevBusy()
{
    uint ret = 0;
    uint i = 0;
    
    while( (i < 500) && (ret = (ReadPort(REG_STATUS) & STATUS_BSY)) )
    {
        i++;
    }
    
    return ret;
}

/**
 * @description: 硬盘设备是否就绪
 * @return 1：是，0：否
 */
static uint IsDevReady()
{
    return !(ReadPort(REG_STATUS) & STATUS_DRDY);
}

/**
 * @description: 硬盘数据是否就绪（可传输数据）
 * @return 1：是，0：否
 */
static uint IsDataReady()
{
    return (ReadPort(REG_STATUS) & STATUS_DRQ);
}

/**
 * @description: 生成device寄存器
 * @param LBA【扇区逻辑地址】
 * @return device寄存器
 */
static uint MakeDevRegValue(uint si)
{
    return (0xE0 | ((si>>24) & 0x0F));
}

void HDModeInit()
{
    
}

static void MakeRegValues(HDRegValue* hdrv, uint si, uint action)
{
    hdrv->LBA_low = si & 0xFF;
    hdrv->LBA_mid = (si >> 8) & 0xFF;
    hdrv->LBA_high = (si >> 16) & 0xFF;
    hdrv->device_reg = MakeDevRegValue(si);
    hdrv->command = action;
}

static void WritePorts(HDRegValue* hdrv)
{
    WritePort(REG_FEATURES, 0);                 //默认属性
    WritePort(REG_NSECTOR, 1);                  //操作一个扇区
    WritePort(REG_LBA_LOW, hdrv->LBA_low);      //设置lba地址
    WritePort(REG_LBA_MID, hdrv->LBA_mid);
    WritePort(REG_LBA_HIGH, hdrv->LBA_high);
    WritePort(REG_DEVICE, hdrv->device_reg);
    WritePort(REG_COMMAND, hdrv->command);      //操作指令

    WritePort(REG_DEV_CTRL, 0);
}

uint GetHDSectors()
{
    static uint ret = -1;
    if((ret == -1) && IsDevReady())
    {
        byte* buf = Malloc(SECT_SIZE);
        HDRegValue hdrv = {0};

        MakeRegValues(&hdrv, 0, ATA_IDENTIFY);
        WritePorts(&hdrv);

        if(!IsDevBusy() && IsDataReady() && buf)
        {
            ushort* data = (ushort*)buf;
            ReadPortW(REG_DATA, data, SECT_SIZE>>1);
            ret = (data[61] << 16) | (data[60]);        //ATA硬盘手册中约定了60、61地址保存的是扇区数
        }
        Free(buf);
    }
    return ret;
}

uint HDWrite(uint si, byte* buf)
{
    uint ret = 0;

    if((si < GetHDSectors()) && buf && !IsDevBusy())
    {
        HDRegValue hdrv = {0};
        MakeRegValues(&hdrv, si, ATA_WRITE);
        WritePorts(&hdrv);

        if(ret = (!IsDevBusy() && IsDataReady()))
        {
            ushort* data = (ushort*)buf;
            WritePortW(REG_DATA, data, SECT_SIZE>>1);
        }
    }

    return ret;
}

uint HDRead(uint si, byte* buf)
{
    uint ret = 0;

    if((si < GetHDSectors()) && buf && !IsDevBusy())
    {
        HDRegValue hdrv = {0};
        MakeRegValues(&hdrv, si, ATA_READ);
        WritePorts(&hdrv);

        if(ret = (!IsDevBusy() && IsDataReady()))
        {
            ushort* data = (ushort*)buf;
            ReadPortW(REG_DATA, data, SECT_SIZE>>1);
        }
    }

    return ret;
}
