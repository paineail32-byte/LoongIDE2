/*
 * Copyright (C) 2021-2023 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 *
 */

#ifndef LS1C102_H_
#define LS1C102_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define bit(x)      (1<<x)

#define SECTION(x)  __attribute__((section(x)))     // 指定段位置

//-------------------------------------------------------------------------------------------------
//  寄存器 Read/Write 操作
//-------------------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------------------
// 地址空间分布
//-------------------------------------------------------------------------------------------------

#define LS1C102_IRAM_ADDR          0x80000000      // - 只能用于取指
#define LS1C102_IRAM_SIZE          0x1000          // - IRAM 大小 = 4K

#define LS1C102_DRAM_ADDR          0x80001000      // - 只能用于取指
#define LS1C102_DRAM_SIZE          0x1000          // - IRAM 大小 = 4K

#define LS1C102_SPIFLASH_ADDR      0xBE000000      // SPI Flash 存储区
#define LS1C102_FLASH_ADDR         0xBF000000      // On-chip Flash 存储区

#define LS1C102_BOOT_ADDR          0x1C000000      // Boot from SPI Flash or On-chip Flash

#define LS1C102_FLASH_BASE         0xBFE60000      // Flash 控制寄存器基地址
#define LS1C102_SPI_BASE           0xBFE70000      // SPI 控制寄存器基地址
#define LS1C102_UART0_BASE         0xBFE80000      // UART0 基地址
#define LS1C102_UART1_BASE         0xBFE88000      // UART1 基地址
#define LS1C102_UART2_BASE         0xBFE8C000      // UART2 基地址
#define LS1C102_I2C_BASE           0xBFE90000      // I2C 基地址

#define LS1C102_INTC_BASE          0xBFEA0000      // 中断控制寄存器基地址
#define LS1C102_PMU_BASE           0xBFEB0000      // 电源管理单元寄存器基地址
#define LS1C102_TSENSOR_BASE       0xBFEB4000      // 触摸传感器寄存器基地址
#define LS1C102_RTC_BASE           0xBFEB8000      // 实时时钟寄存器基地址
#define LS1C102_DMA_BASE           0xBFEC0000      // DMA 寄存器基地址
#define LS1C102_VPWM_BASE          0xBFEC0020      // VPWM 寄存器基地址
#define LS1C102_TIMER_BASE         0xBFED0000      // 定时器寄存器基地址

//-------------------------------------------------------------------------------------------------
// 电源管理单元
//-------------------------------------------------------------------------------------------------

typedef struct 
{
    volatile unsigned int ChipCtrl;             // 0x00 全局配置
    volatile unsigned int CmdSts;               // 0x04 命令与状态
    volatile unsigned int Count;                // 0x08 时间计数器
    volatile unsigned int Compare;              // 0x0c 唤醒时间配置
    volatile unsigned int IOSEL0;               // 0x10 IO复用选择 0
    volatile unsigned int IOSEL1;               // 0x14 IO复用选择 1
    volatile unsigned int IOSEL2;               // 0x18 IO复用选择 2
    volatile unsigned int IOSEL3;               // 0x1c IO复用选择 3
    volatile unsigned int ExIntEn;              // 0x20 外部中断使能
    volatile unsigned int ExIntPol;             // 0x24 外部中断极性
    volatile unsigned int ExIntEdge;            // 0x28 外部中断边沿
    volatile unsigned int ExIntSrc;             // 0x2c 外部中断状态
    volatile unsigned int WdtCfg;               // 0x30 看门狗配置
    volatile unsigned int WdtFeed;              // 0x34 喂狗
    volatile unsigned int PowerCfg;             // 0x38 电源配置
    volatile unsigned int CommandW;             // 0x3C XXX datasheet上没有描述这个寄存器, == CmdSts?
    volatile unsigned int GPIOA_OE;             // 0x40 GPIOA 输出使能
    volatile unsigned int GPIOA_O;              // 0x44 GPIOA 输出
    volatile unsigned int GPIOA_I;              // 0x48 GPIOA 输入
    volatile unsigned int rsv1;                 // 0x4C
    volatile unsigned int GPIOB_OE;             // 0x50 GPIOB 输出使能
    volatile unsigned int GPIOB_O;              // 0x54 GPIOB 输出
    volatile unsigned int GPIOB_I;              // 0x58 GPIOB 输入
    volatile unsigned int rsv2;                 // 0x5C
    volatile unsigned int Pulse0;               // 0x60 脉冲输出配置 0
    volatile unsigned int Pulse1;               // 0x64 脉冲输出配置 1
    volatile unsigned int UserDat;              // 0x68 用户数据
    volatile unsigned int AdcCtrl;              // 0x6c ADC 控制
    volatile unsigned int AdcDat;               // 0x70 ADC 数据
    volatile unsigned int rsv3[3];              // 0x74/0x78/0x7C
    volatile unsigned char GPIOBit[0x40];       // 0x80~0xbf GPIO 位访问
} HW_PMU_t;

/*
 * ChipCtrl 芯片全局配置
 */
