/*
 * Copyright (C) 2020-2023 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 *
 */

#ifndef LS1C103_H_
#define LS1C103_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define bit(x)      (1<<x)

/*
 *  寄存器 Read/Write 操作
 */
/*
 * 8 Bits
 */
#define READ_REG8(Addr)			(*(volatile unsigned char*)(Addr))
#define WRITE_REG8(Addr, Val)	(*(volatile unsigned char*)(Addr) = (Val))
#define OR_REG8(Addr, Val)		(*(volatile unsigned char*)(Addr) |= (Val))
#define AND_REG8(Addr, Val)	    (*(volatile unsigned char*)(Addr) &= (Val))

/*
 * 16 Bits
 */
#define READ_REG16(Addr) 		(*(volatile unsigned short*)(Addr))
#define WRITE_REG16(Addr, Val)	(*(volatile unsigned short*)(Addr) = (Val))
#define OR_REG16(Addr, Val)	    (*(volatile unsigned short*)(Addr) |= (Val))
#define AND_REG16(Addr, Val)	(*(volatile unsigned short*)(Addr) &= (Val))

/*
 * 32 Bits
 */
#define READ_REG32(Addr) 		(*(volatile unsigned int*)(Addr))
#define WRITE_REG32(Addr, Val)	(*(volatile unsigned int*)(Addr) = (Val))
#define OR_REG32(Addr, Val)	    (*(volatile unsigned int*)(Addr) |= (Val))
#define AND_REG32(Addr, Val)	(*(volatile unsigned int*)(Addr) &= (Val))

/**********************************************************************************************
 * 地址空间分布
 **********************************************************************************************/

#define LS1C103_IRAM_ADDR          0x10000000      // - 只能用于取指
#define LS1C103_IRAM_SIZE          0x1000          // - IRAM 大小 = 4K

#define LS1C103_DRAM_ADDR          0x10001000      // - 只能用于取指
#define LS1C103_DRAM_SIZE          0x1000          // - IRAM 大小 = 4K

#define LS1C103_FLASH_ADDR         0x18000000      // On-chip Flash 存储区
#define LS1C103_SPIFLASH_ADDR      0x20000000      // SPI Flash 存储区

#define LS1C103_BOOT_ADDR          0x1C000000      // Boot from SPI Flash or On-chip Flash

#define LS1C103_ATIM_BASE          0x00001000      // ATIM  高级定时器
#define LS1C103_GTIM_BASE          0x00002000      // GTIM  通用定时器
#define LS1C103_BTIM_BASE          0x00003000      // BTIM  基础定时器
#define LS1C103_ADC_BASE           0x00004000      // ADC   ADC 控制器
#define LS1C103_DMA_BASE           0x00005000      // DMA   DMA 控制器
#define LS1C103_CRC_BASE           0x00006000      // CRC   CRC 控制器
#define LS1C103_FLASH_BASE         0x00007000      // Flash Flash 控制器
#define LS1C103_SPI_BASE           0x00008000      // SPI   SPI 控制器
#define LS1C103_UART0_BASE         0x00009000      // UART0  串口0控制器
#define LS1C103_UART1_BASE         0x00009100      // UART1  串口1控制器
#define LS1C103_I2C_BASE           0x0000a000      // I2C   I2C 控制器
#define LS1C103_INTC_BASE          0x0000b000      // INTC  中断控制器
#define LS1C103_PMU_BASE           0x0000c000      // PMU   电源管理单元
#define LS1C103_RTC_BASE           0x0000c800      // RTC   实时时钟
#define LS1C103_AFIO_BASE          0x0000d000      // AFIO  管脚复用控制器
#define LS1C103_EXTI_BASE          0x0000d100      // EXTI  外部中断控制器
#define LS1C103_GPIOA_BASE         0x0000d200      // GPIOA GPIOA 控制器
#define LS1C103_GPIOB_BASE         0x0000d300      // GPIOB GPIOA 控制器

/**********************************************************************************************
 * ATIM 高级定时器
 *
 * 寄存器位域, 使用 ls1c103_tim_hw.h 全部宏定义
 *
 **********************************************************************************************/

typedef struct
{
    volatile unsigned int cr1;                  // 0x00 控制寄存器1
    volatile unsigned int cr2;                  // 0x04 控制寄存器2
    volatile unsigned int smcr;                 // 0x08 从模式控制寄存器
    volatile unsigned int dier;                 // 0x0C DMA/中断使能寄存器
    volatile unsigned int sr;                   // 0x10 状态寄存器
    volatile unsigned int egr;                  // 0x14 事件产生寄存器
    volatile unsigned int ccmr1;                // 0x18 捕获/比较模式寄存器1
    volatile unsigned int ccmr2;                // 0x1C 捕获/比较模式寄存器2
    volatile unsigned int ccer;                 // 0x20 捕获/比较使能寄存器
    volatile unsigned int cnt;                  // 0x24 计数器
    volatile unsigned int psc;                  // 0x28 预分频器
    volatile unsigned int arr;                  // 0x2C 自动重装载寄存器
    volatile unsigned int rcr;                  // 0x30 重复计数寄存器
    volatile unsigned int ccr1;                 // 0x34 捕获/比较寄存器1
    volatile unsigned int ccr2;                 // 0x38 捕获/比较寄存器2
    volatile unsigned int ccr3;                 // 0x3C 捕获/比较寄存器3
    volatile unsigned int ccr4;                 // 0x40 捕获/比较寄存器4
    volatile unsigned int bdtr;                 // 0x44 刹车和死区寄存器
    volatile unsigned int dcr;                  // 0x48 DMA 控制寄存器
    volatile unsigned int dmar;                 // 0x4C 连续模式的DMA地址
} HW_ATIM_t;

/**********************************************************************************************
 * GTIM 通用定时器
 *
 * 寄存器位域, 使用 ls1c103_tim_hw.h 以 G_/B_ 开头的宏定义
 *
 **********************************************************************************************/

typedef struct
{
    volatile unsigned int cr1;                  // 0x00 控制寄存器1
    volatile unsigned int cr2;                  // 0x04 控制寄存器2
    volatile unsigned int smcr;                 // 0x08 从模式控制寄存器
    volatile unsigned int dier;                 // 0x0C DMA/中断使能寄存器
    volatile unsigned int sr;                   // 0x10 状态寄存器
    volatile unsigned int egr;                  // 0x14 事件产生寄存器
    volatile unsigned int ccmr1;                // 0x18 捕获/比较模式寄存器1
    volatile unsigned int ccmr2;                // 0x1C 捕获/比较模式寄存器2
    volatile unsigned int ccer;                 // 0x20 捕获/比较使能寄存器
    volatile unsigned int cnt;                  // 0x24 计数器
    volatile unsigned int psc;                  // 0x28 预分频器
    volatile unsigned int arr;                  // 0x2C 自动重装载寄存器
    volatile unsigned int rsv0;                 // 0x30 重复计数寄存器
    volatile unsigned int ccr1;                 // 0x34 捕获/比较寄存器1
    volatile unsigned int ccr2;                 // 0x38 捕获/比较寄存器2
    volatile unsigned int ccr3;                 // 0x3C 捕获/比较寄存器3
    volatile unsigned int ccr4;                 // 0x40 捕获/比较寄存器4
    volatile unsigned int rsv1;                 // 0x44
    volatile unsigned int dcr;                  // 0x48 DMA 控制寄存器
    volatile unsigned int dmar;                 // 0x4C 连续模式的DMA地址
} HW_GTIM_t;

/**********************************************************************************************
 * BTIM 通用定时器
 *
 * 存器位域, 使用 ls1c103_tim_hw.h 以 B_ 开头的宏定义
 *
 **********************************************************************************************/

typedef struct
{
    volatile unsigned int cr1;                  // 0x00 控制寄存器1
    volatile unsigned int cr2;                  // 0x04 控制寄存器2
    volatile unsigned int rsv0;
    volatile unsigned int dier;                 // 0x0C DMA/中断使能寄存器
    volatile unsigned int sr;                   // 0x10 状态寄存器
    volatile unsigned int egr;                  // 0x14 事件产生寄存器
    volatile unsigned int rsv1[3];
    volatile unsigned int cnt;                  // 0x24 计数器
    volatile unsigned int psc;                  // 0x28 预分频器
    volatile unsigned int arr;                  // 0x2C 自动重装载寄存器
} HW_BTIM_t;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/**
 * 控制寄存器1 - CR1
 */
#define G_CR1_CKD_MASK          0x300           // bit[9:8] 时钟分频因子
#define G_CR1_CKD_0             0               // 0b00: tDTS=1*tCKINT
#define G_CR1_CKD_2             0x100           // 0b01: tDTS=2*tCKINT
#define G_CR1_CKD_4             0x200           // 0b10: tDTS=4*tCKINT
#define B_CR1_ARPE              bit(7)          // 自动重装载预装载使能
#define G_CR1_CMS_MASK          0x60            // bit[6:5] 计数模式选择
#define G_CR1_CMS_EDGE_ALIGN    0               // 0b00: 边沿对齐模式, 计数器依据方向位(dir)向上或向下计数
#define G_CR1_CMS_CENTER_ALIGN1 0x20            // 0b01: 中央对齐模式1, 仅向下计数时被设置
#define G_CR1_CMS_CENTER_ALIGN2 0x40            // 0b10: 中央对齐模式2, 仅向上计数时被设置
#define G_CR1_CMS_CENTER_ALIGN3 0x60            // 0b11: 中央对齐模式3, 向上或者向下计数时均被设置
#define G_CR1_DIR               bit(4)          // 方向. 0=计数器向上计数; 1=计数器向下计数.
                                                //       当中央对齐、编码器模式时, 由硬件自动配置, 只读
#define B_CR1_OPM               bit(3)          // 单脉冲模式. 0: 关闭单脉冲模式
                                                //             1: 发生更新事件时, 清除CEN位, 停止计数
#define B_CR1_URS               bit(2)          // 更新请求源. 0: 计数器上溢或下溢、设置UG位、通过从模式控制器产生的更新
                                                //                均可产生更新中断或者DMA请求
                                                //             1: 只有计数器上溢或下溢可以产生更新中断或者DMA请求
#define B_CR1_UDIS              bit(1)          // 1=禁止更新.
#define B_CR1_CEN               bit(0)          // 1=计数使能

/**
 * 控制寄存器2 - CR2
 */
#define A_CR2_OIS4              bit(14)         // 输出空闲状态4
#define A_CR2_OIS3N             bit(13)         // 互补输出空闲状态3
#define A_CR2_OIS3              bit(12)         // 输出空闲状态3
#define A_CR2_OIS2N             bit(11)         // 互补输出空闲状态2
#define A_CR2_OIS2              bit(10)         // 输出空闲状态2
#define A_CR2_OIS1N             bit(9)          // 互补输出空闲状态1
#define A_CR2_OIS1              bit(8)          // 输出空闲状态1

#define G_CR2_TI1S              bit(7)          // TI1输入选择. 0: CH1引脚连接到TI1输入, 1: CH1、CH2、CH3连接到TI1输入
#define B_CR2_MMS_MASK          0x70            // bit[6:4] 主模式选择, 用于选择在主模式下向从外设发送的触发输入来源
#define B_CR2_MMS_RESET         0               // 0b000: 复位, 使用配置UG位和从模式控制器产生的硬件复位作为触发输出
#define B_CR2_MMS_EN            0x10            // 0b001: 使能, 使用计数器使能信号作为触发输出. 由CEN位共同决定
#define B_CR2_MMS_UPD           0x20            // 0b010: 更新, 更新事件作为触发输出
#define G_CR2_MMS_COMP          0x30            // 0b011: 比较脉冲, 当发生一次捕获或者比较成功、设置CC1IF标志时触发输出一个脉冲
#define G_CR2_MMS_OC1REF        0x40            // 0b100: OC1REF信号被作为触发输出
#define G_CR2_MMS_OC2REF        0x50            // 0b101: OC2REF信号被作为触发输出
#define G_CR2_MMS_OC3REF        0x60            // 0b110: OC3REF信号被作为触发输出
#define G_CR2_MMS_OC4REF        0x70            // 0b111: OC4REF信号被作为触发输出

