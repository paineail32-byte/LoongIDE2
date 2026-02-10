/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k1000.h
 *
 * created: 2022-10-03
 *  author: Bian
 */

#ifndef _LS2K1000LA_H
#define _LS2K1000LA_H

//-------------------------------------------------------------------------------------------------

#define bit(x)                  (1ul<<x)

//-------------------------------------------------------------------------------------------------
// 寄存器 Read/Write 操作, 地址转换为 loongarch64 uncached
//-------------------------------------------------------------------------------------------------

/*
 * 8 Bits
 */
#define READ_REG8(addr)         (*(volatile unsigned char *)(PHYS_TO_UNCACHED(addr)))
#define WRITE_REG8(addr, v)     (*(volatile unsigned char *)(PHYS_TO_UNCACHED(addr))  = (v))
#define OR_REG8(addr, v)        (*(volatile unsigned char *)(PHYS_TO_UNCACHED(addr)) |= (v))
#define AND_REG8(addr, v)       (*(volatile unsigned char *)(PHYS_TO_UNCACHED(addr)) &= (v))

/*
 * 16 Bits
 */
#define READ_REG16(addr)        (*(volatile unsigned short *)(PHYS_TO_UNCACHED(addr)))
#define WRITE_REG16(addr, v)    (*(volatile unsigned short *)(PHYS_TO_UNCACHED(addr))  = (v))
#define OR_REG16(addr, v)       (*(volatile unsigned short *)(PHYS_TO_UNCACHED(addr)) |= (v))
#define AND_REG16(addr, v)      (*(volatile unsigned short *)(PHYS_TO_UNCACHED(addr)) &= (v))

/*
 * 32 Bits
 */
#define READ_REG32(addr)        (*(volatile unsigned int *)(PHYS_TO_UNCACHED(addr)))
#define WRITE_REG32(addr, v)    (*(volatile unsigned int *)(PHYS_TO_UNCACHED(addr))  = (v))
#define OR_REG32(addr, v)       (*(volatile unsigned int *)(PHYS_TO_UNCACHED(addr)) |= (v))
#define AND_REG32(addr, v)      (*(volatile unsigned int *)(PHYS_TO_UNCACHED(addr)) &= (v))

/*
 * 64 Bits
 */
#define READ_REG64(addr)        (*(volatile unsigned long *)(PHYS_TO_UNCACHED(addr)))
#define WRITE_REG64(addr, v)    (*(volatile unsigned long *)(PHYS_TO_UNCACHED(addr))  = (v))
#define OR_REG64(addr, v)       (*(volatile unsigned long *)(PHYS_TO_UNCACHED(addr)) |= (v))
#define AND_REG64(addr, v)      (*(volatile unsigned long *)(PHYS_TO_UNCACHED(addr)) &= (v))

#define HI32(v)                 ((unsigned int)((v >> 32) & 0xFFFFFFFFul))
#define LO32(v)                 ((unsigned int)v & 0xFFFFFFFFul)

//-------------------------------------------------------------------------------------------------

/**
 * Chip Config Registers
 */

#define LS2K1000LA_VSRSION      0x1fe00000          	/* 寄存器版本 */
#define LS2K1000LA_FEATURE      0x1fe00008          	/* 芯片特性 */
#define LS2K1000LA_VENDOR       0x1fe00010          	/* 厂商名称 */
#define LS2K1000LA_ID           0x1fe00020          	/* 芯片名称 */

#define LS2K1000LA_CHIPID       0x1fe03ff8          	/* 芯片版本号 */

/*
 * 窗口地址, n>=0 && n<=7
 */
#define CPU_WIN_BASE(n)         (0x1fe02000+(n)*8)  	/* cache通路二级窗口n的基地址 */
#define CPU_WIN_MASK(n)         (0x1fe02040+(n)*8)  	/* cache通路二级窗口n的掩码 */
#define CPU_WIN_MMAP(n)         (0x1fe02080+(n)*8)  	/* cache通路二级窗口n的新基地址 */

#define PCI_WIN_BASE(n)         (0x1fe02100+(n)*8)  	/* uncache通路二级窗口n的基地址 */
#define PCI_WIN_MASK(n)         (0x1fe02140+(n)*8)  	/* uncache通路二级窗口n的掩码 */
#define PCI_WIN_MMAP(n)         (0x1fe02180+(n)*8)  	/* uncache通路二级窗口n的新基地址 */

#define XBAR_WIN4_BASE(n)       (0x1fe02400+(n)*8)  	/* IO设备(除PCIE)DMA访问窗口n的基地址 */
#define XBAR_WIN4_MASK(n)       (0x1fe02440+(n)*8)  	/* IO设备(除PCIE)DMA访问窗口n的掩码 */
#define XBAR_WIN4_MMAP(n)       (0x1fe02480+(n)*8)  	/* IO设备(除PCIE)DMA访问窗口n的新基地址 */

#define XBAR_WIN5_BASE(n)       (0x1fe02500+(n)*8)  	/* PCIE设备DMA访问窗口n的基地址 */
#define XBAR_WIN5_MASK(n)       (0x1fe02540+(n)*8)  	/* PCIE设备DMA访问窗口n的掩码 */
#define XBAR_WIN5_MMAP(n)       (0x1fe02580+(n)*8)  	/* PCIE设备DMA访问窗口n的新基地址 */

/**
 * 通用配置寄存器0
 */
#define CHIP_CFG0_BASE                  0x1fe00420
#define CFG0_LIO_BIG_MEM                bit(63)             /* LIO big_mem 模式控制 */
#define CFG0_LIO_IOPF_EN                bit(62)             /* 保留 */
#define CFG0_LIO_IO_WIDTH_16            bit(61)             /* IO数据位宽: 0=8位; 1=16位 */
#define CFG0_LIO_IO_COUNT_INIT_MASK     (0x1ful<<56)        /* bit[60:56] IO数据读取延迟(boot时钟周期数) */
#define CFG0_LIO_IO_COUNT_INIT_SHIFT    56
#define CFG0_LIO_ROM_WIDTH_16           bit(55)             /* Rom数据位宽: 0=8位; 1=16位 */
#define CFG0_LIO_ROM_COUNT_INIT_MASK    (0x1ful<<50)        /* bit[54:50] ROM数据读取延迟(boot时钟周期数) */
#define CFG0_LIO_ROM_COUNT_INIT_SHIFT   50
#define CFG0_LIO_CLOCK_PERIOD_MASK      (0x3ul<<48)     	/* bit[49:48] 内部等待计数器步长设置 */
#define CFG0_LIO_CLOCK_PERIOD_SHIFT     48
#define CFG0_LIO_CLOCK_PERIOD_1         0                   /* 0: 步长为1 */
#define CFG0_LIO_CLOCK_PERIOD_4         (0x1ul<<48)     	/* 1: 步长为4 */
#define CFG0_LIO_CLOCK_PERIOD_2         (0x2ul<<48)     	/* 2: 步长为2 */
#define CFG0_LIO_CLOCK_PERIOD__1        (0x3ul<<48)     	/* 3: 步长为1 */
#define CFG0_RTC_RESTART                bit(47)             /* rtc 内部振荡器重启控制 */
#define CFG0_RTC_DS_MASK                (0x7ul<<44)     	/* bit[46:44] 内部振荡器接口控制信号 */
#define CFG0_MC_DEFAULT_REG             bit(41)             /* 存储控制器窗口不命中处理
                                                               0: 关闭内存控制器的该功能
                                                               1: 当所有窗口不命中时, 由内存控制器给出响应, 防止CPU卡死.
                                                             */