#define CHIPCTRL_SOFTFLAG_MASK          (0xF<<28)           /* 读写软件标志位 */
#define CHIPCTRL_SOFTFLAG_SHIFT         28
#define CHIPCTRL_TURBOEN                bit(27)             /* 1=当CPU在RAM中执行指令时, 频率提高至32MHZ */
#define CHIPCTRL_SPI_START              bit(26)             /* SPI 启动速率选择: 1=8us; 0=256us */
#define CHIPCTRL_BATDET_SEL_MASK        (0x3<<24)           /* 掉电信号检测选择 */
#define CHIPCTRL_BATDET_SEL_SHIFT       24
#define CHIPCTRL_BATDET_SEL_ADCI0       (0<<24)
#define CHIPCTRL_BATDET_SEL_ADCI1       (1<<24)
#define CHIPCTRL_BATDET_SEL_GPIO00      (2<<24)
#define CHIPCTRL_BATDET_SEL_GPIO01      (3<<24)
#define CHIPCTRL_ADCI7_EN               bit(23)             /* ADC_I[7] 模拟输入使能 */
#define CHIPCTRL_ADCI6_EN               bit(22)             /* =0: 数字输入, 可复用为GPIO */
#define CHIPCTRL_ADCI5_EN               bit(21)             /* =1: 模拟输入 */
#define CHIPCTRL_ADCI4_EN               bit(20)
#define CHIPCTRL_ADCIO_PU               bit(19)             /* ADC_IO 400K上拉, 0=关闭, 1=打开 */
#define CHIPCTRL_ADCIO_PD               bit(18)             /* ADC_IO 400K下拉, 0=关闭, 1=打开 */
#define CHIPCTRL_ADCIO_IEN              bit(17)             /* ADC_IO 数字输入使能, 0=关闭,引脚上模拟信号, 1=打开,引脚上数字信号 */
#define CHIPCTRL_ADC_POWERON            bit(16)             /* ADC 电源, 0=自动打开, 1=常开 */
#define CHIPCTRL_DRAM_POWERDOWN         bit(15)             /* DRAM 电源, 0=常开,休眠时有数据需要保存, 1=与CPU同时上下电 */
#define CHIPCTRL_UART2_OFF              bit(14)             /* 1=关断 */
#define CHIPCTRL_RTC_OFF                bit(13)             /* 1=关断 */
#define CHIPCTRL_TSENSOR_OFF            bit(12)             /* 1=关断 */
#define CHIPCTRL_FASTEN                 bit(11)             /* 1=3分频; 0=4分频 */
#define CHIPCTRL_INPUT_HOLD             bit(10)             /* GPIO输入保持 */
#define CHIPCTRL_CLKUP_DELAY_MASK       (0x3<<8)            /* 高速晶振开启到可以使用的延迟 */
#define CHIPCTRL_CLKUP_DELAY_SHIFT      8
#define CHIPCTRL_CLKUP_DELAY_5140       (0<<8)
#define CHIPCTRL_CLKUP_DELAY_480        (1<<8)
#define CHIPCTRL_CLKUP_DELAY_1460       (2<<8)
#define CHIPCTRL_CLKUP_DELAY_2440       (3<<8)
#define CHIPCTRL_8M_SEL                 bit(7)              /* 8M 时钟选择, 0=内部, 1=外部 */
#define CHIPCTRL_8M_EN                  bit(6)              /* 高速晶体振荡器使能 */
#define CHIPCTRL_32K_SEL                bit(5)              /* 32K 时钟选择, 0=内部, 1=外部 */
#define CHIPCTRL_32K_SPEED              bit(4)              /* 内部 32K OSC 速度, 1: 1K, 0: 32K */
#define CHIPCTRL_32K_TRIM_MASK          0xF                 /* 内部32K OSC Trimming 值 */

/*
 * CmdSts 命令与状态
 */
#define CMDSR_8M_FAIL                   bit(31)             /* RO 8M 外部时钟失效, 1=失效 */
#define CMDSR_8M_SEL                    bit(30)             /* RO 8M 时钟选择, 1=外部时钟 */
#define CMDSR_32K_FAIL                  bit(29)             /* RO 32K 外部时钟失效, 1=失效 */
#define CMDSR_32K_SEL                   bit(28)             /* RO 32K 时钟选择, 1=外部时钟 */
#define CMDSR_RSTSRC_MASK               (0x3<<26)           /* RO 复位来源 */
#define CMDSR_RSTSRC_SHIFT              26
#define CMDSR_RSTSRC_OUT                (0<<26)             /* 外部复位 */
#define CMDSR_RSTSRC_DOG0               (1<<26)             /* 看门狗复位 */
#define CMDSR_RSTSRC_DOG1               (2<<26)             /* 看门狗复位 */
#define CMDSR_RSTSRC_WAKE               (3<<26)             /* 休眠唤醒 */
#define CMDSR_EXINTEN                   bit(25)             /* 外部中断使能, 1=有效 */
#define CMDSR_INTSRC_MASK               (0x1FF<<16)         /* bit[24:16] RO 中断状态 */
#define CMDSR_INTSRC_SHIFT              16                  /* XXX 发生中断后对应位=1, 往CommandW寄存器对应位写1清除中断 */
#define CMDSR_INTSRC_EXTINT             bit(24)
#define CMDSR_INTSRC_ADC                bit(23)
#define CMDSR_INTSRC_RTC                bit(22)
#define CMDSR_INTSRC_8MFAIL             bit(21)
#define CMDSR_INTSRC_32KFAIL            bit(20)
#define CMDSR_INTSRC_BATFAIL            bit(19)
#define CMDSR_INTSRC_UART2              bit(18)             /* ring */
#define CMDSR_INTSRC_TOUCH              bit(17)
#define CMDSR_INTSRC_WAKE               bit(16)
#define CMDSR_INTEN_MASK                (0xFF<<8)           /* bit[15:8] 中断使能, 每一位对应一个中断源*/
#define CMDSR_INTEN_SHIFT               8
#define CMDSR_INTEN_ADC                 bit(15)
#define CMDSR_INTEN_RTC                 bit(14)
#define CMDSR_INTEN_8MFAIL              bit(13)
#define CMDSR_INTEN_32KFAIL             bit(12)
#define CMDSR_INTEN_BATFAIL             bit(11)
#define CMDSR_INTEN_UART2               bit(10)
#define CMDSR_INTEN_TOUCH               bit(9)
#define CMDSR_INTEN_WAKE                bit(8)

#define CMDSR_WAKE_EN                   bit(7)              /* 定时唤醒使能, 0=关闭, 1=打开 */
#define CMDSR_SLEEP_EN                  bit(0)              /* RO 进入休眠状态. 读出值为1表示可休眠, 往CommandW[0]写1则关闭处理器系统 */

/*
 * Count 时间计数器
 */
#define COUNT_MASK                      0x000FFFFF          /* bit[19:0] 每1/256秒 +1 */

/*
 * Compare 唤醒时间配置
 */
