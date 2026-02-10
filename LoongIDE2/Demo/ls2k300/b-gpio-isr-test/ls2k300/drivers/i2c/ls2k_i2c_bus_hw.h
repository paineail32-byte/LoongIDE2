/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_i2c_bus_hw.h
 *
 * created: 2024-06-09
 *  author: Bian
 */

#ifndef _LS2K_I2C_HW_H
#define _LS2K_I2C_HW_H

//-------------------------------------------------------------------------------------------------
// I2C 设备
//-------------------------------------------------------------------------------------------------

#define I2C0_BASE               0x16108000
#define I2C1_BASE               0x16109000
#define I2C2_BASE               0x1610a000
#define I2C3_BASE               0x1610b000

/*
 * I2C 控制器
 */
typedef struct
{
	volatile unsigned int cr1;					// 0x00 I2C_CR1 I2C 控制寄存器
	volatile unsigned int cr2;					// 0x04 I2C_CR2 I2C 中断控制寄存器
	volatile unsigned int oar;					// 0x08 I2C_OAR I2C 从地址寄存器
	volatile unsigned int rsv;
	volatile unsigned int dr;					// 0x10 I2C_DR I2C 数据寄存器
	volatile unsigned int sr1;					// 0x14 I2C_SR1 I2C 状态寄存器1
	volatile unsigned int sr2;					// 0x18 I2C_SR2 I2C 状态寄存器2
	volatile unsigned int ccr;					// 0x1c I2C_CCR I2C 时钟控制寄存器
	volatile unsigned int trise;				// 0x20 I2C_TRISE I2C 上升时间寄存器
} HW_I2C_t;

/**
 * I2C 控制寄存器(I2C_CR1)
 */
#define I2C_CR1_SWRST			bit(15)			/* RW 软复位. 0: 未复位;  1: 复位. 该复位只复位内部控制逻辑, 不复位寄存器, 应配合CR1_PE 使用软件
                                                 *    可配置和清除该位. */
#define I2C_CR1_RECOVER 		bit(14)			/* RW 总线恢复命令. 发出9 个SCL 时钟和一个停止条件, 可用于解除死锁状态(SCL 为高, 从设备将SDA 拉低).
		 	 	 	 	 	 	 	 	 	 	 *    软件可设置和清除该位, 硬件会在发出停止条件后或CR1_PE 为0时清除该位.
		 	 	 	 	 	 	 	 	 	 	 */
#define I2C_CR1_POS 			bit(11)			/* RW 数据接收的应答位置
												 * 0: CR1_ACK 位控制当前移位寄存器正在接收的字节的应答;
												 * 1: CR1_ACK 位控制将由移位寄存器接收的下一个字节的应答.
												 * 硬件行为是在接收字节的应答周期完成时, 将CR1_ACK 表示的值保存到寄存器中作为应答值; 该位无效时,
												 * 应答寄存器复位为有应答; 故开启时, 第一个应答为有应答, 下一个为在第一个应答周期保存的CR1_ACK 表示的值.
												 * 软件可设置和清除该位, 硬件会在CR1_PE 为0 时清除该位.
												 */
#define I2C_CR1_ACK 			bit(10)			/* RW 应答使能
												 * 0: 无应答返回(在应答拍为高电平);
												 * 1: 收到一个字节后返回有应答(地址命中或接收到数据后, 在应答拍为低电平).
												 * 软件可设置和清除该位, 硬件会在CR1_PE 为0 时清除该位.
												 */
#define I2C_CR1_STOP 			bit(9)			/* RW 停止生成命令.
 	 	 	 	 	 	 	 	 	 	 	 	 * 0: 无命令;
 	 	 	 	 	 	 	 	 	 	 	 	 * 1: 主模式下在当前字节传输(不含地址阶段)或开始/恢复命令完成后, 生成总线停止条件;
                                                 *    从模式下在当前字节传输完成后(不含地址阶段)释放总线, 回到空闲状态. 
                                                 *    停止命令在主接收模式未输出无应答条件时存在总线状态不确定的风险.
 	 	 	 	 	 	 	 	 	 	 	 	 *    软件可设置和清除该位, 硬件会在总线停止条件后清除该位.
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define I2C_CR1_START 			bit(8)			/* RW 开始生成命令
												 * 0: 无命令;
												 * 1: 主模式下在当前字节传输(不含地址阶段)后, 生成重复开始条件; 从模式下在当前字节传输(不含地址阶段)
												 *    后或空闲时, 生成开始条件并进入主模式. 开始命令无视总线状态, 软件应自行检查总线状态(通过SR2_BUSY)
                                                 *    并准备处理可能的仲裁丢失错误.
												 *    软件可设置和清除该位, 硬件会在总线开始条件后或CR1_PE 为0时清除该位.
												 */
