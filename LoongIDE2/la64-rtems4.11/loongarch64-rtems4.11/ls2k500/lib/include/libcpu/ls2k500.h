/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k500.h
 *
 * created: 2022-02-18
 *  author: Bian
 */

#ifndef _LS2K500_H
#define _LS2K500_H

#define bit(x)                  (1<<(x))

//-------------------------------------------------------------------------------------------------

/*
 * 使用扩展中断时共有 128 个中断可响应
 * 使用标准中断时共有  64 个中断可响应
 */
#define USE_EXTINT              1           /* 1: 使用扩展中断, 0: 使用标准中断  */

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

//#define HI32(v)                 ((unsigned int)((v >> 32) & 0xFFFFFFFFul))
//#define LO32(v)                 ((unsigned int)v & 0xFFFFFFFFul)

//-------------------------------------------------------------------------------------------------

/**
 * Chip Control
 */
#define CHIP_CTRL0_BASE                 0x1fe10100
#define CTRL0_USB_CLK_MODE_MASK         0x03                // bit[31:30]
#define CTRL0_USB_CLK_MODE_SHIFT        30
#define CTRL0_LPC_SLAVE_EN              bit(29)
#define CTRL0_EXTIOINT_EN               bit(27)
#define CTRL0_CONF_RTC_RESTART          bit(26)
#define CTRL0_RTC_DS_MASK               0x07                // bit[25:23]
#define CTRL0_RTC_DS_SHIFT              23
#define CTRL0_EJTAG_CONFIG_MASK         0x07                // bit[20:19]
#define CTRL0_EJTAG_CONFIG_SHIFT        19
#define CTRL0_GMAC_TEST_LPBK            bit(18)
#define CTRL0_GMAC1_MII_SEL             bit(17)
#define CTRL0_GMAC0_MII_SEL             bit(16)
#define CTRL0_SDIO1_DMA_SEL_MASK        0x03                // bit[15:14]
#define CTRL0_SDIO1_DMA_SEL_SHIFT       14
#define CTRL0_PCIE_CLK_DIV              bit(13)
#define CTRL0_DDR3_REGS_DEFAULT         bit(12)
#define CTRL0_DDR3_REGS_DIS             bit(11)
#define CTRL0_HDA_CONF_CC               bit(10)
#define CTRL0_HDA_SEL                   bit(9)
#define CTRL0_HDA_BCLKSEL               bit(8)
#define CTRL0_UART1_EN_MASK             0x0F                // bit[7:4]
#define CTRL0_UART1_EN_SHIFT            4
#define CTRL0_UART0_EN_MASK             0x0F                // bit[3:0]

#define CHIP_CTRL1_BASE                 0x1fe10104
#define CTRL1_LIO_ROM_WIDTH16           bit(28)
#define CTRL1_LIO_ROM_COUNT_INIT_MASK   0x1F                // bit[27:23]
#define CTRL1_LIO_ROM_COUNT_INIT_SHIFT  23
#define CTRL1_LIO_CLK_PERIOD_MASK       0x03                // bit[22:21]
#define CTRL1_LIO_CLK_PERIOD_SHIFT      21
#define CTRL1_PCIE_CC                   bit(18)
#define CTRL1_IODMA_SPARE_RD_MASK       0xFF                // bit[17:10]
#define CTRL1_IODMA_SPARE_RD_SHIFT      10
#define CTRL1_USB_FLUSH_IDLE_MASK       0x0F                // bit[9:6]
#define CTRL1_USB_FLUSH_IDLE_SHIFT      6
#define CTRL1_USB_PREFETCH              bit(5)
#define CTRL1_USB_FLUSH_WR              bit(4)
#define CTRL1_USB_STOP_WAW              bit(3)
#define CTRL1_USB_STOP_RAW              bit(2)
#define CTRL1_USB_OTG_SEL               bit(1)
#define CTRL1_USB_CC                    bit(0)

#define CHIP_CTRL2_BASE                 0x1fe10108
#define CTRL2_APB_DMA_ORDER_EN          bit(29)
#define CTRL2_PRINT_ORDER_EN            bit(28)
#define CTRL2_HDA_ORDER_EN              bit(27)
#define CTRL2_USB3_ORDER_EN             bit(26)
#define CTRL2_USB2_ORDER_EN             bit(25)
#define CTRL2_SATA_ORDER_EN             bit(24)
#define CTRL2_GMAC1_ORDER_EN            bit(23)
#define CTRL2_GMAC0_ORDER_EN            bit(22)
#define CTRL2_DC_ORDER_EN               bit(21)
#define CTRL2_GPU_ORDER_EN              bit(20)
#define CTRL2_PCIE_ORDER_EN             bit(19)
#define CTRL2_PCI_ORDER_EN              bit(18)
#define CTRL2_IOREGS_ORDER_EN           bit(17)
#define CTRL2_CPU_ORDER_EN              bit(16)
#define CTRL2_RTC_TIMER_HSPEED          bit(14)
#define CTRL2_GPU_DISABLE_RAMCG         bit(13)
#define CTRL2_PCIE_SOFT_RESET           bit(12)
#define CTRL2_SATA_SOFT_RESET           bit(11)
#define CTRL2_USB3_SOFT_RESET           bit(10)
#define CTRL2_USB_SOFT_RESET            bit(9)
#define CTRL2_GPU_SOFT_RESET            bit(8)
#define CTRL2_LPC_ROM_8MBITS            bit(5)

#define CHIP_CTRL3_BASE                 0x1fe1010c
#define CTRL3_APB_DMA_READ_UPGRADE      bit(27)
#define CTRL3_PRINT_READ_UPGRADE        bit(26)
#define CTRL3_HDA_READ_UPGRADE          bit(25)
#define CTRL3_USB3_READ_UPGRADE         bit(24)
#define CTRL3_USB2_READ_UPGRADE         bit(23)
#define CTRL3_SATA_READ_UPGRADE         bit(22)
#define CTRL3_GMAC1_READ_UPGRADE        bit(21)
#define CTRL3_GMAC0_READ_UPGRADE        bit(20)
#define CTRL3_DC_READ_UPGRADE           bit(19)
#define CTRL3_GPU_READ_UPGRADE          bit(18)
#define CTRL3_PCIE_READ_UPGRADE         bit(17)
#define CTRL3_PCI_READ_UPGRADE          bit(16)
#define CTRL3_APB_DMA_WRITE_UPGRADE     bit(11)
#define CTRL3_PRINT_WRITE_UPGRADE       bit(10)
#define CTRL3_HDA_WRITE_UPGRADE         bit(9)
#define CTRL3_USB3_WRITE_UPGRADE        bit(8)
#define CTRL3_USB2_WRITE_UPGRADE        bit(7)
#define CTRL3_SATA_WRITE_UPGRADE        bit(6)
#define CTRL3_GMAC1_WRITE_UPGRADE       bit(5)
#define CTRL3_GMAC0_WRITE_UPGRADE       bit(4)
#define CTRL3_DC_WRITE_UPGRADE          bit(3)
#define CTRL3_GPU_WRITE_UPGRADE         bit(2)
#define CTRL3_PCIE_WRITE_UPGRADE        bit(1)
#define CTRL3_PCI_WRITE_UPGRADE         bit(0)

