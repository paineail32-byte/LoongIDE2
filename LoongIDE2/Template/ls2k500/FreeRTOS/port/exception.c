/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * exception.c
 *
 * created: 2022-03-23
 *  author: Bian
 */

#include <stdio.h>
#include <larchintrin.h>

#include "regdef.h"
#include "ls2k500.h"
#include "ls2k500_irq.h"

extern void printk(const char *fmt, ...);

//-------------------------------------------------------------------------------------------------

static char *get_irq_name(int vector, int subcode)
{
	char *rt = NULL;

	switch (vector)
	{
		case LA_EXCEPTION_INT:
			rt = "Never ocurred normal interrupt but ocurred?";
			break;
		case LA_EXCEPTION_PIL:
			rt = "Invalid exception for Load operation exception.";
			break;
		case LA_EXCEPTION_PIS:
			rt = "Invalid exception for Store operation exception.";
			break;
		case LA_EXCEPTION_PIF:
			rt = "Invalid exception for Fetch operation exception.";
			break;
		case LA_EXCEPTION_PME:
			rt = "Page Modification Exception.";
			break;
		case LA_EXCEPTION_PNR:
			rt = "Page Non-Readable exception.";
			break;
		case LA_EXCEPTION_PNX:
			rt = "Page Non-eXecutable exception.";
			break;
		case LA_EXCEPTION_PPI:
			rt = "Page Privilege level Illegal exception.";
			break;
		case LA_EXCEPTION_ADEF_ADEM:
			if (subcode)
				rt = "Address error Exception for Memory access instructions exception.";
			else
				rt = "Address error Exception for Fetching instructions exception.";
			break;
		case LA_EXCEPTION_ALE:
			rt = "Address alignment fault Exception.";
			break;
		case LA_EXCEPTION_BCE:
			rt = "Bound Check Exception.";
			break;
		case LA_EXCEPTION_SYSCALL:
			rt = "System call exception.";
			break;
		case LA_EXCEPTION_BREAK:
			rt = "Breakpoint exception.";
			break;
		case LA_EXCEPTION_INE:
			rt = "Instruction Non-defined Exception.";
			break;
		case LA_EXCEPTION_IPE:
			rt = "Instruction Privilege error Exception.";
			break;
		case LA_EXCEPTION_FPD:
			rt = "Floating-Point instruction Disable exception.";
			break;
		case LA_EXCEPTION_SXD:
			rt = "128-bit vector (SIMD instructions) eXpansion instruction Disable exception.";
			break;
		case LA_EXCEPTION_ASXD:
			rt = "256-bit vector (Advanced SIMD instructions) eXpansion instruction Disable exception.";
			break;
		case LA_EXCEPTION_FPE_VFPE:
			if (subcode)
				rt = "Vecctor Floating-Point error Exception.";
			else
				rt = "Floating-Point error Exception.";
			break;
		case LA_EXCEPTION_WPEF_WPEM:
			if (subcode)
				rt = "WatchPoint Exception for Memory load/store watchpoint.";
			else
				rt = "WatchPoint Exception for Fetch watchpoint.";
			break;
		case LA_EXCEPTION_BTD:
			rt = "Binary Translation expansion instruction Disable exception.";
			break;
		case LA_EXCEPTION_BTE:
			rt = "Binary Translation related exceptions.";
			break;
		case LA_EXCEPTION_GSPR:
			rt = "Guest Sensitive Privileged Resource exception.";
			break;
		case LA_EXCEPTION_HVC:
			rt = "HyperVisor Call exception.";
			break;
		case LA_EXCEPTION_GCSC_GCHC:
			if (subcode)
				rt = "Guest CSR Hardware Change exception.";
			else
				rt = "Guest CSR Software Change exception.";
			break;

		case LS2K500_IRQ_SW0: 		rt = "Software0"; 	break;
		case LS2K500_IRQ_SW1: 		rt = "Software1"; 	break;
		case LS2K500_IRQ_HW0: 		rt = "Hardware0"; 	break;
		case LS2K500_IRQ_HW1: 		rt = "Hardware1"; 	break;
		case LS2K500_IRQ_HW2: 		rt = "Hardware2"; 	break;
		case LS2K500_IRQ_HW3: 		rt = "Hardware3"; 	break;
		case LS2K500_IRQ_HW4: 		rt = "Hardware4"; 	break;
		case LS2K500_IRQ_HW5: 		rt = "Hardware5"; 	break;
		case LS2K500_IRQ_HW6: 		rt = "Hardware6"; 	break;
		case LS2K500_IRQ_HW7: 		rt = "Hardware7"; 	break;
		case LS2K500_IRQ_PERF: 		rt = "Performance counter overflow"; break;
		case LS2K500_IRQ_TIMER: 	rt = "Timer"; 			break;
		case LS2K500_IRQ_IPI: 		rt = "Inter-processor"; break;

#if (!USE_EXTINT)   /* 使用传统中断 */

		case INTC0_SDIO0_1_IRQ: 	rt = "SDIO0~1"; 	break;
		case INTC0_USB3_IRQ: 		rt = "USB3"; 		break;
		case INTC0_GPU_IRQ: 		rt = "GPU"; 		break;
		case INTC0_DC_IRQ: 			rt = "DC"; 			break;
		case INTC0_PWM12_15_IRQ: 	rt = "PWM12~15"; 	break;
		case INTC0_PWM8_11_IRQ: 	rt = "PWM8~11"; 	break;
		case INTC0_PWM4_7_IRQ: 		rt = "PWM4~7"; 		break;
		case INTC0_PWM0_3_IRQ: 		rt = "PWM0~3"; 		break;
		case INTC0_I2C1_IRQ: 		rt = "I2C1"; 		break;
		case INTC0_I2C0_IRQ: 		rt = "I2C0"; 		break;
		case INTC0_HPET0_IRQ: 		rt = "HPET0"; 		break;
		case INTC0_NAND_IRQ: 		rt = "NAND"; 		break;
		case INTC0_SATA_IRQ: 		rt = "SATA"; 		break;
		case INTC0_SATA_PME_IRQ: 	rt = "SATA PME"; 	break;
		case INTC0_CAN2_3_IRQ: 		rt = "CAN2~3"; 		break;
		case INTC0_CAN0_1_IRQ: 		rt = "CAN0~1"; 		break;
		case INTC0_PRINT_IRQ: 		rt = "PRINT"; 		break;
		case INTC0_GMAC1_IRQ: 		rt = "GMAC1"; 		break;
		case INTC0_LS132_IRQ: 		rt = "LS132"; 		break;
		case INTC0_GMAC0_IRQ: 		rt = "GMAC0"; 		break;
		case INTC0_KEYBOARD_IRQ: 	rt = "Kypboard"; 	break;
		case INTC0_MOUSE_IRQ: 		rt = "Mouse"; 		break;
		case INTC0_RTC_TICK_IRQ: 	rt = "RTC tick"; 	break;
		case INTC0_TOY_TICK_IRQ: 	rt = "TOY tick"; 	break;
		case INTC0_THSENS_IRQ: 		rt = "Thsens"; 		break;
		case INTC0_AC97_IRQ: 		rt = "AC97"; 		break;
		case INTC0_LPC_IRQ: 		rt = "LPC"; 		break;
		case INTC0_HDA_IRQ: 		rt = "HDA"; 		break;
		case INTC0_UART3_IRQ: 		rt = "UART3"; 		break;
		case INTC0_UART2_IRQ: 		rt = "UART2"; 		break;
		case INTC0_UART1_IRQ: 		rt = "UART1"; 		break;
		case INTC0_UART0_IRQ: 		rt = "UART0"; 		break;
		case INTC1_UART7_9_IRQ: 	rt = "UART7~9"; 	break;
		case INTC1_UART4_6_IRQ: 	rt = "UART4~6"; 	break;
		case INTC1_GPIO96_127_IRQ: 	rt = "GPIO96~127"; 	break;
		case INTC1_GPIO64_95_IRQ: 	rt = "GPIO64~95"; 	break;
		case INTC1_GPIO32_63_IRQ: 	rt = "GPIO32~63"; 	break;
		case INTC1_GPIO0_31_IRQ: 	rt = "GPIO0~31"; 	break;
		case INTC1_I2C4_5_IRQ: 		rt = "I2C4~5"; 		break;
		case INTC1_I2C3_IRQ: 		rt = "I2C3"; 		break;
		case INTC1_I2C2_IRQ: 		rt = "I2C2"; 		break;
		case INTC1_RTC2_IRQ: 		rt = "RTC2"; 		break;
		case INTC1_RTC1_IRQ: 		rt = "RTC1"; 		break;
		case INTC1_RTC0_IRQ: 		rt = "RTC0"; 		break;
		case INTC1_OHCI_IRQ: 		rt = "OHCI"; 		break;
		case INTC1_EHCI_IRQ: 		rt = "EHCI"; 		break;
		case INTC1_OTG_IRQ: 		rt = "OTG"; 		break;
		case INTC1_HPET3_IRQ: 		rt = "HPET3"; 		break;
		case INTC1_DMA3_IRQ: 		rt = "DMA3"; 		break;
		case INTC1_DMA2_IRQ: 		rt = "DMA2"; 		break;
		case INTC1_DMA1_IRQ: 		rt = "DMA1"; 		break;
		case INTC1_DMA0_IRQ: 		rt = "DMA0"; 		break;
		case INTC1_ACPI_IRQ: 		rt = "ACPI"; 		break;
		case INTC1_TOY2_IRQ: 		rt = "TOY2"; 		break;
		case INTC1_TOY1_IRQ: 		rt = "TOY1"; 		break;
		case INTC1_TOY0_IRQ: 		rt = "TOY0"; 		break;
		case INTC1_HPET2_IRQ: 		rt = "HPET2"; 		break;
		case INTC1_HPET1_IRQ: 		rt = "HPET1"; 		break;
		case INTC1_SPI4_5_IRQ: 		rt = "SPI4~5"; 		break;
		case INTC1_SPI2_3_IRQ: 		rt = "SPI2~3"; 		break;
		case INTC1_SPI1_IRQ: 		rt = "SPI1"; 		break;
		case INTC1_SPI0_IRQ: 		rt = "SPI0"; 		break;
		case INTC1_PCIE1_IRQ: 		rt = "PCIE1"; 		break;
		case INTC1_PCIE0_IRQ: 		rt = "PCIE0"; 		break;

#else   /* 使用扩展中断 */

		case EXTINTC0_NAND_IRQ: 		rt = "NAND"; 		break;
		case EXTINTC0_HPET3_IRQ: 		rt = "HPET3"; 		break;
		case EXTINTC0_HPET2_IRQ: 		rt = "HPET2"; 		break;
		case EXTINTC0_HPET1_IRQ: 		rt = "HPET1"; 		break;
		case EXTINTC0_HPET0_IRQ: 		rt = "HPET0"; 		break;
		case EXTINTC0_SPI5_IRQ: 		rt = "SPI5"; 		break;
		case EXTINTC0_SPI4_IRQ: 		rt = "SPI4"; 		break;
		case EXTINTC0_SPI3_IRQ: 		rt = "SPI3"; 		break;
		case EXTINTC0_SPI2_IRQ: 		rt = "SPI2"; 		break;
		case EXTINTC0_MOUSE_IRQ: 		rt = "Mouse"; 		break;
		case EXTINTC0_KEYBOARD_IRQ: 	rt = "Keyboard"; 	break;
		case EXTINTC0_AC97_IRQ: 		rt = "AC97"; 		break;
		case EXTINTC0_I2C5_IRQ: 		rt = "I2C5"; 		break;
		case EXTINTC0_I2C4_IRQ: 		rt = "I2C4"; 		break;
		case EXTINTC0_I2C3_IRQ: 		rt = "I2C3"; 		break;
		case EXTINTC0_I2C2_IRQ: 		rt = "I2C2"; 		break;
		case EXTINTC0_I2C1_IRQ: 		rt = "I2C1"; 		break;
		case EXTINTC0_I2C0_IRQ: 		rt = "I2C0"; 		break;
		case EXTINTC0_CAN3_IRQ: 		rt = "CAN3"; 		break;
		case EXTINTC0_CAN2_IRQ: 		rt = "CAN2"; 		break;
		case EXTINTC0_CAN1_IRQ: 		rt = "CAN1"; 		break;
		case EXTINTC0_CAN0_IRQ: 		rt = "CAN0"; 		break;
		case EXTINTC0_UART9_IRQ: 		rt = "UART9"; 		break;
		case EXTINTC0_UART8_IRQ: 		rt = "UART8"; 		break;
		case EXTINTC0_UART7_IRQ: 		rt = "UART7"; 		break;
		case EXTINTC0_UART6_IRQ: 		rt = "UART6"; 		break;
		case EXTINTC0_UART5_IRQ: 		rt = "UART5"; 		break;
		case EXTINTC0_UART4_IRQ: 		rt = "UART4"; 		break;
		case EXTINTC0_UART3_IRQ: 		rt = "UART3"; 		break;
		case EXTINTC0_UART2_IRQ: 		rt = "UART2"; 		break;
		case EXTINTC0_UART1_IRQ: 		rt = "UART1"; 		break;
		case EXTINTC0_UART0_IRQ: 		rt = "UART0"; 		break;

		case EXTINTC1_LS132_IRQ: 		rt = "LS132"; 		break;
		case EXTINTC1_SDIO1_IRQ: 		rt = "SDIO1"; 		break;
		case EXTINTC1_SDIO0_IRQ: 		rt = "SDIO0"; 		break;
		case EXTINTC1_ACPI_IRQ: 		rt = "ACPI"; 		break;
		case EXTINTC1_PWM15_IRQ: 		rt = "PWM15"; 		break;
		case EXTINTC1_PWM14_IRQ: 		rt = "PWM14"; 		break;
		case EXTINTC1_PWM13_IRQ: 		rt = "PWM13"; 		break;
		case EXTINTC1_PWM12_IRQ: 		rt = "PWM12"; 		break;
		case EXTINTC1_PWM11_IRQ: 		rt = "PWM11"; 		break;
		case EXTINTC1_PWM10_IRQ: 		rt = "PWM10"; 		break;
		case EXTINTC1_PWM9_IRQ: 		rt = "PWM9"; 		break;
		case EXTINTC1_PWM8_IRQ: 		rt = "PWM8"; 		break;
		case EXTINTC1_PWM7_IRQ: 		rt = "PWM7"; 		break;
		case EXTINTC1_PWM6_IRQ: 		rt = "PWM6"; 		break;
		case EXTINTC1_PWM5_IRQ: 		rt = "PWM5"; 		break;
		case EXTINTC1_PWM4_IRQ: 		rt = "PWM4"; 		break;
		case EXTINTC1_PWM3_IRQ: 		rt = "PWM3"; 		break;
		case EXTINTC1_PWM2_IRQ: 		rt = "PWM2"; 		break;
		case EXTINTC1_PWM1_IRQ: 		rt = "PWM1"; 		break;
		case EXTINTC1_PWM0_IRQ: 		rt = "PWM0"; 		break;
		case EXTINTC1_RTC_TICK_IRQ: 	rt = "RTC tick"; 	break;
		case EXTINTC1_RTC2_IRQ: 		rt = "RTC2"; 		break;
		case EXTINTC1_RTC1_IRQ: 		rt = "RTC1"; 		break;
		case EXTINTC1_RTC0_IRQ: 		rt = "RTC0"; 		break;
		case EXTINTC1_TOY_TICK_IRQ: 	rt = "TOY tick"; 	break;
		case EXTINTC1_TOY2_IRQ: 		rt = "TOY2"; 		break;
		case EXTINTC1_TOY1_IRQ: 		rt = "TOY1"; 		break;
		case EXTINTC1_TOY0_IRQ: 		rt = "TOY0"; 		break;

		case EXTINTC2_GPIO32_34_IRQ: 	rt = "GPIO32~34"; 	break;
		case EXTINTC2_GPIO28_30_IRQ: 	rt = "GPIO28~30"; 	break;
		case EXTINTC2_GPIO24_26_IRQ: 	rt = "GPIO24~26"; 	break;
		case EXTINTC2_GPIO20_22_IRQ: 	rt = "GPIO20~22"; 	break;
		case EXTINTC2_GPIO16_18_IRQ: 	rt = "GPIO16~18"; 	break;
		case EXTINTC2_GPIO12_14_IRQ: 	rt = "GPIO12~14"; 	break;
		case EXTINTC2_GPIO8_10_IRQ: 	rt = "GPIO8~10"; 	break;
		case EXTINTC2_GPIO4_6_IRQ: 		rt = "GPIO4~6"; 	break;
		case EXTINTC2_GPIO0_2_IRQ: 		rt = "GPIO0~2"; 	break;
		case EXTINTC2_THSENS_IRQ: 		rt = "Thsens"; 		break;
		case EXTINTC2_HDA_IRQ: 			rt = "HDA"; 		break;
		case EXTINTC2_PRINTE_IRQ: 		rt = "Print0"; 		break;
		case EXTINTC2_PRINTS_IRQ: 		rt = "Print1"; 		break;
		case EXTINTC2_PCIE1_IRQ: 		rt = "PCIE1"; 		break;
		case EXTINTC2_PCIE0_IRQ: 		rt = "PCIE0"; 		break;
		case EXTINTC2_DC_IRQ: 			rt = "DC"; 			break;
		case EXTINTC2_SATA_PME1_IRQ: 	rt = "SATA PME1"; 	break;
		case EXTINTC2_SATA_PME0_IRQ: 	rt = "SATA PME0"; 	break;
		case EXTINTC2_GPU_IRQ: 			rt = "GPU"; 		break;
		case EXTINTC2_SATA1_IRQ: 		rt = "SATA1"; 		break;
		case EXTINTC2_SATA0_IRQ: 		rt = "SATA0"; 		break;
		case EXTINTC2_USB3_IRQ: 		rt = "USB3"; 		break;
		case EXTINTC2_OTG_IRQ: 			rt = "OTG"; 		break;
		case EXTINTC2_OHCI_IRQ: 		rt = "OHCI"; 		break;
		case EXTINTC2_EHCI_IRQ: 		rt = "EHCI"; 		break;
		case EXTINTC2_DMA3_IRQ: 		rt = "DMA3"; 		break;
		case EXTINTC2_DMA2_IRQ: 		rt = "DMA2"; 		break;
		case EXTINTC2_DMA1_IRQ: 		rt = "DMA1"; 		break;
		case EXTINTC2_DMA0_IRQ: 		rt = "DMA0"; 		break;
		case EXTINTC2_LPC_IRQ: 			rt = "LPC"; 		break;
		case EXTINTC2_SPI1_IRQ: 		rt = "SPI1"; 		break;
		case EXTINTC2_SPI0_IRQ: 		rt = "SPI0"; 		break;

		case EXTINTC3_GMAC1_IRQ: 		rt = "GMAC1"; 		break;
		case EXTINTC3_GMAC0_IRQ: 		rt = "GMAC0"; 		break;
		case EXTINTC3_GPIO120_122_IRQ: 	rt = "GPIO120~122"; break;
		case EXTINTC3_GPIO116_118_IRQ: 	rt = "GPIO116~118"; break;
		case EXTINTC3_GPIO112_114_IRQ: 	rt = "GPIO112~114"; break;
		case EXTINTC3_GPIO108_110_IRQ: 	rt = "GPIO108~110"; break;
		case EXTINTC3_GPIO104_106_IRQ: 	rt = "GPIO104~106"; break;
		case EXTINTC3_GPIO100_102_IRQ: 	rt = "GPIO100~102"; break;
		case EXTINTC3_GPIO96_98_IRQ: 	rt = "GPIO96~98"; 	break;
		case EXTINTC3_GPIO92_94_IRQ: 	rt = "GPIO92~94"; 	break;
		case EXTINTC3_GPIO88_90_IRQ: 	rt = "GPIO88~90"; 	break;
		case EXTINTC3_GPIO84_86_IRQ: 	rt = "GPIO84~86"; 	break;
		case EXTINTC3_GPIO80_82_IRQ: 	rt = "GPIO80~82"; 	break;
		case EXTINTC3_GPIO76_78_IRQ: 	rt = "GPIO76~78"; 	break;
		case EXTINTC3_GPIO72_74_IRQ: 	rt = "GPIO72~74"; 	break;
		case EXTINTC3_GPIO68_70_IRQ: 	rt = "GPIO68~70"; 	break;
		case EXTINTC3_GPIO64_66_IRQ: 	rt = "GPIO64~66"; 	break;
		case EXTINTC3_GPIO60_62_IRQ: 	rt = "GPIO60~62"; 	break;
		case EXTINTC3_GPIO56_58_IRQ: 	rt = "GPIO56~58"; 	break;
		case EXTINTC3_GPIO52_54_IRQ: 	rt = "GPIO52~54"; 	break;
		case EXTINTC3_GPIO48_50_IRQ: 	rt = "GPIO48~50"; 	break;
		case EXTINTC3_GPIO44_46_IRQ: 	rt = "GPIO44~46"; 	break;
		case EXTINTC3_GPIO40_42_IRQ: 	rt = "GPIO40~42"; 	break;
		case EXTINTC3_GPIO36_38_IRQ: 	rt = "GPIO36~38"; 	break;

#endif // #if (!USE_EXTINT)

		default:	rt = "";	break;
	}

	return rt;
}