#define CFG0_MC_DISABLE_REG             bit(40)             /* DDR配置空间关闭, 高有效
                                                               DDR控制器在内存空间中开辟了一小段配置空间(1MB @0x0ff0,0000),
                                                               -在关闭后软件就可以使用这段空间. 为避免意外访问, 建议在配置完成后及时关闭
                                                             */
#define CFG0_USB_EHCI64_EN              bit(36)             /* USB端口的EHCI控制器64位地址模式
                                                               0: 32位地址模式
                                                               1: 64位地址模式
                                                             */
#define CFG0_HPET_INT_MODE              bit(35)             /* HPET中断模式
                                                               0: 正常模式, 即timer0/1/2的中断都映射到中断引脚21
                                                               1: 中断分开模式, timer0/1/2的中断分别映射到中断引脚21/38/39
                                                             */
#define CFG0_PCIE_COHERENT              bit(34)             /* PCIE设备的DMA请求类别, 1: 为一致性请求 */
#define CFG0_USB_COHERENT               bit(33)             /* USB设备的DMA请求类别, 1: 为一致性请求 */
#define CFG0_HDA_COHERENT               bit(32)             /* HDA设备的DMA请求类别, 1: 为一致性请求 */
#define CFG0_SDIO_SEL                   bit(20)             /* SDIO管脚复用控制: 0: 管脚为GPIO; 1: 管脚为SDIO */
#define CFG0_CAN0_SEL                   bit(16)             /* CAN管脚复用控制: 当专用通信接口为0时, 0: 管脚为GPIO; 1: 管脚为CAN */
#define CFG0_CAN1_SEL                   bit(17)
#define CFG0_PWM0_SEL                   bit(12)             /* PWM管脚复用控制: 0: 管脚为GPIO; 1: 管脚为PWM */
#define CFG0_PWM1_SEL                   bit(13)
#define CFG0_PWM2_SEL                   bit(14)
#define CFG0_PWM3_SEL                   bit(15)
#define CFG0_I2C0_SEL                   bit(10)             /* I2C管脚复用控制: 0: 管脚为GPIO; 1: 管脚为I2C */
#define CFG0_I2C1_SEL                   bit(11)
#define CFG0_NAND_SEL                   bit(9)              /* NAND管脚复用控制: 0: 管脚为GPIO; 1: 管脚为NAND */
#define CFG0_SATA_SEL                   bit(8)              /* SATA管脚复用控制: 0: 管脚为GPIO; 1: 管脚为SATA */
#define CFG0_LIO_SEL                    bit(7)              /* 当dvo0_sel为0时, LIO管脚复用控制: 0: 管脚为GPIO; 1: 管脚为LIO */
/*
 * 注: dvo0_sel、lio_sel和uart1/2_sel三者只能有一个为1
 */
#define CFG0_I2S_SEL                    bit(6)              /* I2S管脚复用控制: 0: 管脚为HDA或GPIO; 1: 管脚为I2S */
#define CFG0_HDA_SEL                    bit(4)              /* HDA管脚复用控制: 0: 管脚为I2S或GPIO; 1: 管脚为HDA */
/*
 * 注: hda_sel、和i2s_sel只能有一个为1, 二者都设0为GPIO模式
 */
#define CFG0_GMAC1_SEL                  bit(3)              /* GMAC1管脚复用控制: 0: 管脚为GPIO; 1: 管脚为GMAC1 */
#define CFG0_GMAC1_SDB_FLOWCTRL         bit(2)              /* GMAC1的边带流控制, 当GMAC控制器的EFC使能时, 可以让MAC产生Pause Control Frame控制
                                                               Rx FIFO的流量: 0: 关闭该功能; 1: 使能该功能 */
#define CFG0_GMAC0_SDB_FLOWCTRL         bit(1)              /* GMAC0的边带流控制, 当GMAC控制器的EFC使能时, 可以让MAC产生Pause Control Frame控制
                                                               Rx FIFO的流量: 0: 关闭该功能; 1: 使能该功能 */
#define CFG0_GMAC_COHERENT              bit(0)              /* GMAC设备的DMA请求类别, 1: 为一致性请求 */

/**
 * 通用配置寄存器1
 */
#define CHIP_CFG1_BASE                  0x1fe00428
#define CFG1_DELAY_HDA_MASK             (0xful<<60)         /* HDA 访存通路写命令堵塞时间 */
#define CFG1_DELAY_HDA_SHIFT            60
#define CFG1_DELAY_PCIE0_MASK           (0xful<<56)         /* PCIE0 访存通路写命令堵塞时间 */
#define CFG1_DELAY_PCIE0_SHIFT          56
#define CFG1_DELAY_USB_MASK             (0xful<<52)         /* USB 访存通路写命令堵塞时间 */
#define CFG1_DELAY_USB_SHIFT            52
#define CFG1_DELAY_SATA_MASK            (0xful<<48)         /* SATA 访存通路写命令堵塞时间 */
#define CFG1_DELAY_SATA_SHIFT           48
#define CFG1_DELAY_DC_MASK              (0xful<<44)         /* DC 访存通路写命令堵塞时间 */
#define CFG1_DELAY_DC_SHIFT             44
#define CFG1_DELAY_GPU_MASK             (0xful<<40)         /* GPU 访存通路写命令堵塞时间 */
#define CFG1_DELAY_GPU_SHIFT            40
#define CFG1_DELAY_DMA_MASK             (0xful<<36)         /* DMA访存写命令堵塞时间 */
#define CFG1_DELAY_DMA_SHIFT            36
#define CFG1_DELAY_GMAC_MASK            (0xful<<32)         /* GMAC 访存通路写命令堵塞时间 */
#define CFG1_DELAY_GMAC_SHIFT           32
#define CFG1_DELAY_AWMON_CHNL_MASK      (0xful<<28)         /* 访存写命令通路monitor通道选择 */
#define CFG1_DELAY_AWMON_CHNL_SHIFT     28
#define CFG1_DELAY_AWMON_SEL_MASK       (0x3ul<<26)         /* 访存写命令通路monitor类型选择 */
#define CFG1_DELAY_AWMON_SEL_SHIFT      26
#define CFG1_AWMON_CLEAR                bit(25)             /* 访存写命令通路monitor清空 */
#define CFG1_AWMON_START                bit(24)             /* 访存写命令通路monitor开始工作 */
#define CFG1_USB_FLUSH_IDLE_MASK        (0xful<<20)         /* 设置清空write buffer前空闲周期数 */
#define CFG1_USB_FLUSH_IDLE_SHIFT       20
#define CFG1_USB_PREFETCH               bit(19)             /* USB 使能读预取 */
#define CFG1_USB_FLUSH_WR               bit(18)             /* USB 设置写命令发出后是否清空read buffer */
#define CFG1_USB_STOP_WAW               bit(17)             /* USB 是否允许在上一个写完成前发出写命令 */
#define CFG1_USB_STOP_RAW               bit(16)             /* USB 是否允许在上一个写完成前发出读命令 */
/*
 * 串口. 注: dvo0_sel、lio_sel和uart1/2_sel三者只能有一个为1
 */
