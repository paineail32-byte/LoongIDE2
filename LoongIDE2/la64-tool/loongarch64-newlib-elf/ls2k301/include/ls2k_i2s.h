/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_i2s.h
 *
 * created: 2024-10-22
 *  author: 
 */

#ifndef _LS2K_I2S_H
#define _LS2K_I2S_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * I2S interface samples per second
 */
#define SAMPLES_08K			8000			/* set to  8k samples per second */
#define SAMPLES_11K			11025			/* set to 11.025k samples per second */
#define SAMPLES_16K			16000			/* set to 16k samples in per second */
#define SAMPLES_22K			22000			/* set to 22.050k samples per second */
#define SAMPLES_24K			24000			/* set to 24k samples in per second */
#define SAMPLES_32K			32000			/* set to 32k samples in per second */
#define SAMPLES_44K			44100			/* set to 44.1k samples per second */
#define SAMPLES_48K			48000			/* set to 48k samples per second */

/**
 * I2S interface number of bits per sample
 */
#define BITS_PER_SAMP_16	16				/* set 16 bits per sample */
#define BITS_PER_SAMP_24	24				/* set 24 bits per sample */
#define BITS_PER_SAMP_32	32				/* set 32 bits per sample */

/**
 * I2S interface work mode
 */
#define I2S_WORK_PLAYBACK	0x01			/* audio play */
#define I2S_WORK_CAPTURE	0x02			/* audio record */
#define I2S_WORK_DUAL		0x03			/* audio play & record */

//-----------------------------------------------------------------------------
// I2S open() ioctl 参数
//-----------------------------------------------------------------------------

typedef struct
{
	int channels;							/* mono=1; stereo=2 */
	int samples_per_second;					/* SAMPLES_08K~SAMPLES_48K  */
	int bits_per_sample;                    /* BIT_LENGTH_16BITS~BIT_LENGTH_32BITS */
	int workmode;							/* PLAYBACK CAPTURE DUAL */
} I2S_Mode_t;

//-----------------------------------------------------------------------------
// IOCTL 命令参数
//-----------------------------------------------------------------------------

typedef long (*data_callback_t)(void *pDataDev,
								void *pFramesOut,
							/*	const void *pFramesIn, */
								unsigned int frameCount);
typedef void (*done_callback_t)(void *pDataDev);

#define IOCTL_I2S_SET_MODE		0x01		/* 重新初始化I2S设备, 参数: I2S_Mode_t* */
#define IOCTL_I2S_DATA_CB		0x02		/* 设置数据回调函数 */
#define IOCTL_I2S_DATA_DEV		0x04		/* 设置数据源或者目的设备, 回调函数使用 */
#define IOCTL_I2S_DONE_CB		0x08		/* 播放结束回调函数 */

#define IOCTL_I2S_START			0x10		/* 启动 */
#define IOCTL_I2S_PAUSE			0x20		/* 暂停 */
#define IOCTL_I2S_RESUME		0x40		/* 恢复 */
#define IOCTL_I2S_STOP			0x80		/* 停止 */

//-----------------------------------------------------------------------------
// I2S devices
//-----------------------------------------------------------------------------

#if (BSP_USE_I2S)
extern const void *devI2S;
#endif

//-----------------------------------------------------------------------------
// I2S driver implements
//-----------------------------------------------------------------------------

#include "ls2k_drv_io.h"

#if (PACK_DRV_OPS)

extern const driver_ops_t *i2s_drv_ops;

#define ls2k_i2s_init(dev, arg)             i2s_drv_ops->init_entry(dev, arg)
#define ls2k_i2s_open(dev, arg)             i2s_drv_ops->open_entry(dev, arg)
#define ls2k_i2s_close(dev, arg)            i2s_drv_ops->close_entry(dev, arg)
#define ls2k_i2s_read(dev, buf, size, arg)  i2s_drv_ops->read_entry(dev, buf, size, arg)
#define ls2k_i2s_write(dev, buf, size, arg) i2s_drv_ops->write_entry(dev, buf, size, arg)
#define ls2k_i2s_ioctl(dev, cmd, arg)       i2s_drv_ops->ioctl_entry(dev, cmd, arg)

#else

/*
 * 初始化I2S
 * 参数:    dev     NULL 或者 devI2S
 *          arg
 *
 * 返回:    0=成功
 */
int I2S_initialize(const void *dev, void *arg);

/*
 * 打开I2S. 使能I2S, 如果配置为中断模式, 使能中断
 * 参数:    dev     NULL 或者 devI2S
 *          arg
 *
 * 返回:    0=成功
 */
int I2S_open(const void *dev, void *arg);

/*
 * 关闭I2S. 停止I2S, 如果配置为中断模式, 关闭中断
 * 参数:    dev     NULL 或者 devI2S
 *          arg     NULL.
 *
 * 返回:    0=成功
 */
int I2S_close(const void *dev, void *arg);

/*
 * 从串口读数据(接收)
 * 参数:    dev     NULL 或者 devI2S
 *          buf     类型 int *.
 *          size    类型 int.
 *          arg
 *
 * 返回:    读取的字节数
 *
 */
int I2S_read(const void *dev, void *buf, int size, void *arg);

/*
 * 从串口写数据(发送)
 * 参数:    dev     NULL 或者 devI2S
 *          buf     类型 int *.
 *          size    类型 int.
 *          arg
 *
 * 返回:    发送的字节数
 *
 */
int I2S_write(const void *dev, void *buf, int size, void *arg);

/*
 * 向I2S发送控制命令
 * 参数:    dev     NULL 或者 devI2S
 *          cmd     IOCTL_I2S_xxx 定义
 *          arg
 *
 * 返回:    0=成功
 */
int I2S_ioctl(const void *dev, int cmd, void *arg);

#define ls2k_i2s_init(dev, arg)             I2S_initialize(dev, arg)
#define ls2k_i2s_open(dev, arg)             I2S_open(dev, arg)
#define ls2k_i2s_close(dev, arg)            I2S_close(dev, arg)
#define ls2k_i2s_read(dev, buf, size, arg)  I2S_read(dev, buf, size, arg)
#define ls2k_i2s_write(dev, buf, size, arg) I2S_write(dev, buf, size, arg)
#define ls2k_i2s_ioctl(dev, cmd, arg)       I2S_ioctl(dev, cmd, arg)

#endif

//-----------------------------------------------------------------------------
// I2S device name
//-----------------------------------------------------------------------------

const char *ls2k_i2s_get_device_name(void);

//-----------------------------------------------------------------------------
// API after open()
//-----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // _LS2K_I2S_H