#define CHIP_CTRL4_BASE                 0x1fe10110
#define CTRL4_GPU_TRANS_EN              bit(31)
#define CTRL4_GPU_TRANS_MASK            0x0F                // bit[28:25]
#define CTRL4_GPU_TRANS_SHIFT           25
#define CTRL4_GPU_TRANS_ADDR_MASK       0x0F                // bit[24:21]
#define CTRL4_GPU_TRANS_ADDR_SHIFT      21
#define CTRL4_IO_COHERENT_EN            bit(16)
#define CTRL4_DMA_COHERENT              bit(13)
#define CTRL4_PRINT_COHERENT            bit(12)
#define CTRL4_HDA_COHERENT              bit(11)
#define CTRL4_USB3_COHERENT             bit(10)
#define CTRL4_USB2_COHERENT             bit(9)
#define CTRL4_SATA_COHERENT             bit(8)
#define CTRL4_GMAC1_COHERENT            bit(7)
#define CTRL4_GMAC0_COHERENT            bit(6)
#define CTRL4_DC_COHERENT               bit(5)
#define CTRL4_GPU_COHERENT              bit(4)
#define CTRL4_PCIE_COHERENT             bit(3)
#define CTRL4_PCI_COHERENT              bit(2)

#define CHIP_CTRL5_BASE                 0x1fe10114

#define CHIP_SAMP0_BASE                 0x1fe10120
#define SAMP0_PCIX_SPEED_MASK           0x03                // bit[15:14]
#define SAMP0_PCIX_SPEED_SHIFT          14
#define SAMP0_PCIX_MODE                 bit(13)
#define SAMP0_PCI_HOST_MODE             bit(12)
#define SAMP0_PCIE1_EP                  bit(11)
#define SAMP0_PCI_ARB_EXT               bit(10)
#define SAMP0_PCIE_REFCLK_SEL           bit(9)
#define SAMP0_CLK_SEL_MASK              0x03                // bit[8:7]
#define SAMP0_CLK_SEL_SHIFT             7
#define SAMP0_PCIE0_EP                  bit(6)
#define SAMP0_NAND_BOOT_ECC             bit(5)
#define SAMP0_NAND_TYPE_MASK            0x03                // bit[4:3]
#define SAMP0_NAND_TYPE_SHIFT           3
#define SAMP0_BOOT_SEL_MASK             0x07                // bit[2:0]

#define CHIP_HPT0_BASE                  0x1fe10130          /* == rdtimer.d */
#define CHIP_HPT1_BASE                  0x1fe10134

/**
 * PLL Control
 */
#define PLL_NODE0_BASE                  0x1fe10400
#define NODE0_DIV_MASK                  0x3F                // bit[29:24]
#define NODE0_DIV_SHIFT                 24
#define NODE0_DIV_LOOPC_MASK            0xFF                // bit[23:16]
#define NODE0_DIV_LOOPC_SHIFT           16
#define NODE0_DIV_REFC_MASK             0x3F                // bit[13:8]
#define NODE0_DIV_REFC_SHIFT            8
#define NODE0_PLL_LOCKED                bit(7)
#define NODE0_PD_PLL                    bit(5)
#define NODE0_BYPASS                    bit(4)
#define NODE0_PLL_SOFT_RESET            bit(3)
#define NODE0_PLL_SEL                   bit(0)

#define PLL_DDR0_BASE                   0x1fe10408
#define DDR0_DIV_MASK                   0x3F                // bit[29:24]
#define DDR0_DIV_SHIFT                  24
#define DDR0_DIV_LOOPC_MASK             0xFF                // bit[23:16]
#define DDR0_DIV_LOOPC_SHIFT            16
#define DDR0_DIV_REFC_MASK              0x3F                // bit[13:8]
#define DDR0_DIV_REFC_SHIFT             8
#define DDR0_PLL_LOCKED                 bit(7)
#define DDR0_PD_PLL                     bit(5)
#define DDR0_BYPASS                     bit(4)
#define DDR0_PLL_SOFT_RESET             bit(3)
#define DDR0_PLL_SEL_HDA                bit(2)
#define DDR0_PLL_SEL_NETWORK            bit(1)
#define DDR0_PLL_SEL_DDR                bit(0)

#define PLL_DDR1_BASE                   0x1fe1040c
#define DDR1_DIV_HDA_MASK               0x3F                // bit[13:8]
#define DDR1_DIV_HDA_SHIFT              8
#define DDR1_DIV_NETWORK_MASK           0x3F                // bit[5:0]

#define PLL_SOC0_BASE                   0x1fe10410
#define SOC0_DIV_GPU_MASK               0x3F                // bit[29:24]
#define SOC0_DIV_GPU_SHIFT              24
#define SOC0_DIV_LOOPC_MASK             0xFF                // bit[23:16]
#define SOC0_DIV_LOOPC_SHIFT            16
#define SOC0_DIV_REFC_MASK              0x3F                // bit[13:8]
#define SOC0_DIV_REFC_SHIFT             8
#define SOC0_PLL_LOCKED                 bit(7)
#define SOC0_PD_PLL                     bit(5)
#define SOC0_BYPASS                     bit(4)
#define SOC0_PLL_SOFT_RESET             bit(3)
#define SOC0_PLL_SEL_GMAC               bit(2)
#define SOC0_PLL_SEL_SB                 bit(1)
#define SOC0_PLL_SEL_GPU                bit(0)

#define PLL_SOC1_BASE                   0x1fe10414
#define SOC1_DIV_GMAC_MASK              0x3F                // bit[13:8]
#define SOC1_DIV_GMAC_SHIFT             8
#define SOC1_DIV_SB_MASK                0x3F                // bit[5:0]

#define PLL_PIX0_BASE                   0x1fe10418
#define PIX0_DIV_MASK                   0x3F                // bit[29:24]
#define PIX0_DIV_SHIFT                  24
#define PIX0_DIV_LOOPC_MASK             0xFF                // bit[23:16]
#define PIX0_DIV_LOOPC_SHIFT            16
#define PIX0_DIV_REFC_MASK              0x3F                // bit[13:8]
#define PIX0_DIV_REFC_SHIFT             8
#define PIX0_PLL_LOCKED                 bit(7)
#define PIX0_PD_PLL                     bit(5)
#define PIX0_BYPASS                     bit(4)
#define PIX0_PLL_SOFT_RESET             bit(3)
#define PIX0_PLL_SEL                    bit(0)