#define CFG1_UART2_SEL                  bit(13)             /* 当dvo0_sel为0时, UART2管脚复用控制: 0: 管脚为GPIO; 1: 管脚为UART2 */
#define CFG1_UART1_SEL                  bit(12)             /* 当dvo0_sel为0时, UART1管脚复用控制: 0: 管脚为GPIO; 1: 管脚为UART1 */
#define CFG1_UART2_MODE_MASK            (0xful<<8)          /* UART2对应的UART控制器模式 */
#define CFG1_UART2_MODE_SHIFT           8
#define CFG1_UART2_8                    (0x1ul<<8)          /* b0001: 8线模式(仅uart2) */
#define CFG1_UART2_4                    (0x3ul<<8)          /* b0011: 4线模式(uart2+uart9) */
#define CFG1_UART2_2                    (0xful<<8)          /* b1111: 2线模式(uart2+uart9+uart10+uart11) */
#define CFG1_UART1_MODE_MASK            (0xful<<4)          /* UART1对应的UART控制器模式 */
#define CFG1_UART1_MODE_SHIFT           4
#define CFG1_UART1_8                    (0x1ul<<4)          /* b0001: 8线模式(仅uart1) */
#define CFG1_UART1_4                    (0x3ul<<4)          /* b0011: 4线模式(uart1+uart6) */
#define CFG1_UART1_2                    (0xful<<4)          /* b1111: 2线模式(uart1+uart6+uart7+uart8) */
#define CFG1_UART0_MODE_MASK            (0xful<<0)          /* UART0对应的UART控制器模式 */
#define CFG1_UART0_MODE_SHIFT           0
#define CFG1_UART0_8                    (0x1ul<<0)          /* b0001: 8线模式(仅uart0) */
#define CFG1_UART0_4                    (0x3ul<<0)          /* b0011: 4线模式(uart0+uart3) */
#define CFG1_UART0_2                    (0xful<<0)          /* b1111: 2线模式(uart0+uart3+uart4+uart5) */

/**
 * 通用配置寄存器2
 */
#define CHIP_CFG2_BASE                  0x1fe00430
#define CFG2_CAM_FLUSH_IDLE_MASK        (0xful<<28)     	/* bit[31:28] cam接口空闲写入周期数 */
#define CFG2_CAM_FLUSH_IDLE_SHIFT       28
#define CFG2_CAM_PREFETCH               bit(27)             /* cam接口预取 */
#define CFG2_CAM_FLUSH_WR               bit(26)             /* cam接口读请求刷出写请求 */
#define CFG2_CAM_STOP_WAW               bit(25)             /* cam接口停止写后写 */
#define CFG2_CAM_STOP_RAW               bit(24)             /* cam接口停止写后读 */
#define CFG2_VPU_DIS                    bit(20)             /* VPU模块使能信号, 1关闭, 0打开 */
#define CFG2_CAM_DIS                    bit(19)             /* CAMERA模块使能信号, 1关闭, 0打开 */
#define CFG2_CAM_SOFT_RESET_EN          bit(18)             /* CAMERA模块软件复位, 低有效. */
#define CFG2_PCIE1_EN                   bit(17)             /* PCIE1控制器使能信号, 高有效. 当不使能时软件扫描不到该PCIE桥. */
#define CFG2_PCIE0_EN                   bit(16)             /* PCIE0控制器使能信号, 高有效. 当不使能时软件扫描不到该PCIE桥. */
#define CFG2_IODMA_SPARE_RD_MASK        0xff00ul            /* bit[15:8] iodma读操作最大数设置 */
#define CFG2_IODMA_SPARE_RD_SHIFT       8
#define CFG2_CAM_COHERENT               bit(7)              /* CAMERA设备的DMA请求类别, 1: 为一致性请求 */
#define CFG2_VPU_COHERENT               bit(6)              /* VPU设备的DMA请求类别, 1: 为一致性请求 */
#define CFG2_CAM_SEL                    bit(5)              /* 使能CAMERA的管脚功能, 1: 管脚为CAMERA */
#define CFG2_DVO1_SEL                   bit(4)              /* 使能DVO1的管脚功能, 1: 管脚为DVO1 */
/*
 * 注: dvo1_sel、cam_sel二者只能有一个为1
 */
#define CFG2_DC_COHERENT                bit(3)              /* DC设备的DMA请求类别, 1: 为一致性请求 */
#define CFG2_GPU_COHERENT               bit(2)              /* GPU设备的DMA请求类别 , 1: 为一致性请求 */
#define CFG2_DVO0_SEL                   bit(1)              /* 使能DVO0的管脚功能, 1: 管脚为DVO0 */
/*
 * 注: dvo0_sel、lio_sel和uart1/2_sel三者只能有一个为1
 */
#define CFG2_GPU_SOFT_RESET             bit(0)              /* GPU的软件复位, 1=代表复位; 0=代表解复位 */

/**
 * APBDMA配置寄存器
 */
