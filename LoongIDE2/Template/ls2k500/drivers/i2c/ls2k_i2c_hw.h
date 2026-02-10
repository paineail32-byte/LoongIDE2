/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_i2c_hw.h
 *
 * created: 2022-02-24
 *  author: Bian
 */

#ifndef _LS2K_I2C_HW_H
#define _LS2K_I2C_HW_H

//-------------------------------------------------------------------------------------------------
// I2C 设备
//-------------------------------------------------------------------------------------------------

#define I2C0_BASE               0x1ff48000
#define I2C1_BASE               0x1ff48800
#define I2C2_BASE               0x1ff49000
#define I2C3_BASE               0x1ff49800
#define I2C4_BASE               0x1ff4a000
#define I2C5_BASE               0x1ff4a800

/*
 * I2C 控制器
 */
typedef struct
{
    volatile unsigned char prerlo;              // 0x00 分频值低字节寄存器
    volatile unsigned char prerhi;              // 0x01 分频值高字节寄存器
    volatile unsigned char ctrl;                // 0x02 控制寄存器
    volatile unsigned char data;                // 0x03 数据寄存器
    union
	{
    	volatile unsigned char cmd;             // 0x04 命令寄存器
    	volatile unsigned char sr;              // 0x04 状态寄存器
	} CMDSR;
    volatile unsigned char blt;                 // 0x05 总线死锁时间寄存器
    volatile unsigned char rsv;                 // 0x06
    volatile unsigned char saddr;               // 0x07 从模式地址寄存器
} HW_I2C_t;

#define I2C_CTRL_EN             bit(7)          // 模块工作使能. 1=正常工作模式, 0=对分频值寄存器进行操作
#define I2C_CTRL_IEN            bit(6)          // 1=中断使能
#define I2C_CTRL_MASTER         bit(5)          // 主从模式选择. 0=从设备, 1=主设备
#define I2C_CTRL_SLAVE_TXRDY    bit(4)          // 从设备发送数据准备好: 1=要发送的数据已写入data. 自动清零
#define I2C_CTRL_SLAVE_RXRDY    bit(3)          // 从设备接收数据已读出: 1=data内数据已被读出. 自动清零
#define I2C_CTRL_BUSLOCK_CHECK  bit(1)          // 1=总线死锁状态检查使能: 时间buslock_top
#define I2C_CTRL_SLAVE_ATUORST  bit(0)          // 1=总线死锁时从设备自动复位状态机使能

#define I2C_CMD_START           bit(7)          // 1=作为主设备下一次传输产生开始波形
#define I2C_CMD_STOP            bit(6)          // 1=作为主设备下一次传输产生结束波形
#define I2C_CMD_READ            bit(5)          // 1=作为主设备下一次传输为总线读请求
#define I2C_CMD_WRITE           bit(4)          // 1=作为主设备下一次传输为总线写请求
#define I2C_CMD_NACK            bit(3)          /* 主设备应答. 1=下一次读数据返回时应答NACK, 连续读请求结束 */
#define I2C_CMD_ACK             0
#define I2C_CMD_RECOVER         bit(2)          /* 总线死锁恢复命令. 1=主设备解除死锁 */
#define I2C_CMD_IACK            bit(0)          /* 1=产生中断应答信号 */

#define I2C_SR_RXNACK           bit(7)          /* 0=收到应答, 1=收到NACK */
#define I2C_SR_BUSY             bit(6)          /* 1=总线忙状态 */
#define I2C_SR_ALOST            bit(5)          /* 1=主设备失去总线控制权 */
#define I2C_SR_SLAVE_ADDRESSED  bit(4)          /* 1=从设备被寻址成功 */
#define I2C_SR_SLAVE_RW         bit(3)          /* 0=从设备被读, 1=从设备被写 */
#define I2C_SR_BUSLOCK          bit(2)          /* 1=总线死锁 */
#define I2C_SR_TXIP             bit(1)          /* 1=主设备正在传输 */
#define I2C_SR_IFLAG            bit(0)          /* 中断标志位. 1=传输完1个字节或主设备丢失控制权 */

#define I2C_SADDR_MASK          0x7F            /* 从设备地址 */

#endif // _LS2K_I2C_HW_H