#define COMPARE_MASK                    0x000FFFFF          /* bit[19:0] 当该值与 Count 相等且 WakeEn 为 1时产生唤醒事件 */

/*
 * IOSEL0~3 GPIO 复用选择.
 * 共128位, 由低到高每 2 位控制一个 GPIO 的复位状态, 复位期间所有引脚配置为GPIO 输入.
 */
#define IOSEL_GPIO                      0
#define IOSEL_MAIN                      1
#define IOSEL_MUX1                      2
#define IOSEL_MUX2                      3

/*
 * ExIntEn 外部中断使能
 *
 * bit[0:7]   对应GPIO0~GPIO7
 * bit[8:15]  对应GPIO16~GPIO23
 * bit[16:23] 对应GPIO32~GPIO39
 * bit[24:31] 对应GPIO48~GPIO55
 */

/*
 * ExIntPol 外部中断极性
 *
 * 对应关系同上, 0=高电平/上升沿有效
 */

/*
 * ExIntEdge 外部中断边沿
 *
 * 对应关系同上, 0=电平模式, 1=边沿模式
 */

/*
 * ExIntSrc 外部中断状态
 *
 * 对应关系同上, 写1清除
 */

/*
 * WdtCfg 看门狗配置
 */
#define WDTCFG_HI_MASK                  0xFFFF0000          /* bit[31:16], 其值等于低16位取反 */
#define WDTCFG_HI_SHIFT                 16
#define WDTCFG_LO_MASK                  0x0000FFFF          /* bit[15:0], 看门狗复位等待时间, 以秒为单位. 最高位为奇校验位 */

/*
 * WdtFeed 喂狗
 */
#define WDTFEED_FOOD                    0xA55A55AA          /* 喂狗值 */

/*
 * PowerCfg 电源配置
 *
 * XXX 不建议修改
 */

/*
 * CommandW 命令写端口, 以下位对应CmdSrc
 *
 * bit[23:16]  IntSrc        WO 写1清除中断标志
 * bit[0]      Sleep/SleepEn 
 *
 *
 */

/*
 * GPIOA_OE/GPIOB_OE 输出使能
 *
 * 0=输入; 1=输出
 */

/*
 * GPIOA_O/GPIOB_O 输出
 *
 * 0=低电平, 1=高电平. 输入状态时写1, 表示该引脚需要输入保持.
 */

/*
 * GPIOA_I/GPIOB_I 输入
 *
 * 0=低电平, 1=高电平
 */

/*
 * Pulse0/Pulse1 脉冲输出配置
 *
 * XXX 改变时钟或分频系数时应该先关闭, 再打开.
 */
#define PULSE_ENABLE                    bit(17)             /* 脉冲输出使能. 0=关闭, 引脚为GPIO功能, 1=开启,输出占空比50%脉冲 */
#define PULSE_CLK_SEL                   bit(16)             /* 时钟源选择. 0=32K, 1=8M */
#define PULSE_MASK                      0x0000FFFF          /* 脉冲分频系数 */

/*
 * UserDat 用户数据
 *
 * XXX 复位不会清除的数据
 */

/*
 * AdcCtrl ADC 控制
 */
#define ADCCTRL_RUN                     bit(8)              /* 启动单次测量. 写1启动, 结束后自动清除; 产生ADC中断 */
#define ADCCTRL_DIV                     bit(4)              /* 时钟分频选择. 0=2分频, 1=4分频 */
#define ADCCTRL_SEL_MASK                0x07                /* 通道选择 */
#define ADCCTRL_SEL_ADCI0               0
#define ADCCTRL_SEL_ADCI1               1
#define ADCCTRL_SEL_VCORE               2
#define ADCCTRL_SEL_1V                  3
#define ADCCTRL_SEL_ADCI4               4
#define ADCCTRL_SEL_ADCI5               5
#define ADCCTRL_SEL_ADCI6               6
#define ADCCTRL_SEL_ADCI7               7

/*
 * AdcDat ADC 数据
 */
#define ADCDAT_MASK                     0x0FFF              /* 12位有效位 */

/*
 * GPIOBit[0x40] GPIO 位访问端口
 *
 * 每个字节对应一个端口
 */
#define GPIOBIT_OE                      bit(1)              /* GPIO方向. 0=输入, 1=输出 */
#define GPIOBIT_VAL                     bit(0)

//-------------------------------------------------------------------------------------------------
// 外部中断管理
//-------------------------------------------------------------------------------------------------

typedef struct 
{
    volatile unsigned char en;                  // 0x00 中断使能寄存器. 1=中断使能
    volatile unsigned char edge;                // 0x01 中断边沿寄存器. 1=边沿触发, 0=电平触发
    volatile unsigned char pol;                 // 0x02 中断极性寄存器. 1=高电平/上升沿触发
    volatile unsigned char clr;                 // 0x03 中断清除寄存器. 写1清除中断状态
    volatile unsigned char set;                 // 0x04 中断置位寄存器. 写1置中断触发模式的中断状态, XXX 用于测试
    volatile unsigned char out;                 // 0x05 中断输出寄存器. 1=中断触发
    volatile unsigned char srprot;              // 0x06 运行状态及保护寄存器
} HW_INTC_t;

/*
 * 中断对应位域
 */
#define INTC_DMA                bit(7)          /* DMA 中断位 */
#define INTC_VPWM               bit(6)          /* VPWM 中断位 */
#define INTC_SPI                bit(5)          /* SPI 中断位 */
#define INTC_FLASH              bit(4)          /* Flash 中断位 */
#define INTC_UART0              bit(3)          /* UART0 中断位 */
#define INTC_UART1              bit(2)          /* UART1 中断位 */
#define INTC_I2C                bit(1)          /* I2C 中断位 */
#define INTC_TIMER              bit(0)          /* 定时器中断位 */

/*
 * SRPROT 运行状态及保护寄存器
 */
