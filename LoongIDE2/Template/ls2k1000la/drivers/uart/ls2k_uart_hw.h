/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_uart_hw.h
 *
 * created: 2022-02-24
 *  author: Bian
 */

#ifndef _LS2K_UART_HW_H
#define _LS2K_UART_HW_H

//-------------------------------------------------------------------------------------------------
// UART 设备
//-------------------------------------------------------------------------------------------------

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
    	volatile unsigned char ier;         // 0x01 中断使能寄存器
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
} HW_NS16550_t;

#define NS16550_FIFO_SIZE       16

#define NS16550_IER_IME         bit(3)      // Modem状态中断使能
#define NS16550_IER_ILE         bit(2)      // 接收器线路状态中断使能
#define NS16550_IER_ITxE        bit(1)      // 传输保存寄存器为空中断使能
#define NS16550_IER_IRxE        bit(0)      // 接收有效数据中断使能

#define NS16550_ISR_II_MASK     0x0E        // bit[3:1]
#define NS16550_ISR_II_SHIFT    1
#define NS16550_ISR_II_RxSR     0b0110      // 接收线路状态
#define NS16550_ISR_II_RxTRIG   0b0100      // 接收到有效数据
#define NS16550_ISR_II_RxTMO    0b1100      // 接收超时
#define NS16550_ISR_II_TxEMPTY  0b0010      // 传输保存寄存器为空
#define NS16550_ISR_II_MSR      0b0000      // Modem状态
#define NS16550_ISR_INTp        bit(0)      // 该位为低表示存在未处理的中断

#define NS16550_FCR_TL_MASK     0xC0        // bit[7:6] 接收FIFO提出中断申请的trigger值
#define NS16550_FCR_TL_SHIFT    6
#define NS16550_FCR_TL_1B       (0<<6)      // 1  字节
#define NS16550_FCR_TL_4B       (1<<6)      // 4  字节
#define NS16550_FCR_TL_8B       (2<<6)      // 8  字节
#define NS16550_FCR_TL_14B      (3<<6)      // 14 字节
#define NS16550_FCR_TxRESET     bit(2)      // 该位写1, 清除发送FIFO内容并复位其逻辑
#define NS16550_FCR_RxRESET     bit(1)      // 该位写1, 清除接收FIFO内容并复位其逻辑
#define NS16550_FCR_FIFO_EN     bit(0)      /* 使能FIFO? */

#define NS16550_LCR_DLAB        bit(7)      // 1: 访问操作分频锁存器; 0: 访问操作正常寄存器
#define NS16550_LCR_BCB         bit(6)      // 1: 此时串口的输出被置为0(打断状态); 0: 正常操作
#define NS16550_LCR_SPB         bit(5)      // 0: 不用指定奇偶校验位;
                                            // 1: 如果LCR[4]位是1则传输和检查奇偶校验位为0
                                            //    如果LCR[4]位是0则传输和检查奇偶校验位为1
#define NS16550_LCR_EPS         bit(4)      // 0: 在每个字符中有奇数个1 (包括数据和奇偶校验位)
                                            // 1: 在每个字符中有偶数个1
#define NS16550_LCR_PE          bit(3)      // 0: 没有奇偶校验位
                                            // 1: 在输出时生成奇偶校验位, 输入则判断奇偶校验位
#define NS16550_LCR_SB          bit(2)      // 0: 1个停止位
                                            // 1: 在5位字符长度时是1.5个停止位, 其他长度是2个停止位
#define NS16550_LCR_BEC_MASK    0x03        // bit[1:0] 定每个字符的位数
#define NS16550_LCR_BEC_5b      0           // 5 位
#define NS16550_LCR_BEC_6b      1           // 6 位
#define NS16550_LCR_BEC_7b      2           // 7 位
#define NS16550_LCR_BEC_8b      3           // 8 位

#define NS16550_MCR_INFRARED     bit(7)     // 红外载波使能; 0: 关闭, 1: 使能
#define NS16550_MCR_RX_POL       bit(6)     // 载波使能时RX极性; 0: 正常, 低电平有效, 1: 反转, 高电平有效
#define NS16550_MCR_INFRARED_POL bit(5)     // 波使能时TX极性;   0: 正常, 低电平有效, 1: 反转, 高电平有效
#define NS16550_MCR_LOOP         bit(4)     // 回环模式控制位;   0: 正常操作, 1: 回环模式
#define NS16550_MCR_OUT2 	     bit(3)     // 在回环模式中连到DCD输入
#define NS16550_MCR_OUT1 	     bit(2)     // 在回环模式中连到RI输入
#define NS16550_MCR_RTSC 	     bit(1)     // RTS信号控制位
#define NS16550_MCR_DTRC 	     bit(0)     // DTR信号控制位

#define NS16550_LSR_ERROR       bit(7)      // 错误表示位; 1: 至少有奇偶校验位错误, 帧错误或打断中断的一个
#define NS16550_LSR_TE          bit(6)      // 传输为空表示位, 1: 传输FIFO和传输移位寄存器都为空, 给传输FIFO写数据时清零, 0: 有数据
#define NS16550_LSR_TFE         bit(5)      // 传输 FIFO 位空表示位, 1: 当前传输FIFO为空, 给传输FIFO写数据时清零, 0: 有数据
#define NS16550_LSR_BI          bit(4)      // 打断中断表示位, 1: 接收到起始位+数据+奇偶位+停止位都是0, 即有打断中断, 0: 没有打断
#define NS16550_LSR_FE          bit(3)      // 帧错误表示位, 1: 接收的数据没有停止位, 0: 没有错误
#define NS16550_LSR_PE          bit(2)      // 奇偶校验位错误表示位, 1: 当前接收数据有奇偶错误, 0: 没有奇偶错误
#define NS16550_LSR_OE          bit(1)      // 数据溢出表示位, 1: 有数据溢出, 0: 无溢出
#define NS16550_LSR_DR          bit(0)      // 接收数据有效表示位, 1: 在FIFO中有数据, 0: 在FIFO中无数据

#define NS16550_MSR_CDCD        bit(7)      // 接DCD或者在回环模式中连到, 输入值的反Out2
#define NS16550_MSR_CRI         bit(6)      // 接RI或者在回环模式中连到,  输入值的反OUT1
#define NS16550_MSR_CDSR        bit(5)      // 接DSR或者在回环模式中连到, 输入值的反DTR
#define NS16550_MSR_CCTS        bit(4)      // 接CTS或者在回环模式中连到, 输入值的反RTS
#define NS16550_MSR_DDCD        bit(3)      // 接DDCD指示位
#define NS16550_MSR_TERI        bit(2)      // 接RI, 边沿检测RI状态从低到高变化
#define NS16550_MSR_DDSR        bit(1)      // 接DDSR指示位
#define NS16550_MSR_DCTS        bit(0)      // 接DCTS指示位

#endif // _LS2K_UART_HW_H

