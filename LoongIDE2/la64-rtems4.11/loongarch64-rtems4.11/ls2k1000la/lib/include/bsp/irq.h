/*
 * LS2K1000 irq.h
 *
 * Interrupt related API declarations.
 *
 * This file contains the API prototypes for configuring LS2K1000 INTC
 */

#ifndef _LS2K1000_IRQ_H
#define _LS2K1000_IRQ_H

#ifndef ASM

#include <rtems.h>
#include <rtems/irq.h>
#include <rtems/irq-extension.h>
#include <rtems/score/loongarch64.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 * Interrupt Vector Numbers
 * LOONGARCH_INTERRUPT_BASE should be 32
 **************************************************************************************************/

/*
 * Loongarch64 ECFG (0x4)  LIP bit(12:0)=IPI, TI, PMI, HWI[7:0], SWI[1:0]
 *             ESTAT(0x5)  IS  bit(12:0) if interrupt, status bit equal 1
 *
 */
#define LS2K1000_IRQ_SW0            (LOONGARCH_INTERRUPT_BASE+0)    // 软中断 SWI0~SWI1
#define LS2K1000_IRQ_SW1            (LOONGARCH_INTERRUPT_BASE+1)
#define LS2K1000_IRQ_HW0            (LOONGARCH_INTERRUPT_BASE+2)    // 硬中断 HWI0~HWI7
#define LS2K1000_IRQ_HW1            (LOONGARCH_INTERRUPT_BASE+3)
#define LS2K1000_IRQ_HW2            (LOONGARCH_INTERRUPT_BASE+4)
#define LS2K1000_IRQ_HW3            (LOONGARCH_INTERRUPT_BASE+5)
#define LS2K1000_IRQ_HW4            (LOONGARCH_INTERRUPT_BASE+6)
#define LS2K1000_IRQ_HW5            (LOONGARCH_INTERRUPT_BASE+7)
#define LS2K1000_IRQ_HW6            (LOONGARCH_INTERRUPT_BASE+8)
#define LS2K1000_IRQ_HW7            (LOONGARCH_INTERRUPT_BASE+9)
#define LS2K1000_IRQ_PERF           (LOONGARCH_INTERRUPT_BASE+10)   // 性能监测计数溢出中断
#define LS2K1000_IRQ_TIMER          (LOONGARCH_INTERRUPT_BASE+11)   // 定时器中断
#define LS2K1000_IRQ_IPI            (LOONGARCH_INTERRUPT_BASE+12)  	// 核间中断

/**************************************************************************************************
 * 使用系统标准中断模式, 共支持64个中断
 **************************************************************************************************/

/*
 * Interrupt Control 0 Interrupts
 */
#define LS2K1000_IRQ0_BASE          (LOONGARCH_INTERRUPT_BASE+13) 	// 45

#define INTC0_UART0_3_IRQ           (LS2K1000_IRQ0_BASE+0)
#define INTC0_UART4_7_IRQ           (LS2K1000_IRQ0_BASE+1)
#define INTC0_UART8_11_IRQ          (LS2K1000_IRQ0_BASE+2)
#define INTC0_E1_IRQ                (LS2K1000_IRQ0_BASE+3)
#define INTC0_HDA_IRQ               (LS2K1000_IRQ0_BASE+4)
#define INTC0_I2S_IRQ               (LS2K1000_IRQ0_BASE+5)
#define INTC0_THSENS_IRQ            (LS2K1000_IRQ0_BASE+7)
#define INTC0_TOY_TICK_IRQ          (LS2K1000_IRQ0_BASE+8)
#define INTC0_RTC_TICK_IRQ          (LS2K1000_IRQ0_BASE+9)
#define INTC0_CAMERA_IRQ            (LS2K1000_IRQ0_BASE+10)
#define INTC0_GMAC0_SBD_IRQ         (LS2K1000_IRQ0_BASE+12)
#define INTC0_GMAC0_PMT_IRQ         (LS2K1000_IRQ0_BASE+13)
#define INTC0_GMAC1_SBD_IRQ         (LS2K1000_IRQ0_BASE+14)
#define INTC0_GMAC1_PMT_IRQ         (LS2K1000_IRQ0_BASE+15)
#define INTC0_CAN0_IRQ              (LS2K1000_IRQ0_BASE+16)
#define INTC0_CAN1_IRQ              (LS2K1000_IRQ0_BASE+17)
#define INTC0_SPI_IRQ               (LS2K1000_IRQ0_BASE+18)
#define INTC0_SATA_IRQ              (LS2K1000_IRQ0_BASE+19)
#define INTC0_NAND_IRQ              (LS2K1000_IRQ0_BASE+20)
#define INTC0_HPET0_IRQ             (LS2K1000_IRQ0_BASE+21)
#define INTC0_I2C0_IRQ              (LS2K1000_IRQ0_BASE+22)
#define INTC0_I2C1_IRQ              (LS2K1000_IRQ0_BASE+23)
#define INTC0_PWM0_IRQ              (LS2K1000_IRQ0_BASE+24)
#define INTC0_PWM1_IRQ              (LS2K1000_IRQ0_BASE+25)
#define INTC0_PWM2_IRQ              (LS2K1000_IRQ0_BASE+26)
#define INTC0_PWM3_IRQ              (LS2K1000_IRQ0_BASE+27)
#define INTC0_DC_IRQ                (LS2K1000_IRQ0_BASE+28)
#define INTC0_GPU_IRQ               (LS2K1000_IRQ0_BASE+29)
#define INTC0_VPU_IRQ               (LS2K1000_IRQ0_BASE+30)
#define INTC0_SDIO_IRQ              (LS2K1000_IRQ0_BASE+31)