#define SRPROT_ADDR_CHECK_EN    bit(7)          /* 地址检查使能. 1=进行地址检查, 当软件发出未定义的地址时触发NMI中断,
                                                 * 往此寄存器连续写入 0x00, 0x5A, 0xA5 打开此位写使能. */
#define SRPROT_JTAG_LOCK        bit(5)          /* RO JTAG 锁定, 1=JTAG被禁用 */
#define SRPROT_OPT_LOCK         bit(4)          /* RO OTP 锁定, 1=OPT区域被禁用 */
#define SRPROT_NEW_PKG          bit(3)          /* RO 新封装模式 */
#define SRPROT_JTAG_MUX         bit(2)          /* RO JTAG 复用. 1=复用为GPIO */
#define SRPROT_INSTALL_MODE     bit(1)          /* RO 安装模式. 1=安装模式 */
#define SRPROT_BOOT_SPI         bit(0)          /* RO SPI 启动. 1=当前从SPI启动 */

//-------------------------------------------------------------------------------------------------
// 片内 Flash
//-------------------------------------------------------------------------------------------------

#define FLASH_PAGE_COUNT        1024
#define FLASH_PAGE_SIZE         128
#define FLASH_BYTES             (1024*128)
#define FLASH_OPT_ADDR          0xBF020000

typedef struct 
{
    volatile unsigned int cmd;                  // 0x00 命令寄存器
    volatile unsigned int cah;                  // 0x04 加密地址上界寄存器
    volatile unsigned int cal;                  // 0x08 加密地址下界寄存器
    volatile unsigned int rsv;                  // 0x0C
    volatile unsigned int vrf;                  // 0x10 数据校验寄存器: 待校验数据
    volatile unsigned int sts;                  // 0x14 状态寄存器
    volatile unsigned int pet;                  // 0x18 擦写时间寄存器
} HW_FLASH_t;

/*
 * CMD 命令寄存器
 */
#define FLASH_CMD_CMD_MASK      (0xF<<28)       /* RW 命令 */
#define FLASH_CMD_CMD_SHIFT     28
#define FLASH_CMD_VERIFY        (1<<28)         /* 4’b0001 数据校验 */
#define FLASH_CMD_CLR_INT       (3<<28)         /* 4’b0011 清中断 */
#define FLASH_CMD_PAGE_LATCH    (4<<28)         /* 4’b0100 清page_latch */
#define FLASH_CMD_UPD_OPT       (9<<28)         /* 4’b1001 更新区域保护 */
#define FLASH_CMD_ERASE         (10<<28)        /* 4’b1010 擦除目标页 */
#define FLASH_CMD_SLEEP         (12<<28)        /* 4’b1100 进入休眠模式 */
#define FLASH_CMD_WRITE         (14<<28)        /* 4’b1110 编程目标页 */
#define FLASH_CMD_UPD_KEY       (15<<28)        /* 4’b1111 更新密钥 */

#define FLASH_CMD_PAGEADDR_MASK 0x0003FFFF      /* 擦除或编程的目标页地址, 128k以内 */

/*
 * CAH 加密地址上界寄存器
 */
#define FLASH_CADDR_HI_MASK     0x0003FFFF      /* 加密范围的上界地址 */

/*
 * CAL 加密地址下界寄存器
 */
#define FLASH_CADDR_LO_MASK     0x0003FFFF      /* 加密范围的下界地址 */

/*
 * STS 状态寄存器
 */
#define FLASH_SR_NO_PERMISSION  bit(3)          /* RO 无权限, 表示上一次操作无权限 */
#define FLASH_SR_PE_END         bit(2)          /* RO 擦写结束 */
#define FLASH_SR_VERIFY_END     bit(1)          /* RO 校验结束 */
#define FLASH_SR_VERIFY_OK      bit(0)          /* RO 校验正确 */

/*
 * PET 擦写时间寄存器
 */
#define FLASH_PET_INTEN_NOPER   bit(17)         /* no_permission 中断使能 */
#define FLASH_PET_INTEN_PEEND   bit(16)         /* pe_end 中断使能 */

#define FLASH_PET_ETIME_MASK    (7<<3)          /* 擦除时间. 以8M时钟计算, 默认擦除时间2.5ms */
#define FLASH_PET_ETIME_SHIFT   3
#define FLASH_PET_ETIME_15      (0<<3)          /* 0:1.5ms */
#define FLASH_PET_ETIME_20      (1<<3)          /* 1:2.0ms */
#define FLASH_PET_ETIME_25      (2<<3)          /* 2:2.5ms */
#define FLASH_PET_ETIME_30      (3<<3)          /* 3:3.0ms */
#define FLASH_PET_ETIME_35      (4<<3)          /* 4:3.5ms */
#define FLASH_PET_ETIME_40      (5<<3)          /* 5:4.0ms */
#define FLASH_PET_ETIME_45      (6<<3)          /* 6:4.5ms */
#define FLASH_PET_ETIME_50      (7<<3)          /* 7:5.0ms */

#define FLASH_PET_PTIME_MASK    7               /* 编程时间. 以8M时钟计算, 默认编程时间2.5ms */
#define FLASH_PET_PTIME_15      0               /* 0:1.5ms */
#define FLASH_PET_PTIME_20      1               /* 1:2.0ms */
#define FLASH_PET_PTIME_25      2               /* 2:2.5ms */
#define FLASH_PET_PTIME_30      3               /* 3:3.0ms */
#define FLASH_PET_PTIME_35      4               /* 4:3.5ms */
#define FLASH_PET_PTIME_40      5               /* 5:4.0ms */
#define FLASH_PET_PTIME_45      6               /* 6:4.5ms */
#define FLASH_PET_PTIME_50      7               /* 7:5.0ms */

//-------------------------------------------------------------------------------------------------
// 定时器
//-------------------------------------------------------------------------------------------------

typedef struct 
{
    volatile unsigned int cfg;                  // 0x00 配置寄存器
    volatile unsigned int cnt;                  // 0x04 计数值寄存器
    volatile unsigned int cmp;                  // 0x08 比较值寄存器
    volatile unsigned int step;                 // 0x0c 步进值寄存器
} HW_TIMER_t;