#define CHIP_APBDMA_BASE                0x1fe00438
#define DMA_I2S_RX_SEL_MASK             (0x7ul<<21)         /* bit[23:21] I2S控制器接收端所用的DMA控制器 */
#define DMA_I2S_RX_SEL_SHIFT            21
#define DMA_I2S_RX_SEL_0                (0x0ul<<21)         /* 0x0: 对应DMA0控制器 */
#define DMA_I2S_RX_SEL_1                (0x1ul<<21)         /* 0x1: 对应DMA1控制器 */
#define DMA_I2S_RX_SEL_2                (0x2ul<<21)         /* 0x2: 对应DMA2控制器 */
#define DMA_I2S_RX_SEL_3                (0x3ul<<21)         /* 0x3: 对应DMA3控制器 */
#define DMA_I2S_RX_SEL_4                (0x4ul<<21)         /* 0x4: 对应DMA4控制器 */
#define DMA_I2S_TX_SEL_MASK             (0x7ul<<18)         /* bit[20:18] I2S控制器发送端所用的DMA控制器 */
#define DMA_I2S_TX_SEL_SHIFT            18
#define DMA_I2S_TX_SEL_0                (0x0ul<<18)         /* 0x0: 对应DMA0控制器 */
#define DMA_I2S_TX_SEL_1                (0x1ul<<18)         /* 0x1: 对应DMA1控制器 */
#define DMA_I2S_TX_SEL_2                (0x2ul<<18)         /* 0x2: 对应DMA2控制器 */
#define DMA_I2S_TX_SEL_3                (0x3ul<<18)         /* 0x3: 对应DMA3控制器 */
#define DMA_I2S_TX_SEL_4                (0x4ul<<18)         /* 0x4: 对应DMA4控制器 */
#define DMA_SDIO_SEL_MASK               (0x7ul<<15)         /* bit[17:15] SDIO控制器所用的DMA控制器 */
#define DMA_SDIO_SEL_SHIFT              15
#define DMA_SDIO_SEL_0                  (0x0ul<<15)         /* 0x0: 对应DMA0控制器 */
#define DMA_SDIO_SEL_1                  (0x1ul<<15)         /* 0x1: 对应DMA1控制器 */
#define DMA_SDIO_SEL_2                  (0x2ul<<15)         /* 0x2: 对应DMA2控制器 */
#define DMA_SDIO_SEL_3                  (0x3ul<<15)         /* 0x3: 对应DMA3控制器 */
#define DMA_SDIO_SEL_4                  (0x4ul<<15)         /* 0x4: 对应DMA4控制器 */
#define DMA_DES_WR_SEL_MASK             (0x7ul<<12)         /* bit[14:12] DES控制器写操作所用的DMA控制器 */
#define DMA_DES_WR_SEL_SHIFT            12
#define DMA_DES_WR_SEL_0                (0x0ul<<12)         /* 0x0: 对应DMA0控制器 */
#define DMA_DES_WR_SEL_1                (0x1ul<<12)         /* 0x1: 对应DMA1控制器 */
#define DMA_DES_WR_SEL_2                (0x2ul<<12)         /* 0x2: 对应DMA2控制器 */
#define DMA_DES_WR_SEL_3                (0x3ul<<12)         /* 0x3: 对应DMA3控制器 */
#define DMA_DES_WR_SEL_4                (0x4ul<<12)         /* 0x4: 对应DMA4控制器 */
#define DMA_DES_RD_SEL_MASK             (0x7ul<<9)          /* bit[11:9] DES控制器读操作所用的DMA控制器 */
#define DMA_DES_RD_SEL_SHIFT            9
#define DMA_DES_RD_SEL_0                (0x0ul<<9)          /* 0x0: 对应DMA0控制器 */
#define DMA_DES_RD_SEL_1                (0x1ul<<9)          /* 0x1: 对应DMA1控制器 */
#define DMA_DES_RD_SEL_2                (0x2ul<<9)          /* 0x2: 对应DMA2控制器 */
#define DMA_DES_RD_SEL_3                (0x3ul<<9)          /* 0x3: 对应DMA3控制器 */
#define DMA_DES_RD_SEL_4                (0x4ul<<9)          /* 0x4: 对应DMA4控制器 */
#define DMA_AES_WR_SEL_MASK             (0x7ul<<6)          /* bit[8:6] AES控制器写操作所用的DMA控制器 */
#define DMA_AES_WR_SEL_SHIFT            6
#define DMA_AES_WR_SEL_0                (0x0ul<<6)          /* 0x0: 对应DMA0控制器 */
#define DMA_AES_WR_SEL_1                (0x1ul<<6)          /* 0x1: 对应DMA1控制器 */
#define DMA_AES_WR_SEL_2                (0x2ul<<6)          /* 0x2: 对应DMA2控制器 */
#define DMA_AES_WR_SEL_3                (0x3ul<<6)          /* 0x3: 对应DMA3控制器 */
#define DMA_AES_WR_SEL_4                (0x4ul<<6)          /* 0x4: 对应DMA4控制器 */
#define DMA_AES_RD_SEL_MASK             (0x7ul<<3)          /* bit[5:3] AES控制器读操作所用的DMA控制器 */
#define DMA_AES_RD_SEL_SHIFT            3
#define DMA_AES_RD_SEL_0                (0x0ul<<3)          /* 0x0: 对应DMA0控制器 */
#define DMA_AES_RD_SEL_1                (0x1ul<<3)          /* 0x1: 对应DMA1控制器 */
#define DMA_AES_RD_SEL_2                (0x2ul<<3)          /* 0x2: 对应DMA2控制器 */
#define DMA_AES_RD_SEL_3                (0x3ul<<3)          /* 0x3: 对应DMA3控制器 */
#define DMA_AES_RD_SEL_4                (0x4ul<<3)          /* 0x4: 对应DMA4控制器 */
#define DMA_NAND_SEL_MASK               (0x7ul)             /* bit[2:0] NAND控制器所用的DMA控制器 */
#define DMA_NAND_SEL_0                  (0x0ul)             /* 0x0: 对应DMA0控制器 */
#define DMA_NAND_SEL_1                  (0x1ul)             /* 0x1: 对应DMA1控制器 */
#define DMA_NAND_SEL_2                  (0x2ul)             /* 0x2: 对应DMA2控制器 */
#define DMA_NAND_SEL_3                  (0x3ul)             /* 0x3: 对应DMA3控制器 */
#define DMA_NAND_SEL_4                  (0x4ul)             /* 0x4: 对应DMA4控制器 */

/**
 * USB PHY0/1配置寄存器
 */
#define USB_PHY01_BASE                  0x1fe00440
#define USB_PHY1_POWER_OFF              bit(61)
#define USB_PHY1_CFG_EN                 bit(32)             /* 配置使能, 0=使用硬件默认配置; 1=使用该寄存器的软件配置 */
#define USB_PHY0_POWER_OFF              bit(29)
#define USB_PHY0_CFG_EN                 bit(0)              /* 配置使能, 0=使用硬件默认配置; 1=使用该寄存器的软件配置 */

/**
 * USB PHY2/3配置寄存器
 */
#define USB_PHY23_BASE                  0x1fe00448
#define USB_PHY3_POWER_OFF              bit(61)
#define USB_PHY3_CFG_EN                 bit(32)             /* 配置使能, 0=使用硬件默认配置; 1=使用该寄存器的软件配置 */
#define USB_PHY2_POWER_OFF              bit(29)
#define USB_PHY2_CFG_EN                 bit(0)              /* 配置使能, 0=使用硬件默认配置; 1=使用该寄存器的软件配置 */

/**
 * SATA配置寄存器
 */
#define SATA_BASE                       0x1fe00450
#define SATA_DISABLE                    bit(63)             /* 关闭SATA PHY */
#define SATA_DMA_COHERENT               bit(10)             /* dma cache一致性使能 */
#define SATA_LAN0_RESET                 bit(3)              /* LANE0软复位, 0有效 */
#define SATA_PHY_RESET                  bit(2)              /* PHY软复位, 0有效 */
#define SATA_PHY_CLOCK_SEL              bit(1)              /* 参考时钟输入选择: 0=选择内部时钟; 1=选择外部时钟 */
#define SATA_PHY_CLOCK_EN               bit(0)              /* PHY参考时钟使能 */

//-------------------------------------------------------------------------------------------------
// 系统时钟配置寄存器
//-------------------------------------------------------------------------------------------------

/**
 * NODE PLL低64位配置寄存器
 */
#define NODE_PLL0_BASE                  0x1fe00480
#define NODE_PLL0_L1_DIV_LOOPC_MASK     (0x3fful<<32)       /* bit[41:32] L1 PLL倍频系数 */
#define NODE_PLL0_L1_DIV_LOOPC_SHIFT    32
#define NODE_PLL0_L1_DIV_REF_MASK       (0x3ful<<26)        /* bit[31:26] L1 PLL参考时钟分频系数 */
#define NODE_PLL0_L1_DIV_REF_SHIFT      26
#define NODE_PLL0_L1_PD                 bit(19)             /* L1 PLL关电控制, 1代表关电 */
#define NODE_PLL0_L1_LOCKED             bit(16)             /* L1 PLL锁定标志, 1代表锁定 */
#define NODE_PLL0_L1_LOCKC_MASK         (0x3ul<<10)         /* bit[11:10] 判定L1 PLL是否锁定使用的相位的精度 */
#define NODE_PLL0_L1_LOCKC_SHIFT        10
#define NODE_PLL0_L1_LOCK_EN            bit(7)              /* 允许锁定L1 PLL */
#define NODE_PLL0_L1_BYPASS             bit(3)              /* Bypass L1 PLL */
#define NODE_PLL0_SOFT_SET              bit(2)              /* 允许软件设置PLL */
#define NODE_PLL0_SEL_NODE              bit(0)              /* NODE时钟非软件bypass整个PLL */

/**
 * NODE PLL高64位配置寄存器
 */
#define NODE_PLL1_BASE                  0x1fe00488
#define NODE_PLL1_L2_DIV_OUT_MASK       0x3f                /* bit[5:0] L2 NODE PLL分频系数 */

/**
 * DDR PLL低64位配置寄存器
 */
