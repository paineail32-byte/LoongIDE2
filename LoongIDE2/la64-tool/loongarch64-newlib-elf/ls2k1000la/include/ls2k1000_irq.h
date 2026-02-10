/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k1000_irq.h
 *
 * created: 2022-10-03
 *  author: Bian
 */

#ifndef _LS2K1000LA_IRQ_H
#define _LS2K1000LA_IRQ_H

#include "ls2k1000.h"

/**************************************************************************************************
 * Loongarch64 Vector numbers for exception. This is a direct map to the ESTAT.
 **************************************************************************************************/

#define LA_EXCEPTION_BASE           0

#define LA_EXCEPTION_INT            LA_EXCEPTION_BASE+0     // 仅当CSR.ECFG.VS=0时, 表示是中断
#define LA_EXCEPTION_PIL            LA_EXCEPTION_BASE+1     // load  操作页无效例外
#define LA_EXCEPTION_PIS            LA_EXCEPTION_BASE+2     // store 操作页无效例外
#define LA_EXCEPTION_PIF            LA_EXCEPTION_BASE+3     // 取指操作页无效例外
#define LA_EXCEPTION_PME            LA_EXCEPTION_BASE+4     // 页修改例外
#define LA_EXCEPTION_PNR            LA_EXCEPTION_BASE+5     // 页不可读例外
#define LA_EXCEPTION_PNX            LA_EXCEPTION_BASE+6     // 页不可执行例外
#define LA_EXCEPTION_PPI            LA_EXCEPTION_BASE+7     // 页特权等级不合规例外
#define LA_EXCEPTION_ADEF_ADEM      LA_EXCEPTION_BASE+8     // subcode=0: 取指地址错例外/subcode=1: 访存指令地址错例外
#define LA_EXCEPTION_ALE            LA_EXCEPTION_BASE+9     // 地址非对齐例外
#define LA_EXCEPTION_BCE            LA_EXCEPTION_BASE+10    // 边界约束检查错例外
#define LA_EXCEPTION_SYSCALL        LA_EXCEPTION_BASE+11    // 系统调用例外
#define LA_EXCEPTION_BREAK          LA_EXCEPTION_BASE+12    // 断点例外
#define LA_EXCEPTION_INE            LA_EXCEPTION_BASE+13    // 指令不存在例外
#define LA_EXCEPTION_IPE            LA_EXCEPTION_BASE+14    // 指令特权等级错例外
#define LA_EXCEPTION_FPD            LA_EXCEPTION_BASE+15    // 浮点指令未使能例外
#define LA_EXCEPTION_SXD            LA_EXCEPTION_BASE+16    // 128位向量扩展指令未使能例外
#define LA_EXCEPTION_ASXD           LA_EXCEPTION_BASE+17    // 256位向量扩展指令未使能例外
#define LA_EXCEPTION_FPE_VFPE       LA_EXCEPTION_BASE+18    // subcode=0: 基础浮点指令例外/subcode=1: 向量浮点指令例外
#define LA_EXCEPTION_WPEF_WPEM      LA_EXCEPTION_BASE+19    // subcode=0: 取指监测点例外/subcode=1: load/store操作监测点例外
#define LA_EXCEPTION_BTD            LA_EXCEPTION_BASE+20    // 二进制翻译扩展指令未使能例外
#define LA_EXCEPTION_BTE            LA_EXCEPTION_BASE+21    // 二进制翻译相关例外
#define LA_EXCEPTION_GSPR           LA_EXCEPTION_BASE+22    // 客户机敏感特权资源例外
#define LA_EXCEPTION_HVC            LA_EXCEPTION_BASE+23    // 虚拟机监控调用例外
#define LA_EXCEPTION_GCSC_GCHC      LA_EXCEPTION_BASE+24    // subcode=0: 客户机CSR软件修改例外/ subcode=1: 客户机CSR硬件修改例外

/*0x1A-0x3E 保留 */

#define LA_INTERRUPT_BASE           LA_EXCEPTION_BASE+64    // 64

/**************************************************************************************************
 * Interrupt Vector Numbers
 * Loongarch64 LA_INTERRUPT_BASE should be 64 (0x40)
 **************************************************************************************************/

/*
 * Loongarch64 ECFG (0x4)  LIP bit(12:0)=IPI, TI, PMI, HWI[7:0], SWI[1:0]
 *             ESTAT(0x5)  IS  bit(12:0) if interrupt, status bit equal 1
 *
 */