/*
 * CFG 配置寄存器, 用的 8M 时钟
 */
#define TIMER_CFG_INT_SR        bit(8)          /* 中断状态/清中断. 1=有中断, 写1清中断 */
#define TIMER_CFG_PERIODIC      bit(2)          /* 1=周期触发 */
#define TIMER_CFG_INT_EN        bit(1)          /* 1=中断使能 */
#define TIMER_CFG_START         bit(0)          /* 1=计数使能, 此时不能修改 CNT/CMP/STP 的值 */

//-------------------------------------------------------------------------------------------------
// I2C 控制器
//-------------------------------------------------------------------------------------------------

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

/*
 * PRERH 和 PRERL 共同组成分频值 PRER, 则输出的 SCL 频率为: clkin / (4*(PRER+1))
 */

/*
 * CTL 控制寄存器
 */
#define IIC_CTRL_EN             bit(7)          /* 模块工作使能. 1=正常工作模式, 0=对分频值寄存器进行操作 */
#define IIC_CTRL_IEN            bit(6)          /* 1=中断使能  */
#define IIC_CTRL_MASTER         bit(5)          /* 主从模式选择. 0=从设备, 1=主设备 */
#define IIC_CTRL_SLAVE_TXRDY    bit(4)          /* "从设备"发送数据准备好: 1=要发送的数据已写入data. 自动清零 */
#define IIC_CTRL_SLAVE_RXRDY    bit(3)          /* "从设备"接收数据已读出: 1=data内数据已被读出. 自动清零 */
#define IIC_CTRL_BUSLOCK_CHECK  bit(1)          /* 1=总线死锁状态检查使能: 时间buslock_top */
#define IIC_CTRL_SLAVE_ATUORST  bit(0)          /* 1=总线死锁时从设备自动复位状态机使能 */

/*
 * DATA 数据寄存器
 *
 * 写入时为待发送数据, 读出时为收到的数据
 */

/*
 * CMD 命令寄存器
 */
#define IIC_CMD_START           bit(7)          /* 1="主设备"产生"传输开始波形" */
#define IIC_CMD_STOP            bit(6)          /* 1="主设备"产生"传输结束波形" */
#define IIC_CMD_READ            bit(5)          /* 1="主设备"下一次传输为总线读请求 */
#define IIC_CMD_WRITE           bit(4)          /* 1="主设备"下一次传输为总线写请求 */
#define IIC_CMD_NACK            bit(3)          /* 主设备应答. 1=下一次读数据返回时应答NACK, 连续读请求结束 */
#define IIC_CMD_ACK             0
#define IIC_CMD_RECOVER         bit(2)          /* 总线死锁恢复命令. 1="主设备"解除死锁 */
#define IIC_CMD_IACK            bit(0)          /* 中断应答. 写1清中断 */

/*
 * SR 状态寄存器
 */
#define IIC_SR_RXNACK           bit(7)          /* 0=收到应答, 1=收到NACK */
#define IIC_SR_BUSY             bit(6)          /* 1=总线忙状态 */
#define IIC_SR_ALOST            bit(5)          /* 1="主设备"失去总线控制权 */
#define IIC_SR_SLAVE_ADDRESSED  bit(4)          /* 1="从设备"被寻址成功 */
#define IIC_SR_SLAVE_RW         bit(3)          /* 0="从设备"被读, 1="从设备"被写 */
#define IIC_SR_BUSLOCK          bit(2)          /* 1=总线死锁 */
#define IIC_SR_TXIP             bit(1)          /* 1="主设备"正在传输 */
#define IIC_SR_INT_FLAG         bit(0)          /* 中断标志位. 1=传输完1个字节或"主设备"丢失控制权 */

/*
 * SADDR 从模式地址寄存器
 */
#define IIC_SADDR_MASK          0x7F            /* 从设备地址 */

//-------------------------------------------------------------------------------------------------
// SPI 控制器
//-------------------------------------------------------------------------------------------------

typedef struct 
{
    volatile unsigned char ctrl;                // 0x00 控制寄存器
    volatile unsigned char sr;                  // 0x01 状态寄存器
    volatile unsigned char data;                // 0x02 数据寄存器
    volatile unsigned char er;                  // 0x03 外部寄存器
    volatile unsigned char param;               // 0x04 参数控制寄存器
    volatile unsigned char cs;                  // 0x05 片选寄存器
    volatile unsigned char timing;              // 0x06 时序控制寄存器
} HW_SPI_t;

/*
 * CTRL 控制寄存器
 */
#define SPI_CTRL_INT_EN         bit(7)          /* 1=中断使能 */
#define SPI_CTRL_EN             bit(6)          /* 1=系统工作使能 */
#define SPI_CTRL_MASTER         bit(4)          /* 1=master 模式, 0=slave 模式 */
#define SPI_CTRL_CPOL           bit(3)          /* 时钟极性, 表示无时钟时 CLK 的电平. 1=高电平, 0=低电平 */
#define SPI_CTRL_CPHA           bit(2)          /* 时钟相位, 0=相位相同, 1=相位相反 */
#define SPI_CTRL_SPR_MASK       0x03            /* 时钟分频位, 与 SPER 一起使用 */

/*
 * SR 状态寄存器
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
#define SPI_ER_INT_CNT_MASK     0xC0            /* 传输多少字节后发中断 */
#define SPI_ER_INT_CNT_SHIFT    6
#define SPI_ER_INT_1BYTES       0
#define SPI_ER_INT_2BYTES       1
#define SPI_ER_INT_3BYTES       2
#define SPI_ER_INT_4BYTES       3
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
#define SPI_ER_SPRE_1024        9               /* 4’b1001 */
#define SPI_ER_SPRE_2048        10              /* 4’b1010 */
#define SPI_ER_SPRE_4096        11              /* 4’b1011 */
#endif

/*
 * PARAM 参数控制寄存器
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

//-------------------------------------------------------------------------------------------------
// UART 控制器
//-------------------------------------------------------------------------------------------------
/*
 * XXX UART0/1 使用8M时钟, UART2 使用 32K 时钟
 */