#define I2C_CR1_NOSTRETCH 		bit(7)			/* RW 时钟伸展无效(从模式). 0: 时钟伸展有效; 1: 时钟伸展无效. 软件可配置和清除该位. */
#define I2C_CR1_ENGC 			bit(6)			/* RW 广播呼叫使能. 使能广播呼叫, 地址0x00 将得到应答. 软件可配置和清除该位. */
#define I2C_CR1_PE 				bit(0)			/* RW 控制器使能. 1: 使能. 在工作中清除该位会立刻重置控制器状态机, 无视总线. 软件可配置和清除该位. */

/**
 * I2C 控制寄存器2(I2C_CR2)
 */
#define I2C_CR2_DMA 			bit(11)			/* RW DMA 请求使能. 1: 使能. */
#define I2C_IEN_ITBUF 			bit(10)			/* RW 缓冲类中断使能. 1: 使能缓冲类中断, 包括TXE 和RXNE */
#define I2C_IEN_ITEVT 			bit(9)			/* RW 事件类中断使能. 1: 使能事件类中断, 包括SB、ADDR、STOPF、BTF、TXE、RXNE */
#define I2C_IEN_ITERR 			bit(8)  		/* RW 错误类中断使能. 1: 错误类中断使能, 包括BERR、ARLO、AF、OVR */

#define I2C_IEN_IRQ_MASK		(0x7<<8)        // (I2C_IEN_ITBUFEN | I2C_IEN_ITEVTEN | I2C_IEN_ITERREN)

#define I2C_CR2_FREQ_MASK		0x3F			/* RW bit[5:0] 设备时钟频率. 表示APB 接口时钟频率(MHz), 用于生成数据建立和保持事件(从模式).
												 *    若使用外部8M 晶体, 则该位域配置为8; 若使用内部时钟, 该位域配置为11(内部32M 时钟3 分频).
												 */

/**
 * I2C 从地址寄存器(I2C_OAR)
 */
#define I2C_OAR_MASK			0xFE 			/* RW bit[7:1] 从设备地址. 作为从模式地址阶段的[7:1]位地址进行地址命中的判断 */
#define I2C_OAR_SHIFT			1

/**
 * I2C 数据寄存器(I2C_DR)
 */
#define I2C_DR_MASK				0xFF			/* RW bit[7:0] 字节数据寄存器. 写入时, 写到发送数据寄存器(TXR)中;
												 * 读出时, 读出接收数据寄存器(RXR)的值. 从模式接收的地址不会放入RXR 中.
												 */

/**
 * I2C 状态寄存器(I2C_SR1)
 */
#define I2C_SR1_OVR 			bit(11)			/* R 溢出状态位. 软件向该位写0 清除状态, 硬件在CR1_PE 为0 时清除该位. */
#define I2C_SR1_AF		 		bit(10)			/* R 应答失败状态位. 软件向该位写0 清除状态, 硬件在CR1_PE 为0 时清除该位. */
#define I2C_SR1_ARLO 			bit(9)			/* R 仲裁丢失状态位. 软件向该位写0 清除状态, 硬件在CR1_PE 为0 时清除该位. */
#define I2C_SR1_BERR 			bit(8)			/* R 总线错误状态位. 软件向该位写0 清除状态, 硬件在CR1_PE 为0 时清除该位. */
#define I2C_SR1_TXE 			bit(7)			/* R 发送数据寄存器空. 发送模式时数据寄存器为空. 在地址阶段不会置位, 收到无应答(NACK)条件时不会置位.
												 *   软件写数据寄存器后清除, 硬件在开始或停止条件后或在CR1_PE 为0 时清除.
												 *   该位在写数据寄存器清除后可能很快再次置起, 此时可再次写入.
												 */
#define I2C_SR1_RXNE 			bit(6)			/* R 接收数据寄存器非空. 接收模式时数据寄存器非空, 即收到了有效的数.
 	 	 	 	 	 	 	 	 	 	 	 	 *   在地址阶段不会置位, 在出现仲裁丢失时不会置位. 软件写或读数据寄存器后清除, 硬件在CR1_PE 为0 时清除.
 	 	 	 	 	 	 	 	 	 	 	 	 *   该位在读数据寄存器清除后可能很快再次置起, 因为移位寄存器中的数放到了接收数据寄存器, 此时可再次读出.
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define I2C_SR1_STOPF 			bit(4)  		/* R 从模式停止位检测. 从模式时, 在正常的应答后检测到总线停止条件后置位. 收到无应答(NACK)时不会置位.
 	 	 	 	 	 	 	 	 	 	 	 	 *   要清除该位, 软件在读出该位为1 后, 再写I2C_CR1 寄存器; 硬件在CR1_PE 为0 时清除.
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define I2C_SR1_BTF 			bit(2)			/* R 字节传输结束状态. 当CR1_NOSTRETCH 为0 时可置位. 接收模式时, 接收到一个字节且接收数据寄存器非空时置位;
 	 	 	 	 	 	 	 	 	 	 	 	 *   发送模式时, 发送完一个数据字节后且发送数据寄存器为空时置位. 在收到无应答(NACK)条件时不会置位.
 	 	 	 	 	 	 	 	 	 	 	 	 *   要清除该位, 软件在读出该位为1 后, 再写或读数据寄存器; 硬件收到开始或停止状态, 或在CR1_PE 为0 时清除.
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define I2C_SR1_ADDR 			bit(1)  		/* R 地址阶段成功. 主模式时发送地址成功, 或从模式时接收地址命中.
                                                 *   收到NACK 时不会置位. 
 	 	 	 	 	 	 	 	 	 	 	 	 *   要清除该位, 软件在读出该位为1 后, 再读I2C_SR2 寄存器; 硬件在CR1_PE 为0 时清除.
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define I2C_SR1_SB 				bit(0)			/* R 开始条件成功. 主模式成功生成开始条件后置位. 要清除改位, 软件在读出该位为1 后, 再写数据寄存器(地址);
 	 	 	 	 	 	 	 	 	 	 	 	 *   硬件在CR1_PE 为0 时清除.
 	 	 	 	 	 	 	 	 	 	 	 	 */

