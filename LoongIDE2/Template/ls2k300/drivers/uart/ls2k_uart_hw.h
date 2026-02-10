/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_uart_hw.h
 *
 * created: 2024-06-08
 *  author: Bian
 */

#ifndef _LS2K_UART_HW_H
#define _LS2K_UART_HW_H

//-------------------------------------------------------------------------------------------------
// UART 设备
//-------------------------------------------------------------------------------------------------

#define UART0_BASE              0x16100000
#define UART1_BASE              0x16100400
#define UART2_BASE              0x16100800
#define UART3_BASE              0x16100c00
#define UART4_BASE              0x16101000
#define UART5_BASE              0x16101400
#define UART6_BASE              0x16101800
#define UART7_BASE              0x16101c00
#define UART8_BASE              0x16102000
#define UART9_BASE              0x16102400

/*
 * UART 设备类型
 */
typedef struct
{
    union
	{
    	volatile unsigned char dat;         // 0x00 数据寄存器
    	volatile unsigned char dll;         // 0x00 分频值低字节寄存器
	} R0;
    union
	{
    	volatile unsigned char ien;         // 0x01 中断使能寄存器
    	volatile unsigned char dlh;         // 0x01 分频值高字节寄存器
	} R1;
    union
	{
		volatile unsigned char isr;         // 0x02 中断状态寄存器
		volatile unsigned char fcr;         // 0x02 FIFO控制寄存器
		volatile unsigned char dld;         // 0x02 分频值小数寄存器
	} R2;
    volatile unsigned char lcr;             // 0x03 线路控制寄存器
    volatile unsigned char mcr;             // 0x04 Modem控制寄存器
    volatile unsigned char lsr;             // 0x05 线路状态寄存器
    volatile unsigned char msr;             // 0x06 Modem状态寄存器
} HW_UART_t;

#define UART_FIFO_SIZE       16

#define UART_IEN_ACTS        bit(7)      // RW CTS 自动流控使能. 0: 关闭; 1: 打开. 打开时, 若CTSn 的输入为0, 则暂停发送
#define UART_IEN_ARTS        bit(6)      // RW RTS 自动流控使能. 0: 关闭; 1: 打开. 打开时, 若rxfifo 为满, 则RTSn 的输出为0
#define UART_IEN_TXDMA       bit(5)      // RW 发送状态DMA 使能. 0: 关闭; 1: 打开. 打开时, 若txfifo 未满, 向dma 发送发送请求
#define UART_IEN_RXDMA       bit(4)      // RW 接收状态DMA 使能. 0: 关闭; 1: 打开. 打开时, 若rxfifo 未满, 向dma 发送接收请求
#define UART_IEN_IM          bit(3)      // RW Modem 状态中断使能. 0: 关闭; 1: 打开
#define UART_IEN_IL          bit(2)      // RW 接收器线路状态中断使能. 0: 关闭; 1: 打开
#define UART_IEN_ITx         bit(1)      // RW 传输保存寄存器为空中断使能. 0: 关闭; 1: 打开
#define UART_IEN_IRx         bit(0)      // RW 接收有效数据中断使能. 0: 关闭; 1: 打开

#define UART_IEN_MASK        0xCF

#define UART_ISR_MASK        0x0E        // RO bit[3:1]
#define UART_ISR_SHIFT       1
/*
 * -------------------------------------------------------------------------------------------------------------------
 * | Bit3 | Bit2 | Bit1	| 优先级|     中断类型	   |	中断源						  |	中断复位控制 				  |
 * |------|------|------|-------|------------------|----------------------------------|-------------------------------|
 * |  0   |  1   |  1 	| 1st 	| 接收线路状态	   |奇偶、溢出或帧错误,或打断中断     | 读LSR						  |
 * |------|------|------|-------|------------------|----------------------------------|-------------------------------|
 * |  0   |  1   |  0 	| 2nd 	| 接收到有效数据   |FIFO 的字符个数达到trigger 的水平 |FIFO 的字符个数低于trigger 的值|
 * |------|------|------|-------|------------------|----------------------------------|-------------------------------|
 * |  1   |  1   |  0 	| 2nd 	| 接收超时		   |在FIFO至少有一个字符, 但在4 个字符| 读接收FIFO					  |
 * |      |	 	 |	 	| 	 	| 	 	 	 	   |时间内没有任何操作, 包括读和写操作|								  |
 * |------|------|------|-------|------------------|----------------------------------|-------------------------------|
 * |  0   |  0   |  1 	| 3rd 	|传输保存寄存器为空|	传输保存寄存器为空			  | 写数据到THR 或者多IIR		  |
 * |------|------|------|-------|------------------|----------------------------------|-------------------------------|
 * |  0   |  0   |  0 	| 4th 	| Modem 状态	   | CTS, DSR, RI or DCD			  | 读MSR						  |
 * -------------------------------------------------------------------------------------------------------------------
 */
#define UART_ISR_RxSR        0b0110      // 接收线路状态
#define UART_ISR_RxTRIG      0b0100      // 接收到有效数据
#define UART_ISR_RxTMO       0b1100      // 接收超时
#define UART_ISR_TxEMPTY     0b0010      // 传输保存寄存器为空
#define UART_ISR_MSR         0b0000      // Modem状态
#define UART_ISR_INTp        bit(0)      // RO 该位为低表示存在未处理的中断

