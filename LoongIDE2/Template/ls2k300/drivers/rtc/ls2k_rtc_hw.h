/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_rtc_hw.h
 *
 * created: 2024-08-04
 *  author: Bian
 */

#ifndef _LS2K_RTC_HW_H
#define _LS2K_RTC_HW_H

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
// RTC 设备
//-------------------------------------------------------------------------------------------------

#define RTC_BASE        0x16128000

/*
 * RTC 控制器
 */
typedef struct
{
    volatile unsigned int rsv1[8];
	volatile unsigned int toytrim;          /* 0x20 32 RW 对 32.768kHZ 的分频系数(计数器时钟) */
    volatile unsigned int toywritelo;       /* 0x24 32 W  TOY 低32位数值写入 */
	volatile unsigned int toywritehi;       /* 0x28 32 W  TOY 高32位数值写入 */
	volatile unsigned int toyreadlo;        /* 0x2C 32 R  TOY 低32位数值读出 */
	volatile unsigned int toyreadhi;        /* 0x30 32 R  TOY 高32位数值读出 */
    volatile unsigned int toymatch[3];      /* 0x34~0x3C 32 RW TOY 定时中断0 */
	volatile unsigned int rtcctrl;          /* 0x40 32 RW TOY 和 RTC 控制寄存器 */
	volatile unsigned int rsv2[7];
	volatile unsigned int rtctrim;          /* 0x60 32 RW 对 32.768kHZ 的分频系数(定时器) */
	volatile unsigned int rtcwrite;         /* 0x64 32 R  RTC 定时计数写入 */
	volatile unsigned int rtcread;          /* 0x68 32 W  RTC 定时计数读出 */
    volatile unsigned int rtcmatch[3];      /* 0x6C~0x74 32 RW RTC 时钟定时中断0 */
} HW_RTC_t;

/*
 * TOY Read/Write low fields
 */
#define TOY_LO_MONTH(v)         ((v >> 26) & 0x3F)  // 31:26 	MONTH 	 范围1~12
#define TOY_LO_DAY(v)           ((v >> 21) & 0x1F)  // 25:21 	DAY 	 范围1~31
#define TOY_LO_HOUR(v)          ((v >> 16) & 0x1F)  // 20:16 	HOUR 	 范围0~23
#define TOY_LO_MIN(v)           ((v >> 10) & 0x3F)  // 15:10 	MIN 	 范围0~59
#define TOY_LO_SEC(v)           ((v >>  4) & 0x3F)  // 9:4 	    SEC 	 范围0~59
#define TOY_LO_MILLISEC(v)      ((v >>  0) & 0x0F)  // 3:0 	    MILLISEC 范围0~9

/*
 * TOYMATCH fields
 */
#define TOY_MATCH_YEAR(v)       ((v >> 26) & 0x3F)  // 31:26 	YEAR 	范围 0～16383
#define TOY_MATCH_MONTH(v)      ((v >> 22) & 0x0F)  // 25:22 	MONTH   范围1~12
#define TOY_MATCH_DAY(v)        ((v >> 17) & 0x1F)  // 21:17 	DAY     范围1~31
#define TOY_MATCH_HOUR(v)       ((v >> 12) & 0x1F)  // 16:12 	HOUR 	范围0~23
#define TOY_MATCH_MIN(v)        ((v >>  6) & 0x3F)  // 11:6 	MIN 	范围0~59
#define TOY_MATCH_SEC(v)        ((v >>  0) & 0x3F)  // 5:0 	    SEC 	范围0~59

/*
 * RTC control bits
 */
#define RTC_CTRL_ERS            (1<<23)             // R    REN(bit13)写状态
#define RTC_CTRL_RTS            (1<<20)             // R    Sys_rtctrim写状态
#define RTC_CTRL_RM2            (1<<19)             // R 	Sys_rtcmatch2写状态
#define RTC_CTRL_RM1            (1<<18)             // R 	Sys_rtcmatch1写状态
#define RTC_CTRL_RM0            (1<<17)             // R 	Sys_rtcmatch0写状态
#define RTC_CTRL_RS             (1<<16)             // R    Sys_rtcwrite写状态
#define RTC_CTRL_REN            (1<<13)             // R/W 	RTC使能, 高有效. 需要初始化为1
#define RTC_CTRL_TEN            (1<<11)             // R/W 	TOY使能, 高有效. 需要初始化为1
#define RTC_CTRL_EO             (1<<8)              // R/W 	0:32.768k晶振禁止; 1:32.768k晶振使能
#define RTC_CTRL_32S            (1<<5)              // R 	0:32.768k晶振不工作; 1:32.768k晶振正常工作
#define RTC_CTRL_TM2            (1<<3)              // R 	Sys_toymatch2写状态
#define RTC_CTRL_TM1            (1<<2)              // R 	Sys_toymatch1写状态
#define RTC_CTRL_TM0            (1<<1)              // R 	Sys_toymatch0写状态
#define RTC_CTRL_TS             (1<<0)              // R 	Sys_toywrite写状态

#ifdef __cplusplus
}
#endif

#endif // _LS2K_RTC_HW_H