#define G_CR2_CCDS              bit(3)          // 捕获/比较的DMA选择. 0: 当发生比较/捕获事件时, 发送CCx的DMA请求
                                                //                     1: 当发生更新事件时, 发送CCx的DMA请求

#define A_CR2_CCUS              bit(2)          // 捕获/比较控制更新选择
#define A_CR2_CCPS              bit(0)          // 捕获/比较预装载控制

/**
 * 从模式控制寄存器 - SMCR
 */

/* 滤波器参数
 */
typedef enum {
    filter_0    = 0x0,              // 0b0000: 无滤波器, 以fDTS采样
    filter_1_2  = 0x1,              // 0b0001: fSAMP=fCK_INT, N=2
    filter_1_4  = 0x2,              // 0b0010: fSAMP=fCK_INT, N=4
    filter_1_8  = 0x3,              // 0b0011: fSAMP=fCK_INT, N=8
    filter_2_6  = 0x4,              // 0b0100: fSAMP=fDTS/2,  N=6
    filter_2_8  = 0x5,              // 0b0101: fSAMP=fDTS/2,  N=8
    filter_4_6  = 0x6,              // 0b0110: fSAMP=fDTS/4,  N=6
    filter_4_8  = 0x7,              // 0b0111: fSAMP=fDTS/4,  N=8
    filter_8_6  = 0x8,              // 0b1000: fSAMP=fDTS/8,  N=6
    filter_8_8  = 0x9,              // 0b1001: fSAMP=fDTS/8,  N=8
    filter_16_5 = 0xA,              // 0b1010: fSAMP=fDTS/16, N=5
    filter_16_6 = 0xB,              // 0b1011: fSAMP=fDTS/16, N=6
    filter_16_8 = 0xC,              // 0b1100: fSAMP=fDTS/16, N=8
    filter_32_5 = 0xD,              // 0b1101: fSAMP=fDTS/32, N=5
    filter_32_6 = 0xE,              // 0b1110: fSAMP=fDTS/32, N=6
    filter_32_8 = 0xF,              // 0b1111: fSAMP=fDTS/32, N=8
} samp_filter_t;

#define G_SMCR_ETP              bit(15)         // 外部触发极性. 1=ETR反相
#define G_SMCR_ECE              bit(14)         // 外部时钟使能位. 1=使能外部时钟模式2
#define G_SMCR_ETPS_MASK        0x3000          // bit[13:12] 外部触发预分频. 外部触发信号的频率最多是定时器内部时钟频率的1/4
#define G_SMCR_ETPS_DIS         0               // 0b00: 关闭预分频
#define G_SMCR_ETPS_DIV2        0x1000          // 0b01: ETR频率除于2
#define G_SMCR_ETPS_DIV4        0x2000          // 0b10: ETR频率除于4
#define G_SMCR_ETPS_DIV8        0x3000          // 0b11: ETR频率除于8
#define G_SMCR_ETF_MASK         0xF00           // bit[11:8] 外部触发滤波. 配置对ETR信号采样的频率和数字滤波的带宽
                                                //           数字滤波器是一个事件计数器, 记录到N个事件后产生输出跳变
#define G_SMCR_ETF_SHIFT        8               // XXX samp_filter_t
#define G_SMCR_MSM              bit(7)          // 主/从模式. 0=关闭主从模式
#define G_SMCR_TS_MASK          0x70            // bit[6:4] 触发来源选择
#define G_SMCR_TS_INNER         0               // 0b000: 内部触发
#define G_SMCR_TS_TI1           0x40            // 0b100: TI1的边沿检测器
#define G_SMCR_TS_CH1           0x50            // 0b101: 滤波后的CH1输入
#define G_SMCR_TS_CH2           0x60            // 0b110: 滤波后的CH2输入
#define G_SMCR_TS_ETR           0x70            // 0b111: 外部触发输入ETR
#define G_SMCR_SMS_MASK         0x07            // bit[2:0] 从模式选择
#define G_SMCR_SMS_DIS          0               // 0b000: 关闭从模式
#define G_SMCR_SMS_ENCODER1     0x01            // 0b001: 编码器模式1: 根据TI1电平, 计数器在TI2的边沿向上/向下计数
#define G_SMCR_SMS_ENCODER2     0x02            // 0b010: 编码器模式2: 根据TI2电平, 计数器在TI1的边沿向上/向下计数
#define G_SMCR_SMS_ENCODER3     0x03            // 0b011: 编码器模式3: 根据另一个信号电平, 计数器在TI1/TI2的边沿向上/向下计数
#define G_SMCR_SMS_RESET        0x04            // 0b100: 复位模式
#define G_SMCR_SMS_GATE         0x05            // 0b101: 门控模式
#define G_SMCR_SMS_TRIG         0x06            // 0b110: 触发模式
#define G_SMCR_SMS_OCLK         0x07            // 0b111: 外部时钟模式

/**
 * DMA/中断使能寄存器 - DIER
 */
#define G_DIER_TDE              bit(14)         // 触发事件DMA请求使能
#define A_DIER_COMDE            bit(13)         // COM事件DMA请求使能
#define G_DIER_CC4DE            bit(12)         // 捕获/请求通道4的DMA请求使能
#define G_DIER_CC3DE            bit(11)         // 捕获/请求通道3的DMA请求使能
#define G_DIER_CC2DE            bit(10)         // 捕获/请求通道2的DMA请求使能
#define G_DIER_CC1DE            bit(9)          // 捕获/请求通道1的DMA请求使能
#define B_DIER_UDE              bit(8)          // 更新事件DMA请求使能0:...........
#define G_DIER_BIE              bit(7)          // 刹车事件中断使能
#define G_DIER_TIE              bit(6)          // 触发事件中断使能
#define G_DIER_COMIE            bit(5)          // COM事件中断使能
#define G_DIER_CC4IE            bit(4)          // 捕获/请求通道4的中断使能
#define G_DIER_CC3IE            bit(3)          // 捕获/请求通道3的中断使能
#define G_DIER_CC2IE            bit(2)          // 捕获/请求通道2的中断使能
#define G_DIER_CC1IE            bit(1)          // 捕获/请求通道1的中断使能
#define B_DIER_UIE              bit(0)          // 更新事件中断使能

/**
 * 状态寄存器 - SR
 */
#define G_SR_CC4OF              bit(12)         // 捕获/请求通道4重复捕获事件标志位
#define G_SR_CC3OF              bit(11)         // 捕获/请求通道3重复捕获事件标志位
#define G_SR_CC2OF              bit(10)         // 捕获/请求通道2重复捕获事件标志位
#define G_SR_CC1OF              bit(9)          // 捕获/请求通道1重复捕获事件标志位
#define A_SR_BIF                bit(7)          // 刹车事件标志位
#define G_SR_TIF                bit(6)          // 触发事件标志位
#define A_SR_COMIF              bit(5)          // COM事件标志位
#define G_SR_CC4IF              bit(4)          // 捕获/请求通道4事件标志位
#define G_SR_CC3IF              bit(3)          // 捕获/请求通道3事件标志位
#define G_SR_CC2IF              bit(2)          // 捕获/请求通道2事件标志位
#define G_SR_CC1IF              bit(1)          // 捕获/请求通道1事件标志位
#define B_SR_UIF                bit(0)          // 更新事件标志位

/**
 * 事件产生寄存器 - EGR
 */
#define A_EGR_BG                bit(7)          // 产生刹车事件
#define G_EGR_TG                bit(6)          // 产生触发事件
#define A_EGR_COMG              bit(5)          // 产生COM事件
#define G_EGR_CC4G              bit(4)          // 产生捕获/比较4事件
#define G_EGR_CC3G              bit(3)          // 产生捕获/比较3事件
#define G_EGR_CC2G              bit(2)          // 产生捕获/比较2事件
#define G_EGR_CC1G              bit(1)          // 产生捕获/比较1事件
#define B_EGR_UG                bit(0)          // 产生更新事件

/**
 * 捕获/比较模式寄存器1 - 输出 - CCMR1
 */

/* 输出比较模式
 */
typedef enum {
    out_comp_hold = 0x0,            // 0b000: 冻结, OCxREF保持当前值不变
    out_comp_hi   = 0x1,            // 0b001: 当计数器CNT的值与捕获/比较寄存器CCRx相同时, 强制OCxREF为高电平
    out_comp_lo   = 0x2,            // 0b010: 当计数器CNT的值与捕获/比较寄存器CCRx相同时, 强制OCxREF为低电平
    out_comp_inv  = 0x3,            // 0b011: 当计数器CNT的值与捕获/比较寄存器CCRx相同时, 翻转OCxREF的电平
    out_comp_high = 0x4,            // 0b100: 强制OCxREF为低电平
    out_comp_low  = 0x5,            // 0b101: 强制OCxREF为高电平
    out_comp_pwm1 = 0x6,            // 0b110: PWM模式1
    out_comp_pwm2 = 0x7,            // 0b111: PWM模式2
} out_compare_t;

/* 输出比较选择
 */
typedef enum {
    channel_out    = 0x0,           // 0b00: CCx通道被配置为输出
    channel_in_ti2 = 0x1,           // 0b01: CCx通道被配置为输入, ICx映射在TI2上
    channel_in_ti1 = 0x2,           // 0b10: CCx通道被配置为输入, ICx映射在TI1上
    channel_in_trc = 0x3,           // 0b11: CCx通道被配置为输入, ICx映射在TRC上
} out_channel_t;

#define G_CCMR1_OC2CE           bit(15)         // 输出比较2清零使能. 1=检测到ETR输入高电平, 清除OC2REF=0
#define G_CCMR1_OC2M_MASK       0x7000          // bit[14:12] 输出比较2模式
#define G_CCMR1_OC2M_SHIFT      12              // XXX out_compare_t
#define G_CCMR1_OC2PE           bit(11)         // 输出比较2预装载使能
#define G_CCMR1_OC2FE           bit(10)         // 输出比较2快速使能
#define G_CCMR1_CC2S_MASK       0x300           // bit[9:8] 输出/比较2选择
#define G_CCMR1_CC2S_SHIFT      8               // XXX out_channel_t
#define G_CCMR1_OC1CE           bit(7)          // 输出比较1清零使能. 1=检测到ETR输入高电平, 清除OC1REF=0
#define G_CCMR1_OC1M_MASK       0x70            // bit[6:4] 输出比较1模式
#define G_CCMR1_OC1M_SHIFT      4               // XXX out_compare_t
#define G_CCMR1_OC1PE           bit(3)          // 输出比较1预装载使能
#define G_CCMR1_OC1FE           bit(2)          // 输出比较1快速使能
#define G_CCMR1_CC1S_MASK       0x3             // bit[1:0] 输出/比较1选择
#define G_CCMR1_CC1S_SHIFT      0               // XXX out_channel_t

/**
 * 捕获/比较模式寄存器1 - 输入 - CCMR1
 */

/* 捕获预分频
 */
typedef enum {
    cap_prescale_0 = 0x0,           // 0b00: 无预分频
    cap_prescale_2 = 0x1,           // 0b01: 每2个事件触发一次捕获
    cap_prescale_4 = 0x2,           // 0b10: 每4个事件触发一次捕获
    cap_prescale_8 = 0x3,           // 0b11: 每8个事件触发一次捕获
} capture_prescale_t;