#define PLL_PIX1_BASE                   0x1fe10420
#define PIX1_DIV_MASK                   0x3F                // bit[29:24]
#define PIX1_DIV_SHIFT                  24
#define PIX1_DIV_LOOPC_MASK             0xFF                // bit[23:16]
#define PIX1_DIV_LOOPC_SHIFT            16
#define PIX1_DIV_REFC_MASK              0x3F                // bit[13:8]
#define PIX1_DIV_REFC_SHIFT             8
#define PIX1_PLL_LOCKED                 bit(7)
#define PIX1_PD_PLL                     bit(5)
#define PIX1_BYPASS                     bit(4)
#define PIX1_PLL_SOFT_RESET             bit(3)
#define PIX1_PLL_SEL                    bit(0)

#define PLL_FREQSCALE_BASE              0x1fe10428
#define FREQSCALE_LSU_MASK              0x1F                // bit[31:27]
#define FREQSCALE_LSU_SHIFT             27
#define FREQSCALE_PRINT_MASK            0x07                // bit[26:24]
#define FREQSCALE_PRINT_SHIFT           24
#define FREQSCALE_APB_MASK              0x07                // bit[22:20]
#define FREQSCALE_APB_SHIFT             20
#define FREQSCALE_USB_MASK              0x07                // bit[18:16]
#define FREQSCALE_USB_SHIFT             16
#define FREQSCALE_SATA_MASK             0x07                // bit[14:12]
#define FREQSCALE_SATA_SHIFT            12
#define FREQSCALE_SB_MASK               0x07                // bit[10:8]
#define FREQSCALE_SB_SHIFT              8
#define FREQSCALE_GPU_MASK              0x07                // bit[6:4]
#define FREQSCALE_GPU_SHIFT             4
#define FREQSCALE_NODE_MASK             0x07                // bit[2:0]

/**
 * GPIO Control 32bits
 */
#define GPIO0_31_OEN_BASE               0x1fe10430
#define GPIO0_31_IN_BASE                0x1fe10438
#define GPIO0_31_OUT_BASE               0x1fe10440

#define GPIO32_63_OEN_BASE              0x1fe10434
#define GPIO32_63_IN_BASE               0x1fe1043c
#define GPIO32_63_OUT_BASE              0x1fe10444

#define GPIO64_95_OEN_BASE              0x1fe10450
#define GPIO64_95_IN_BASE               0x1fe10458
#define GPIO64_95_OUT_BASE              0x1fe10460

#define GPIO96_127_OEN_BASE             0x1fe10454
#define GPIO96_127_IN_BASE              0x1fe1045c
#define GPIO96_127_OUT_BASE             0x1fe10464

#define GPIO128_154_OEN_BASE            0x1fe10470
#define GPIO128_154_IN_BASE             0x1fe10478
#define GPIO128_154_OUT_BASE            0x1fe10480

/*
 * n>=0 && n<=5
 */
#define GPIO_OEN_ADDR(n)    ((n<=1) ? (GPIO0_31_OEN_BASE+n*4) : (n<=3) ? (GPIO64_95_OEN_BASE+(n-2)*4) : GPIO128_154_OEN_BASE)
#define GPIO_IN_ADDR(n)     ((n<=1) ? (GPIO0_31_IN_BASE +n*4) : (n<=3) ? (GPIO64_95_IN_BASE +(n-2)*4) : GPIO128_154_IN_BASE )
#define GPIO_OUT_ADDR(n)    ((n<=1) ? (GPIO0_31_OUT_BASE+n*4) : (n<=3) ? (GPIO64_95_OUT_BASE+(n-2)*4) : GPIO128_154_OUT_BASE)

#define GPIOx_MUX_BASE                  0x1fe10490
#define GPIO_MUX_ADDR(n)                (GPIOx_MUX_BASE+n*4)    /* n>=0 && n<=19, 每寄存器8个 */

#define GPIOx_INTEN_BASE                0x1fe104e0
#define GPIO_INTENx_ADDR(n)             (GPIOx_INTEN_BASE+n*4)  /* n>=0 && n<=3, 每寄存器32个 */

/*
 * 可用的 GPIO 总数
 */
#define GPIO_ALL_COUNT                  155

/*
 * 可用中断的 GPIO 总数
 */
#if (!USE_EXTINT)
#define GPIO_INT_COUNT                  128                     
#else
#define GPIO_INT_COUNT                  124
#endif

/**
 * USB Phy
 */
#define USB_PHYx_BASE                   0x1fe10500
#define USB_PHY_ADDR(n)                 (USB_PHYx_BASE+n*4)     /* n>=0 && n<=3 */

#define USB_PHY0_CFG_FS_DATA_MOD        bit(30)
#define USB_PHY_COMMONPOWERDOWN         bit(29)
#define USB_PHY_DMPULLDOWN              bit(28)
#define USB_PHY_DPPULLDOWN              bit(27)
#define USB_PHY_TXRESTUNE_MASK          0x03                /* bit[26:25] */
#define USB_PHY_TXRESTUNE_SHIFT         25
#define USB_PHY_TXPREEMPPULSETUNE       bit(24)
#define USB_PHY_TXPREEMPAMPTUNE_MASK    0x03                /* bit[23:22] */
#define USB_PHY_TXPREEMPAMPTUNE_SHIFT   22
#define USB_PHY_TXHSXVTUNE_MASK         0x03                /* bit[21:20] */
#define USB_PHY_TXHSXVTUNE_SHIFT        20
#define USB_PHY_TXRISETUNE_MASK         0x03                /* bit[19:18] */
#define USB_PHY_TXRISETUNE_SHIFT        18
#define USB_PHY_TXVREFTUNE_MASK         0x0F                /* bit[17:14] */
#define USB_PHY_TXVREFTUNE_SHIFT        14
#define USB_PHY_TXFSLSTUNE_MASK         0x0F                /* bit[13:10] */
#define USB_PHY_TXFSLSTUNE_SHIFT        10
#define USB_PHY_SQRXTUNE_MASK           0x07                /* bit[9:7] */
#define USB_PHY_SQRXTUNE_SHIFT          7
#define USB_PHY_COMPDISTUNE_MASK        0x07                /* bit[6:4] */
#define USB_PHY_COMPDISTUNE_SHIFT       4
#define USB_PHY_OTGTUNE_MASK            0x07                /* bit[3:1] */
#define USB_PHY_OTGTUNE_SHIFT           1
#define USB_PHY_CFG_EN                  bit(0)

/**
 * PCIE Cfg
 */