#define LS2K1000LA_IRQ_SW0             (LA_INTERRUPT_BASE+0)   // 软中断 SWI0~SWI1
#define LS2K1000LA_IRQ_SW1             (LA_INTERRUPT_BASE+1)
#define LS2K1000LA_IRQ_HW0             (LA_INTERRUPT_BASE+2)   // 硬中断 HWI0~HWI7
#define LS2K1000LA_IRQ_HW1             (LA_INTERRUPT_BASE+3)
#define LS2K1000LA_IRQ_HW2             (LA_INTERRUPT_BASE+4)
#define LS2K1000LA_IRQ_HW3             (LA_INTERRUPT_BASE+5)
#define LS2K1000LA_IRQ_HW4             (LA_INTERRUPT_BASE+6)
#define LS2K1000LA_IRQ_HW5             (LA_INTERRUPT_BASE+7)
#define LS2K1000LA_IRQ_HW6             (LA_INTERRUPT_BASE+8)
#define LS2K1000LA_IRQ_HW7             (LA_INTERRUPT_BASE+9)
#define LS2K1000LA_IRQ_PERF            (LA_INTERRUPT_BASE+10)  // 性能监测计数溢出中断
#define LS2K1000LA_IRQ_TIMER           (LA_INTERRUPT_BASE+11)  // 定时器中断
#define LS2K1000LA_IRQ_IPI             (LA_INTERRUPT_BASE+12)  // 核间中断

/**************************************************************************************************
 * 使用系统传统中断模式, 共支持64个中断
 **************************************************************************************************/
 
/*
 * Interrupt Control 0 Interrupts
 */
#define LS2K1000LA_IRQ0_BASE            (LA_INTERRUPT_BASE+16)      // 80

#define INTC0_UART0_3_IRQ               (LS2K1000LA_IRQ0_BASE+0)
#define INTC0_UART4_7_IRQ               (LS2K1000LA_IRQ0_BASE+1)
#define INTC0_UART8_11_IRQ              (LS2K1000LA_IRQ0_BASE+2)
#define INTC0_E1_IRQ                    (LS2K1000LA_IRQ0_BASE+3)
#define INTC0_HDA_IRQ                   (LS2K1000LA_IRQ0_BASE+4)
#define INTC0_I2S_IRQ                   (LS2K1000LA_IRQ0_BASE+5)
#define INTC0_THSENS_IRQ                (LS2K1000LA_IRQ0_BASE+7)
#define INTC0_TOY_TICK_IRQ              (LS2K1000LA_IRQ0_BASE+8)
#define INTC0_RTC_TICK_IRQ              (LS2K1000LA_IRQ0_BASE+9)
#define INTC0_CAMERA_IRQ                (LS2K1000LA_IRQ0_BASE+10)
#define INTC0_GMAC0_SBD_IRQ             (LS2K1000LA_IRQ0_BASE+12)
#define INTC0_GMAC0_PMT_IRQ             (LS2K1000LA_IRQ0_BASE+13)
#define INTC0_GMAC1_SBD_IRQ             (LS2K1000LA_IRQ0_BASE+14)
#define INTC0_GMAC1_PMT_IRQ             (LS2K1000LA_IRQ0_BASE+15)
#define INTC0_CAN0_IRQ                  (LS2K1000LA_IRQ0_BASE+16)
#define INTC0_CAN1_IRQ                  (LS2K1000LA_IRQ0_BASE+17)
#define INTC0_SPI_IRQ                   (LS2K1000LA_IRQ0_BASE+18)
#define INTC0_SATA_IRQ                  (LS2K1000LA_IRQ0_BASE+19)
#define INTC0_NAND_IRQ                  (LS2K1000LA_IRQ0_BASE+20)
#define INTC0_HPET0_IRQ                 (LS2K1000LA_IRQ0_BASE+21)
#define INTC0_I2C0_IRQ                  (LS2K1000LA_IRQ0_BASE+22)
#define INTC0_I2C1_IRQ                  (LS2K1000LA_IRQ0_BASE+23)
#define INTC0_PWM0_IRQ                  (LS2K1000LA_IRQ0_BASE+24)
#define INTC0_PWM1_IRQ                  (LS2K1000LA_IRQ0_BASE+25)
#define INTC0_PWM2_IRQ                  (LS2K1000LA_IRQ0_BASE+26)
#define INTC0_PWM3_IRQ                  (LS2K1000LA_IRQ0_BASE+27)
#define INTC0_DC_IRQ                    (LS2K1000LA_IRQ0_BASE+28)
#define INTC0_GPU_IRQ                   (LS2K1000LA_IRQ0_BASE+29)
#define INTC0_VPU_IRQ                   (LS2K1000LA_IRQ0_BASE+30)
#define INTC0_SDIO_IRQ                  (LS2K1000LA_IRQ0_BASE+31)