#define G_CCMR1_IC2F_MASK       0xF000          // bit[15:12] 输入捕获2滤波器
#define G_CCMR1_IC2F_SHIFT      12              // XXX samp_filter_t
#define G_CCMR1_IC2PSC_MASK     0xC00           // bit[11:10] 输入捕获2预分频器
#define G_CCMR1_IC2PSC_SHIFT    10              // XXX capture_prescale_t
#define G_CCMR1_CC2S_MASK       0x300           // bit[9:8] 输出/比较2选择
#define G_CCMR1_CC2S_SHIFT      8               // XXX out_channel_t
#define G_CCMR1_IC1F_MASK       0xF0            // bit[7:4] 输入捕获1滤波器
#define G_CCMR1_IC1F_SHIFT      4               // XXX samp_filter_t
#define G_CCMR1_IC1PSC_MASK     0xC             // bit[3:2] 输入捕获1预分频器
#define G_CCMR1_IC1PSC_SHIFT    2               // XXX capture_prescale_t
#define G_CCMR1_CC1S_MASK       0x3             // bit[1:0] 输出/比较1选择
#define G_CCMR1_CC1S_SHIFT      0               // XXX out_channel_t

/**
 * 捕获/比较模式寄存器2 - 输出 - CCMR2
 */
#define G_CCMR2_OC4CE           bit(15)         // 输出比较4清零使能. 1=检测到ETR输入高电平, 清除OC4REF=0
#define G_CCMR2_OC4M_MASK       0x7000          // bit[14:12] 输出比较4模式
#define G_CCMR2_OC4M_SHIFT      12              // XXX out_compare_t
#define G_CCMR2_OC4PE           bit(11)         // 输出比较4预装载使能
#define G_CCMR2_OC4FE           bit(10)         // 输出比较4快速使能
#define G_CCMR2_CC4S_MASK       0x300           // bit[9:8] 输出/比较4选择
#define G_CCMR2_CC4S_SHIFT      8               // XXX out_channel_t
#define G_CCMR2_OC3CE           bit(7)          // 输出比较3清零使能. 1=检测到ETR输入高电平, 清除OC3REF=0
#define G_CCMR2_OC3M_MASK       0x70            // bit[6:4] 输出比较3模式
#define G_CCMR2_OC3M_SHIFT      4               // XXX out_compare_t
#define G_CCMR2_OC3PE           bit(3)          // 输出比较3预装载使能
#define G_CCMR2_OC3FE           bit(2)          // 输出比较3快速使能
#define G_CCMR2_CC3S_MASK       0x3             // bit[1:0] 输出/比较3选择
#define G_CCMR2_CC3S_SHIFT      0               // XXX out_channel_t

/**
 * 捕获/比较模式寄存器2 - 输入 - CCMR2
 */
#define G_CCMR2_IC4F_MASK       0xF000          // bit[15:12] 输入捕获4滤波器
#define G_CCMR2_IC4F_SHIFT      12              // XXX samp_filter_t
#define G_CCMR2_IC4PSC_MASK     0xC00           // bit[11:10] 输入捕获4预分频器
#define G_CCMR2_IC4PSC_SHIFT    10              // XXX capture_prescale_t
#define G_CCMR2_CC4S_MASK       0x300           // bit[9:8] 输出/比较4选择
#define G_CCMR2_CC4S_SHIFT      8               // XXX out_channel_t
#define G_CCMR2_IC3F_MASK       0xF0            // bit[7:4] 输入捕获3滤波器
#define G_CCMR2_IC3F_SHIFT      4               // XXX samp_filter_t
#define G_CCMR2_IC3PSC_MASK     0xC             // bit[3:2] 输入捕获3预分频器
#define G_CCMR2_IC3PSC_SHIFT    2               // XXX capture_prescale_t
#define G_CCMR2_CC3S_MASK       0x3             // bit[1:0] 输出/比较3选择
#define G_CCMR2_CC3S_SHIFT      0               // XXX out_channel_t

/**
 * 捕获/比较使能寄存器 - CCER
 */
#define G_CCER_CC4P             bit(13)         // 输入/捕获4极性.
                                                // CC4通道配置为输出. 0: OC4=OC4REF, 1: OC4为OC4REF反相输出
                                                // CC4通道配置为输入. 0: 不反相, 捕获发生在IC4的上升沿,
                                                //                    1: 反相, 捕获发生在IC4的下降沿
#define G_CCER_CC4E             bit(12)         // 输入/捕获4使能
                                                // CC4通道配置为输出. 0: OC4禁止输出, 1: OC4信号输出到对应引脚
                                                // CC4通道配置为输入. 0: 捕获禁止, 1: 捕获使能
#define A_CCER_CC3NP            bit(11)         // 输入/捕获3互补输出极性. 0: OC3N高电平有效, 1: OC3N低电平有效
#define A_CCER_CC3NE            bit(10)         // 输入/捕获3互补输出极性. 0: 关闭OC3N输出, 1: 开启OC3N输出
#define G_CCER_CC3P             bit(9)          // 输入/捕获3极性.
#define G_CCER_CC3E             bit(8)          // 输入/捕获3使能.
#define A_CCER_CC2NP            bit(7)          // 输入/捕获2互补输出极性. 0: OC2N高电平有效, 1: OC2N低电平有效
#define A_CCER_CC2NE            bit(6)          // 输入/捕获2互补输出极性. 0: 关闭OC2N输出, 1: 开启OC2N输出
#define G_CCER_CC2P             bit(5)          // 输入/捕获2极性.
#define G_CCER_CC2E             bit(4)          // 输入/捕获2使能.
#define A_CCER_CC1NP            bit(3)          // 输入/捕获1互补输出极性. 0: OC1N高电平有效, 1: OC1N低电平有效
#define A_CCER_CC1NE            bit(2)          // 输入/捕获1互补输出极性. 0: 关闭OC1N输出, 1: 开启OC1N输出
#define G_CCER_CC1P             bit(1)          // 输入/捕获1极性.
#define G_CCER_CC1E             bit(0)          // 输入/捕获3使能.

/**
 * 计数器 - CNT
 */
#define B_CNT_MASK              0xFFFF          // bit[15:0] 计数器数值

/**
 * 预分频器 - PSC
 */
#define B_PSC_MASK              0xFFFF          // bit[15:0] 预分频器数值

/**
 * 自动重装载寄存器 - ARR
 */
#define B_ARR_MASK              0xFFFF          // bit[15:0] 自动重装载数值

/**
 * 重复计数寄存器 -RCR
 */
#define A_RCR_MASK              0xFFFF          // bit[15:0] 重复计数数值

/**
 * 捕获/比较寄存器1 - CCR1
 */
#define G_CCR1_MASK             0xFFFF          // bit[15:0] 捕获/比较1数值
                                                // 输入时, 装入当前捕获/比较1寄存器的值(预装载值)
                                                // 输出时, 上次输入捕获1事件时传输的计数器CNT值

/**
 * 捕获/比较寄存器2 - CCR2
 */
#define G_CCR2_MASK             0xFFFF          // bit[15:0] 捕获/比较2数值
                                                // 输入时, 装入当前捕获/比较2寄存器的值(预装载值)
                                                // 输出时, 上次输入捕获2事件时传输的计数器CNT值
/**
 * 捕获/比较寄存器3 - CCR3
 */
#define G_CCR3_MASK             0xFFFF          // bit[15:0] 捕获/比较3数值
                                                // 输入时, 装入当前捕获/比较3寄存器的值(预装载值)
                                                // 输出时, 上次输入捕获3事件时传输的计数器CNT值

/**
 * 捕获/比较寄存器4 - CCR4
 */
#define G_CCR4_MASK             0xFFFF          // bit[15:0] 捕获/比较4数值
                                                // 输入时, 装入当前捕获/比较4寄存器的值(预装载值)
                                                // 输出时, 上次输入捕获4事件时传输的计数器CNT值

/**
 * 刹车/死区寄存器 - BDTR
 */
#define A_BDTR_MOE              bit(15)         // 主输出使能. 1=开启OC和OCN输出
#define A_BDTR_AOE              bit(14)         // 自动输出使能. 0: MOE只能被软件置1
#define A_BDTR_BKP              bit(13)         // 刹车输入极性. 0: 低电平有效, 1: 高电平有效
#define A_BDTR_BKE              bit(12)         // 刹车输入使能. 0：禁止刹车输入, 1=使能刹车输入
#define A_BDTR_OSSR             bit(11)         // 运行模式下"关闭状态"选择. 该位用于当MOE=1且通道为互补输出时
#define A_BDTR_OSSI             bit(10)         // 空闲模式下"关闭状态"选择. 该位用于当MOE=0且通道为互补输出时
#define A_BDTR_LOCK_MASK        0x300           // bit[9:8] 锁定设置
#define A_BDTR_LOCK_NONE        0               // 0b00: 关闭, 寄存器无写保护
#define A_BDTR_LOCK_1           0x100           // 0b01: 锁定级别1, low
#define A_BDTR_LOCK_2           0x200           // 0b10: 锁定级别2, middle
#define A_BDTR_LOCK_3           0x300           // 0b11: 锁定级别3, high
#define A_BDTR_UTG_MASK         0xFF            // bit[7:0] 死区发生器设置 TODO

/**
 * DMA连续传输控制寄存器 - DCR
 */
#define G_DCR_DBL_MASK          0x1F00          // bit[12:8] DMA连续传输长度
#define G_DCR_DBL_SHIFT         8
#define G_DCR_DBA_MASK          0x1F            // bit[4:0] DMA连续传输基地址

/**
 * 连续模式的DMA地址 - DMAR
 */
#define G_DMAR_MASK             0xFFFF          // bit[15:0] DMA连续传送寄存器

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/**********************************************************************************************
 * ADC 控制器
 **********************************************************************************************/
 
typedef struct
{
    volatile unsigned int sr;                   // 0x00 状态寄存器
    volatile unsigned int cr1;                  // 0x04 控制寄存器
    volatile unsigned int cr2;                  // 0x08
    volatile unsigned int smpr1;                // 0x0c 采样时间寄存器
    volatile unsigned int smpr2;                // 0x10
    volatile unsigned int jofr1;                // 0x14 注入通道偏移寄存器
    volatile unsigned int jofr2;                // 0x18
    volatile unsigned int jofr3;                // 0x1c
    volatile unsigned int jofr4;                // 0x20
    volatile unsigned int htr;                  // 0x24 看门狗高阀值寄存器
    volatile unsigned int ltr;                  // 0x28 看门狗低阀值寄存器
    volatile unsigned int sqr1;                 // 0x2c 规则序列寄存器
    volatile unsigned int sqr2;                 // 0x30
    volatile unsigned int sqr3;                 // 0x34
    volatile unsigned int jsqr;                 // 0x38 注入序列寄存器
    volatile unsigned int jdr1;                 // 0x3c 注入数据寄存器
    volatile unsigned int jdr2;                 // 0x40
    volatile unsigned int jdr3;                 // 0x44
    volatile unsigned int jdr4;                 // 0x48
    volatile unsigned int dr;                   // 0x4c 规则数据寄存器
} HW_ADC_t;

/**
 * ADC状态寄存器 - SR
 */
#define ADC_SR_START            bit(4)          // 规则通道开始位
#define ADC_SR_JSTART           bit(3)          // 注入通道开始位
#define ADC_SR_JEOC             bit(2)          // 注入通道转换结束位
#define ADC_SR_EOC              bit(1)          // 转换结束位
#define ADC_SR_AWD              bit(0)          // 模拟开门狗标志位

/**
 * ADC控制寄存器1 - CR1
 */