#define DDR_PLL0_BASE                   0x1fe00490
#define DDR_PLL0_L1_DIV_LOOPC_MASK      (0x3fful<<32)       /* bit[41:32] L1 PLL倍频系数 */
#define DDR_PLL0_L1_DIV_LOOPC_SHIFT     32
#define DDR_PLL0_L1_DIV_REF_MASK        (0x3ful<<26)        /* bit[31:26] L1 PLL参考时钟分频系数 */
#define DDR_PLL0_L1_DIV_REF_SHIFT       26
#define DDR_PLL0_L1_PD                  bit(19)             /* L1 PLL关电控制, 1代表关电 */
#define DDR_PLL0_L1_LOCKED              bit(16)             /* L1 PLL锁定标志, 1代表锁定 */
#define DDR_PLL0_L1_LOCKC_MASK          (0x3ul<<10)         /* bit[11:10] 判定L1 PLL是否锁定使用的相位的精度 */
#define DDR_PLL0_L1_LOCK_EN             bit(7)              /* 允许锁定L1 PLL */
#define DDR_PLL0_L1_BYPASS              bit(3)              /* Bypass L1 PLL */
#define DDR_PLL0_SOFT_SET               bit(2)              /* 允许软件设置PLL */
#define DDR_PLL0_SEL_GPU                bit(1)              /* GPU/HDA时钟非软件bypass整个PLL */
#define DDR_PLL0_SEL_DDR                bit(0)              /* DDR时钟非软件bypass整个PLL */

/**
 * DDR PLL高64位配置寄存器
 */
#define DDR_PLL1_BASE                   0x1fe00498
#define DDR_PLL1_L2_DIV_OUT_HDA_MASK    (0x7ful<<44)        /* bit[50:44] L2 HDA PLL分频系数 */
#define DDR_PLL1_L2_DIV_OUT_HDA_SHIFT   44
#define DDR_PLL1_L2_DIV_OUT_GPU_MASK    (0x3ful<<22)        /* bit[27:22] L2 GPU PLL分频系数 */
#define DDR_PLL1_L2_DIV_OUT_GPU_SHIFT   22
#define DDR_PLL1_L2_DIV_OUT_MASK        0x3f                /* bit[5:0] L2 DDR PLL分频系数 */

/**
 * DC PLL低64位配置寄存器
 */
#define DC_PLL0_BASE                    0x1fe004a0
#define DC_PLL0_L1_DIV_LOOPC_MASK       (0x3fful<<32)       /* bit[41:32] L1 PLL倍频系数 */
#define DC_PLL0_L1_DIV_LOOPC_SHIFT      32
#define DC_PLL0_L1_DIV_REF_MASK         (0x3ful<<26)        /* bit[31:26] L1 PLL参考时钟分频系数 */
#define DC_PLL0_L1_DIV_REF_SHIFT        26
#define DC_PLL0_L1_PD                   bit(19)             /* L1 PLL关电控制, 1代表关电 */
#define DC_PLL0_L1_LOCKED               bit(16)             /* L1 PLL锁定标志, 1代表锁定 */
#define DC_PLL0_L1_LOCKC_MASK           (0x3ul<<10)         /* bit[11:10] 判定L1 PLL是否锁定使用的相位的精度 */
#define DC_PLL0_L1_LOCKC_SHIFT          10
#define DC_PLL0_L1_LOCK_EN              bit(7)              /* 允许锁定L1 PLL */
#define DC_PLL0_L1_BYPASS               bit(3)              /* Bypass L1 PLL */
#define DC_PLL0_SOFT_SET                bit(2)              /* 允许软件设置PLL */
#define DC_PLL0_SEL_GMAC                bit(1)              /* GMAC时钟非软件bypass整个PLL */
#define DC_PLL0_SEL_DC                  bit(0)              /* DC时钟非软件bypass整个PLL */

/**
 * DC PLL高64位配置寄存器
 */
#define DC_PLL1_BASE                    0x1fe004a8
#define DC_PLL1_L2_DIV_OUT_GMAC_MASK    (0x3ful<<22)        /* bit[27:22] L2 GMAC PLL分频系数 */
#define DC_PLL1_L2_DIV_OUT_GMAC_SHIFT   22
#define DC_PLL1_L2_DIV_OUT_MASK         0x3f                /* bit[5:0] L2 DC PLL分频系数 */

/**
 * PIX0 PLL低64位配置寄存器
 */
#define PIX0_PLL0_BASE                  0x1fe004b0
#define PIX0_PLL0_L1_DIV_LOOPC_MASK     (0x3fful<<32)       /* bit[41:32] L1 PLL倍频系数 */
#define PIX0_PLL0_L1_DIV_LOOPC_SHIFT    32
#define PIX0_PLL0_L1_DIV_REF_MASK       (0x3ful<<26)        /* bit[31:26] L1 PLL参考时钟分频系数 */
#define PIX0_PLL0_L1_DIV_REF_SHIFT      26
#define PIX0_PLL0_L1_PD                 bit(19)             /* L1 PLL关电控制, 1代表关电 */
#define PIX0_PLL0_L1_LOCKED             bit(16)             /* L1 PLL锁定标志, 1代表锁定 */
#define PIX0_PLL0_L1_LOCKC_MASK         (0x3ul<<10)         /* bit[11:10] 判定L1 PLL是否锁定使用的相位的精度 */
#define PIX0_PLL0_L1_LOCKC_SHIFT        10
#define PIX0_PLL0_L1_LOCK_EN            bit(7)              /* 允许锁定L1 PLL */
#define PIX0_PLL0_L1_BYPASS             bit(3)              /* Bypass L1 PLL */
#define PIX0_PLL0_SOFT_SET              bit(2)              /* 允许软件设置PLL */
#define PIX0_PLL0_SEL_PIX0              bit(0)              /* PIX0时钟非软件bypass整个PLL */

/**
 * PIX0 PLL高64位配置寄存器
 */
#define PIX0_PLL1_BASE                  0x1fe004b8
#define PIX0_PLL1_L2_DIV_OUT_MASK       0x3f                /* bit[5:0] L2 PIX0 PLL分频系数 */

/**
 * PIX1 PLL低64位配置寄存器
 */
#define PIX1_PLL0_BASE                  0x1fe004c0
#define PIX1_PLL0_L1_DIV_LOOPC_MASK     (0x3fful<<32)       /* bit[41:32] L1 PLL倍频系数 */
#define PIX1_PLL0_L1_DIV_LOOPC_SHIFT    32
#define PIX1_PLL0_L1_DIV_REF_MASK       (0x3ful<<26)        /* bit[31:26] L1 PLL参考时钟分频系数 */
#define PIX1_PLL0_L1_DIV_REF_SHIFT      26
#define PIX1_PLL0_L1_PD                 bit(19)             /* L1 PLL关电控制, 1代表关电 */
#define PIX1_PLL0_L1_LOCKED             bit(16)             /* L1 PLL锁定标志, 1代表锁定 */
#define PIX1_PLL0_L1_LOCKC_MASK         (0x3ul<<10)         /* bit[11:10] 判定L1 PLL是否锁定使用的相位的精度 */
#define PIX1_PLL0_L1_LOCKC_SHIFT        10
#define PIX1_PLL0_L1_LOCK_EN            bit(7)              /* 允许锁定L1 PLL */
#define PIX1_PLL0_L1_BYPASS             bit(3)              /* Bypass L1 PLL */
#define PIX1_PLL0_SOFT_SET              bit(2)              /* 允许软件设置PLL */
#define PIX1_PLL0_SEL_PIX1              bit(0)              /* PIX1时钟非软件bypass整个PLL */

