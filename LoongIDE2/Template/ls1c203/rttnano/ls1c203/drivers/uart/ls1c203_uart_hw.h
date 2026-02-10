/*
 * Copyright (C) 2021-2025 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls1c203_uart_hw.h
 *
 * created: 2025-05-19
 *  author: Bian
 */

#ifndef _203_UART_HW_H
#define _203_UART_HW_H

//-------------------------------------------------------------------------------------------------
// UART 设备
//-------------------------------------------------------------------------------------------------

#define UART0_BASE              0x00009000
#define UART1_BASE              0x00009100

/*
 * UART 设备类型
 */
typedef struct
{
    union
	{
    	volatile unsigned int dat;          // 0x00 数据寄存器
    	volatile unsigned int dll;          // 0x00 分频值低字节寄存器
	} R0;
    union
	{
    	volatile unsigned int ien;          // 0x04 中断使能寄存器
    	volatile unsigned int dlh;          // 0x04 分频值高字节寄存器
	} R1;
    union
	{
		volatile unsigned int isr;          // 0x08 中断状态寄存器
		volatile unsigned int fcr;          // 0x08 FIFO控制寄存器
		volatile unsigned int dld;          // 0x08 分频值小数寄存器
	} R2;
    volatile unsigned int lcr;              // 0x0C 线路控制寄存器
    volatile unsigned int mcr;              // 0x10 Modem控制寄存器
    volatile unsigned int lsr;              // 0x14 线路状态寄存器
    volatile unsigned int msr;              // 0x18 Modem状态寄存器
} HW_UART_t;

#define UART_FIFO_SIZE       16

/**
 * 中断使能寄存器 - IER
 */
#define UART_IEN_TXDE        bit(5)          /* 发生状态DMA使能 */
#define UART_IEN_RXDE        bit(4)          /* 接收状态DMA使能 */
#define UART_IEN_LINE        bit(2)          /* 线路状态中断使能 */
#define UART_IEN_TX          bit(1)          /* 发送状态中断使能 */
#define UART_IEN_RX          bit(0)          /* 接收状态中断使能 */

/**
 * 中断状态寄存器 - ISR
 */
#define UART_ISR_SRC_MASK    0x0E            /* RO 中断源 */
#define UART_ISR_SRC_SHIFT   1
#define UART_ISR_LINE        (0x3<<1)        /* 3’b011 线路状态中断, 优先级1, 奇偶/溢出/帧错误/打断时中断, 读LSR清除 */
#define UART_ISR_RX          (0x2<<2)        /* 3’b010 接收状态中断, 优先级2, RX数量达到trigger的值, 读data清除 */
#define UART_ISR_RXTMO       (0x6<<1)        /* 3’b110 接收状态中断, 优先级2, RX超时, 读data清除 */
#define UART_ISR_TX          (0x1<<1)        /* 3’b001 发送状态中断, 优先级3, TX FIFO为空, 写data或读isr清除 */
#define UART_ISR_PENDING     bit(0)          /* 中断未决状态 */

/**
 * FIFO 控制寄存器 - FCR
 */
#define UART_FCR_TRIG_MASK   0xC0            /* bit[7:6] 接收中断状态所需trigger. 0/1=1字节, 最多16字节 */
#define UART_FCR_TRIG_SHIFT  6
#define UART_FCR_TRIGGER(n)  ((n<<6)&0xC0)
#define UART_FCR_DMA_MODE    bit(3)          /* Enable DMA Mode */
#define UART_FCR_TXFIFO_RST  bit(2)          /* 复位发送FIFO */
#define UART_FCR_RXFIFO_RST  bit(1)          /* 复位接收FIFO */
#define UART_FCR_FIFO_EN     bit(0)          /* 使能FIFO? */

/**
 * 线路控制寄存器 - LCR
 */
#define UART_LCR_DLAB        bit(7)          /* 分频器模式. 0=访问正常寄存器, 1=访问分频寄存器 */
#define UART_LCR_BCB         bit(6)          /* 打断控制位. 0=正常操作, 1=串口输出置0(打断状态) */
#define UART_LCR_SPD         bit(5)          /* 指定奇偶校验位. 0:不指定奇偶校验位, 1: eps=1则校验位为0, eps=0则校验位为1 */
#define UART_LCR_EPS         bit(4)          /* 奇偶校验位选择. 0=奇校验, 1=偶校验 */
#define UART_LCR_PE          bit(3)          /* 1=奇偶校验位使能 */
#define UART_LCR_SB          bit(2)          /* 生成停止位位数. 0:1个停止位, 1:bec=5时1.5个停止位, 其它2个停止位 */
#define UART_LCR_BITS_MASK   0x03            /* 字符位数 */
#define UART_LCR_BITS_5      0
#define UART_LCR_BITS_6      1
#define UART_LCR_BITS_7      2
#define UART_LCR_BITS_8      3

/**
 * MODEM 控制寄存器 - MCR
 */
#define UART_MCR_LOOPBACK    bit(4)          // 自回环模式

/**
 * 线路状态寄存器 - LSR
 */
#define UART_LSR_ERR         bit(7)          /* 1=有错误, 校验/帧错误或打断中断 */
#define UART_LSR_TE          bit(6)          /* 0=有数据, 1=TX FIFO和传输移位寄存器为空. 写TXFIFO时清除 */
#define UART_LSR_TFE         bit(5)          /* 1=传输FIFO 为空 */
#define UART_LSR_BI          bit(4)          /* 打断中断. 0=没有中断 */
#define UART_LSR_FE          bit(3)          /* 帧错误 */
#define UART_LSR_PE          bit(2)          /* 奇偶校验位错误 */
#define UART_LSR_OE          bit(1)          /* 数据溢出 */
#define UART_LSR_DR          bit(0)          /* 接收数据有效. 0=RXFIFO无数据, 1=RXFIFO有数据 */

#endif // _203_UART_HW_H