#define ADC_CR1_OPS_MASK        (0x3<<30)       // bit[31:30] ADC控制信号相位选择. 0=与sck对齐; 1=比sck早一个时钟周期; 2=比sck晚一个时钟周期
#define ADC_CR1_OPS_SHIFT       30
#define ADC_CR1_CLKDIV_MASK     (0xF<<24)       // bit[27:24] ADC时钟分频系数. 分频系数(cldiv+1)*2
#define ADC_CR1_CLKDIV_SHIFT    24
#define ADC_CR1_AWDEN           bit(23)         // 规则通道启用模拟看门狗
#define ADC_CR1_JAWDEN          bit(22)         // 注入通道启用模拟看门狗
#define ADC_CR1_JSTART_MASK     (0x7<<13)       // bit[15:13] 注入通道开始位
#define ADC_CR1_JSTART_SHIFT    13
#define ADC_CR1_JDISCEN         bit(12)         // 在注入通道上的间断模式
#define ADC_CR1_DISCEN          bit(11)         // 在规则通道上的间断模式
#define ADC_CR1_JAUTO           bit(10)         // 自动注入通道组开转换
#define ADC_CR1_AWDSGL          bit(9)          // 扫描模式中单通道模拟看门狗使能
#define ADC_CR1_SCAN            bit(8)          // 启用扫描模式
#define ADC_CR1_JEOCIE          bit(7)          // 允许产生jeoc中断
#define ADC_CR1_AWDIE           bit(6)          // 允许产生模拟看门狗中断
#define ADC_CR1_EOCIE           bit(5)          // 允许产生eoc中断
#define ADC_CR1_AWDCH_MASK      0x1F            // 模拟看门狗通道选择位(0~17)

/**
 * ADC控制寄存器2 - CR2
 */
#define ADC_CR2_CLKMASK         bit(31)         // ADC时钟屏蔽使能. 0=ADC时钟始终有效; 1=ADC时钟仅在工作时有效
#define ADC_CR2_EDGE            bit(30)         // ADC采样沿选择. 0=ADC上升沿有效; 1=ADC下降沿有效
#define ADC_CR2_JTRIGMOD_MASK   (0x3<<24)       // bit[25:24] 注入触发模式选择
#define ADC_CR2_JTRIGMOD_SHIFT  24
#define ADC_CR2_SWSTART         bit(22)         // 开始转换规则通道
#define ADC_CR2_JSWSTART        bit(21)         // 开始转换注入通道
#define ADC_CR2_EXTTRIG         bit(20)         // 规则通道的外部触发转换模式
#define ADC_CR2_EXTSEL_MASK     (0x7<<17)       // bit[19:17] 选择启动规则通道组转换的外部事件
#define ADC_CR2_EXTSEL_SHIFT    17
#define ADC_CR2_EXT_ATIM_CC1    (0x0<<17)       // 0b000: ATIM_CC1事件
#define ADC_CR2_EXT_ATIM_CC2    (0x1<<17)       // 0b001: ATIM_CC2事件
#define ADC_CR2_EXT_ATIM_CC3    (0x2<<17)       // 0b010: ATIM_CC3事件
#define ADC_CR2_EXT_GTIM_CC2    (0x3<<17)       // 0b011: GTIM_CC2事件
#define ADC_CR2_EXT_EXTI_11     (0x6<<17)       // 0b110: EXTI线11
#define ADC_CR2_EXT_SWSTART     (0x7<<17)       // 0b111: swstart
#define ADC_CR2_JEXTTRIG        bit(15)         // 注入通道的外部触发转换模式
#define ADC_CR2_JEXTSEL_MASK    (0x7<<12)       // 选择启动注入通道组转换的外部事件
#define ADC_CR2_JEXTSEL_SHIFT   12
#define ADC_CR2_JEXT_ATIM_TRGO  (0x0<<12)       // 0b000: ATIM_TRGO事件
#define ADC_CR2_JEXT_ATIM_CC4   (0x1<<12)       // 0b001: ATIM_CC4事件
#define ADC_CR2_JEXT_GTIM_TRGO  (0x2<<12)       // 0b010: GTIM_TRGO事件
#define ADC_CR2_JEXT_GTIM_CC1   (0x3<<12)       // 0b011: GTIM_CC1事件
#define ADC_CR2_JEXT_EXTI_15    (0x6<<12)       // 0b110: EXTI侵15
#define ADC_CR2_JEXT_JSWSTART   (0x7<<12)       // 0b111: jswstart
#define ADC_CR2_ALIGN           bit(11)         // 数据对齐. 0=右对齐; 1=左对齐
#define ADC_CR2_DMA             bit(8)          // 直接存储器访问模式. 1=使用DMA模式
#define ADC_CR2_RESETCAL        bit(3)          // 复位校准. 1=初始化校正寄存器
#define ADC_CR2_CAL             bit(2)          // A/D校准. 0=校正完成; 1=开始校正
#define ADC_CR2_CONTINUE        bit(1)          // 连续转换. 0=单次转换模式; 1=连续转换模式
#define ADC_CR2_ADON            bit(0)          // 开/关A/D转换器. 0=关闭ADC转换/校正, 并进入断电模式; 1=开启ADC并启动转换

/**
 * ADC采样时间寄存器1 - SMPR1
 */
#define ADC_SAMP1_T(x, t)   ((t&0x7)<<(x*3))    // bit[23:0] 通道x的采样时间t, 共8个通道
#define ADC_SAMP_1T             0               // 0b000: 1个周期
#define ADC_SAMP_2T             1               // 0b001: 2个周期
#define ADC_SAMP_4T             2               // 0b010: 4个周期
#define ADC_SAMP_8T             3               // 0b011: 8个周期
#define ADC_SAMP_16T            4               // 0b100: 16个周期
#define ADC_SAMP_32T            5               // 0b101: 32个周期
#define ADC_SAMP_64T            6               // 0b110: 64个周期
#define ADC_SAMP_128T           7               // 0b111: 128个周期

/**
 * ADC采样时间寄存器2 - SMPR2
 */
#define ADC_SAMP2_T(x, t)   ((t&0x7)<<(x*3))    // bit[29:0] 通道x的采样时间t, 共10个通道

/**
 * ADC输入通道数据偏移寄存器 - JOFRx
 */
#define ADC_JOFFSET_MASK        0xFFF           // bit[11:0] 注入通道x的数据偏移

/**
 * ADC看门狗高阈值寄存器 - HTR
 */
#define ADC_HT_MASK             0xFFF           // bit[11:0] 模拟看门狗高阈值

/**
 * ADC看门狗低阈值寄存器 - LTR
 */
#define ADC_LT_MASK             0xFFF           // bit[11:0] 模拟看门狗低阈值

/**
 * ADC规则序列寄存器1 - SQR1
 */

/* 转换通道的编号与引脚的映射关系 TODO
 */
typedef enum {
    map_pa13   = 0,
    map_pa12   = 1,
    map_pa11   = 2,
    map_pa10   = 3,
    map_pa9    = 4,
    map_pa8    = 5,
    map_pa7    = 6,
    map_pa6    = 7,
    map_1v_ref = 9,
    map_pb7    = 10,
    map_pb6    = 11,
} chnl_map_pin_t;

#define ADC_SQR1_LEN_MASK       (0xF<<20)       // 规则通道序列长度
#define ADC_SQR1_LEN_SHIFT      20
#define ADC_SQR1_SQ16_MASK      (0x1F<<15)      // bit[19:15] 注入序列的第16个转换
#define ADC_SQR1_SQ16_SHIFT     15
#define ADC_SQR1_SQ15_MASK      (0x1F<<10)      // bit[14:10] 注入序列的第15个转换
#define ADC_SQR1_SQ15_SHIFT     10
#define ADC_SQR1_SQ14_MASK      (0x1F<<5)       // bit[9:5] 注入序列的第14个转换
#define ADC_SQR1_SQ14_SHIFT     5
#define ADC_SQR1_SQ13_MASK      (0x1F<<0)       // bit[4:0] 注入序列的第13个转换
#define ADC_SQR1_SQ13_SHIFT     0

/**
 * ADC规则序列寄存器2 - SQR2
 */
#define ADC_SQR2_SQ12_MASK      (0x1F<<25)      // bit[29:25] 注入序列的第12个转换
#define ADC_SQR2_SQ12_SHIFT     25
#define ADC_SQR2_SQ11_MASK      (0x1F<<20)      // bit[24:20] 注入序列的第11个转换
#define ADC_SQR2_SQ11_SHIFT     20
#define ADC_SQR2_SQ10_MASK      (0x1F<<15)      // bit[19:15] 注入序列的第10个转换
#define ADC_SQR2_SQ10_SHIFT     15
#define ADC_SQR2_SQ9_MASK       (0x1F<<10)      // bit[14:10] 注入序列的第9个转换
#define ADC_SQR2_SQ9_SHIFT      10
#define ADC_SQR2_SQ8_MASK       (0x1F<<5)       // bit[9:5] 注入序列的第8个转换
#define ADC_SQR2_SQ8_SHIFT      5
#define ADC_SQR2_SQ7_MASK       (0x1F<<0)       // bit[4:0] 注入序列的第7个转换
#define ADC_SQR2_SQ7_SHIFT      0

/**
 * ADC规则序列寄存器3 - SQR3
 */
#define ADC_SQR2_SQ6_MASK       (0x1F<<25)      // bit[29:25] 注入序列的第6个转换
#define ADC_SQR2_SQ6_SHIFT      25
#define ADC_SQR2_SQ5_MASK       (0x1F<<20)      // bit[24:20] 注入序列的第5个转换
#define ADC_SQR2_SQ5_SHIFT      20
#define ADC_SQR2_SQ4_MASK       (0x1F<<15)      // bit[19:15] 注入序列的第4个转换
#define ADC_SQR2_SQ4_SHIFT      15
#define ADC_SQR2_SQ3_MASK       (0x1F<<10)      // bit[14:10] 注入序列的第3个转换
#define ADC_SQR2_SQ3_SHIFT      1
#define ADC_SQR2_SQ2_MASK       (0x1F<<5)       // bit[9:5] 注入序列的第2个转换
#define ADC_SQR2_SQ2_SHIFT      5
#define ADC_SQR2_SQ1_MASK       (0x1F<<0)       // bit[4:0] 注入序列的第1个转换
#define ADC_SQR2_SQ1_SHIFT      0

/**
 * ADC注入序列寄存器 - JSQR
 */
#define ADC_JSQR_JLEN_MASK      (0x3<<20)       // bit[21:20] 注入通道序列长度
#define ADC_JSQR_JLEN_SHIFT     20
#define ADC_JSQR_JSQ4_MASK      (0x1F<<15)      // bit[19:15] 注入序列的第4个转换
#define ADC_JSQR_JSQ4_SHIFT     15
#define ADC_JSQR_JSQ3_MASK      (0x1F<<10)      // bit[14:10] 注入序列的第3个转换
#define ADC_JSQR_JSQ3_SHIFT     1
#define ADC_JSQR_JSQ2_MASK      (0x1F<<5)       // bit[9:5] 注入序列的第2个转换
#define ADC_JSQR_JSQ2_SHIFT     5
#define ADC_JSQR_JSQ1_MASK      (0x1F<<0)       // bit[4:0] 注入序列的第1个转换
#define ADC_JSQR_JSQ1_SHIFT     0

/**
 * ADC注入数据寄存器 - JDRx
 */
#define ADC_JDR_JDATA_MASK      0xFFFF          // bit[15:0] ADC注入转换结果

/**
 * ADC规则数据寄存器 - DR
 */
#define ADC_DR_DATA_MASK        0xFFFF          // bit[15:0] ADC规则转换结果

/**********************************************************************************************
 * DMA 控制器
 **********************************************************************************************/