#define UART_FCR_TL_MASK     0xC0        // WO bit[7:6] 接收FIFO提出中断申请的trigger值
#define UART_FCR_TL_SHIFT    6
#define UART_FCR_TL_1B       (0<<6)      // 1 字节
#define UART_FCR_TL_2B       (1<<6)      // 2 字节
#define UART_FCR_TL_3B       (2<<6)      // 3 字节
#define UART_FCR_TL_4B       (3<<6)      // 4 字节
#define UART_FCR_TxRESET     bit(2)      // WO 该位写1, 清除发送FIFO内容并复位其逻辑
#define UART_FCR_RxRESET     bit(1)      // WO 该位写1, 清除接收FIFO内容并复位其逻辑
#define UART_FCR_FIFO_EN  	 bit(0)	     /* XXX enable fifo */

#define UART_LCR_DLAB        bit(7)      // RW 分频锁存器访问位. 1: 访问操作分频锁存器, 0: 访问操作正常寄存器
#define UART_LCR_BCB         bit(6)      // RW 打断控制位. 1: 此时串口的输出被置为0(打断状态), 0: 正常操作
#define UART_LCR_SPB         bit(5)      // RW 指定奇偶校验位. 0: 不用指定奇偶校验位,
                                         // 					1: 如果LCR[4]位是1 则传输和检查奇偶校验位为0. 如果LCR[4]位是0 则传输和检查奇偶校验位为1.
#define UART_LCR_EPS         bit(4)      // RW 奇偶校验位选择. 0: 在每个字符中有奇数个1)包括数据和奇偶校验位, 1: 在每个字符中有偶数个1
#define UART_LCR_PE          bit(3)      // RW 奇偶校验位使能. 0: 没有奇偶校验位, 1: 在输出时生成奇偶校验位, 输入则判断奇偶校验位
#define UART_LCR_SB          bit(2)      // RW 定义生成停止位的位数. 0: 1 个停止位, 1: 在5 位字符长度时是1.5 个停止位, 其他长度是2 个停止位
#define UART_LCR_BEC_MASK    0x03        // RW bit[1:0] 设定每个字符的位数
#define UART_LCR_BEC_5b      0           // 5 位
#define UART_LCR_BEC_6b      1           // 6 位
#define UART_LCR_BEC_7b      2           // 7 位
#define UART_LCR_BEC_8b      3           // 8 位

#define UART_MCR_LOOP        bit(4)      // WO 回环模式控制位;   0: 正常操作, 1: 回环模式
#define UART_MCR_OUT2 	     bit(3)      // WO 在回环模式中连到DCD输入
#define UART_MCR_OUT1 	     bit(2)      // WO 在回环模式中连到RI输入
#define UART_MCR_RTSC 	     bit(1)      // WO RTS信号控制位
#define UART_MCR_DTRC 	     bit(0)      // WO DTR信号控制位

#define UART_LSR_ERROR       bit(7)      // R 错误表示位. 1: 至少有奇偶校验位错误, 帧错误或打断中断的一个, 0: 没有错误
#define UART_LSR_TE          bit(6)      // R 传输为空表示位. 1: 传输FIFO 和传输移位寄存器都为空, 给传输FIFO 写数据时清零, 0: 有数据
#define UART_LSR_TFE         bit(5)      // R 传输FIFO 为空表示位. 1: 当前传输FIFO 为空, 给传输FIFO 写数据时清零, 0: 有数据
#define UART_LSR_BI          bit(4)      // R 打断中断表示位. 1: 接收到起始位＋数据＋奇偶位＋停止位都是0, 即有打断中断, 0: 没有打断
#define UART_LSR_FE          bit(3)      // R 帧错误表示位. 1: 接收的数据没有停止位, 0: 没有错误
#define UART_LSR_PE          bit(2)      // R 奇偶校验位错误表示位. 1: 当前接收数据有奇偶错误, 0: 没有奇偶错误
#define UART_LSR_OE          bit(1)      // R 数据溢出表示位. 1: 有数据溢出, 0: 无溢出
#define UART_LSR_DR          bit(0)      // R 接收数据有效表示位. 0: 在FIFO 中无数据, 1: 在FIFO 中有数据

#define UART_MSR_CDCD        bit(7)      // R DCD或者在回环模式中连到, 输入值的反Out2
#define UART_MSR_CRI         bit(6)      // R RI或者在回环模式中连到,  输入值的反OUT1
#define UART_MSR_CDSR        bit(5)      // R DSR或者在回环模式中连到, 输入值的反DTR
#define UART_MSR_CCTS        bit(4)      // R CTS或者在回环模式中连到, 输入值的反RTS
#define UART_MSR_DDCD        bit(3)      // R DDCD指示位
#define UART_MSR_TERI        bit(2)      // R RI, 边沿检测RI状态从低到高变化
#define UART_MSR_DDSR        bit(1)      // R DDSR指示位
#define UART_MSR_DCTS        bit(0)      // R DCTS指示位

#endif // _LS2K_UART_HW_H