/**
 * PIX1 PLL高64位配置寄存器
 */
#define PIX1_PLL1_BASE                  0x1fe004c8
#define PIX1_PLL1_L2_DIV_OUT_MASK       0x3f                /* bit[5:0] L2 PIX1 PLL分频系数 */

/**
 * FREQSCALE配置寄存器
 *
 * Freqscale分频计算公式为: f = fref *(freqscale + 1) / 8.
 *
 */
#define FREQSCALE_BASE                  0x1fe004d0
#define FREQSCALE_CORE1_EN              bit(33)             /* CPU核1时钟使能 */
#define FREQSCALE_CORE0_EN              bit(32)             /* CPU核0时钟使能 */
#define FREQSCALE_APB_MASK              (0x7ul<<20)         /* bit[22:20] apb时钟分频系数 */
#define FREQSCALE_APB_SHIFT             20
#define FREQSCALE_USB_MASK              (0x7ul<<16)         /* bit[18:16] usb时钟分频系数 */
#define FREQSCALE_USB_SHIFT             16
#define FREQSCALE_SATA_MASK             (0x7ul<<12)         /* bit[14:12] sata时钟分频系数 */
#define FREQSCALE_SATA_SHIFT            12
#define FREQSCALE_BOOT_MASK             (0x7ul<<8)          /* bit[10:8] boot时钟分频系数 */
#define FREQSCALE_BOOT_SHIFT            8
#define FREQSCALE_NODE_MASK             0x7                 /* bit[2:0] node时钟分频系数 */

/**
 * PCIE0配置寄存器0
 */
#define PCIE0_CFG_R0_BASE               0x1fe00580

/**
 * PCIE0配置寄存器1
 */
#define PCIE0_CFG_R1_BASE               0x1fe00588

/**
 * PCIE0 PHY配置控制寄存器
 */
#define PCIE0_CFG_PHY_BASE              0x1fe00590

/**
 * PCIE1配置寄存器0
 */
#define PCIE1_CFG_R0_BASE               0x1fe005a0

/**
 * PCIE1配置寄存器1
 */
#define PCIE1_CFG_R1_BASE               0x1fe005a8

/**
 * PCIE1 PHY配置控制寄存器
 */
#define PCIE1_CFG_PHY_BASE              0x1fe005b0

//-------------------------------------------------------------------------------------------------
// DMA
//-------------------------------------------------------------------------------------------------

#if 0
/**
 * DMA命令控制寄存器(dma_order). see "ls2k_dma_hw.h"
 */
#define DMA0_BASE                       0x1fe00c00
#define DMA1_BASE                       0x1fe00c10
#define DMA2_BASE                       0x1fe00c20
#define DMA3_BASE                       0x1fe00c30
#define DMA4_BASE                       0x1fe00c40

#define DMA_ASK_ADDR_MASK               (~0x3ful)           /* bit[63:5] 64位地址高59 */
#define DMA_STOP                        bit(4)              /* 停止DMA操作. DMA控制器完成当前数据读写后停止. */
#define DMA_START                       bit(3)              /* 开始DMA操作. DMA控制器读取描述符地址(ask_addr)后将该位清零. */
#define DMA_ASK_VALID                   bit(2)              /* DMA工作寄存器写回到(ask_addr)所指向的内存, 完成后清零. */
#define DMA_AXI_COHERENT                bit(1)              /* DMA访问地址一致性使能 */
#define DMA_64BIT                       bit(0)              /* DMA控制器64位地址支持 */
#endif

//-------------------------------------------------------------------------------------------------
// PCI
//-------------------------------------------------------------------------------------------------

/**
 * PCICFG2_RECFG寄存器
 */
#define PCI_CFG_APB_BASE                0x1fe03800

/**
 * PCICFG30_RECFG寄存器
 */
#define PCI_CFG_GMAC0_BASE              0x1fe03808

/**
 * PCICFG31_RECFG寄存器
 */
#define PCI_CFG_GMAC1_BASE              0x1fe03810

/**
 * PCICFG40_RECFG寄存器
 */
#define PCI_CFG_OTG_BASE                0x1fe03818

/**
 * PCICFG41_RECFG寄存器
 */
#define PCI_CFG_EHCI_BASE               0x1fe03820

/**
 * PCICFG42_RECFG寄存器
 */
#define PCI_CFG_OHCI_BASE               0x1fe03828

/**
 * PCICFG5_RECFG寄存器
 */
#define PCI_CFG_GPU_BASE                0x1fe03830

/**
 * PCICFG6_RECFG寄存器
 */
#define PCI_CFG_DC_BASE                 0x1fe03838

/**
 * PCICFG7_RECFG寄存器
 */
#define PCI_CFG_HDA_BASE                0x1fe03840

/**
 * PCICFG8_RECFG寄存器
 */
#define PCI_CFG_SATA_BASE               0x1fe03848

/**
 * PCICFGf_RECFG寄存器
 */
#define PCI_CFG_DMA_BASE                0x1fe03850

/**
 * PCICFG10_RECFG寄存器
 */
#define PCI_CFG_VPU_BASE                0x1fe03858

/**
 * PCICFG11_RECFG寄存器
 */
#define PCI_CFG_CAMERA_BASE             0x1fe03860

//-------------------------------------------------------------------------------------------------
// IO设备配置头
//-------------------------------------------------------------------------------------------------

#define APB_CFG_HEAD_BASE               0x800000FE00001000ul
#define GMAC0_CFG_HEAD_BASE             0x800000FE00001800ul
#define GMAC1_CFG_HEAD_BASE             0x800000FE00001900ul
#define OTG_CFG_HEAD_BASE               0x800000FE00002000ul
#define EHCI_CFG_HEAD_BASE              0x800000FE00002100ul
#define OHCI_CFG_HEAD_BASE              0x800000FE00002200ul
#define GPU_CFG_HEAD_BASE               0x800000FE00002800ul
#define DC_CFG_HEAD_BASE                0x800000FE00003000ul
#define HDA_CFG_HEAD_BASE               0x800000FE00003800ul
#define SATA_CFG_HEAD_BASE              0x800000FE00004000ul
#define PCIE00_CFG_HEAD_BASE            0x800000FE00004800ul
#define PCIE01_CFG_HEAD_BASE            0x800000FE00005000ul
#define PCIE02_CFG_HEAD_BASE            0x800000FE00005800ul
#define PCIE03_CFG_HEAD_BASE            0x800000FE00006000ul
#define PCIE10_CFG_HEAD_BASE            0x800000FE00006800ul
#define PCIE11_CFG_HEAD_BASE            0x800000FE00007000ul
#define DMA_CFG_HEAD_BASE               0x800000FE00007800ul

/**
 * PCI Type0类型的格式
 */