typedef struct
{
    volatile unsigned int isr;                  // 0x00 中断状态寄存器
    volatile unsigned int ifcr;                 // 0x04 中断标志清除寄存器
    volatile unsigned int ccr1;                 // 0x08 通道1配置寄存器
    volatile unsigned int cndtr1;               // 0x0c 通道1传输寄存器
    volatile unsigned int cpar1;                // 0x10 通道1外设地址寄存器
    volatile unsigned int cmar1;                // 0x14 通道1存储地址寄存器
    volatile unsigned int rsv1;
    volatile unsigned int ccr2;                 // 0x1c 通道2配置寄存器
    volatile unsigned int cndtr2;               // 0x20 通道2传输寄存器
    volatile unsigned int cpar2;                // 0x24 通道2外设地址寄存器
    volatile unsigned int cmar2;                // 0x28 通道2存储地址寄存器
    volatile unsigned int rsv2;
    volatile unsigned int ccr3;                 // 0x30 通道3配置寄存器
    volatile unsigned int cndtr3;               // 0x34 通道3传输寄存器
    volatile unsigned int cpar3;                // 0x38 通道3外设地址寄存器
    volatile unsigned int cmar3;                // 0x3c 通道3存储地址寄存器
    volatile unsigned int rsv3;
    volatile unsigned int ccr4;                 // 0x44 通道4配置寄存器
    volatile unsigned int cndtr4;               // 0x48 通道4传输寄存器
    volatile unsigned int cpar4;                // 0x4c 通道4外设地址寄存器
    volatile unsigned int cmar4;                // 0x50 通道4存储地址寄存器
} HW_DMA_t;

/**
 * DMA中断状态寄存器 - ISR
 */
#define DMA_ISR_TEIF(x)         (1<<(x*4+3))    // bit[15,11,7,3] TEIFx 通道x传输错误标志
#define DMA_ISR_HTIF(x)         (1<<(x*4+2))    // bit[14,10,6,2] HTIFx 通道x传输过半标志
#define DMA_ISR_TCIF(x)         (1<<(x*4+1))    // bit[13,9,5,1]  TCIFx 通道x传输完成标志
#define DMA_ISR_GIF(x)          (1<<(x*4+0))    // bit[12,8,4,0]  GIFx  通道x全局中断标志

/**
 * DMA中断标志清除寄存器 - IFCR
 */
#define DMA_IFCR_CTEIF(x)       (1<<(x*4+3))    // bit[15,11,7,3] CTEIFx 清除通道x传输错误标志
#define DMA_IFCR_CHTIF(x)       (1<<(x*4+2))    // bit[14,10,6,2] CHTIFx 清除通道x传输过半标志
#define DMA_IFCR_CTCIF(x)       (1<<(x*4+1))    // bit[13,9,5,1]  CTCIFx 清除通道x传输完成标志
#define DMA_IFCR_CGIF(x)        (1<<(x*4+0))    // bit[12,8,4,0]  CGIFx 清除通道x全局中断标志

/**
 * DMA通道x配置寄存器 - CCRx
 */
#define DMA_CCR_MEM2MEM         bit(14)         // 存储器到存储器模式
#define DMA_CCR_PL_MASK         (0x3<<12)       // bit[13:12] 通道优先级
#define DMA_CCR_PL_LO           0
#define DMA_CCR_PL_MID          (0x1<<12)
#define DMA_CCR_PL_HI           (0x2<<12)
#define DMA_CCR_PL_HIGHEST      (0x3<<12)
#define DMA_CCR_MSIZE_MASK      (0x3<<10)       // bit[11:10] 存储器数据宽度
#define DMA_CCR_MSIZE_8         0
#define DMA_CCR_MSIZE_16        (0x1<<10)
#define DMA_CCR_MSIZE_32        (0x2<<10)
#define DMA_CCR_PSIZE_MASK      (0x3<<8)        // bit[9:8] 外设数据宽度
#define DMA_CCR_PSIZE_8         0
#define DMA_CCR_PSIZE_16        (0x1<<8)
#define DMA_CCR_PSIZE_32        (0x2<<8)
#define DMA_CCR_MINC            bit(7)          // 存储器地址增量模式
#define DMA_CCR_PINC            bit(6)          // 外设地址增量模式
#define DMA_CCR_CIRC            bit(5)          // 循环模式
#define DMA_CCR_DIR             bit(4)          // 数据传输方向
#define DMA_CCR_TEIE            bit(3)          // 传输错误中断使能
#define DMA_CCR_HTIE            bit(2)          // 传输过半中断使能
#define DMA_CCR_TCIE            bit(1)          // 传输完成中断使能
#define DMA_CCR_EN              bit(0)          // 通道开启

/**
 * DMA通道x传输数量寄存器 - CNDTRx
 */
#define DMA_CNDTR_MASK          0xFFFF          // bit[15:0] 数据传输个数

/**
 * DMA通道x外设地址寄存器 - DMA_CPARx
 */
#define DMA_CPAR_MASK           0xFFFF          // bit[15:0] 外设地址

/**
 * DMA通道x存储器地址寄存器 - CMARx
 */
#define DMA_CMAR_MASK           0xFFFF          // bit[15:0] 存储器地址

/**********************************************************************************************
 * CRC 控制器
 **********************************************************************************************/

typedef struct
{
    volatile unsigned int dr;           // 0x00 数据寄存器
    volatile unsigned int idr;          // 0x04 独立数据寄存器
    volatile unsigned int cr;           // 0x08 控制寄存器
} HW_CRC_t;

/**********************************************************************************************
 * Flash 控制器
 **********************************************************************************************/

#define FLASH_PAGE_COUNT        512
#define FLASH_PAGE_SIZE         128
#define FLASH_BYTES             (512*128)
#define FLASH_OPT_ADDR          0x18000000

typedef struct
{
    volatile unsigned int cmd;                  // 0x00 命令寄存器
    volatile unsigned int rsv1[4];
    volatile unsigned int sts;                  // 0x14 状态寄存器
    volatile unsigned int pet;                  // 0x18 擦写时间寄存器
    volatile unsigned int rsv2[3];
    volatile unsigned int trim1;                // 0x28
    volatile unsigned int trim2;                // 0x2c
    volatile unsigned int trim3;                // 0x30
    volatile unsigned int bound;                // 0x34
    volatile unsigned int opt_cfg;              // 0x38
} HW_FLASH_t;

/**
 * 命令寄存器 - CMD
 */
#define FLASH_CMD_MASK          (0xF<<28)       /* RW 命令 */
#define FLASH_CMD_SHIFT         28
#define FLASH_CMD_CLR_INT       (3<<28)         /* 4’b0011 清中断 */
#define FLASH_CMD_PAGE_LATCH    (4<<28)         /* 4’b0100 清page_latch */
#define FLASH_CMD_UPD_OPT       (9<<28)         /* 4’b1001 更新区域保护 */
#define FLASH_CMD_ERASE         (10<<28)        /* 4’b1010 擦除目标页 */
#define FLASH_CMD_SLEEP         (12<<28)        /* 4’b1100 进入休眠模式 */
#define FLASH_CMD_WRITE         (14<<28)        /* 4’b1110 编程目标页 */

#define FLASH_CMD_PAGEADDR_MASK 0x0001FFFF      /* 擦除或编程的目标页地址, 64k以内 */

/**
 * 状态寄存器 - STS
 */
#define FLASH_SR_NO_PERMISSION  bit(3)          /* RO 无权限, 表示上一次操作无权限 */
#define FLASH_SR_PE_END         bit(2)          /* RO 擦写结束 */

/**
 * 擦写时间寄存器 - PET
 */
#define FLASH_PET_INTEN_NOPER   bit(17)         /* no_permission 中断使能 */
#define FLASH_PET_INTEN_PEEND   bit(16)         /* pe_end 中断使能 */
#define FLASH_PET_ETIME_MASK    (0x7<<3)        /* bit[5:3] 擦除时间. 以8M时钟计算, 默认擦除时间2.5ms */
#define FLASH_PET_ETIME_SHIFT   3
#define FLASH_PET_ETIME_15      (0<<3)          /* 0:1.5ms */
#define FLASH_PET_ETIME_20      (1<<3)          /* 1:2.0ms */
#define FLASH_PET_ETIME_25      (2<<3)          /* 2:2.5ms */
#define FLASH_PET_ETIME_30      (3<<3)          /* 3:3.0ms */
#define FLASH_PET_ETIME_35      (4<<3)          /* 4:3.5ms */
#define FLASH_PET_ETIME_40      (5<<3)          /* 5:4.0ms */
#define FLASH_PET_ETIME_45      (6<<3)          /* 6:4.5ms */
#define FLASH_PET_ETIME_50      (7<<3)          /* 7:5.0ms */
#define FLASH_PET_PTIME_MASK    0x7             /* bit[2:0] 编程时间. 以8M时钟计算, 默认编程时间2.5ms */
#define FLASH_PET_PTIME_15      0               /* 0:1.5ms */
#define FLASH_PET_PTIME_20      1               /* 1:2.0ms */
#define FLASH_PET_PTIME_25      2               /* 2:2.5ms */
#define FLASH_PET_PTIME_30      3               /* 3:3.0ms */
#define FLASH_PET_PTIME_35      4               /* 4:3.5ms */
#define FLASH_PET_PTIME_40      5               /* 5:4.0ms */
#define FLASH_PET_PTIME_45      6               /* 6:4.5ms */
#define FLASH_PET_PTIME_50      7               /* 7:5.0ms */

/**
 * TRIM1寄存器 - TRIM1
 */
#define FLASH_TRIM1_CL_MASK     0x3F00          // bit[13:8] RO
#define FLASH_TRIM1_C_MASK      0x3F            // bit[5:0]  RO

/**
 * TRIM2寄存器 - TRIM2
 */
#define FLASH_TRIM2_ITIM_MASK   (0xF<<24)       // bit[27:24] RO
#define FLASH_TRIM2_NDACP_MASK  (0xF<<20)       // bit[23:20] RO
#define FLASH_TRIM2_NDACE_MASK  (0xF<<16)       // bit[19:16] RO
#define FLASH_TRIM2_PDACP_MASK  (0xF<<12)       // bit[15:12] RO
#define FLASH_TRIM2_PDACE_MASK  (0xF<<8)        // bit[11:8]  RO
#define FLASH_TRIM2_S_MASK      0x1F            // bit[4:0]   RO

/**
 * TRIM3寄存器 - TRIM3
 */
#define FLASH_TRIM3_ABS_MASK    (0x1F<<16)      // bit[20:16] RO
#define FLASH_TRIM3_TC_MASK     (0x7<<8)        // bit[10:8]  RO
#define FLASH_TRIM3_BDAC_MASK   0xF             // bit[3:0]   RO

/**
 * BOUND寄存器
 */
#define FLASH_BOUND_MASK        0xFFFF          // bit[15:0] RO 区域保护

/**
 * OTP配置寄存器 - OTP_CFG
 */
#define FLASH_OPT_LOCK_MASK     0xFFFF0000      // bit[31:16] RO OTP锁定配置
#define FLASH_JTAG_LOCK_MASK    0xFFFF          // bit[15:0]  RO JTAG锁定配置

/**********************************************************************************************
 * SPI 控制器
 **********************************************************************************************/

typedef struct
{
    volatile unsigned int cr;                   // 0x00 控制寄存器
    volatile unsigned int sr;                   // 0x04 状态寄存器
    volatile unsigned int data;                 // 0x08 数据寄存器
    volatile unsigned int er;                   // 0x0C 外部寄存器
    volatile unsigned int param;                // 0x10 参数控制寄存器
    volatile unsigned int cs;                   // 0x14 片选寄存器
    volatile unsigned int timing;               // 0x18 时序控制寄存器
} HW_SPI_t;

/**
 * 控制寄存器 - CR
 */
#define SPI_CR_INT_EN           bit(7)          /* 1=中断使能 */
#define SPI_CR_EN               bit(6)          /* 1=系统工作使能 */
#define SPI_CR_MASTER           bit(4)          /* 1=master 模式, 0=slave 模式 */
#define SPI_CR_CPOL             bit(3)          /* 时钟极性, 表示无时钟时 CLK 的电平. 1=高电平, 0=低电平 */
#define SPI_CR_CPHA             bit(2)          /* 时钟相位, 0=相位相同, 1=相位相反 */
#define SPI_CR_SPR_MASK         0x03            /* 时钟分频位, 与 SPER 一起使用 */

