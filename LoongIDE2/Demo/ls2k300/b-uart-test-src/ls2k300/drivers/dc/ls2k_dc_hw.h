/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_dc_hw.h
 *
 * created: 2022-03-16
 *  author: Bian
 */

#ifndef _LS2K_DC_HW_H
#define _LS2K_DC_HW_H

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
// DC 设备
//-------------------------------------------------------------------------------------------------

#define DC_BASE	    0x16090000+0x1240

/*
 * DC 控制器
 */
typedef struct 
{
	volatile unsigned int config;           /* 0x0000 - 0x1240: 配置寄存器 */
	volatile unsigned int rsv01[7];
	volatile unsigned int buf_addr;         /* 0x0020 - 0x1260: 帧缓冲地址寄存器0, 图像数据内存首地址 */
	volatile unsigned int rsv02[7];
	volatile unsigned int stride;           /* 0x0040 - 0x1280: 显示屏一行的字节数 */
	volatile unsigned int rsv03[7];
	volatile unsigned int origin;           /* 0x0060 - 0x12A0: 显示屏左侧原有字节数, 一般写 0 */
	volatile unsigned int rsv04[47];
	volatile unsigned int di_config;        /* 0x0120 - 0x1360: 显示抖动配置寄存器 */
	volatile unsigned int rsv05[7];
	volatile unsigned int di_tablelo;       /* 0x0140 - 0x1380: 显示抖动表 LOW */
	volatile unsigned int rsv06[7];
	volatile unsigned int di_tablehi;       /* 0x0160 - 0x13A0: 显示抖动表 HIGH */
	volatile unsigned int rsv07[7];
	volatile unsigned int pan_config;       /* 0x0180 - 0x13C0: 液晶面板配置寄存器 */
	volatile unsigned int rsv08[7];
	volatile unsigned int pan_timing;       /* 0x01A0 - 0x13E0: 液晶面板时序寄存器 */
	volatile unsigned int rsv09[7];
	volatile unsigned int hdisplay;         /* 0x01C0 - 0x1400: 水平显示 */
	volatile unsigned int rsv10[7];
	volatile unsigned int hsync;            /* 0x01E0 - 0x1420: 水平同步 */
	volatile unsigned int rsv11[23];
	volatile unsigned int vdisplay;         /* 0x0240 - 0x1480: 垂直显示 */
	volatile unsigned int rsv12[7];
	volatile unsigned int vsync;            /* 0x0260 - 0x14A0: 垂直同步 */
	volatile unsigned int rsv13[55];
	volatile unsigned int dbl_buf_addr;     /* 0x0340 - 0x1580: 帧缓冲地址寄存器1 */
} HW_DC_t;

#define DC_BURST_SIZE       0x7F    // 0xFF

/*
 * Config Register
 */
#define DC_CFG_RESET            (1<<20)		// 0: reset
#define DC_CFG_GAMMA_EN         (1<<12)		// 1: enable
#define DC_CFG_SWITCH_PANEL     (1<<9)		// 1: to switch to another panel. XXX (2 DC)
#define DC_CFG_OUTPUT_EN        (1<<8)		// 1: enable
#define DC_CFG_FORMAT_MASK      0x07		// color format
#define DC_CFG_COLOR_NONE       0
#define DC_CFG_COLOR_R4G4B4     1
#define DC_CFG_COLOR_R5G5B5     2
#define DC_CFG_COLOR_R5G6B5     3
#define DC_CFG_COLOR_R8G8B8     4

/*
 * Display Dither Register
 */
#define DC_DITHER_ENABLE        (1<<31)	    // 1: enable
#define DC_DITHER_RED_MASK      0x0F0000	// bit[19:16]
#define DC_DITHER_RED_SHIFT     16
#define DC_DITHER_GREEN_MASK    0x000F00	// bit[11:8]
#define DC_DITHER_GREEN_SHIFT   8
#define DC_DITHER_BLUE_MASK     0x00000F	// bit[3:0]

/*
 * Panel Configure Register
 */
#define DC_PANEL_CLOCK_POL      (1<<9)		// 1: 时钟极性置反, default=0
#define DC_PANEL_CLOCK_EN       (1<<8)		// 1: 时钟使能
#define DC_PANEL_DATA_EN_POL    (1<<1)		// 1: 数据使能极性置反 , default=0
#define DC_PANEL_DATA_EN        (1<<0)		// 1: 数据使能输出

/*
 * HDisplay Register
 */
#define DC_HDISP_TOTAL_MASK     0x0FFF0000	// bit[27:16], 显示屏一行的总体像素数(包括非显示区)
#define DC_HDISP_TOTAL_SHIFT    16
#define DC_HDISP_DISP_MASK      0x00000FFF	// bit[11:0], 显示屏一行的显示区像素数
#define DC_HDISP_DISP_SHIFT     0

/*
 * HSync Register
 */
#define DC_HSYNC_POLARITY       (1<<31)		// HSync 信号的极性, 1: 取反, default=0
#define DC_HSYNC_PULSE          (1<<30)		// HSync 信号使能, 1: 使能输出
#define DC_HSYNC_END_MASK       0x0FFF0000	// bit[27:16], HSync 信号结束的像素数
#define DC_HSYNC_END_SHIFT      16
#define DC_HSYNC_START_MASK     0x00000FFF	// bit[11:0], HSync 信号开始的像素数

/*
 * VDisplay Register
 */
#define DC_VDISP_TOTAL_MASK     0x07FF0000	// bit[26:16], 显示屏的总体行数(包括消隐区)
#define DC_VDISP_TOTAL_SHIFT    16
#define DC_VDISP_DISP_MASK      0x000007FF	// bit[10:0], 显示屏显示区的行数

/*
 * VSync Register
 */
#define DC_VSYNC_POLARITY       (1<<31)		// VSync 信号的极性, 1: 取反, default=0
#define DC_VSYNC_PULSE          (1<<30)		// VSync 信号使能, 1: 使能输出
#define DC_VSYNC_END_MASK       0x0FFF0000	// bit[27:16], VSync 信号结束的行数
#define DC_VSYNC_END_SHIFT      16
#define DC_VSYNC_START_MASK     0x00000FFF	// bit[11:0], VSync 信号开始的行数

#ifdef __cplusplus
}
#endif

#endif // _LS2K_DC_HW_H