/*
 * 共 3 类事件和中断
 */
#define I2C_SR1_ITEVTEN_MASK	(I2C_SR1_BTF | I2C_SR1_ADDR | I2C_SR1_SB)
#define I2C_SR1_ITBUFEN_MASK	(I2C_SR1_TXE | I2C_SR1_RXNE)
#define I2C_SR1_ITERREN_MASK	(I2C_SR1_AF  | I2C_SR1_ARLO | I2C_SR1_BERR)

/**
 * I2C 状态寄存器2(I2C_SR2)
 */
#define I2C_SR2_GENCALL 		bit(4)			/* R 广播地址命中. 当CR1_ENGC 有效时, 接收到了广播地址后置位. 硬件接收到开始或停止条件后清除,
                                                 *   或在CR1_PE 为0 时清除. */
#define I2C_SR2_TRA 			bit(2)			/* R 发送模式. 0: 接收模式; 1: 发送模式. 当内部状态机为发送相关的状态时, 该位为1, 否则均为0.
 	 	 	 	 	 	 	 	 	 	 	 	 *   当CR1_PE 为0, 或各种会导致状态机离开发送状态的事件均会清除该位.
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define I2C_SR2_BUSY 			bit(1)  		/* R 总线忙. 硬件检测到SCL 或SDA 线上为低时置位, 检测到停止条件后清除. 该位在CR1_PE 为0 时仍会更新. */
#define I2C_SR2_MSL 			bit(0)			/* R 主从模式. 0: 从模式; 1: 主模式. 默认为从模式, 从模式下收到开始或恢复命令后变为主模式.
 	 	 	 	 	 	 	 	 	 	 	 	 *   主模式接收到停止条件或仲裁丢失时回到从模式, 在CR1_PE 为0 时为从模式.
 	 	 	 	 	 	 	 	 	 	 	 	 */

/**
 * I2C 时钟控制寄存器(I2C_CCR)
 */
#define I2C_CCR_F_S 			bit(15)			/* RW 主模式选择. 0: 标准模式; 1: 快速模式. */
#define I2C_CCR_DUTY 			bit(14)			/* RW 占空比控制. 0: 快速模式SCL 高低电平时间为1:2; 1: 快速模式SCL 高低电平时间为9:16. */
#define I2C_CCR_MASK 			0xFFF			/* RW bit[11:0] 时钟分频控制. 定义SCL 的高电平时间T_high 和低电平时间T_low.
												 *    标准模式时: 				    T_high=CCR_CCR*T_PCLK; 	 T_low=CCR_CCR*T_PCLK
												 *    快速模式且CCR_DUTY 为0 时: 	T_high=CCR_CCR*T_PCLK; 	 T_low=2*CCR_CCR*T_PCLK
												 *    快速模式且CCR_DUTY 为1 时: 	T_high=9*CCR_CCR*T_PCLK; T_low=16*CCR_CCR*T_PCLK
												 *    例如, 要配置标准模式100KHz 时钟, APB 时钟为8MHz 时: CCR_CCR = 5000ns/125ns = 40
												 */

/**
 * I2C 上升时间寄存器(I2C_TRISE)
 */
#define I2C_TRISE_MASK			0x3F 			/* RW bit[5:0] SCL 最大上升时间(主模式). 根据总线协议规定的最大SCL 上升时间与APB 时钟进行配置.
												 *    例如, 标准模式下最大上升事件为1000ns, 则TRISE 配置为TRISE = 1000ns/125ns + 1 = 9.
												 *    当除法结果不是整数时, 向下取整即可. 该位域用于在合适的时间判断是否出现时钟伸展, 从而有助
                                                 *    于保持SCL 频率稳定.
												 */

#endif // _LS2K_I2C_HW_H