/**
 * 状态寄存器 - SR
 */
#define SPI_SR_INT_FLAG         bit(7)          /* 1=有中断, 写1清零 */
#define SPI_SR_WCOL             bit(6)          /* "写寄存器"溢出标志位, 1=溢出, 写1清零 */
#define SPI_SR_BUSY             bit(4)          /* 1=控制器忙 */
#define SPI_SR_WFFULL           bit(3)          /* 1="写寄存器"满标志 */
#define SPI_SR_WFEMPTY          bit(2)          /* 1="写寄存器"空标志 */
#define SPI_SR_RFFULL           bit(1)          /* 1="读寄存器"满标志 */
#define SPI_SR_RFEMPTY          bit(0)          /* 1="读寄存器"空标志 */

/*
 * SPER 外部寄存器
 */
#define SPI_ER_INT_CNT_MASK     0xC0            /* bit[7:6] 传输多少字节后发中断 */
#define SPI_ER_INT_CNT_SHIFT    6
#define SPI_ER_INT_1B           0
#define SPI_ER_INT_2B           1
#define SPI_ER_INT_3B           2
#define SPI_ER_INT_4B           3
#define SPI_ER_DMAE             bit(3)          /* DMA使能 */
#define SPI_ER_MODE             bit(2)          /* 接口模式. 0:采样与发送时机同步, 1:采样与发送时机错开半周期 */
#define SPI_ER_SPRE_MASK        0x03            /* 时钟分频位. 与SPR 一起设定分频比率 */
#if 0
#define SPI_ER_SPRE_2           0               /* 4’b0000 */
#define SPI_ER_SPRE_4           1               /* 4’b0001 */
#define SPI_ER_SPRE_16          2               /* 4’b0010 */
#define SPI_ER_SPRE_32          3               /* 4’b0011 */
#define SPI_ER_SPRE_8           4               /* 4’b0100 */
#define SPI_ER_SPRE_64          5               /* 4’b0101 */
#define SPI_ER_SPRE_128         6               /* 4’b0110 */
#define SPI_ER_SPRE_256         7               /* 4’b0111 */
#define SPI_ER_SPRE_512         8               /* 4’b1000 */
#define SPI_ER_SPRE_1034        9               /* 4’b1001 */
#define SPI_ER_SPRE_2048        10              /* 4’b1010 */
#define SPI_ER_SPRE_4096        11              /* 4’b1011 */
#endif

/**
 * 参数控制寄存器 - PARAM
 */
#define SPI_PARAM_CLKDIV_MASK   0xF0            /* 时钟分频数选择: 与{spre,spr}组合相同 */
#define SPI_PARAM_DUALIO        bit(3)          /* 双IO 模式, 优先级高于fast_read */
#define SPI_PARAM_FASTREAD      bit(2)          /* 快速读模式 */
#define SPI_PARAM_BURST_EN      bit(1)          /* SPI Flash 支持连续地址读模式 */
#define SPI_PARAM_MEMORY_EN     bit(0)          /* SPI Flash 读使能. 0=可通过CS0控制Flash读写 */

/*
 * CS 片选控制寄存器
 */
#define SPI_CS_MASK             0xF0            /* 片选 */
#define SPI_CS_SHIFT            4
#define SPI_CS_FLASH            0x10            /* 片选 Flash */
#define SPI_CS_1                0x20
#define SPI_CS_2                0x40
#define SPI_CS_3                0x80
#define SPI_CS_EN_MASK          0x0F            /* 片选使能, 高有效 */
#define SPI_CS_EN_FLASH         0x01            /* 使能片选 Flash */
#define SPI_CS_EN_1             0x02
#define SPI_CS_EN_2             0x04
#define SPI_CS_EN_3             0x08

/*
 * TIMING 时序控制寄存器
 */
#define SPI_TIMING_FAST         bit(2)          /* SPI flash 读采样模式. 0=上沿采样,间隔半个SPI周期; 1=上沿采样,间隔1个SPI周期*/
#define SPI_TIMING_CSH_MASK     0x03            /* SPI flash 片选信号最短无效时间, 以分频后的时钟周期T计算 */
#define SPI_TIMING_CSH_1T       0
#define SPI_TIMING_CSH_2T       1
#define SPI_TIMING_CSH_4T       2
#define SPI_TIMING_CSH_8T       3

/**********************************************************************************************
 * UART 控制器
 **********************************************************************************************/

typedef struct
{
    union
	{
    	volatile unsigned int dat;              // 0x00 数据寄存器
    	volatile unsigned int dll;              // 0x00 分频值低字节寄存器
	} R0;
    union
	{
    	volatile unsigned int ier;              // 0x04 中断使能寄存器
    	volatile unsigned int dlh;              // 0x04 分频值高字节寄存器
	} R1;
    union
	{
		volatile unsigned int isr;              // 0x08 中断状态寄存器
		volatile unsigned int fcr;              // 0x08 FIFO控制寄存器
		volatile unsigned int dld;              // 0x08 分频值小数寄存器
	} R2;
    volatile unsigned int lcr;                  // 0x0C 线路控制寄存器
    volatile unsigned int mcr;                  // 0x10 MODEM控制寄存器
    volatile unsigned int lsr;                  // 0x14 线路状态寄存器
} HW_NS16550_t;

#define NS16550_FIFO_SIZE       16              /* XXX 16 bytes? */

/**
 * 中断使能寄存器 - IER
 */
#define NS16550_IER_TXDE        bit(5)          /* 发生状态DMA使能 */
#define NS16550_IER_RXDE        bit(4)          /* 接收状态DMA使能 */
#define NS16550_IER_LINE        bit(2)          /* 线路状态中断使能 */
#define NS16550_IER_TX          bit(1)          /* 发送状态中断使能 */
#define NS16550_IER_RX          bit(0)          /* 接收状态中断使能 */

/**
 * 中断状态寄存器 - ISR
 */
#define NS16550_ISR_SRC_MASK    0x0E            /* RO 中断源 */
#define NS16550_ISR_SRC_SHIFT   1
#define NS16550_ISR_LINE        (0x3<<1)        /* 3’b011 线路状态中断, 优先级1, 奇偶/溢出/帧错误/打断时中断, 读LSR清除 */
#define NS16550_ISR_RX          (0x2<<2)        /* 3’b010 接收状态中断, 优先级2, RX数量达到trigger的值, 读data清除 */
#define NS16550_ISR_RXTMO       (0x6<<1)        /* 3’b110 接收状态中断, 优先级2, RX超时, 读data清除 */
#define NS16550_ISR_TX          (0x1<<1)        /* 3’b001 发送状态中断, 优先级3, TX FIFO为空, 写data或读isr清除 */
#define NS16550_ISR_PENDING     bit(0)          /* 中断未决状态 */

/**
 * FIFO 控制寄存器 - FCR
 */
#define NS16550_FCR_TRIG_MASK   0xC0            /* bit[7:6] 接收中断状态所需trigger. 0/1=1字节, 最多16字节 */
#define NS16550_FCR_TRIG_SHIFT  6
#define NS16550_FCR_TRIGGER(n)  ((n<<6)&0xC0)
#define NS16550_FCR_TXFIFO_RST  bit(2)          /* 复位发送FIFO */
#define NS16550_FCR_RXFIFO_RST  bit(1)          /* 复位接收FIFO */
#define NS16550_FCR_FIFO_EN     bit(0)          /* 使能FIFO? */

/**
 * 线路控制寄存器 - LCR
 */
#define NS16550_LCR_DLAB        bit(7)          /* 分频器模式. 0=访问正常寄存器, 1=访问分频寄存器 */
#define NS16550_LCR_BCB         bit(6)          /* 打断控制位. 0=正常操作, 1=串口输出置0(打断状态) */
#define NS16550_LCR_SPD         bit(5)          /* 指定奇偶校验位. 0:不指定奇偶校验位, 1: eps=1则校验位为0, eps=0则校验位为1 */
#define NS16550_LCR_EPS         bit(4)          /* 奇偶校验位选择. 0=奇校验, 1=偶校验 */
#define NS16550_LCR_PE          bit(3)          /* 1=奇偶校验位使能 */
#define NS16550_LCR_SB          bit(2)          /* 生成停止位位数. 0:1个停止位, 1:bec=5时1.5个停止位, 其它2个停止位 */
#define NS16550_LCR_BITS_MASK   0x03            /* 字符位数 */
#define NS16550_LCR_BITS_5      0
#define NS16550_LCR_BITS_6      1
#define NS16550_LCR_BITS_7      2
#define NS16550_LCR_BITS_8      3

/**
 * MODEM 控制寄存器 - MCR
 */
#define NS16550_MCR_LOOPBACK    bit(4)          // 自回环模式

/**
 * 线路状态寄存器 - LSR
 */
#define NS16550_LSR_ERR         bit(7)          /* 1=有错误, 校验/帧错误或打断中断 */
#define NS16550_LSR_TE          bit(6)          /* 0=有数据, 1=TX FIFO和传输移位寄存器为空. 写TXFIFO时清除 */
#define NS16550_LSR_TFE         bit(5)          /* 1=传输FIFO 为空 */
#define NS16550_LSR_BI          bit(4)          /* 打断中断. 0=没有中断 */
#define NS16550_LSR_FE          bit(3)          /* 帧错误 */
#define NS16550_LSR_PE          bit(2)          /* 奇偶校验位错误 */
#define NS16550_LSR_OE          bit(1)          /* 数据溢出 */
#define NS16550_LSR_DR          bit(0)          /* 接收数据有效. 0=RXFIFO无数据, 1=RXFIFO有数据 */

/**********************************************************************************************
 * I2C 控制器
 **********************************************************************************************/

typedef struct
{
    volatile unsigned int cr1;                  // 0x00 控制寄存器
    volatile unsigned int cr2;                  // 0x04
    volatile unsigned int saddr;                // 0x08 从地址寄存器
    volatile unsigned int rsv;
    volatile unsigned int data;                 // 0x10 数据寄存器
    volatile unsigned int sr1;                  // 0x14 状态寄存器
    volatile unsigned int sr2;                  // 0x18
    volatile unsigned int clk;                  // 0x1c 时钟控制寄存器
    volatile unsigned int rise;                 // 0x20 上升时间寄存器
} HW_I2C_t;

/**
 * I2C控制寄存器1 - CR1
 */
#define I2C_CR1_SWRST           bit(15)         // 软复位
#define I2C_CR1_RECOVER         bit(14)         // 总线恢复命令
#define I2C_CR1_POS             bit(11)         // 数据接收的应答位置
#define I2C_CR1_ACK             bit(10)         // 应答使能
#define I2C_CR1_STOP            bit(9)          // 停止生成命令
#define I2C_CR1_START           bit(8)          // 开始生成命令
#define I2C_CR1_NOSTRETCH       bit(7)          // 时钟伸展无效(从模式)
#define I2C_CR1_ENGC            bit(6)          // 广播呼叫使能
#define I2C_CR1_PE              bit(0)          // 控制器使能

/**
 * I2C控制寄存器2 - CR2
 */
#define I2C_CR2_DMAEN           bit(11)         // DMA请求使能
#define I2C_CR2_ITBUFEN         bit(10)         // 缓冲类中断使能
#define I2C_CR2_ITEVTEN         bit(9)          // 事件类中断使能
#define I2C_CR2_ITERREN         bit(8)          // 错误类中断使能
#define I2C_CR2_FREQ_MASK       0x3F            // bit[5:0] 设备时钟频率
                                                // 外部8M时配置为8; 内部32M时配置为11
/**
 * I2C从地址寄存器 - OAR
 */
#define I2C_SADDR_MASK          0xFE            // bit[7:1] 从设备地址

/**
 * I2C数据寄存器 - DR (字节)
 */