typedef struct _pci_type0
{
    volatile unsigned short VendorID;               // 0x00  RO     制造商ID号
    volatile unsigned short DeviceID;               // 0x02  RO     设备ID号
    volatile unsigned short Command;                // 0x04  RW     命令寄存器
    volatile unsigned short Status;                 // 0x06  RO/RW  状态寄存器
    volatile unsigned char  RevisionID;             // 0x08  RO     版本索引
    volatile unsigned char  ProgInf;                // 0x09  RO     编程接口
    volatile unsigned char  SubclassCode;           // 0x0A  RO     子类码
    volatile unsigned char  ClassCode;              // 0x0B  RO     类别码
    volatile unsigned char  CacheLineSize;          // 0x0C  RO     缓存行大小
    volatile unsigned char  LatencyTimer;           // 0x0D  RO     等待计时器
    volatile unsigned char  HeaderType;             // 0x0E  RO     配置头类型
    volatile unsigned char  BIST;                   // 0x0F  RW     内自建测试
    volatile unsigned long  BaseAddress0;           // 0x10  RW/RO  0号基址寄存器
    volatile unsigned long  BaseAddress1;           // 0x18  RW/RO  1号基址寄存器
    volatile unsigned long  BaseAddress2;           // 0x20  RW/RO  2号基址寄存器
    volatile unsigned int   CardbusCISPointer;      // 0x28  RW     CIS卡总线指针
    volatile unsigned char  SubsystemVendorID;      // 0x2C  RO     子系统供应商ID号
    volatile unsigned char  SubsystemID;            // 0x2D  RO     子系统版本ID号
    volatile unsigned short rsv1;
    volatile unsigned int   ExpROMBaseAddress;      // 0x30  RW     扩展ROM基地址寄存器
    volatile unsigned char  CapabilitiesPointer;    // 0x34  RW     扩展能力指针
    volatile unsigned char  rsv2[7];
    volatile unsigned char  InterruptLine;          // 0x3C  RW     中断线寄存器
    volatile unsigned char  InterruptPin;           // 0x3D  RO     中断引脚寄存器
    volatile unsigned char  MinGnt;                 // 0x3E  RW     最小突发期时间
    volatile unsigned char  MaxLat;                 // 0x3F  RW     获取PCI总线最大时间
} pci_type0_t;

/**
 * PCI Type1类型的格式
 */
typedef struct _pci_type1
{
    volatile unsigned short VendorID;               // 0x00  RO     制造商ID号
    volatile unsigned short DeviceID;               // 0x02  RO     设备ID号
    volatile unsigned short Command;                // 0x04  RW     命令寄存器
    volatile unsigned short Status;                 // 0x06  RO/RW  状态寄存器
    volatile unsigned char  RevisionID;             // 0x08  RO     版本索引
    volatile unsigned char  ProgInf;                // 0x09  RO     编程接口
    volatile unsigned char  SubclassCode;           // 0x0A  RO     子类码
    volatile unsigned char  ClassCode;              // 0x0B  RO     类别码
    volatile unsigned char  CacheLineSize;          // 0x0C  RO     缓存行大小
    volatile unsigned char  LatencyTimer;           // 0x0D  RO     等待计时器
    volatile unsigned char  HeaderType;             // 0x0E  RO     配置头类型
    volatile unsigned char  BIST;                   // 0x0F  RW     内自建测试
    volatile unsigned long  BaseAddress0;           // 0x10  RW/RO  0号基址寄存器
    volatile unsigned char  PrimaryBusNum;          // 0x18  RW     Primary Bus编号
    volatile unsigned char  SecondaryBusNum;        // 0x19  RW     Secondary Bus编号
    volatile unsigned char  SubordinateBusNum;      // 0x1A  RW     Subordinate Bus编号
    volatile unsigned char  rsv1;
    volatile unsigned char  IOBase;                 // 0x1C  RW/RO
    volatile unsigned char  IOLimit;                // 0X1D  RW/RO
    volatile unsigned short SecondaryLantencyTimer; // 0x1E  RO     Secondary Bus状态寄存器
    volatile unsigned short MemoryBase;             // 0x20  RW/RO  (Non-Prefetchable) Memory Base
    volatile unsigned short MemoryLimit;            // 0x22  RW/RO  (Non-Prefetchable) Memory Limit
    volatile unsigned short PrefetchableMemBase;    // 0x24  RW/RO  Prefetchable Memory Base
    volatile unsigned short PrefetchableMemLimit;   // 0x26  RW/RO  Prefetchable Memory Limit
    volatile unsigned int   PrefetchableMemBaseHi;  // 0x28  RW     Prefetchable Memory Base Upper 32bits
    volatile unsigned int   PrefetchableMemLimitHi; // 0x2C  RW     Prefetchable Memory Limit Upper 32bits
    volatile unsigned short IOBaseHi16;             // 0x30  RW     IO Base Upper 16 Bits
    volatile unsigned short IOLimitHi16;            // 0x32  RW     IO Limit Upper 16 Bits
    volatile unsigned char  CapabilitiesPointer;    // 0x34  RW     扩展能力指针
    volatile unsigned char  rsv2[3];
    volatile unsigned int   ExpROMBaseAddress;      // 0x38  RW     扩展ROM基地址寄存器
    volatile unsigned char  InterruptLine;          // 0x3C  RW     中断线寄存器
    volatile unsigned char  InterruptPin;           // 0x3D  RO     中断引脚寄存器
    volatile unsigned short BridgeControl;          // 0x3E  RW/RO
} pci_type1_t;

/*
    APB设备地址译码

    ---------------------------------------------------------------------------
        地址[15:12] |   设备        |   备注
    ---------------------------------------------------------------------------
        0x0         | UART0~UART11  | 用地址的[11:8]做下一级地址译码：
                    | CAN0/CAN1     | 0x0：UART0
                    |               | 0x1：UART1
                    |               | 0x2：UART2
                    |               | 0x3：UART3
                    |               | 0x4：UART4
                    |               | 0x5：UART5
                    |               | 0x6：UART6
                    |               | 0x7：UART7
                    |               | 0x8：UART8
                    |               | 0x9：UART9
                    |               | 0xA：UART10
                    |               | 0xB：UART11
                    |               | 0xC：CAN0
                    |               | 0xD：CAN1
    ---------------------------------------------------------------------------
        0x1         | I2C0/I2C1     | 用地址[11]做下一级地址译码：
                    |               | 0x0：I2C0
                    |               | 0x1：I2C1
    ---------------------------------------------------------------------------
        0x2         | PWM           |
    ---------------------------------------------------------------------------
        0x4         | HPET          |
    ---------------------------------------------------------------------------
        0x6         | NAND          |
    ---------------------------------------------------------------------------
        0x7         | ACPI/RTC      | ACPI的偏移地址为 0x7000
                    |               | RTC 的偏移地址为 0x7800
    ---------------------------------------------------------------------------
        0x8         | DES           |
    ---------------------------------------------------------------------------
        0x9         | AES           |
    ---------------------------------------------------------------------------
        0xA         | RSA           |
    ---------------------------------------------------------------------------
        0xB         | RNG           |
    ---------------------------------------------------------------------------
        0xC         | SDIO          |
    ---------------------------------------------------------------------------
        0xD         | I2S           |
    ---------------------------------------------------------------------------
        0xE         | E1            |
    ---------------------------------------------------------------------------

 */

//-------------------------------------------------------------------------------------------------
// 核间中断
//-------------------------------------------------------------------------------------------------

/*
 * 0号处理器核核间中断与通信寄存器列表
 */
#define CORE0_IPISR                     0x1fe01000      // R    0号处理器核的IPI_Status
#define CORE0_IPIEN                     0x1fe01004      // RW   0号处理器核的IPI_Enalbe
#define CORE0_IPISET                    0x1fe01008      // W    0号处理器核的IPI_Set
#define CORE0_IPICLR                    0x1fe0100c      // W    0号处理器核的IPI_Clear
#define CORE0_BUF0                      0x1fe01020      // RW   0号处理器核的IPI_MailBox0
#define CORE0_BUF1                      0x1fe01028      // RW   0号处理器核的IPI_MailBox1
#define CORE0_BUF2                      0x1fe01030      // RW   0号处理器核的IPI_MailBox2
#define CORE0_BUF3                      0x1fe01038      // RW   0号处理器核的IPI_MailBox3
#define CORE0_INTISR0                   0x1fe01040      // RO   路由给CORE0的低32位中断状态
#define CORE0_INTISR1                   0x1fe01048      // RO   路由给CORE0的高32位中断状态