/*
 * Interrupt Control 1 Interrupts
 */
#define LS2K1000LA_IRQ1_BASE            (LS2K1000LA_IRQ0_BASE+32)   // 112

#define INTC1_PCIE00_IRQ                (LS2K1000LA_IRQ1_BASE+0)
#define INTC1_PCIE01_IRQ                (LS2K1000LA_IRQ1_BASE+1)
#define INTC1_PCIE02_IRQ                (LS2K1000LA_IRQ1_BASE+2)
#define INTC1_PCIE03_IRQ                (LS2K1000LA_IRQ1_BASE+3)
#define INTC1_PCIE10_IRQ                (LS2K1000LA_IRQ1_BASE+4)
#define INTC1_PCIE11_IRQ                (LS2K1000LA_IRQ1_BASE+5)
#define INTC1_HPET1_IRQ                 (LS2K1000LA_IRQ1_BASE+6)
#define INTC1_HPET2_IRQ                 (LS2K1000LA_IRQ1_BASE+7)
#define INTC1_TOY0_IRQ                  (LS2K1000LA_IRQ1_BASE+8)
#define INTC1_TOY1_IRQ                  (LS2K1000LA_IRQ1_BASE+9)
#define INTC1_TOY2_IRQ                  (LS2K1000LA_IRQ1_BASE+10)
#define INTC1_TOY3_IRQ                  (LS2K1000LA_IRQ1_BASE+11)
#define INTC1_DMA0_IRQ                  (LS2K1000LA_IRQ1_BASE+12)
#define INTC1_DMA1_IRQ                  (LS2K1000LA_IRQ1_BASE+13)
#define INTC1_DMA2_IRQ                  (LS2K1000LA_IRQ1_BASE+14)
#define INTC1_DMA3_IRQ                  (LS2K1000LA_IRQ1_BASE+15)
#define INTC1_DMA4_IRQ                  (LS2K1000LA_IRQ1_BASE+16)
#define INTC1_OTG_IRQ                   (LS2K1000LA_IRQ1_BASE+17)
#define INTC1_EHCI_IRQ                  (LS2K1000LA_IRQ1_BASE+18)
#define INTC1_OHCI_IRQ                  (LS2K1000LA_IRQ1_BASE+19)
#define INTC1_RTC0_IRQ                  (LS2K1000LA_IRQ1_BASE+20)
#define INTC1_RTC1_IRQ                  (LS2K1000LA_IRQ1_BASE+21)
#define INTC1_RTC2_IRQ                  (LS2K1000LA_IRQ1_BASE+22)
#define INTC1_RSA_IRQ                   (LS2K1000LA_IRQ1_BASE+23)
#define INTC1_AES_IRQ                   (LS2K1000LA_IRQ1_BASE+24)
#define INTC1_DES_IRQ                   (LS2K1000LA_IRQ1_BASE+25)
#define INTC1_GPIO4_31_IRQ              (LS2K1000LA_IRQ1_BASE+26)
#define INTC1_GPIO32_63_IRQ             (LS2K1000LA_IRQ1_BASE+27)
#define INTC1_GPIO0_IRQ                 (LS2K1000LA_IRQ1_BASE+28)
#define INTC1_GPIO1_IRQ                 (LS2K1000LA_IRQ1_BASE+29)
#define INTC1_GPIO2_IRQ                 (LS2K1000LA_IRQ1_BASE+30)
#define INTC1_GPIO3_IRQ                 (LS2K1000LA_IRQ1_BASE+31)

#define LS2K1000LA_IRQ_COUNT            (LS2K1000LA_IRQ1_BASE+32)   // 传统中断总数

#endif // _LS2K1000LA_IRQ_H

/*
 * @@ END
 */
 