#define PCIE0_REG0_BASE                         0x1fe10530
#define PICE_REG0_PCS_TX_DEEMPH_GEN1_4_0_MASK   0x1F        /* bit[31:27] */
#define PICE_REG0_PCS_TX_DEEMPH_GEN1_4_0_SHIFT  27
#define PICE_REG0_PHY_RX3_EQ_MASK               0x07        /* bit[26:24] */
#define PICE_REG0_PHY_RX3_EQ_SHIFT              24
#define PICE_REG0_PHY_RX2_EQ_MASK               0x07        /* bit[23:21] */
#define PICE_REG0_PHY_RX2_EQ_SHIFT              21
#define PICE_REG0_PHY_RX1_EQ_MASK               0x07        /* bit[20:18] */
#define PICE_REG0_PHY_RX1_EQ_SHIFT              18
#define PICE_REG0_PHY_RX0_EQ_MASK               0x07        /* bit[17:15] */
#define PICE_REG0_PHY_RX0_EQ_SHIFT              15
#define PICE_REG0_PCS_COMMON_CLOCKS             bit(14)
#define PICE_REG0_PCS_CLK_REQ                   bit(13)
#define PICE_REG0_PHY_MPLL_MULTIPLIER_MASK      0x7F        /* bit[11:5] */
#define PICE_REG0_PHY_MPLL_MULTIPLIER_SHIFT     5
#define PICE_REG0_APP_REQ_RETRY_EN              bit(3)
#define PICE_REG0_DO_SLEEP                      bit(2)
#define PICE_REG0_POWER_FAULT                   bit(1)
#define PICE_REG0_SLOT_WAKE_n                   bit(0)

#define PCIE0_REG1_BASE                          0x1fe10534
#define PCIE_REG1_PHY_TX0_TERM_OFFSET_MASK       0x1F       /* bit[31:27] */
#define PCIE_REG1_PHY_TX0_TERM_OFFSET_SHIFT      27
#define PCIE_REG1_PCS_TX_SWING_LOW_MASK          0x7F       /* bit[26:20] */
#define PCIE_REG1_PCS_TX_SWING_LOW_SHIFT         20
#define PCIE_REG1_PCS_TX_SWING_FULL_MASK         0x7F       /* bit[19:13] */
#define PCIE_REG1_PCS_TX_SWING_FULL_SHIFT        13
#define PCIE_REG1_PCS_TX_DEEMPH_GEN2_6DB_MASK    0x3F       /* bit[12:7] */
#define PCIE_REG1_PCS_TX_DEEMPH_GEN2_6DB_SHIFT   7
#define PCIE_REG1_PCS_TX_DEEMPH_GEN2_3P5DB_MASK  0x3F       /* bit[6:1] */
#define PCIE_REG1_PCS_TX_DEEMPH_GEN2_3P5DB_SHIFT 7
#define PCIE_REG1_PCS_TX_DEEMPH_GEN1_5           bit(0)

#define PCIE0_REG2_BASE                         0x1fe10538
#define PCIE_REG2_PHY_POWERDOWN                 bit(24)
#define PCIE_REG2_REFCLK_EN_MASK                0x0F        /* bit[23:20] */
#define PCIE_REG2_REFCLK_EN_SHIFT               20
#define PCIE_REG2_PHY_RTUNE_REQ                 bit(19)
#define PCIE_REG2_VREG_BYPASS                   bit(18)
#define PCIE_REG2_PHY_TX_VBOOST_LVL_MASK        0x07        /* bit[17:15] */
#define PCIE_REG2_PHY_TX_VBOOST_LVL_SHIFT       15
#define PCIE_REG2_PHY_TX3_TERM_OFFSET_MASK      0x1F        /* bit[14:10] */
#define PCIE_REG2_PHY_TX3_TERM_OFFSET_SHIFT     10
#define PCIE_REG2_PHY_TX2_TERM_OFFSET_MASK      0x1F        /* bit[9:5] */
#define PCIE_REG2_PHY_TX2_TERM_OFFSET_SHIFT     5
#define PCIE_REG2_PHY_TX1_TERM_OFFSET_MASK      0x1F        /* bit[4:0] */

#define PCIE0_PHY0_BASE                         0x1fe10540
#define PCIE_PHY0_PHY_CFG_DATA_MASK             0xFFFF      /* bit[31:16] */
#define PCIE_PHY0_PHY_CFG_DATA_SHIFT            16
#define PCIE_PHY0_PHY_CFG_ADDR_MASK             0xFFFF      /* bit[15:0] */

#define PCIE0_PHY1_BASE                         0x1fe10544
#define PCIE_PHY1_CFG_RESET                     bit(6)
#define PCIE_PHY1_CFG_STATE_MASK                0x07        /* bit[5:3] */
#define PCIE_PHY1_CFG_STATE_SHIFT               3
#define PCIE_PHY1_CFG_DONE                      bit(2)
#define PCIE_PHY1_CFG_DISABLE                   bit(1)
#define PCIE_PHY1_CFG_RW                        bit(0)

#define PCIE1_REG0_BASE                         0x1fe10550
#define PCIE1_REG1_BASE                         0x1fe10554
#define PCIE1_REG2_BASE                         0x1fe10558
#define PCIE1_PHY0_BASE                         0x1fe10560
#define PCIE1_PHY1_BASE                         0x1fe10564

/**
 * SATA Cfg
 */
#define SATA0_REG0_BASE                         0x1fe10570
#define SATA_REG0_P0_TX_AMPLITUDE_GEN2_0        bit(31)
#define SATA_REG0_P0_TX_AMPLITUDE_GEN1_MASK     0x7F        /* bit[30:24] */
#define SATA_REG0_P0_TX_AMPLITUDE_GEN1_SHIFT    24
#define SATA_REG0_PREFETCH                      bit(23)
#define SATA_REG0_FLUSH_WR                      bit(22)
#define SATA_REG0_STOP_WAW                      bit(21)
#define SATA_REG0_STOP_RAW                      bit(20)
#define SATA_REG0_FLUSH_IDLE_MASK               0x0F        /* bit[19:16] */
#define SATA_REG0_FLUSH_IDLE_SHIFT              16
#define SATA_REG0_VREG_BYPASS                   bit(15)
#define SATA_REG0_P0_RX_EQ_MASK                 0x07        /* bit[14:12] */
#define SATA_REG0_P0_RX_EQ_SHIFT                12
#define SATA_REG0_DMA_COHERENT                  bit(10)
#define SATA_REG0_P0_TX_INVERT                  bit(9)
#define SATA_REG0_P0_RX_INVERT                  bit(8)
#define SATA_REG0_SSC_EN                        bit(7)
#define SATA_REG0_SSC_RANGE_MASK                0x07        /* bit[6:4] */
#define SATA_REG0_SSC_RANGE_SHIFT               4
#define SATA_REG0_P0_RESETn                     bit(3)
#define SATA_REG0_PHY_RESETn                    bit(2)
#define SATA_REG0_REF_USE_PAD                   bit(1)
#define SATA_REG0_REF_SSP_EN                    bit(0)

#define SATA0_REG1_BASE                         0x1fe10574
#define SATA_REG1_PHY_POWERDOWN                 bit(31)
#define SATA_REG1_P0_TX_PREEMPH_GEN3_MASK       0x3F        /* bit[30:25] */
#define SATA_REG1_P0_TX_PREEMPH_GEN3_SHIFT      25
#define SATA_REG1_P0_TX_PREEMPH_GEN2_MASK       0x3F        /* bit[24:19] */
#define SATA_REG1_P0_TX_PREEMPH_GEN2_SHIFT      19
#define SATA_REG1_P0_TX_PREEMPH_GEN1_MASK       0x3F        /* bit[18:13] */
#define SATA_REG1_P0_TX_PREEMPH_GEN1_SHIFT      13
#define SATA_REG1_P0_TX_AMPLITUDE_GEN3_MASK     0x7F        /* bit[12:6] */
#define SATA_REG1_P0_TX_AMPLITUDE_GEN3_SHIFT    6
#define SATA_REG1_P0_TX_AMPLITUDE_GEN2_6_1_MASK 0x3F        /* bit[5:0] */