/*
 * Interrupt Control 1 Interrupts
 */
#define LS2K1000_IRQ1_BASE          (LS2K1000_IRQ0_BASE+32)   	// 77

#define INTC1_PCIE00_IRQ            (LS2K1000_IRQ1_BASE+0)
#define INTC1_PCIE01_IRQ            (LS2K1000_IRQ1_BASE+1)
#define INTC1_PCIE02_IRQ            (LS2K1000_IRQ1_BASE+2)
#define INTC1_PCIE03_IRQ            (LS2K1000_IRQ1_BASE+3)
#define INTC1_PCIE10_IRQ            (LS2K1000_IRQ1_BASE+4)
#define INTC1_PCIE11_IRQ            (LS2K1000_IRQ1_BASE+5)
#define INTC1_HPET1_IRQ             (LS2K1000_IRQ1_BASE+6)
#define INTC1_HPET2_IRQ             (LS2K1000_IRQ1_BASE+7)
#define INTC1_TOY0_IRQ              (LS2K1000_IRQ1_BASE+8)
#define INTC1_TOY1_IRQ              (LS2K1000_IRQ1_BASE+9)
#define INTC1_TOY2_IRQ              (LS2K1000_IRQ1_BASE+10)
#define INTC1_TOY3_IRQ              (LS2K1000_IRQ1_BASE+11)
#define INTC1_DMA0_IRQ              (LS2K1000_IRQ1_BASE+12)
#define INTC1_DMA1_IRQ              (LS2K1000_IRQ1_BASE+13)
#define INTC1_DMA2_IRQ              (LS2K1000_IRQ1_BASE+14)
#define INTC1_DMA3_IRQ              (LS2K1000_IRQ1_BASE+15)
#define INTC1_DMA4_IRQ              (LS2K1000_IRQ1_BASE+16)
#define INTC1_OTG_IRQ               (LS2K1000_IRQ1_BASE+17)
#define INTC1_EHCI_IRQ              (LS2K1000_IRQ1_BASE+18)
#define INTC1_OHCI_IRQ              (LS2K1000_IRQ1_BASE+19)
#define INTC1_RTC0_IRQ              (LS2K1000_IRQ1_BASE+20)
#define INTC1_RTC1_IRQ              (LS2K1000_IRQ1_BASE+21)
#define INTC1_RTC2_IRQ              (LS2K1000_IRQ1_BASE+22)
#define INTC1_RSA_IRQ               (LS2K1000_IRQ1_BASE+23)
#define INTC1_AES_IRQ               (LS2K1000_IRQ1_BASE+24)
#define INTC1_DES_IRQ               (LS2K1000_IRQ1_BASE+25)
#define INTC1_GPIO4_31_IRQ          (LS2K1000_IRQ1_BASE+26)
#define INTC1_GPIO32_63_IRQ         (LS2K1000_IRQ1_BASE+27)
#define INTC1_GPIO0_IRQ             (LS2K1000_IRQ1_BASE+28)
#define INTC1_GPIO1_IRQ             (LS2K1000_IRQ1_BASE+29)
#define INTC1_GPIO2_IRQ             (LS2K1000_IRQ1_BASE+30)
#define INTC1_GPIO3_IRQ             (LS2K1000_IRQ1_BASE+31)

#define LS2K1000_IRQ_COUNT          (LS2K1000_IRQ1_BASE+32)   	// 标准中断总数

/*
 * for rtems interrupt initialize.
 *
 * XXX VECTOR_MIN / VECTOR_MAX are not same as 4.10
 *
 */
#define LS2K1000_MAXIMUM_VECTORS    109

#define BSP_INTERRUPT_VECTOR_MIN    0
#define BSP_INTERRUPT_VECTOR_MAX    LS2K1000_MAXIMUM_VECTORS

#ifdef __cplusplus
}
#endif

#endif /* ASM */

#endif /* _LS2K1000_IRQ_H */

