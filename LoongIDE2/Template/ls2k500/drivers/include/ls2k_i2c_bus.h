/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

#ifndef _LS2K_I2C_BUS_H
#define _LS2K_I2C_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ls2k_drv_io.h"

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
#if BSP_USE_I2C4
extern const void *busI2C4;
#endif
#if BSP_USE_I2C5
extern const void *busI2C5;
#endif

/******************************************************************************
 * bus operators
 */
#if (PACK_DRV_OPS)

extern const libi2c_ops_t *i2c_drv_ops;

#define ls2k_i2c_initialize(i2c)            i2c_drv_ops->init(i2c)
#define ls2k_i2c_send_start(i2c, addr)      i2c_drv_ops->send_start(i2c, addr)
#define ls2k_i2c_send_stop(i2c, addr)       i2c_drv_ops->send_stop(i2c, addr)
#define ls2k_i2c_send_addr(i2c, addr, rw)   i2c_drv_ops->send_addr(i2c, addr, rw)
#define ls2k_i2c_read_bytes(i2c, buf, len)  i2c_drv_ops->read_bytes(i2c, buf, len)
#define ls2k_i2c_write_bytes(i2c, buf, len) i2c_drv_ops->write_bytes(i2c, buf, len)
#define ls2k_i2c_ioctl(i2c, cmd, arg)       i2c_drv_ops->ioctl(i2c, cmd, arg)

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

#define ls2k_i2c_initialize(i2c)            I2C_initialize(i2c)
#define ls2k_i2c_send_start(i2c, addr)      I2C_send_start(i2c, addr)
#define ls2k_i2c_send_stop(i2c, addr)       I2C_send_stop(i2c, addr)
#define ls2k_i2c_send_addr(i2c, addr, rw)   I2C_send_addr(i2c, addr, rw)
#define ls2k_i2c_read_bytes(i2c, buf, len)  I2C_read_bytes(i2c, buf, len)
#define ls2k_i2c_write_bytes(i2c, buf, len) I2C_write_bytes(i2c, buf, len)
#define ls2k_i2c_ioctl(i2c, cmd, arg)       I2C_ioctl(i2c, cmd, arg)

#endif

#ifdef __cplusplus
}
#endif

#endif // _LS2K_I2C_BUS_H

