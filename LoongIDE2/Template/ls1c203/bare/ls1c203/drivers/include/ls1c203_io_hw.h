/*
 * Copyright (C) 2021-2025 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls1c203_io_hw.h
 *
 * created: 2025-05-20
 *  author: Bian
 */

#ifndef _LS1C_IO_HW_H
#define _LS1C_IO_HW_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// AFIO 管脚复用控制器
//-----------------------------------------------------------------------------

#define AFIO_BASE			0x0000d000

#define AFIO_SEL0_ADDR		(AFIO_BASE+0x0)
#define AFIO_SEL1_ADDR		(AFIO_BASE+0x4)

//-----------------------------------------------------------------------------
// EXTI 外部中断控制器, GPIO0~GPIO28
//-----------------------------------------------------------------------------

#define EXTI_BASE			0x0000d100

#define EXTI_EN_ADDR		(EXTI_BASE+0x00)
#define EXTI_IEN_ADDR		(EXTI_BASE+0x04)
#define EXTI_POL_ADDR		(EXTI_BASE+0x08)
#define EXTI_RIS_ADDR		(EXTI_BASE+0x0c)
#define EXTI_FAL_ADDR		(EXTI_BASE+0x10)
#define EXTI_SRC_ADDR		(EXTI_BASE+0x14)
#define EXTI_CLR_ADDR		EXTI_SRC_ADDR

#define EXTI_EN(gpioNum)	OR_REG32(EXTI_EN_ADDR, (1<<(gpioNum)))
#define EXTI_DIS(gpioNum)	AND_REG32(EXTI_EN_ADDR, ~(1<<(gpioNum)))

#define EXTI_IEN(gpioNum)	OR_REG32(EXTI_IEN_ADDR, (1<<(gpioNum)))
#define EXTI_IDIS(gpioNum)	AND_REG32(EXTI_IEN_ADDR, ~(1<<(gpioNum)))

/*
 * 0: 高电平/上升沿有效
 */
#define EXTI_POL_FALL(gpio)  OR_REG32(EXTI_POL_ADDR, (1<<(gpioNum)))
#define EXTI_POL_RISE(gpio)  AND_REG32(EXTI_POL_ADDR, ~(1<<(gpioNum)))

/*
 * 0: 电平模式; 1:上升沿模式
 */
#define EXTI_RISE(gpioNum)	OR_REG32(EXTI_RIS_ADDR, (1<<(gpioNum())
#define EXTI_R_LVL(gpioNum)	AND_REG32(EXTI_RIS_ADDR, ~(1<<(gpioNum)))

/*
 * 0: 电平模式; 1:下降沿模式
 */
#define EXTI_FALL(gpioNum)	OR_REG32(EXTI_FAL_ADDR, (1<<(gpioNum())
#define EXTI_F_LVL(gpioNum)	AND_REG32(EXTI_FAL_ADDR, ~(1<<(gpioNum)))

#define EXTI_SRC(gpioNum)	(READ_REG32(EXTI_SRC_ADDR) & (1<<(gpioNum)))
#define EXTI_CLR(gpioNum)	OR_REG32(EXTI_SRC_ADDR, (1<<(gpioNum)))

//-----------------------------------------------------------------------------
// GPIO 控制器
//-----------------------------------------------------------------------------

#define GPIOA_BASE			0x0000d200
#define GPIOB_BASE			0x0000d300

typedef struct
{
    volatile unsigned int crl;			// 0x00 GPIO 端口配置低寄存器
    volatile unsigned int crh;			// 0x04 GPIO 端口配置高寄存器
    volatile unsigned short idr;		// 0x08 GPIO 端口输入数据寄存器
    volatile unsigned short idr_hi;
    volatile unsigned short odr;		// 0x0c GPIO 端口输出数据寄存器
    volatile unsigned short odr_hi;
    volatile unsigned short reset;		// 0x10 GPIO 端口设置清除寄存器
    volatile unsigned short set;
} HW_GPIO_t;


#ifdef __cplusplus
}
#endif

#endif // _LS1C_IO_HW_H