/*
 * 1号处理器核的核间中断与通信寄存器列表
 */
#define CORE1_IPISR                     0x1fe01100      // R    1号处理器核的IPI_Status
#define CORE1_IPIEN                     0x1fe01104      // RW   1号处理器核的IPI_Enalbe
#define CORE1_IPISET                    0x1fe01108      // W    1号处理器核的IPI_Set
#define CORE1_IPICLR                    0x1fe0110c      // W    1号处理器核的IPI_Clear
#define CORE1_BUF0                      0x1fe01120      // RW   1号处理器核的IPI_MailBox0
#define CORE1_BUF1                      0x1fe01128      // RW   1号处理器核的IPI_MailBox1
#define CORE1_BUF2                      0x1fe01130      // RW   1号处理器核的IPI_MailBox2
#define CORE1_BUF3                      0x1fe01138      // RW   1号处理器核的IPI_MailBox3
#define CORE1_INTISR0                   0x1fe01140      // RO   路由给CORE1的低32位中断状态
#define CORE1_INTISR1                   0x1fe01148      // RO   路由给CORE1的高32位中断状态

//-------------------------------------------------------------------------------------------------
// 中断
//-------------------------------------------------------------------------------------------------

/*
 * 中断控制0 32bits
 */
#define INTC0_SR_BASE                   0x1fe01420
#define INTC0_EN_BASE                   0x1fe01424
#define INTC0_SET_BASE                  0x1fe01428
#define INTC0_CLR_BASE                  0x1fe0142c
#define INTC0_POL_BASE                  0x1fe01430
#define INTC0_EDGE_BASE                 0x1fe01434
#define INTC0_BOUNCE_BASE               0x1fe01438
#define INTC0_AUTO_BASE                 0x1fe0143c

#define INTC0_UART0_3_BIT               bit(0)
#define INTC0_UART4_7_BIT               bit(1)
#define INTC0_UART8_11_BIT              bit(2)
#define INTC0_E1_BIT                    bit(3)
#define INTC0_HDA_BIT                   bit(4)
#define INTC0_I2S_BIT                   bit(5)
#define INTC0_THSENS_BIT                bit(7)
#define INTC0_TOY_TICK_BIT              bit(8)
#define INTC0_RTC_TICK_BIT              bit(9)
#define INTC0_CAMERA_BIT                bit(10)
#define INTC0_GMAC0_SBD_BIT             bit(12)
#define INTC0_GMAC0_PMT_BIT             bit(13)
#define INTC0_GMAC1_SBD_BIT             bit(14)
#define INTC0_GMAC1_PMT_BIT             bit(15)
#define INTC0_CAN0_BIT                  bit(16)
#define INTC0_CAN1_BIT                  bit(17)
#define INTC0_SPI_BIT                   bit(18)
#define INTC0_SATA_BIT                  bit(19)
#define INTC0_NAND_BIT                  bit(20)
#define INTC0_HPET0_BIT                 bit(21)
#define INTC0_I2C0_BIT                  bit(22)
#define INTC0_I2C1_BIT                  bit(23)
#define INTC0_PWM0_BIT                  bit(24)
#define INTC0_PWM1_BIT                  bit(25)
#define INTC0_PWM2_BIT                  bit(26)
#define INTC0_PWM3_BIT                  bit(27)
#define INTC0_DC_BIT                    bit(28)
#define INTC0_GPU_BIT                   bit(29)
#define INTC0_VPU_BIT                   bit(30)
#define INTC0_SDIO_BIT                  bit(31)

/*
 * 中断控制1 32bits
 */
#define INTC1_SR_BASE                   0x1fe01460
#define INTC1_EN_BASE                   0x1fe01464
#define INTC1_SET_BASE                  0x1fe01468
#define INTC1_CLR_BASE                  0x1fe0146c
#define INTC1_POL_BASE                  0x1fe01470
#define INTC1_EDGE_BASE                 0x1fe01474
#define INTC1_BOUNCE_BASE               0x1fe01478
#define INTC1_AUTO_BASE                 0x1fe0147c

#define INTC1_PCIE00_BIT                bit(0)
#define INTC1_PCIE01_BIT                bit(1)
#define INTC1_PCIE02_BIT                bit(2)
#define INTC1_PCIE03_BIT                bit(3)
#define INTC1_PCIE10_BIT                bit(4)
#define INTC1_PCIE11_BIT                bit(5)
#define INTC1_HPET1_BIT                 bit(6)
#define INTC1_HPET2_BIT                 bit(7)
#define INTC1_TOY0_BIT                  bit(8)
#define INTC1_TOY1_BIT                  bit(9)
#define INTC1_TOY2_BIT                  bit(10)
#define INTC1_TOY3_BIT                  bit(11)
#define INTC1_DMA0_BIT                  bit(12)
#define INTC1_DMA1_BIT                  bit(13)
#define INTC1_DMA2_BIT                  bit(14)
#define INTC1_DMA3_BIT                  bit(15)
#define INTC1_DMA4_BIT                  bit(16)
#define INTC1_OTG_BIT                   bit(17)
#define INTC1_EHCI_BIT                  bit(18)
#define INTC1_OHCI_BIT                  bit(19)
#define INTC1_RTC0_BIT                  bit(20)
#define INTC1_RTC1_BIT                  bit(21)
#define INTC1_RTC2_BIT                  bit(22)
#define INTC1_RSA_BIT                   bit(23)
#define INTC1_AES_BIT                   bit(24)
#define INTC1_DES_BIT                   bit(25)
#define INTC1_GPIO4_31_BIT              bit(26)
#define INTC1_GPIO32_63_BIT             bit(27)
#define INTC1_GPIO0_BIT                 bit(28)
#define INTC1_GPIO1_BIT                 bit(29)
#define INTC1_GPIO2_BIT                 bit(30)
#define INTC1_GPIO3_BIT                 bit(31)

/**
 * 中断路由
 */
#define INTC0_ROUTE_BASE                0x1fe01400
#define INTC1_ROUTE_BASE                0x1fe01440

#define INT_ROUTE_CORE0                 0x01
#define INT_ROUTE_CORE1                 0x02

#define INT_ROUTE_IP0                   0x10
#define INT_ROUTE_IP1                   0x20
#define INT_ROUTE_IP2                   0x40
#define INT_ROUTE_IP3                   0x80

/**
 * 中断触发模式
 */
#define INT_TRIGGER_LEVEL               0x04            /* 电平触发中断 */
#define INT_TRIGGER_PULSE               0x08            /* 脉冲触发中断 */

//-------------------------------------------------------------------------------------------------
// GPIO
//-------------------------------------------------------------------------------------------------

#define GPIO_OEN_BASE                   0x1fe00500      /* 64位, 低有效 */
#define GPIO_OUT_BASE                   0x1fe00510      /* 64位, 输出值 */
#define GPIO_IN_BASE                    0x1fe00520      /* 64位, 输入值 */
#define GPIO_INTEN_BASE                 0x1fe00530      /* 64位, 中断使能 */

#endif // _LS2K1000LA_H

//-------------------------------------------------------------------------------------------------

/*
 * @@ END
 */