#define SATA0_PHY0_BASE                         0x1fe10590
#define SATA_PHY0_CFG_DATA_MASK                 0xFFFF      /* bit[31:16] */
#define SATA_PHY0_CFG_DATA_SHIFT                16
#define SATA_PHY0_CFG_ADDR_MASK                 0xFFFF      /* bit[15:0] */

#define SATA0_PHY1_BASE                         0x1fe10594
#define SATA_PHY1_CFG_EN                        bit(0)

#define SATA_CFG0_BASE                          0x1fe10580
#define SATA_CFG0_STOP_CPU_RD_FOR_DMA_WR        bit(31)
#define SATA_CFG0_INVALID_PREF_ON_CPU_WR        bit(30)
#define SATA_CFG0_STOP_CPU_WR_FOR_DMA_WR        bit(29)
#define SATA_CFG0_PREF_COND_MASK                0x07        /* bit[26:24] */
#define SATA_CFG0_PREF_COND_SHIFT               24
#define SATA_CFG0_TOLERANT_CYCLE_MASK           0x3F        /* bit[21:16] */
#define SATA_CFG0_TOLERANT_CYCLE_SHIFT          16
#define SATA_CFG0_PREF_LIMIT_MASK               0x0F        /* bit[15:12] */
#define SATA_CFG0_PREF_LIMIT_SHIFT              12
#define SATA_CFG0_PREF_MAX_NUM_MASK             0x0F        /* bit[11:8] */
#define SATA_CFG0_PREF_MAX_NUM_SHIFT            8
#define SATA_CFG0_PREF_PREF_NUM_ONSTATIC_MASK   0x0F        /* bit[7:4] */
#define SATA_CFG0_PREF_PREF_NUM_ONSTSTIC_SHIFT  4
#define SATA_CFG0_PREF_STATIC_EN                bit(3)
#define SATA_CFG0_INVALID_PREF_ON_DMA_WR        bit(2)
#define SATA_CFG0_PREF_DISABLE                  bit(1)
#define SATA_CFG0_RD_WAIT_WR                    bit(0)

#define SATA1_REG0_BASE                 0x1fe10578
#define SATA1_REG1_BASE                 0x1fe1057c
#define SATA1_PHY0_BASE                 0x1fe10598
#define SATA1_PHY1_BASE                 0x1fe1059c

//-------------------------------------------------------------------------------------------------

#if 0
/**
 * DMA Cfg. see "ls2k_dma_hw.h"
 */
#define DMA0_BASE                       0x1fe10c00
#define DMA1_BASE                       0x1fe10c10
#define DMA2_BASE                       0x1fe10c20
#define DMA3_BASE                       0x1fe10c30

#define DMA_ASK_ADDR_MASK               (~0x1Full)          /* bit[63:5] */ /* aligned 32 */
#define DMA_ASK_ADDR_SHIFT              5
#define DMA_STOP                        bit(4)
#define DMA_START                       bit(3)
#define DMA_ASK_VALID                   bit(2)
#endif

//-------------------------------------------------------------------------------------------------

#if (!USE_EXTINT)   /* 使用标准中断 */
/**
 * Interrupt Control
 *
 * DMA控制器中断为脉冲触发类型
 * GPIO中断根据需要可以配置成电平触发或者脉冲触发
 * 其余中断均为电平触发类型
 *
 */
#define INTC_CORE_IPISR                 0x1fe11000          // RO   NA  处理器核的IPI_Status寄存器
#define INTC_CORE_IPIEN                 0x1fe11004          // RW   0x0 处理器核的IPI_Enalbe寄存器
#define INTC_CORE_IPISET                0x1fe11008          // WO   NA 	处理器核的IPI_Set寄存器
#define INTC_CORE_IPICLR                0x1fe1100c          // WO   NA  处理器核的IPI_Clear寄存器

#define INTC_CORE_ISR0                  0x1fe11040          // 路由给CORE的低32位中断状态
#define INTC_INT_ISR0                   0x1fe11044          // 低32位中断状态寄存器
#define INTC_CORE_ISR1                  0x1fe11048          // 路由给CORE的高32位中断状态
#define INTC_INT_ISR1                   0x1fe1104c          // 高32位中断状态寄存器

/*
 * 中断路由0 32bits
 */
#define INTC_ENTRY0_3                   0x1fe11400          // ENTRY0_3     中断路由寄存器[0-3]
#define ENTRY0_UART3_SHIFT              28
#define ENTRY0_UART2_SHIFT              20
#define ENTRY0_UART1_SHIFT              12
#define ENTRY0_UART0_SHIFT              4

#define INTC_ENTRY4_7                   0x1fe11404          // ENTRY4_7     中断路由寄存器[4-7]
#define ENTRY4_THSENS_SHIFT             28
#define ENTRY4_AC97_SHIFT               20
#define ENTRY4_LPC_SHIFT                12
#define ENTRY4_HDA_SHIFT                4

#define INTC_ENTRY8_11                  0x1fe11408          // ENTRY8_11    中断路由寄存器[8-11]
#define ENTRY8_KEYBOARD_SHIFT           28
#define ENTRY8_MOUSE_SHIFT              20
#define ENTRY8_RTC_TICK_SHIFT           12
#define ENTRY8_TOY_TICK_SHIFT           4

#define INTC_ENTRY12_15                 0x1fe1140c          // ENTRY12_15   中断路由寄存器[12-15]
#define ENTRY12_PRINT_SHIFT             28
#define ENTRY12_GMAC1_SHIFT             20
#define ENTRY12_LS132_SHIFT             12
#define ENTRY12_GMAC0_SHIFT             4

#define INTC_ENTRY16_19                 0x1fe11410          // ENTRY16_19   中断路由寄存器[16-19]
#define ENTRY16_SATA_SHIFT              28
#define ENTRY16_SATA_PME_SHIFT          20
#define ENTRY16_CAN1_SHIFT              12
#define ENTRY16_CAN0_SHIFT              4

#define INTC_ENTRY20_23                 0x1fe11414          // ENTRY20_23   中断路由寄存器[20-23]
#define ENTRY20_I2C1_SHIFT              28
#define ENTRY20_I2C0_SHIFT              20
#define ENTRY20_HPET0_SHIFT             12
#define ENTRY20_NAND_SHIFT              4

#define INTC_ENTRY24_27                 0x1fe11418          // ENTRY24_27   中断路由寄存器[24-27]
#define ENTRY24_PWM3_SHIFT              28
#define ENTRY24_PWM2_SHIFT              20
#define ENTRY24_PWM1_SHIFT              12
#define ENTRY24_PWM0_SHIFT              4