void dump_registers(void *stack)
{
	unsigned long *regs = (unsigned long *)stack;

	printk("Dump Registers:\r\n");

	printk("zero = 0x%016lx\r\n", regs[R_ZERO]);
	printk("  ra = 0x%016lx\r\n", regs[R_RA]);
	printk("  tp = 0x%016lx\r\n", regs[R_TP]);
	printk("  sp = 0x%016lx\r\n", regs[R_SP]);
	printk("  a0 = 0x%016lx\r\n", regs[R_A0]);
	printk("  a1 = 0x%016lx\r\n", regs[R_A1]);
	printk("  a2 = 0x%016lx\r\n", regs[R_A2]);
	printk("  a3 = 0x%016lx\r\n", regs[R_A3]);
	printk("  a4 = 0x%016lx\r\n", regs[R_A4]);
	printk("  a5 = 0x%016lx\r\n", regs[R_A5]);
	printk("  a6 = 0x%016lx\r\n", regs[R_A6]);
	printk("  a7 = 0x%016lx\r\n", regs[R_A7]);
	printk("  t0 = 0x%016lx\r\n", regs[R_T0]);
	printk("  t1 = 0x%016lx\r\n", regs[R_T1]);
	printk("  t2 = 0x%016lx\r\n", regs[R_T2]);
	printk("  t3 = 0x%016lx\r\n", regs[R_T3]);
	printk("  t4 = 0x%016lx\r\n", regs[R_T4]);
	printk("  t5 = 0x%016lx\r\n", regs[R_T5]);
	printk("  t6 = 0x%016lx\r\n", regs[R_T6]);
	printk("  t7 = 0x%016lx\r\n", regs[R_T7]);
	printk("  t8 = 0x%016lx\r\n", regs[R_T8]);
	printk("  x  = 0x%016lx\r\n", regs[R_X ]);
	printk("  fp = 0x%016lx\r\n", regs[R_FP]);
	printk("  s0 = 0x%016lx\r\n", regs[R_S0]);
	printk("  s1 = 0x%016lx\r\n", regs[R_S1]);
	printk("  s2 = 0x%016lx\r\n", regs[R_S2]);
	printk("  s3 = 0x%016lx\r\n", regs[R_S3]);
	printk("  s4 = 0x%016lx\r\n", regs[R_S4]);
	printk("  s5 = 0x%016lx\r\n", regs[R_S5]);
	printk("  s6 = 0x%016lx\r\n", regs[R_S6]);
	printk("  s7 = 0x%016lx\r\n", regs[R_S7]);
	printk("  s8 = 0x%016lx\r\n", regs[R_S8]);
	printk("prmd = 0x%016lx\r\n", regs[R_PRMD]);
	printk("ecfg = 0x%016lx\r\n", regs[R_ECFG]);
	printk("estat= 0x%016lx\r\n", regs[R_ESTAT]);
	printk("epc  = 0x%016lx\r\n", regs[R_EPC]);

	printk("\r\n");
}