/**
 * I2C状态寄存器1 - SR1
 */
#define I2C_SR1_OVR             bit(11)         // 溢出状态位
#define I2C_SR1_AF              bit(10)         // 应答失败状态位
#define I2C_SR1_ARLO            bit(9)          // 仲裁丢失状态位
#define I2C_SR1_BERR            bit(8)          // 总线错误状态位
#define I2C_SR1_TXE             bit(7)          // 发送数据寄存器空
#define I2C_SR1_RXNE            bit(6)          // 接收数据寄存器非空
#define I2C_SR1_STOPF           bit(4)          // 从模式停止位检测
#define I2C_SR1_BTF             bit(2)          // 字节传输结束状态
#define I2C_SR1_ADDR            bit(1)          // 地址阶段成功
#define I2C_SR1_SB              bit(0)          // 开始条件成功

/**
 * I2C状态寄存器2 - SR2
 */
#define I2C_SR2_GENCALL         bit(4)          // 广播地址命中
#define I2C_SR2_TRA             bit(2)          // 1=发送模式; 0=接收模式
#define I2C_SR2_BUSY            bit(1)          // 总线忙
#define I2C_SR2_MSL             bit(0)          // 主从模式. 1=主模式; 0=从模式

/**
 * I2C时钟控制寄存器 - CCR
 */
#define I2C_CCR_F_S             bit(15)         // 主模式选择. 0=标准模式; 1=快速模式
#define I2C_CCR_DUTY            bit(14)         // 占空比控制. 0=快速模式SCL高低电平时间为 1:2
                                                //             1=快速模式SCL高低电平时间为 9:16
#define I2C_CCR_MASK            0xFFF           // bit[11:0] 时钟分频控制, 定义SCL的高电平时间和低电平时间

//-------------------------------------------------------------------------------------------------
//
// 标准模式时:              Thigh=CCR_CCR×tPCLK, Tlow=CCR_CCR×tPCLK
//
// 快速模式且CCR_DUTY=0时:  Thigh=CCR_CCR×tPCLK, Tlow=2×CCR_CCR×tPCLK
//
// 快速模式且CCR_DUTY=1时:  Thigh=9×CCR_CCR×tPCLK, Tlow=16×CCR_CCR×tPCLK
//
// 例如要配置标准模式 100KHz 时钟, APB时钟为 8MHz 时: CCR_CCR=5000ns/125ns=40
//
//-------------------------------------------------------------------------------------------------

/**
 * I2C上升时间寄存器 - TRISE
 */
#define I2C_TRISE_MAKS          0x3F            // SCL最大上升时间(主模式)

/**********************************************************************************************
 * INT 控制器 - HW-IP3
 **********************************************************************************************/

#define INTC_EN_BASE    LS1C103_INTC_BASE       // 中断使能寄存器
#define INTC_SR_BASE    LS1C103_INTC_BASE+8     // 中断状态寄存器

#define INTC_DMA3_BIT           bit(9)          // DMA中断使能位
#define INTC_DMA2_BIT           bit(8)
#define INTC_DMA1_BIT           bit(7)
#define INTC_DMA0_BIT           bit(6)
#define INTC_SPI_BIT            bit(5)          // SPI中断使能位
#define INTC_FLASH_BIT          bit(4)          // Flash中断使能位
#define INTC_UART0_BIT          bit(3)          // UART0中断使能位
#define INTC_UART1_BIT          bit(2)          // UART1中断使能位
#define INTC_I2C_BIT            bit(1)          // I2C中断使能位
#define INTC_BTIM_BIT           bit(0)          // BTIM中断使能位

/**********************************************************************************************
 * 电源及顶层控制
 **********************************************************************************************/

typedef struct
{
    volatile unsigned int ChipCtrl;             // 0x00 全局配置
    volatile unsigned int CmdSts;               // 0x04 命令与状态
    volatile unsigned int Count;                // 0x08 时间计数器
    volatile unsigned int Compare;              // 0x0c 唤醒时间配置
    volatile unsigned int RstCtrl;              // 0x10 复位控制寄存器
    volatile unsigned int ClkCtrl;              // 0x14 时钟控制寄存器
    volatile unsigned int SrProt;               // 0x18 运行状态及保护寄存器
    volatile unsigned int rsv1[5];
    volatile unsigned int WdtCfg;               // 0x30 看门狗配置
    volatile unsigned int WdtFeed;              // 0x34 看门狗重置
    volatile unsigned int PowerCfg;             // 0x38 电源配置
    volatile unsigned int CommandW;             // 0x3c 状态清除寄存器
    volatile unsigned int rsv2[8];
    volatile unsigned int UserDat0;             // 0x60 用户数据0
    volatile unsigned int UserDat1;             // 0x64 用户数据1
    volatile unsigned int UserDat2;             // 0x68 用户数据2
    volatile unsigned int UserDat3;             // 0x6c 用户数据3
} HW_PMU_t;

/**
 * 芯片全局配置 - ChipCtrl
 */
#define CHIPCTRL_TURBOEN        bit(27)         // CPU加力模式. 1=CPU在RAM中执行指令时, 频率提高至32MHZ
#define CHIPCTRL_SPI_START      bit(26)         // SPI启动速率选择. 0=256us; 1=8us
#define CHIPCTRL_IRAM_AS_DRAM   bit(25)         // 1=iram 复用为 dram
#define CHIPCTRL_BTIM_STOP      bit(14)         // 基础定时器计数停止
#define CHIPCTRL_GTIM_STOP      bit(13)         // 通用定时器计数停止
#define CHIPCTRL_ATIM_STOP      bit(12)         // 高级定时器计数停止
#define CHIPCTRL_FASTEN         bit(11)         // 时钟提速使能. 0=4分频(8MHZ); 1=3分频(11MHZ)
#define CHIPCTRL_INPUT_HOLD     bit(10)         // 输入保持
#define CHIPCTRL_CLKUP_DLY_MASK 0x300           // bit[9:8] 高速晶振开启到可以使用的延迟
#define CHIPCTRL_CLKUP_DLY_5140 0               // 0: 5.14ms
#define CHIPCTRL_CLKUP_DLY_480  0x100           // 1: 480us
#define CHIPCTRL_CLKUP_DLY_1460 0x200           // 2: 1.46ms
#define CHIPCTRL_CLKUP_DLY_2440 0x300           // 3: 2.44ms
#define CHIPCTRL_8M_SEL         bit(7)          // 8M时钟选择. 0=内部时钟; 1=外部时钟
#define CHIPCTRL_8M_EN          bit(6)          // 高速晶体振荡器使能. 仅支持外部有源晶振
#define CHIPCTRL_32K_SEL        bit(5)          // 32K时钟选择. 0=内部时钟; 1=外部时钟
#define CHIPCTRL_32K_SPEED      bit(4)          // 内部32K OSC速度. 0=32K; 1=1K
#define CHIPCTRL_32K_TRIM_MASK  0xF             // bit[3:0] 内部32K OSC Trimming值

/**
 * 命令与状态 - CmdSts
 */
#define CMDSR_8M_FAIL          bit(31)          // 8M外部时钟失效. 1=失效
#define CMDSR_8M_SEL           bit(30)          // 8M时钟选择. 1=外部时钟
#define CMDSR_32K_FAIL         bit(29)          // 32K外部时钟失效. 1=失效
#define CMDSR_32K_SEL          bit(28)          // 32K时钟选择. 1=外部时钟
#define CMDSR_RSTSRC_MASK      (3<<26)          // bit[27:26] 复位来源
#define CMDSR_RSTSRC_OUTER     0                // 外部复位
#define CMDSR_RSTSRC_WD1       (1<<26)          // 看门狗复位, 01/10每次复位时切换
#define CMDSR_RSTSRC_WD2       (2<<26)
#define CMDSR_RSTSRC_WAKE      (3<<26)          // 休眠唤醒
#define CMDSR_EXTINT_EN        bit(25)          // 外部中断使能
#define CMDSR_INTSRC_MASK      0x1FF0000        // bit[24:16] 中断状态, 往CommandW对应位写1可清除中断状态
#define CMDSR_INTSRC_EXTINT    0x1000000        // [8]: e_ExtInt
#define CMDSR_INTSRC_RTC       0x0100000        // [4]: e_RTC
#define CMDSR_INTSRC_8MFAIL    0x0080000        // [3]: e_C8MFail
#define CMDSR_INTSRC_32KFAIL   0x0040000        // [2]: e_C32KFail
#define CMDSR_INTSRC_BATFAIL   0x0020000        // [1]: e_BatFail
#define CMDSR_INTSRC_WAKE      0x0010000        // [0]: e_Wake
#define CMDSR_INTEN_MASK       0xFF00           // bit[15:8] 中断使能, 每一位对应一个中断源
#define CMDSR_INTEN_RTC        0x1000           // [4]: e_RTC
#define CMDSR_INTEN_8MFAIL     0x0800           // [3]: e_C8MFail
#define CMDSR_INTEN_32KFAIL    0x0400           // [2]: e_C32KFail
#define CMDSR_INTEN_BATFAIL    0x0200           // [1]: e_BatFail
#define CMDSR_INTEN_WAKE       0x0100           // [0]: e_Wake
#define CMDSR_WAKE_EN          bit(7)           // 定时唤醒使能
#define CMDSR_SLEEP_EN         bit(0)           // 进入休眠状态

/**
 * 时间计数器 - Count
 */
#define RTC_COUNT_MASK          0xFFFFF         // bit[19:0] RTC 时间计数器, 每1/256秒加1

/**
 * 唤醒时间配置 - Compare
 */
#define RTC_WAKECMP_MASK        0xFFFFF         // bit[19:0] WakeCmp 唤醒时间配置.
                                                // 当该值与Count相等且WakeEn=1时产生唤醒事件

/**
 * 软件复位 - RstCtrl
 */
#define RSTCTRL_GPIOB           bit(18)         // gpiob复位
#define RSTCTRL_GPIOA           bit(17)         // gpioa复位
#define RSTCTRL_AFIO            bit(16)         // afio复位
#define RSTCTRL_ADC             bit(10)         // adc复位
#define RSTCTRL_CRC             bit(9)          // crc复位
#define RSTCTRL_ATIM            bit(8)          // atim复位
#define RSTCTRL_GTIM            bit(7)          // gtim复位
#define RSTCTRL_BTIM            bit(6)          // btim复位
#define RSTCTRL_I2C             bit(5)          // i2c复位
#define RSTCTRL_UART1           bit(4)          // uart1复位
#define RSTCTRL_UART0           bit(3)          // uart0复位
#define RSTCTRL_DMA             bit(2)          // dma复位
#define RSTCTRL_SPI             bit(1)          // spi复位
#define RSTCTRL_FLASH           bit(0)          // flash复位

/**
 * 时钟关断 - ClkCtrl
 */
#define CLKCTRL_GPIOB           bit(18)         // gpiob时钟
#define CLKCTRL_GPIOA           bit(17)         // gpioa时钟
#define CLKCTRL_AFIO            bit(16)         // afio时钟
#define CLKCTRL_ADC             bit(10)         // adc时钟
#define CLKCTRL_CRC             bit(9)          // crc时钟
#define CLKCTRL_ATIM            bit(8)          // atim时钟
#define CLKCTRL_GTIM            bit(7)          // gtim时钟
#define CLKCTRL_BTIM            bit(6)          // btim时钟
#define CLKCTRL_I2C             bit(5)          // i2c时钟
#define CLKCTRL_UART1           bit(4)          // uart1时钟
#define CLKCTRL_UART0           bit(3)          // uart0时钟
#define CLKCTRL_DMA             bit(2)          // dma时钟
#define CLKCTRL_SPI             bit(1)          // spi时钟
#define CLKCTRL_FLASH           bit(0)          // flash时钟

/**
 * 运行状态及保护寄存器 - SrProt
 */