#define NS16550_FIFO_SIZE       16              /* XXX 16 bytes? */

typedef struct 
{
    union
	{
    	volatile unsigned char dat;             // 0x00 数据寄存器
    	volatile unsigned char dll;             // 0x00 分频值低字节寄存器
	} R0;
    union
	{
    	volatile unsigned char ier;             // 0x01 中断使能寄存器
    	volatile unsigned char dlh;             // 0x01 分频值高字节寄存器
	} R1;
    union
	{
		volatile unsigned char isr;             // 0x02 中断状态寄存器
		volatile unsigned char fcr;             // 0x02 FIFO控制寄存器
		volatile unsigned char dld;             // 0x02 分频值小数寄存器
	} R2;
    volatile unsigned char lcr;                 // 0x03 线路控制寄存器
    volatile unsigned char samp;                // 0x04 bit 窗口划分和采样控制寄存器
    volatile unsigned char lsr;                 // 0x05 线路状态寄存器
    volatile unsigned char tfcnt;               // 0x06 发送队列数据存量
    volatile unsigned char sr;                  // 0x07 状态寄存器
} HW_NS16550_t;

/*
 * IER 中断使能寄存器
 */
#define NS16550_IER_MODEM       bit(3)          /* Modem 状态中断使能 */
#define NS16550_IER_LINE        bit(2)          /* 线路状态中断使能 */
#define NS16550_IER_TX          bit(1)          /* 发送状态中断使能 */
#define NS16550_IER_RX          bit(0)          /* 接收状态中断使能 */

/*
 * ISR 中断状态寄存器
 */
#define NS16550_ISR_SRC_MASK    0x0E            /* RO 中断源 */
#define NS16550_ISR_SRC_SHIFT   1
#define NS16550_ISR_LINE        (0x3<<1)        /* 3’b011 线路状态中断, 优先级1, 奇偶/溢出/帧错误/打断时中断, 读LSR清除 */
#define NS16550_ISR_RX          (0x2<<2)        /* 3’b010 接收状态中断, 优先级2, RX数量达到trigger的值, 读data清除 */
#define NS16550_ISR_RXTMO       (0x6<<1)        /* 3’b110 接收状态中断, 优先级2, RX超时, 读data清除 */
#define NS16550_ISR_TX          (0x1<<1)        /* 3’b001 发送状态中断, 优先级3, TX FIFO为空, 写data或读isr清除 */
#define NS16550_ISR_MODEM       (0x0<<1)        /* 3’b000 Modem状态中断, 优先级4, reserved, 读 Modem状态寄存器清除 */
#define NS16550_ISR_PENDING     bit(0)          /* 中断未决状态 */

/*
 * FCR FIFO 控制寄存器
 */
#define NS16550_FCR_TRIG_MASK   0xF8            /* 接收中断状态所需trigger. 0/1=1字节, 最多16字节 */
#define NS16550_FCR_TRIG_SHIFT  3
#define NS16550_FCR_TRIGGER(n)  ((n<<3)&NS16550_FCR_TRIG_MASK)
#define NS16550_FCR_TXFIFO_RST  bit(2)          /* 复位发送FIFO */
#define NS16550_FCR_RXFIFO_RST  bit(1)          /* 复位接收FIFO */
#define NS16550_FCR_FIFO_EN     bit(0)          /* 使能FIFO? */

/*
 * LCR 线路控制寄存器
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

/*
 * LSR 线路状态寄存器
 */
#define NS16550_LSR_ERR         bit(7)          /* 1=有错误, 校验/帧错误或打断中断 */
#define NS16550_LSR_TE          bit(6)          /* 0=有数据, 1=TX FIFO和传输移位寄存器为空. 写TXFIFO时清除 */
#define NS16550_LSR_TFE         bit(5)          /* 1=传输FIFO 为空 */
#define NS16550_LSR_BI          bit(4)          /* 打断中断. 0=没有中断 */
#define NS16550_LSR_FE          bit(3)          /* 帧错误 */
#define NS16550_LSR_PE          bit(2)          /* 奇偶校验位错误 */
#define NS16550_LSR_OE          bit(1)          /* 数据溢出 */
#define NS16550_LSR_DR          bit(0)          /* 接收数据有效. 0=RXFIFO无数据, 1=RXFIFO有数据 */

/*
 * TF_CNT 发送队列中待发送的数据量
 */
#define NS16550_TFCNT_LOOPBACK  bit(7)          /* 自回环模式控制位 */
#define NS16550_TFCNT_MASK      0x1F            /* 发送队列中待发送的数据量 */

/*
 * SR 状态寄存器寄存器
 */
#define NS16550_SR_RX_RST       bit(7)          /* 接收数据通路中32K 时钟域的复位状态. 1=正在复位 */
#define NS16550_SR_CLK32K_RST   bit(6)          /* 控制逻辑32K 时钟域的复位状态. 1=正在复位 */
#define NS16550_SR_FLUSH_WAIT   bit(5)          /* 接收数据通路中数据丢弃等待标识. 1=正在丢弃 */
#define NS16550_SR_RFCNT_MASK   0x1F            /* 接收队列中的数据量 */

//-------------------------------------------------------------------------------------------------
// 实时时钟
//-------------------------------------------------------------------------------------------------

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

//-------------------------------------------------------------------------------------------------
// DMA 控制器
//-------------------------------------------------------------------------------------------------

typedef struct 
{
	volatile unsigned int source;               // 0x00 DMA 命令源地址读写端口. 动态变化
	volatile unsigned int count;                // 0x04 DMA 命令数据长度读写端口, 搬运的字数, 动态变化
	volatile unsigned int cmd_sr;               // 0x08 命令和状态寄存器
	volatile unsigned int int_sr;               // 0x0c 中断和状态寄存器
	volatile unsigned int src0;                 // 0x10 命令队列项 0 的源地址参数
	volatile unsigned int src1;                 // 0x14 命令队列项1的源地址参数
	volatile unsigned int cnt0;                 // 0x18 命令队列项 0 的DMA长度参数
	volatile unsigned int cnt1;                 // 0x1c 命令队列项 1 的DMA长度参数
} HW_DMA_t;

