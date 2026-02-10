/*
 * Copyright (C) 2021-2025 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

#ifndef _LS2K_I2C_BUS_H
#define _LS2K_I2C_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ls2k_drv_io.h"

/******************************************************************************
 * 关于新的 I2C 控制器
 *
 * 在执行序列:
 *
 *   ls2k_i2c_send_start()
 *   ls2k_i2c_send_addr()
 *
 *    ... ls2k_i2c_read_bytes()和ls2k_i2c_write_bytes() 只能各出现一次
 *
 *   ls2k_i2c_send_stop()
 ******************************************************************************/

#if defined(LS2K300) || defined(LS2K301)
#define I2C_DEVICE_NEW      1
#endif

//-----------------------------------------------------------------------------
// I2C devices
//-----------------------------------------------------------------------------

#if BSP_USE_I2C0
extern const void *busI2C0;
#endif
#if BSP_USE_I2C1
extern const void *busI2C1;
#endif
#if BSP_USE_I2C2
extern const void *busI2C2;
#endif
#if BSP_USE_I2C3
extern const void *busI2C3;
#endif

//-----------------------------------------------------------------------------
// I2C io control command                          param type
//-----------------------------------------------------------------------------

#define IOCTL_I2C_SET_WORKMODE      0x1001      // int: I2C work mode, see below

#define IOCTL_I2C_GET_WORKMODE      0x1002      // return: I2C work mode, see below

#define IOCTL_I2C_PRINT_WORKMODE    0x1003

/*
 * 工作模式: DMA or INT else POLL
 */
#define I2C_WORK_DMA        0x01
#define I2C_WORK_INT        0x02
#define I2C_WORK_POLL       0x04     /* default work mode */

//-----------------------------------------------------------------------------
// I2C function
//-----------------------------------------------------------------------------

#if (PACK_DRV_OPS)

extern const libi2c_ops_t *i2c_drv_ops;

#define ls2k_i2c_initialize(bus)            i2c_drv_ops->init(bus)
#define ls2k_i2c_send_start(bus, addr)      i2c_drv_ops->send_start(bus, addr)
#define ls2k_i2c_send_stop(bus, addr)       i2c_drv_ops->send_stop(bus, addr)
#define ls2k_i2c_send_addr(bus, addr, rw)   i2c_drv_ops->send_addr(bus, addr, rw)
#define ls2k_i2c_read_bytes(bus, buf, len)  i2c_drv_ops->read_bytes(bus, buf, len)
#define ls2k_i2c_write_bytes(bus, buf, len) i2c_drv_ops->write_bytes(bus, buf, len)
#define ls2k_i2c_ioctl(bus, cmd, arg)       i2c_drv_ops->ioctl(bus, cmd, arg)

#else

/*
 * 初始化I2C总线
 * 参数:    none
 *
 * 返回:    0=成功
 *
 * 说明:    I2C总线在使用前, 必须要先调用该初始化函数
 */
int I2C_initialize(const void *bus);

/*
 * 开始I2C总线操作. 本函数获取I2C总线的控制权
 * 参数:    Addr    总线bus上挂接的某个I2C从设备的 7 位I2C地址
 *
 * 返回:    0=成功
 */
int I2C_send_start(const void *bus, unsigned int Addr);

/*
 * 结束I2C总线操作. 本函数释放I2C总线的控制权
 * 参数:    Addr    总线bus上挂接的某个I2C从设备的 7 位I2C地址
 *
 * 返回:    0=成功
 */
int I2C_send_stop(const void *bus, unsigned int Addr);

/*
 * 读写I2C总线前发送读写请求命令
 * 参数:    Addr    总线bus上挂接的某个I2C从设备的 7 位I2C地址
 *          rw      1: 读操作; 0: 写操作.
 *
 * 返回:    0=成功
 */
int I2C_send_addr(const void *bus, unsigned int Addr, int rw);

/*
 * 从I2C从设备读取数据
 * 参数:    buf     类型 unsigned char *, 用于存放读取数据的缓冲区
 *          len     类型 int, 待读取的字节数, 长度不能超过 buf 的容量
 *
 * 返回:    本次读操作的字节数
 */
int I2C_read_bytes(const void *bus, unsigned char *buf, int len);

/*
 * 向I2C从设备写入数据
 * 参数:    buf     类型 unsigned char *, 用于存放待写数据的缓冲区
 *          len     类型 int, 待写的字节数, 长度不能超过 buf 的容量
 *
 * 返回:    本次写操作的字节数
 */
int I2C_write_bytes(const void *bus, unsigned char *buf, int len);

/*
 * 向I2C总线发送控制命令
 * 参数:
 *      ---------------------------------------------------------------------------------
 *          cmd                             |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_SPI_I2C_SET_TFRMODE       |   类型: unsigned int
 *                                          |   用途: 设置I2C总线的通信速率
 *      ---------------------------------------------------------------------------------
 *
 * 返回:    0=成功
 *
 * 说明:    该函数调用的时机是: I2C设备已经初始化且空闲, 或者已经获取总线控制权
 */
int I2C_ioctl(const void *bus, int cmd, void *arg);

#define ls2k_i2c_initialize(bus)            I2C_initialize(bus)
#define ls2k_i2c_send_start(bus, addr)      I2C_send_start(bus, addr)
#define ls2k_i2c_send_stop(bus, addr)       I2C_send_stop(bus, addr)
#define ls2k_i2c_send_addr(bus, addr, rw)   I2C_send_addr(bus, addr, rw)
#define ls2k_i2c_read_bytes(bus, buf, len)  I2C_read_bytes(bus, buf, len)
#define ls2k_i2c_write_bytes(bus, buf, len) I2C_write_bytes(bus, buf, len)
#define ls2k_i2c_ioctl(bus, cmd, arg)       I2C_ioctl(bus, cmd, arg)

#endif

#ifdef __cplusplus
}
#endif

#endif // _LS2K_I2C_BUS_H