#define SRPROT_ADDR_CHECK_EN    bit(7)          // 非法地址检查中断使能. 1=进行地址检查, 当软件操作未定义的地址时
                                                // 触发NMI中断. 向该寄存器连续写入0x00,0x5A,0xA5打开该位的写使能.
#define SRPROT_ISP_FLAG         bit(6)          // ISP标志位
#define SRPROT_JTAG_LOCK        bit(5)          // JTAG锁定
#define SRPROT_OTP_LOCK         bit(4)          // OTP锁定
#define SRPROT_INS_MODE1        bit(3)          // 安装模式1
#define SRPROT_JTAG_FUNC        bit(2)          // JTAG复用
#define SRPROT_INS_MODE0        bit(1)          // 安装模式0
#define SRPROT_BOOT_SPI         bit(0)          // SPI启动

/**
 * 看门狗配置寄存器 - WdtCfg
 *
 * wdtcfg_hi = ~wdtcfg_lo
 *
 */
#define WDT_HI_MASK             0xFFFF0000      // 看门狗配置高位
#define WDT_LO_MASK             0x0000FFFF      // 看门狗配置低位

/**
 * 看门狗重置寄存器 - WdtFeed
 *
 * 写入 0xa55a55aa 喂狗
 *
 */
#define WDTFEED_FOOD            0xa55a55aa

/**
 * 电源配置 - PowerCfg
 */
#define POWERCFG_TCTRIM_MASK    (0x7<<29)       // bit[31:29] 电压调节参数, 硬件自动配置, 不建议更改
#define POWERCFG_ABSTRIM_MASK   (0x1F<<24)      // bit[28:24] 电压调节参数, 硬件自动配置, 不建议更改

/**
 * 命令写端口 - CommandW
 */
#define CMDW_INTCLR_MASK        0x1F0000        // bit[20:16] 中断状态清除
#define CMDW_INTCLR_RTC         bit(20)         // [4]: e_RTC
#define CMDW_INTCLR_8MFAIL      bit(19)         // [3]: e_C8MFail
#define CMDW_INTCLR_32KFAIL     bit(18)         // [2]: e_C32KFail
#define CMDW_INTCLR_BATFAIL     bit(17)         // [1]: e_BatFail
#define CMDW_INTCLR_WAKE        bit(16)         // [0]: e_Wake
#define CMDW_SLEEP              bit(0)          // 进入休眠状态, SLeepEn=1

/**********************************************************************************************
 * 实时时钟
 **********************************************************************************************/

typedef struct
{
	volatile unsigned int freq;                 // 0x00 分频值寄存器
	volatile unsigned int cfg;                  // 0x04 配置寄存器
	volatile unsigned int rtc0;                 // 0x08  时间值寄存器0
	volatile unsigned int rtc1;                 // 0x0C 时间值寄存器1
} HW_RTC_t;

/*
 * FREQ 分频值寄存器, freqscale = freq_in/16
 */
#define RTC_FREQ_I_MASK         0x0FFF0000      /* bit[27:16], 分频系数整数部分 */
#define RTC_FREQ_I_SHIFT        16
#define RTC_FREQ_F_MASK         0x0000FFC0      /* bit[15:6], 分频系数小数部分 */
#define RTC_FREQ_F_SHIFT        6

/*
 * CFG 配置寄存器
 */
#define RTC_CFG_STATE           bit(31)         /* 操作进行状态. 1=读写操作执行中, 写1清零, 用于硬件调试 */
#define RTC_CFG_TIMER_EN        bit(30)         /* 定时器使能. 1=使能, 时间到后自动清零 */
#define RTC_CFG_MONTH_MASK      0x3C000000      /* bit[29:26] 月 */
#define RTC_CFG_MONTH_SHIFT     26
#define RTC_CFG_DAY_MASK        0x03E00000      /* bit[25:21] 日 */
#define RTC_CFG_DAY_SHIFT       21
#define RTC_CFG_HOUR_MASK       0x001F0000      /* bit[20:16] 时 */
#define RTC_CFG_HOUR_SHIFT      16
#define RTC_CFG_MINUTE_MASK     0x0000FC00      /* bit[15:10] 分 */
#define RTC_CFG_MINUTE_SHIFT    10
#define RTC_CFG_SECOND_MASK     0x000003F0      /* bit[9:4] 秒 */
#define RTC_CFG_SECOND_SHIFT    4
#define RTC_CFG_SIXTEENTH_MASK  0x0000000F      /* bit[3:0] 十六分之一秒 */
#define RTC_CFG_SIXTEENTH_SHIFT 0

/*
 * RTC0 时间值寄存器0
 */
#define RTC0_BAD_TIME           bit(31)         /* 无效数值 */
#define RTC0_HOUR_MASK          0x001F0000      /* bit[20:16] 时 */
#define RTC0_HOUR_SHIFT         16
#define RTC0_MINUTE_MASK        0x0000FC00      /* bit[15:10] 分 */
#define RTC0_MINUTE_SHIFT       10
#define RTC0_SECOND_MASK        0x000003F0      /* bit[9:4] 秒 */
#define RTC0_SECOND_SHIFT       4
#define RTC0_SIXTEENTH_MASK     0x0000000F      /* bit[3:0] 十六分之一秒 */
#define RTC0_SIXTEENTH_SHIFT    0

/*
 * RTC1 时间值寄存器1
 */
#define RTC1_BAD_TIME           bit(31)          /* 无效数值 */
#define RTC1_YEAR_MASK          0x0000FE00       /* bit[15:9] 年, 从2000年起 */
#define RTC1_YEAR_SHIFT         9
#define RTC1_MONTH_MASK         0x000001E0       /* bit[8:5] 月 */
#define RTC1_MONTH_SHIFT        5
#define RTC1_DAY_MASK           0x0000001F       /* bit[4:0] 日 */
#define RTC1_DAY_SHIFT          0

/**********************************************************************************************
 * AFIO 管脚复用控制器
 **********************************************************************************************/

#define AFIO_SEL0_ADDR          (LS1C103_AFIO_BASE+0x0)
#define AFIO_SEL1_ADDR          (LS1C103_AFIO_BASE+0x4)

#define IOSEL_GPIO              0
#define IOSEL_MAIN              1
#define IOSEL_MUX1              2
#define IOSEL_MUX2              3

/**
 * 将 GPIOn 设置为复用 v
 */
static inline int ls1c103_iosel(int gpioNum, unsigned mux)
{
    if ((gpioNum >= 0) && (gpioNum <= 32))  // 28
    {
        register unsigned int regVal, regAddr = AFIO_SEL0_ADDR;

        if (gpioNum >= 16)
        {
            gpioNum -= 16;
            regAddr += 4;   // AFIO_SEL1_ADDR;
        }

        gpioNum *= 2;
        regVal  = READ_REG32(regAddr);
        regVal &= ~(0x3 << gpioNum);
        regVal |= (mux & 0x3) << gpioNum;
        WRITE_REG32(regAddr, regVal);

        return 0;
    }
    
    return -1;
}

/**********************************************************************************************
 * EXTI 外部中断控制器, GPIO0~GPIO28
 **********************************************************************************************/

#define EXTI_EN_ADDR            (LS1C103_EXTI_BASE+0x00)
#define EXTI_POL_ADDR           (LS1C103_EXTI_BASE+0x04)
#define EXTI_EDGE_ADDR          (LS1C103_EXTI_BASE+0x08)
#define EXTI_SRC_ADDR           (LS1C103_EXTI_BASE+0x0C)
#define EXTI_CLR_ADDR           EXTI_SRC_ADDR

#define EXTI_ENABLE(gpioNum)    OR_REG32(EXTI_EN_ADDR, (1<<(gpioNum)))
#define EXTI_DISABLE(gpioNum)   AND_REG32(EXTI_EN_ADDR, ~(1<<(gpioNum)))

#define EXTI_POL_FALL(gpioNum)  OR_REG32(EXTI_POL_ADDR, (1<<(gpioNum)))
#define EXTI_POL_RISE(gpioNum)  AND_REG32(EXTI_POL_ADDR, ~(1<<(gpioNum)))

#define EXTI_EDGE(gpioNum)      OR_REG32(EXTI_EDGE_ADDR, (1<<(gpioNum())
#define EXTI_LEVEL(gpioNum)     AND_REG32(EXTI_EDGE_ADDR, ~(1<<(gpioNum)))

#define EXTI_SRC(gpioNum)       READ_REG32(EXTI_SRC_ADDR) // & (1<<(gpioNum))
#define EXTI_CLR(gpioNum)       OR_REG32(EXTI_SRC_ADDR, (1<<(gpioNum)))

/**********************************************************************************************
 * GPIO 控制器, GPIO0~GPIO28
 **********************************************************************************************/

/**
 * GPIO 端口配置
 */
#define IN_ANALOG       0x0             // 模拟输入模式
#define IN_FLOAT        0x1             // 浮空输入模式
#define IN_PULL         0x2             // 上拉/下拉输入模式
#define OUT_PULL        0x2             // 上拉/下拉输出模式

/**
 * GPIO 端口模式
 */
#define IO_IN           0x0             // 输入模式
#define IO_OUT          0x1             // 输出模式

typedef struct
{
    volatile unsigned int CRL;          // 0x00 GPIO 端口配置低寄存器
    volatile unsigned int CRH;          // 0x04 GPIO 端口配置高寄存器
    volatile unsigned int IDR;          // 0x08 GPIO 端口输入数据寄存器
    volatile unsigned int ODR;          // 0x0c GPIO 端口输出数据寄存器
    volatile unsigned int BSRR;         // 0x10 GPIO 端口设置清除寄存器
    volatile unsigned int BRR;          // 0x14 GPIO 端口位清除寄存器
    volatile unsigned int LCKR;         // 0x18 GPIO 端口配置锁定寄存器
} HW_GPIO_t;

//---------------------------------------------------------------------
// GPIO 位操作
//---------------------------------------------------------------------

/**
 * GPIO位访问配置端口 - GPIOBit_CFG
 */
static inline int ls1c103_gpio_cfg(int gpioNum, unsigned cfg, unsigned mode)
{
    if ((gpioNum >= 0) && (gpioNum <= 32))  // 28
    {
        register unsigned int regAddr = LS1C103_GPIOA_BASE;
        
        if (gpioNum >= 16)
        {
            gpioNum -= 16;
            regAddr += 0x100;   // LS1C103_GPIOB_BASE
        }
        
        regAddr += 0x80 + gpioNum;
        WRITE_REG8(regAddr, ((cfg & 0x3) << 2) + (mode & 0x3));
        
        return 0;
    }
    
    return -1;
}

/**
 * GPIO位访问输入端口 - GPIOBit_IDR
 */
static inline int ls1c103_gpio_in(int gpioNum)
{
    if ((gpioNum >= 0) && (gpioNum <= 32))  // 28
    {
        register unsigned int regAddr = LS1C103_GPIOA_BASE;

        if (gpioNum >= 16)
        {
            gpioNum -= 16;
            regAddr += 0x100;   // LS1C103_GPIOB_BASE
        }

        regAddr += 0x90 + gpioNum;
        
        return READ_REG8(regAddr) & 0x1;
    }

    return -1;
}

/**
 * GPIO位访问输出端口 - GPIOBit_ODR
 */
static inline int ls1c103_gpio_out(int gpioNum, unsigned outVal)
{
    if ((gpioNum >= 0) && (gpioNum <= 32))  // 28
    {
        register unsigned int regAddr = LS1C103_GPIOA_BASE;

        if (gpioNum >= 16)
        {
            gpioNum -= 16;
            regAddr += 0x100;   // LS1C103_GPIOB_BASE
        }

        regAddr += 0xA0 + gpioNum;
        WRITE_REG8(regAddr, outVal & 0x1);

        return 0;
    }

    return -1;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LS1C103_H_ */