#define INTC_ENTRY28_31                 0x1fe1141c          // ENTRY28_31   中断路由寄存器[28-31]
#define ENTRY28_SDIO_SHIFT              28
#define ENTRY28_USB3_SHIFT              20
#define ENTRY28_GPU_SHIFT               12
#define ENTRY28_DC_SHIFT                4

/*
 * 中断控制0 32bits
 */
#define INTC0_SR_BASE                   0x1fe11420
#define INTC0_EN_BASE                   0x1fe11424
#define INTC0_SET_BASE                  0x1fe11428          /* 使能中断 */
#define INTC0_CLR_BASE                  0x1fe1142c          /* 禁止中断 */
#define INTC0_POL_BASE                  0x1fe11430
#define INTC0_EDGE_BASE                 0x1fe11434

#define INTC0_SDIO0_1_BIT               bit(31)
#define INTC0_USB3_BIT                  bit(30)
#define INTC0_GPU_BIT                   bit(29)
#define INTC0_DC_BIT                    bit(28)
#define INTC0_PWM12_15_BIT              bit(27)
#define INTC0_PWM8_11_BIT               bit(26)
#define INTC0_PWM4_7_BIT                bit(25)
#define INTC0_PWM0_3_BIT                bit(24)
#define INTC0_I2C1_BIT                  bit(23)
#define INTC0_I2C0_BIT                  bit(22)
#define INTC0_HPET0_BIT                 bit(21)
#define INTC0_NAND_BIT                  bit(20)
#define INTC0_SATA_BIT                  bit(19)
#define INTC0_SATA_PME_BIT              bit(18)
#define INTC0_CAN2_3_BIT                bit(17)
#define INTC0_CAN0_1_BIT                bit(16)
#define INTC0_PRINT_BIT                 bit(15)
#define INTC0_GMAC1_BIT                 bit(14)
#define INTC0_LS132_BIT                 bit(13)
#define INTC0_GMAC0_BIT                 bit(12)
#define INTC0_KEYBOARD_BIT              bit(11)
#define INTC0_MOUSE_BIT                 bit(10)
#define INTC0_RTC_TICK_BIT              bit(9)
#define INTC0_TOY_TICK_BIT              bit(8)
#define INTC0_THSENS_BIT                bit(7)
#define INTC0_AC97_BIT                  bit(6)
#define INTC0_LPC_BIT                   bit(5)
#define INTC0_HDA_BIT                   bit(4)
#define INTC0_UART3_BIT                 bit(3)
#define INTC0_UART2_BIT                 bit(2)
#define INTC0_UART1_BIT                 bit(1)
#define INTC0_UART0_BIT                 bit(0)

/*
 * 中断路由1 32bits
 */
#define INTC_ENTRY32_35                 0x1fe11440          // ENTRY32_35 	中断路由寄存器[32-35]
#define ENTRY32_SPI1_SHIFT              28
#define ENTRY32_SPI0_SHIFT              20
#define ENTRY32_PCIE1_SHIFT             12
#define ENTRY32_PCIE0_SHIFT             4

#define INTC_ENTRY36_39                 0x1fe11444          // ENTRY36_39 	中断路由寄存器[36-39]
#define ENTRY36_HPET2_SHIFT             28
#define ENTRY36_HPET1_SHIFT             20
#define ENTRY36_SPI3_SHIFT              12
#define ENTRY36_SPI2_SHIFT              4

#define INTC_ENTRY40_43                 0x1fe11448          // ENTRY40_43 	中断路由寄存器[40-43]
#define ENTRY40_ACPI_SHIFT              28
#define ENTRY40_TOY2_SHIFT              20
#define ENTRY40_TOY1_SHIFT              12
#define ENTRY40_TOY0_SHIFT              4

#define INTC_ENTRY44_47                 0x1fe1144c          // ENTRY44_47 	中断路由寄存器[44-47]
#define ENTRY44_DMA3_SHIFT              28
#define ENTRY44_DMA2_SHIFT              20
#define ENTRY44_DMA1_SHIFT              12
#define ENTRY44_DMA0_SHIFT              4

#define INTC_ENTRY48_51                 0x1fe11450          // ENTRY48_51 	中断路由寄存器[48-51]
#define ENTRY48_OHCI_SHIFT              28
#define ENTRY48_EHCI_SHIFT              20
#define ENTRY48_OTG_SHIFT               12
#define ENTRY48_HPET3_SHIFT             4

#define INTC_ENTRY52_55                 0x1fe11454          // ENTRY52_55 	中断路由寄存器[52-55]
#define ENTRY52_I2C2_SHIFT              28
#define ENTRY52_RTC2_SHIFT              20
#define ENTRY52_RTC1_SHIFT              12
#define ENTRY52_RTC0_SHIFT              4

#define INTC_ENTRY56_59                 0x1fe11458          // ENTRY56_59 	中断路由寄存器[56-59]
#define ENTRY56_GPIO32_63_SHIFT         28
#define ENTRY56_GPIO0_31_SHIFT          20
#define ENTRY56_I2C4_SHIFT              12
#define ENTRY56_I2C3_SHIFT              4

#define INTC_ENTRY60_63                 0x1fe1145c          // ENTRY60_63 	中断路由寄存器[60-63]
#define ENTRY60_UART5_SHIFT             28
#define ENTRY60_UART4_SHIFT             20
#define ENTRY60_GPIO96_127_SHIFT        12
#define ENTRY60_GPIO64_95_SHIFT         4

/*
 * 中断控制1 32bits
 */
#define INTC1_SR_BASE                   0x1fe11460
#define INTC1_EN_BASE                   0x1fe11464
#define INTC1_SET_BASE                  0x1fe11468          /* 使能中断 */
#define INTC1_CLR_BASE                  0x1fe1146c          /* 禁止中断 */
#define INTC1_POL_BASE                  0x1fe11470
#define INTC1_EDGE_BASE                 0x1fe11474

#define INTC1_UART7_9_BIT               bit(31)
#define INTC1_UART4_6_BIT               bit(30)
#define INTC1_GPIO96_127_BIT            bit(29)
#define INTC1_GPIO64_95_BIT             bit(28)
#define INTC1_GPIO32_63_BIT             bit(27)
#define INTC1_GPIO0_31_BIT              bit(26)
#define INTC1_I2C4_5_BIT                bit(25)
#define INTC1_I2C3_BIT                  bit(24)
#define INTC1_I2C2_BIT                  bit(23)
#define INTC1_RTC2_BIT                  bit(22)
#define INTC1_RTC1_BIT                  bit(21)
#define INTC1_RTC0_BIT                  bit(20)
#define INTC1_OHCI_BIT                  bit(19)
#define INTC1_EHCI_BIT                  bit(18)
#define INTC1_OTG_BIT                   bit(17)
#define INTC1_HPET3_BIT                 bit(16)
#define INTC1_DMA3_BIT                  bit(15)
#define INTC1_DMA2_BIT                  bit(14)
#define INTC1_DMA1_BIT                  bit(13)
#define INTC1_DMA0_BIT                  bit(12)
#define INTC1_ACPI_BIT                  bit(11)
#define INTC1_TOY2_BIT                  bit(10)
#define INTC1_TOY1_BIT                  bit(9)
#define INTC1_TOY0_BIT                  bit(8)
#define INTC1_HPET2_BIT                 bit(7)
#define INTC1_HPET1_BIT                 bit(6)
#define INTC1_SPI4_5_BIT                bit(5)
#define INTC1_SPI2_3_BIT                bit(4)
#define INTC1_SPI1_BIT                  bit(3)
#define INTC1_SPI0_BIT                  bit(2)
#define INTC1_PCIE1_BIT                 bit(1)
#define INTC1_PCIE0_BIT                 bit(0)