/*
 * SMD & STATUS 命令和状态寄存器
 */
#define DMA_CMDSR_SOFT_RST      bit(31)         /* DMA 控制器软复位. 写1复位, 完成自动清零 */
#define DMA_CMDSR_IND_1ST       bit(1)          /* 标识DMA 的第一个数据 */
#define DMA_CMDSR_EN            bit(0)          /* DMA 命令生效 */

/*
 * INT & STATUS 中断和状态寄存器
 */
#define DMA_INTSR_BUF_RPTR      bit(19)         /* DMA 数据缓冲的读指针值 */
#define DMA_INTSR_BUF_WPTR      bit(18)         /* DMA 数据缓冲的写指针值 */
#define DMA_INTSR_BUFCNT_MASK   0x00030000      /* DMA 数据缓冲存储的有效数据项数 */
#define DMA_INTSR_BUFCNT_SHIFT  16
#define DMA_INTSR_CMD_RPTR      bit(13)         /* DMA 命令队列的读指针值 */
#define DMA_INTSR_CMD_WPTR      bit(12)         /* DMA 命令队列的写指针值 */
#define DMA_INTSR_CMD1_MASK     0x00000C00      /* bit[11:10] DMA 命令队列1 的命令值 */
#define DMA_INTSR_CMD1_SHIFT    10
#define DMA_INTSR_CMD1_INVALID  (0<<10)         /* 命令无效 */
#define DMA_INTSR_CMD1_NO_1ST   (1<<10)         /* 命令有效, 执行时不需要表示第一个数据 */
#define DMA_INTSR_CMD1_IND_1ST  (3<<10)         /* 命令有效, 执行时需要表示第一个数据 */
#define DMA_INTSR_CMD0_MASK     0x00000C00      /* bit[9:8] DMA 命令队列0 的命令值 */
#define DMA_INTSR_CMD0_SHIFT    8
#define DMA_INTSR_CMD0_INVALID  (0<<8)          /* 命令无效 */
#define DMA_INTSR_CMD0_NO_1ST   (1<<8)          /* 命令有效, 执行时不需要表示第一个数据 */
#define DMA_INTSR_CMD0_IND_1ST  (3<<8)          /* 命令有效, 执行时需要表示第一个数据 */
#define DMA_INTSR_INT_CNT_MASK  0x03            /* bit[1:0] 中断计数器 */
#define DMA_INTSR_INT_CLR       bit(0)          /* 每写入1,int_cnt-=1, 当=0时, 清除中断 */

//-------------------------------------------------------------------------------------------------
// VPWM 模块
//-------------------------------------------------------------------------------------------------

typedef struct 
{
	volatile unsigned int cfg;                  // 0x00 算法配置
	volatile unsigned int rsv;                  // 0x04
	volatile unsigned int sr;                   // 0x08 数据写端口状态
	volatile unsigned int data;                 // 0x0C 数据写端口
} HW_VPWM_t;

/*
 * VPWMCFG 算法配置
 */
#define VPWM_CFG_FORCEOUT       bit(31)         /* 强制输出 */
#define VPWM_CFG_OUT_INV        bit(30)         /* 输出取反 */
#define VPWM_CFG_OUT_DLY        bit(29)         /* 输出延迟 */
#define VPWM_CFG_OUT_DUAL       bit(28)         /* 输出模式 */
#define VPWM_CFG_FREQ_SEL       bit(27)         /* 时钟选择. 0=32M, 1=8M */
#define VPWM_CFG_FREQ_RSTn      bit(26)         /* 时钟复位. */
#define VPWM_CFG_UPSAMP_MASK    0x03000000      /* bit[25:24] 上采样系数 */
#define VPWM_CFG_UPSAMP_SHIFT   24
#define VPWM_CFG_OUTBITS_MASK   0x00F00000      /* bit[23:20] 输出数据位数 */
#define VPWM_CFG_OUTBITS_SHIFT  20
#define VPWM_CFG_DMA_EN         bit(19)         /* DMA 使能 */
#define VPWM_CFG_ADPCM_EN       bit(16)         /* ADPCM 压缩使能 */
#define VPWM_CFG_PWM_PER_MASK   0x0000FFF0      /* PWM 周期 */
#define VPWM_CFG_PWM_PER_SHIFT  4
#define VPWM_CFG_VOLUME_MASK    0x07            /* 音量 */

/*
 * AUDIO STATUS 数据写端口状态
 */
#define VPWM_SR_UNDERFLOW       bit(2)          /* 缓存区下溢 */
#define VPWM_SR_FIRST_DATA      bit(1)          /* 第一个数据 */
#define VPWM_SR_READY           bit(0)          /* 读请求. 1=VPWM向处理器发送读请求 */

/*
 * AUDIO DATA 数据写端口: 当配置为通过寄存器接口获取数据时, 软件将音频数据写入该寄存器
 */

//-------------------------------------------------------------------------------------------------
// TSENSOR 触摸按键控制器
//-------------------------------------------------------------------------------------------------

typedef struct 
{
	volatile unsigned int ctrl;                 // 0x00 控制寄存器
	volatile unsigned int sr;                   // 0x04 状态寄存器
	volatile unsigned int osc_th;               // 0x08 环振/阀值寄存器
	volatile unsigned int polltim;              // 0x0c 扫描时序寄存器
	volatile unsigned int rsv1[12];             // 0x10~0x3C
	volatile unsigned int chnl_attr[12];        // 0x40~0x6c 通道特征0~11
	volatile unsigned int rsv2[4];              // 0x70~0x7C
	volatile unsigned int cnt_res[12];          // 0x80~0xac 计数结果0~11
} HW_TSENSOR_t;

/*
 * TS-CTRL 控制寄存器
 */