//-------------------------------------------------------------------------------------------------

void dump_exception_info_then_dead(int vector, uint64_t *stack)
{
    unsigned int subcode;

    loongarch_interrupt_disable();

    subcode = (unsigned int)stack[R_ESTAT];
    subcode &= CSR_ESTAT_ESUBCODE_MASK;
    subcode >>= CSR_ESTAT_ESUBCODE_SHIFT;

    if (vector < LS2K500_IRQ_SW0)
    {
        printk("\r\nUnhandled %s\r\n\r\n", get_irq_name(vector, subcode));
    }
    else
    {
        printk("\r\nUnhandled %s interrupt.\r\n\r\n", get_irq_name(vector, subcode));
    }

    dump_registers(stack);

    /*
     * dead loop
     */
    for (;;);

    /*
     * NEVER GO HERE!!!
     */
}

//-------------------------------------------------------------------------------------------------

void c_exception_handler(uint64_t *stack)
{
//  unsigned int ecfg  = (unsigned int)stack[R_ECFG];
	unsigned int estat = (unsigned int)stack[R_ESTAT];

    if ((estat & CSR_ESTAT_IS_MASK) == 0)
    {
        /**
         * exception 待处理
         */
        unsigned int exccode = (estat & CSR_ESTAT_EXC_MASK) >> CSR_ESTAT_EXC_SHIFT;

        if (exccode != 0)
        {
            dump_exception_info_then_dead(exccode, stack);
        }
    }

    return;
}

//-------------------------------------------------------------------------------------------------

/*
 * @@END
 */