/*
 * 中断寄存器索引(n>=0 && n<=1)
 */
#define INTC_SR(n)                      (INTC0_SR_BASE  + n*0x40)
#define INTC_EN(n)                      (INTC0_EN_BASE  + n*0x40)
#define INTC_SET(n)                     (INTC0_SET_BASE + n*0x40)
#define INTC_CLR(n)                     (INTC0_CLR_BASE + n*0x40)
#define INTC_POL(n)                     (INTC0_POL_BASE + n*0x40)
#define INTC_EDGE(n)                    (INTC0_EDGE_BASE+ n*0x40)

/*
 * 中断映射
 */
#define INT_ROUTE_IP2                   0x10                /* 中断路由到 IP2 */
#define INT_ROUTE_IP3                   0x20                /* 中断路由到 IP3 */
#define INT_ROUTE_IP4                   0x40                /* 中断路由到 IP4 */
#define INT_ROUTE_IP5                   0x80                /* 中断路由到 IP5 */

#else   /* 使用扩展中断 */

/**
 * EXT Interrupt Control
 */
#define EXT_IOI_ACK_BASE                0x1fe11148          // 扩展IO中断处理完成握手配置寄存器

/*
 * 扩展中断控制0 32bits
 */
#define EXTINTC0_EN_BASE                0x1fe11600
#define EXTINTC0_SR_BASE                0x1fe11700
#define EXTINTC0_CLR_BASE               0x1fe11700
#define CORE_EXTI0_SR_BASE              0x1fe11800
#define CORE_EXTI0_CLR_BASE             0x1fe11800

#define EXTINTC0_NAND_BIT               bit(31)
#define EXTINTC0_HPET3_BIT              bit(30)
#define EXTINTC0_HPET2_BIT              bit(29)
#define EXTINTC0_HPET1_BIT              bit(28)
#define EXTINTC0_HPET0_BIT              bit(27)
#define EXTINTC0_SPI5_BIT               bit(26)
#define EXTINTC0_SPI4_BIT               bit(25)
#define EXTINTC0_SPI3_BIT               bit(24)
#define EXTINTC0_SPI2_BIT               bit(23)
#define EXTINTC0_MOUSE_BIT              bit(22)
#define EXTINTC0_KEYBOARD_BIT           bit(21)
#define EXTINTC0_AC97_BIT               bit(20)
#define EXTINTC0_I2C5_BIT               bit(19)
#define EXTINTC0_I2C4_BIT               bit(18)
#define EXTINTC0_I2C3_BIT               bit(17)
#define EXTINTC0_I2C2_BIT               bit(16)
#define EXTINTC0_I2C1_BIT               bit(15)
#define EXTINTC0_I2C0_BIT               bit(14)
#define EXTINTC0_CAN3_BIT               bit(13)
#define EXTINTC0_CAN2_BIT               bit(12)
#define EXTINTC0_CAN1_BIT               bit(11)
#define EXTINTC0_CAN0_BIT               bit(10)
#define EXTINTC0_UART9_BIT              bit(9)
#define EXTINTC0_UART8_BIT              bit(8)
#define EXTINTC0_UART7_BIT              bit(7)
#define EXTINTC0_UART6_BIT              bit(6)
#define EXTINTC0_UART5_BIT              bit(5)
#define EXTINTC0_UART4_BIT              bit(4)
#define EXTINTC0_UART3_BIT              bit(3)
#define EXTINTC0_UART2_BIT              bit(2)
#define EXTINTC0_UART1_BIT              bit(1)
#define EXTINTC0_UART0_BIT              bit(0)

/*
 * 扩展中断控制1 32bits
 */
#define EXTINTC1_EN_BASE                0x1fe11604
#define EXTINTC1_SR_BASE                0x1fe11704
#define EXTINTC1_CLR_BASE               0x1fe11704
#define CORE_EXTI1_SR_BASE              0x1fe11804
#define CORE_EXTI1_CLR_BASE             0x1fe11804

#define EXTINTC1_LS132_BIT              bit(27)
#define EXTINTC1_SDIO1_BIT              bit(26)
#define EXTINTC1_SDIO0_BIT              bit(25)
#define EXTINTC1_ACPI_BIT               bit(24)
#define EXTINTC1_PWM15_BIT              bit(23)
#define EXTINTC1_PWM14_BIT              bit(22)
#define EXTINTC1_PWM13_BIT              bit(21)
#define EXTINTC1_PWM12_BIT              bit(20)
#define EXTINTC1_PWM11_BIT              bit(19)
#define EXTINTC1_PWM10_BIT              bit(18)
#define EXTINTC1_PWM9_BIT               bit(17)
#define EXTINTC1_PWM8_BIT               bit(16)
#define EXTINTC1_PWM7_BIT               bit(15)
#define EXTINTC1_PWM6_BIT               bit(14)
#define EXTINTC1_PWM5_BIT               bit(13)
#define EXTINTC1_PWM4_BIT               bit(12)
#define EXTINTC1_PWM3_BIT               bit(11)
#define EXTINTC1_PWM2_BIT               bit(10)
#define EXTINTC1_PWM1_BIT               bit(9)
#define EXTINTC1_PWM0_BIT               bit(8)
#define EXTINTC1_RTC_TICK_BIT           bit(7)
#define EXTINTC1_RTC2_BIT               bit(6)
#define EXTINTC1_RTC1_BIT               bit(5)
#define EXTINTC1_RTC0_BIT               bit(4)
#define EXTINTC1_TOY_TICK_BIT           bit(3)
#define EXTINTC1_TOY2_BIT               bit(2)
#define EXTINTC1_TOY1_BIT               bit(1)
#define EXTINTC1_TOY0_BIT               bit(0)

/*
 * 扩展中断控制2 32bits
 */
#define EXTINTC2_EN_BASE                0x1fe11608
#define EXTINTC2_SR_BASE                0x1fe11708
#define EXTINTC2_CLR_BASE               0x1fe11708
#define CORE_EXTI2_SR_BASE              0x1fe11808
#define CORE_EXTI2_CLR_BASE             0x1fe11808