#define TS_CTRL_CHNLEN_MASK     0x0FFF0000      /* bit[27:16] 扫描对应位为1的通道 */
#define TS_CTRL_CHNLEN_SHIFT    16

#define TS_CTRL_DBC_EN          bit(15)         /* 去抖使能 */
#define TS_CTRL_DBC_NUM_MASK    0x00007000      /* bit[14:12] 去抖计数, 连续检测到 0~7 次按键才发中断 */
#define TS_CTRL_DBC_NUM_SHIFT   12

#define TS_CTRL_EOS_OV          bit(11)         /* 扫描结束强制溢出 */

#define TS_CTRL_FLT_LVL_MASK    0x00000300      /* bit[9:8] 基线滤波强度 */
#define TS_CTRL_FLT_LVL_SHIFT   8

#define TS_CTRL_IEN_MASM        0x000000F0      /* bit[7:4] 中断使能 */
#define TS_CTRL_IEN_SHIFT       4
#define TS_CTRL_IEN_OFLOW       bit(7)          /* 计数溢出中断使能 */
#define TS_CTRL_IEN_MULTI_DOWN  bit(6)          /* 多键同按中断使能 */
#define TS_CTRL_IEN_UP          bit(5)          /* 抬起中断使能 */
#define TS_CTRL_IEN_DOWN        bit(4)          /* 按下中断使能 */

#define TS_CTRL_TEST_EN         bit(3)          /* 测试模式 */
#define TS_CTRL_POLL_EN         bit(1)          /* 循环扫描使能 */
#define TS_CTRL_SCAN_EN         bit(0)          /* 单次扫描使能 */

/*
 * TS-STATE 状态寄存器
 */
#define TS_SR_KEYCODE_MASK      0x0FFF0000      /* bit[27:16] 按键编码状态 */
#define TS_SR_KEYCODE_SHIFT     16

#define TS_SR_PSM_MASK          0x00000C00      /* bit[11:10] 循环状态 */
#define TS_SR_PSM_SHIFT         10

#define TS_SR_WSM_MASK          0x00000300      /* bit[9:8] 等待状态 */
#define TS_SR_WSM_SHIFT         8

#define TS_SR_SSM_MASK          0x000000F0      /* bit[7:4] 扫描状态 */
#define TS_SR_SSM_SHIFT         4

#define TS_SR_OVERFLOW          bit(3)          /* 计数溢出 */
#define TS_SR_MULTI_DOWN        bit(2)          /* 多键同按 */
#define TS_SR_UP                bit(1)          /* 所有键抬起 */
#define TS_SR_DOWN              bit(0)          /* 单键按下 */

/*
 * OSC-THRESH 环振/阀值寄存器
 */
#define TS_OSCTH_UP_TH_MASK     0xFF000000      /* bit[31:24] 按键抬起阀值 */
#define TS_OSCTH_UP_TH_SHIFT    24

#define TS_OSCTH_BAS_TH_MASK    0x00F00000      /* bit[23:16] 基线滤波阀值 */
#define TS_OSCTH_BAS_TH_SHIFT   16

#define TS_CFG_CNT_NEG          bit(15)         /* 使用双沿计数 */

#define TS_CFG_CNT_PRD_MASK     0x00001F00      /* bit[12:8] 计数长度, 以32K时钟周期为单位? */
#define TS_CFG_CNT_PRD_SHIFT    8

#define TS_CFG_RSEL_MASK        0x0000000F      /* bit[3:0] 电阻选择, 1~14=1K~14K */

/*
 * PollTim 扫描时序寄存器
 */
#define TS_POLL_DBC_PRD_MASK    0xFF000000      /* bit[31:24] 去抖测量间隔, 以32K时钟周期为单位, 0=256 */
#define TS_POLL_DBC_PRD_SHIFT   24

#define TS_POLL_ACT_STB_MASK    0x00C00000      /* bit[23:22] 强制切换扫描模式 */
#define TS_POLL_ACT_STB_SHIFT   22
#define TS_POLL_ACTIVE          bit(23)
#define TS_POLL_STANDBY         bit(22)

#define TS_POLL_STB_PRD_MASK    0x003F0000      /* bit[21:16] 待机模式间隔, 以激活模式扫描周期为单位, 0=256 */
#define TS_POLL_STB_PRD_SHIFT   16

#define TS_POLL_ACT_PRD_MASK    0x00000F00      /* bit[11:8] 激活模式间隔, 以256个32K时钟为单位, 0=2^12 */
#define TS_POLL_ACT_PRD_SHIFT   8

#define TS_POLL_ACT_NUM_MASK    0x000000FF      /* bit[7:0] 激活模式持续时间, 以待机模式周期为单位, 0=4 */

/*
 * Channel Attribute 按键通道特征值寄存器
 */
#define TS_CHNLATTR_PRESS_MASK  0xFF000000      /* bit[31:24] 按键通道按下阈值 */
#define TS_CHNLATTR_PRESS_SHIFT 24

#define TS_CHNLATTR_BASVAL_MASK 0x00000FFF      /* bit[11:0] 按键通道计数基准值 */

/*
 * Count Result 计数结果0~11
 */
#define TS_RES_CNT_OV           bit(31)         /* 计数溢出 */
#define TS_RES_BAS_OV           bit(30)         /* 基线溢出 */
#define TS_RES_VAL_MASK         0x00000FFF      /* bit[11:0] 计数值, 经修正或原始值 */

//-------------------------------------------------------------------------------------------------
// ioctl command for spi & iic bus
//-------------------------------------------------------------------------------------------------

#define IOCTL_SPI_I2C_SET_TFRMODE       0x1000
#define IOCTL_FLASH_FAST_READ_ENABLE    0x2000  // none.          - set spi flash param to memory-en
#define IOCTL_FLASH_FAST_READ_DISABLE   0x4000  // none.          - unset spi flash param to memory-en
#define IOCTL_FLASH_GET_FAST_READ_MODE  0x8000  // unsigned int * - get spi flash is set to memory-en


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LS1C102_H_ */