#define EXTINTC2_GPIO32_34_BIT          bit(31)
#define EXTINTC2_GPIO28_30_BIT          bit(30)
#define EXTINTC2_GPIO24_26_BIT          bit(29)
#define EXTINTC2_GPIO20_22_BIT          bit(28)
#define EXTINTC2_GPIO16_18_BIT          bit(27)
#define EXTINTC2_GPIO12_14_BIT          bit(26)
#define EXTINTC2_GPIO8_10_BIT           bit(25)
#define EXTINTC2_GPIO4_6_BIT            bit(24)
#define EXTINTC2_GPIO0_2_BIT            bit(23)
#define EXTINTC2_THSENS_BIT             bit(22)
#define EXTINTC2_HDA_BIT                bit(21)
#define EXTINTC2_PRINTE_BIT             bit(20)
#define EXTINTC2_PRINTST_BIT            bit(19)
#define EXTINTC2_PCIE1_BIT              bit(18)
#define EXTINTC2_PCIE0_BIT              bit(17)
#define EXTINTC2_DC_BIT                 bit(16)
#define EXTINTC2_SATA_PME1_BIT          bit(15)
#define EXTINTC2_SATA_PME0_BIT          bit(14)
#define EXTINTC2_GPU_BIT                bit(13)
#define EXTINTC2_SATA1_BIT              bit(12)
#define EXTINTC2_SATA0_BIT              bit(11)
#define EXTINTC2_USB3_BIT               bit(10)
#define EXTINTC2_OTG_BIT                bit(9)
#define EXTINTC2_OHCI_BIT               bit(8)
#define EXTINTC2_EHCI_BIT               bit(7)
#define EXTINTC2_DMA3_BIT               bit(6)
#define EXTINTC2_DMA2_BIT               bit(5)
#define EXTINTC2_DMA1_BIT               bit(4)
#define EXTINTC2_DMA0_BIT               bit(3)
#define EXTINTC2_LPC_BIT                bit(2)
#define EXTINTC2_SPI1_BIT               bit(1)
#define EXTINTC2_SPI0_BIT               bit(0)

/*
 * 扩展中断控制3 32bits
 */
#define EXTINTC3_EN_BASE                0x1fe1160c
#define EXTINTC3_SR_BASE                0x1fe1170c
#define EXTINTC3_CLR_BASE               0x1fe1170c
#define CORE_EXTI3_SR_BASE              0x1fe1180c
#define CORE_EXTI3_CLR_BASE             0x1fe1180c

#define EXTINTC3_GMAC1_BIT              bit(23)
#define EXTINTC3_GMAC0_BIT              bit(22)
#define EXTINTC3_GPIO120_122_BIT        bit(21)
#define EXTINTC3_GPIO116_118_BIT        bit(20)
#define EXTINTC3_GPIO112_114_BIT        bit(19)
#define EXTINTC3_GPIO108_110_BIT        bit(18)
#define EXTINTC3_GPIO104_106_BIT        bit(17)
#define EXTINTC3_GPIO100_102_BIT        bit(16)
#define EXTINTC3_GPIO96_98_BIT          bit(15)
#define EXTINTC3_GPIO92_94_BIT          bit(14)
#define EXTINTC3_GPIO88_90_BIT          bit(13)
#define EXTINTC3_GPIO84_86_BIT          bit(12)
#define EXTINTC3_GPIO80_82_BIT          bit(11)
#define EXTINTC3_GPIO76_78_BIT          bit(10)
#define EXTINTC3_GPIO72_74_BIT          bit(9)
#define EXTINTC3_GPIO68_70_BIT          bit(8)
#define EXTINTC3_GPIO64_66_BIT          bit(7)
#define EXTINTC3_GPIO60_62_BIT          bit(6)
#define EXTINTC3_GPIO56_58_BIT          bit(5)
#define EXTINTC3_GPIO52_54_BIT          bit(4)
#define EXTINTC3_GPIO48_50_BIT          bit(3)
#define EXTINTC3_GPIO44_46_BIT          bit(2)
#define EXTINTC3_GPIO40_42_BIT          bit(1)
#define EXTINTC3_GPIO36_38_BIT          bit(0)

/*
 * 中断寄存器索引 (n>=0 && n<=3)
 */
#define EXTINTC_EN(n)                   (EXTINTC0_EN_BASE  + n*4)
#define EXTINTC_SR(n)                   (EXTINTC0_SR_BASE  + n*4)
#define EXTINTC_CLR(n)                  (EXTINTC0_CLR_BASE + n*4)

/*
 * 扩展中断路由: 整组映射
 */
#define EXTINT_MAP0                     0x1fe114c0          // EXT_IOI[31:0]   对应的处理器核中断引脚向量路由配置
#define EXTINT_MAP1                     0x1fe114c1          // EXT_IOI[63:32]  对应的处理器核中断引脚向量路由配置
#define EXTINT_MAP2                     0x1fe114c2          // EXT_IOI[95:64]  对应的处理器核中断引脚向量路由配置
#define EXTINT_MAP3                     0x1fe114c3          // EXT_IOI[127:96] 对应的处理器核中断引脚向量路由配置

#define EXTINT_ROUTE_IP0                0x01                /* 中断路由到 IP0 */
#define EXTINT_ROUTE_IP1                0x02                /* 中断路由到 IP1 */
#define EXTINT_ROUTE_IP2                0x04                /* 中断路由到 IP2 */
#define EXTINT_ROUTE_IP3                0x08                /* 中断路由到 IP3 */

#endif // #if (!USE_EXTINT)

/**
 * 中断触发模式
 */
#define INT_TRIGGER_LEVEL               0x04                /* 电平触发中断 */
#define INT_TRIGGER_PULSE               0x08                /* 脉冲触发中断 */

/**
 * Thsens
 */
#define THSENS_INTC_HI0_BASE            0x1fe11500          // 温度传感器高温中断控制寄存器0
#define THSENS_INTC_HI1_BASE            0x1fe11504          // 温度传感器高温中断控制寄存器1
#define THSENS_INTC_LO0_BASE            0x1fe11508          // 温度传感器低温中断控制寄存器0
#define THSENS_INTC_LO1_BASE            0x1fe1150c          // 温度传感器低温中断控制寄存器1
#define THSENS_INT_SR0_BASE             0x1fe11510          // 温度传感器中断状态寄存器0
#define THSENS_INT_SR1_BASE             0x1fe11514          // 温度传感器中断状态寄存器1
#define THSENS_SCALE_HI0_BASE           0x1fe11520          // 温度传感器测量寄存器0
#define THSENS_SCALE_HI1_BASE           0x1fe11524          // 温度传感器测量寄存器1
#define THSENS_SCALE_LO0_BASE           0x1fe11528          // 保留
#define THSENS_SCALE_LO1_BASE           0x1fe1152c          // 保留

/**
 * Chip ID
 */
#define CHIP_ID0_BASE                   0x1fe13ff8
#define CHIP_ID1_BASE                   0x1fe13ffc

#endif // _LS2K500_H

//-------------------------------------------------------------------------------------------------

/*
 * @@ END
 */

