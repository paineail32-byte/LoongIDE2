/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * exception.c
 *
 * created: 2024-06-20
 *  author: Bian
 */

#include <stdio.h>
#include <larchintrin.h>

#include "regdef.h"
#include "ls2k300.h"
#include "ls2k300_irq.h"

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

		case LS2K300_IRQ_SW0: 		rt = "Software0"; 	break;
		case LS2K300_IRQ_SW1: 		rt = "Software1"; 	break;
		case LS2K300_IRQ_HW0: 		rt = "Hardware0"; 	break;
		case LS2K300_IRQ_HW1: 		rt = "Hardware1"; 	break;
		case LS2K300_IRQ_HW2: 		rt = "Hardware2"; 	break;
		case LS2K300_IRQ_HW3: 		rt = "Hardware3"; 	break;
		case LS2K300_IRQ_HW4: 		rt = "Hardware4"; 	break;
		case LS2K300_IRQ_HW5: 		rt = "Hardware5"; 	break;
		case LS2K300_IRQ_HW6: 		rt = "Hardware6"; 	break;
		case LS2K300_IRQ_HW7: 		rt = "Hardware7"; 	break;
		case LS2K300_IRQ_PERF: 		rt = "Performance counter overflow"; break;
		case LS2K300_IRQ_TIMER: 	rt = "Timer"; 			break;
		case LS2K300_IRQ_IPI: 		rt = "Inter-processor"; break;

#if (!USE_EXTINT)   /* 使用传统中断 */

		case INTC0_UART0_IRQ: 	        rt = "UART0"; 	    break;
		case INTC0_UART1_IRQ: 	        rt = "UART1"; 	    break;
		case INTC0_UART_2_5_IRQ: 	    rt = "UART_2_5"; 	break;
		case INTC0_UART_6_9_IRQ: 	    rt = "UART_6_9"; 	break;
		case INTC0_I2C_0_1_IRQ: 	    rt = "I2C_0_1"; 	break;
		case INTC0_I2C_2_3_IRQ: 	    rt = "I2C_2_3"; 	break;
		case INTC0_SPI2_IRQ: 	 	    rt = "SPI2"; 	    break;
		case INTC0_SPI3_IRQ: 	        rt = "SPI3"; 	    break;
		case INTC0_CAN0_IRQ: 	        rt = "CAN0"; 	    break;
		case INTC0_CAN1_IRQ: 	        rt = "CAN1"; 	    break;
		case INTC0_CAN2_IRQ: 	        rt = "CAN2"; 	    break;
		case INTC0_CAN3_IRQ: 	        rt = "CAN3"; 	    break;
		case INTC0_I2S_IRQ: 	        rt = "I2S"; 	    break;
		case INTC0_ATIMER_IRQ: 	        rt = "ATIMER"; 	    break;
		case INTC0_GTIMER_IRQ: 	        rt = "GTIMER"; 	    break;
		case INTC0_BTIMER_IRQ: 	        rt = "BTIMER"; 	    break;
		case INTC0_PWM_0_1_IRQ: 	    rt = "PWM_0_1"; 	break;
		case INTC0_PWM_2_3_IRQ: 	    rt = "PWM_2_3"; 	break;
		case INTC0_ADC_IRQ: 	        rt = "ADC"; 	    break;
		case INTC0_HPET0_IRQ: 	        rt = "HPET0"; 	    break;
		case INTC0_HPET1_IRQ: 	        rt = "HPET1"; 	    break;
		case INTC0_HPET2_IRQ: 	        rt = "HPET2"; 	    break;
		case INTC0_HPET3_IRQ: 	        rt = "HPET3"; 	    break;
		case INTC0_DMA0_IRQ: 	        rt = "DMA0"; 	    break;
		case INTC0_DMA1_IRQ: 	        rt = "DMA1"; 	    break;
		case INTC0_DMA2_IRQ: 	        rt = "DMA2"; 	    break;
		case INTC0_DMA3_IRQ: 	        rt = "DMA3"; 	    break;
		case INTC0_DMA4_IRQ: 	        rt = "DMA4"; 	    break;
		case INTC0_DMA5_IRQ: 	        rt = "DMA5"; 	    break;
		case INTC0_DMA6_IRQ: 	        rt = "DMA6"; 	    break;
		case INTC0_DMA7_IRQ: 	        rt = "DMA7"; 	    break;
		case INTC0_SDIO0_IRQ: 	        rt = "SDIO0"; 	    break;

		case INTC1_SDIO1_IRQ: 	        rt = "SDIO1"; 	    break;
		case INTC1_SDIO0_DMA_IRQ: 	    rt = "SDIO0_DMA"; 	break;
		case INTC1_SDIO1_DMA_IRQ: 	    rt = "SDIO1_DMA"; 	break;
		case INTC1_ENCRYPT_DMA_IRQ: 	rt = "ENCRYPT_DMA"; break;
		case INTC1_AES_IRQ: 	        rt = "AES"; 	    break;
		case INTC1_DES_IRQ: 	        rt = "DES"; 	    break;
		case INTC1_SM3_IRQ: 	        rt = "SM3"; 	    break;
		case INTC1_SM4_IRQ: 	        rt = "SM4"; 	    break;
		case INTC1_RTC_IRQ: 	        rt = "RTC"; 	    break;
		case INTC1_TOY_IRQ: 	        rt = "TOY"; 	    break;
		case INTC1_RTC_TICK_IRQ: 	    rt = "RTC_TICK"; 	break;
		case INTC1_TOY_TICK_IRQ: 	    rt = "TOY_TICK"; 	break;
		case INTC1_SPI0_IRQ: 	        rt = "SPI0"; 	    break;
		case INTC1_SPI1_IRQ: 	        rt = "SPI1"; 	    break;
		case INTC1_EHCI_IRQ: 	        rt = "EHCI"; 	    break;
		case INTC1_OHCI_IRQ: 	        rt = "OHCI"; 	    break;
		case INTC1_OTG_IRQ: 	        rt = "OTG"; 	    break;
		case INTC1_GMAC0_IRQ: 	        rt = "GMAC0"; 	    break;
		case INTC1_GMAC1_IRQ: 	        rt = "GMAC1"; 	    break;
		case INTC1_DC_IRQ: 	            rt = "DC"; 	        break;
		case INTC1_THSENS_IRQ: 	        rt = "THSENS"; 	    break;
		case INTC1_GPIO_0_15_IRQ: 	    rt = "GPIO_0_15"; 	break;
		case INTC1_GPIO_16_31_IRQ: 	    rt = "GPIO_16_31"; 	break;
		case INTC1_GPIO_32_47_IRQ: 	    rt = "GPIO_32_47"; 	break;
		case INTC1_GPIO_48_63_IRQ: 	    rt = "GPIO_48_63"; 	break;
		case INTC1_GPIO_64_79_IRQ: 	    rt = "GPIO_64_79"; 	break;
		case INTC1_GPIO_80_95_IRQ: 	    rt = "GPIO_80_95"; 	break;
		case INTC1_GPIO_96_105_IRQ: 	rt = "GPIO_96_105"; break;
		case INTC1_DDR_ECC0_IRQ: 	    rt = "DDR_ECC0"; 	break;
		case INTC1_DDR_ECC1_IRQ: 	    rt = "DDR_ECC1"; 	break;

#else   /* 使用扩展中断 */

		case EXTI0_UART0_IRQ:           rt = "UART0"; 	    break;
		case EXTI0_UART1_IRQ: 	        rt = "UART1"; 	    break;
		case EXTI0_UART2_IRQ:           rt = "UART2"; 	    break;
		case EXTI0_UART3_IRQ: 	        rt = "UART3"; 	    break;
		case EXTI0_UART4_IRQ: 	        rt = "UART4"; 	    break;
		case EXTI0_UART5_IRQ: 	        rt = "UART5"; 	    break;
		case EXTI0_UART6_IRQ: 	        rt = "UART6"; 	    break;
		case EXTI0_UART7_IRQ: 	        rt = "UART7";     	break;
		case EXTI0_UART8_IRQ: 	        rt = "UART8"; 	    break;
		case EXTI0_UART9_IRQ: 	        rt = "UART9"; 	    break;
		case EXTI0_I2C0_IRQ: 	        rt = "I2C0"; 	    break;
		case EXTI0_I2C1_IRQ: 	        rt = "I2C1"; 	    break;
		case EXTI0_I2C2_IRQ: 	        rt = "I2C2"; 	    break;
		case EXTI0_I2C3_IRQ: 	        rt = "I2C3"; 	    break;
		case EXTI0_SPI2_IRQ: 	        rt = "SPI2"; 	    break;
		case EXTI0_SPI3_IRQ: 	        rt = "SPI3"; 	    break;
		case EXTI0_CAN0_CORE_IRQ: 	    rt = "CAN0_CORE"; 	break;
		case EXTI0_CAN0_BUF_IRQ: 	    rt = "CAN0_BUF"; 	break;
		case EXTI0_CAN1_CORE_IRQ: 	    rt = "CAN1_CORE"; 	break;
		case EXTI0_CAN1_BUF_IRQ: 	    rt = "CAN1_BUF"; 	break;
		case EXTI0_CAN2_CORE_IRQ: 	    rt = "CAN2_CORE"; 	break;
		case EXTI0_CAN2_BUF_IRQ: 	    rt = "CAN2_BUF"; 	break;
		case EXTI0_CAN3_CORE_IRQ: 	    rt = "CAN3_CORE"; 	break;
		case EXTI0_CAN3_BUF_IRQ: 	    rt = "CAN3_BUF"; 	break;
		case EXTI0_I2S_IRQ: 	        rt = "I2S"; 	    break;
		case EXTI0_ATIMER_IRQ: 	        rt = "ATIMER"; 	    break;
		case EXTI0_GTIMER_IRQ: 	        rt = "GTIMER"; 	    break;
		case EXTI0_BTIMER_IRQ: 	        rt = "BTIMER"; 	    break;
		case EXTI0_PWM0_IRQ: 	        rt = "PWM0"; 	    break;
		case EXTI0_PWM1_IRQ: 	        rt = "PWM1"; 	    break;
		case EXTI0_PWM2_IRQ: 	        rt = "PWM2"; 	    break;
		case EXTI0_PWM3_IRQ: 	        rt = "PWM3"; 	    break;

		case EXTI1_ADC_IRQ: 	        rt = "ADC"; 	    break;
		case EXTI1_HPET0_0_IRQ: 	    rt = "HPET0_0"; 	break;
		case EXTI1_HPET0_1_IRQ: 	    rt = "HPET0_1"; 	break;
		case EXTI1_HPET0_2_IRQ: 	    rt = "HPET0_2"; 	break;
		case EXTI1_HPET1_0_IRQ: 	    rt = "HPET1_0"; 	break;
		case EXTI1_HPET1_1_IRQ: 	    rt = "HPET1_1"; 	break;
		case EXTI1_HPET1_2_IRQ: 	    rt = "HPET1_2"; 	break;
		case EXTI1_HPET2_0_IRQ: 	    rt = "HPET2_0"; 	break;
		case EXTI1_HPET2_1_IRQ: 	    rt = "HPET2_1"; 	break;
		case EXTI1_HPET2_2_IRQ: 	    rt = "HPET2_2"; 	break;
		case EXTI1_HPET3_0_IRQ: 	    rt = "HPET3_0"; 	break;
		case EXTI1_HPET3_1_IRQ: 	    rt = "HPET3_1"; 	break;
		case EXTI1_HPET3_2_IRQ: 	    rt = "HPET3_2"; 	break;
		case EXTI1_DMA0_IRQ: 	        rt = "DMA0"; 	    break;
		case EXTI1_DMA1_IRQ: 	        rt = "DMA1"; 	    break;
		case EXTI1_DMA2_IRQ: 	        rt = "DMA2"; 	    break;
		case EXTI1_DMA3_IRQ: 	        rt = "DMA3"; 	    break;
		case EXTI1_DMA4_IRQ: 	        rt = "DMA4"; 	    break;
		case EXTI1_DMA5_IRQ: 	        rt = "DMA5"; 	    break;
		case EXTI1_DMA6_IRQ: 	        rt = "DMA6"; 	    break;
		case EXTI1_DMA7_IRQ: 	        rt = "DMA7"; 	    break;
		case EXTI1_SDIO0_CTRL_IRQ: 	    rt = "SDIO0_CTRL"; 	break;
		case EXTI1_SDIO1_CTRL_IRQ: 	    rt = "SDIO1_CTRL"; 	break;
		case EXTI1_SDIO0_DMA_IRQ: 	    rt = "SDIO0_DMA"; 	break;
		case EXTI1_SDIO1_DMA_IRQ: 	    rt = "SDIO1_DMA"; 	break;
		case EXTI1_ENCRYPT_DMA_IRQ: 	rt = "ENCRYPT_DMA"; break;
		case EXTI1_AES_IRQ: 	        rt = "AES"; 	    break;
		case EXTI1_DES_IRQ: 	        rt = "DES"; 	    break;
		case EXTI1_SM3_IRQ: 	        rt = "SM3"; 	    break;
		case EXTI1_SM4_IRQ: 	        rt = "SM4"; 	    break;
		case EXTI1_RTC_0_IRQ: 	        rt = "RTC_0"; 	    break;
		case EXTI1_RTC_1_IRQ: 	        rt = "RTC_1"; 	    break;

		case EXTI2_RTC_2_IRQ: 	        rt = "RTC_2"; 	    break;
		case EXTI2_TOY_0_IRQ: 	        rt = "TOY_0"; 	    break;
		case EXTI2_TOY_1_IRQ: 	        rt = "TOY_1"; 	    break;
		case EXTI2_TOY_2_IRQ: 	        rt = "TOY_2"; 	    break;
		case EXTI2_RTC_TICK_IRQ: 	    rt = "RTC_TICK"; 	break;
		case EXTI2_TOY_TICK_IRQ: 	    rt = "TOY_TICK"; 	break;
		case EXTI2_SPI0_IRQ: 	        rt = "SPI0"; 	    break;
		case EXTI2_SPI1_IRQ: 	        rt = "SPI1"; 	    break;
		case EXTI2_EHCI_IRQ: 	        rt = "EHCI"; 	    break;
		case EXTI2_OHCI_IRQ: 	        rt = "OHCI"; 	    break;
		case EXTI2_OTG_IRQ: 	        rt = "OTG"; 	    break;
		case EXTI2_GMAC0_IRQ: 	        rt = "GMAC0"; 	    break;
		case EXTI2_GMAC1_IRQ: 	        rt = "GMAC1"; 	    break;
		case EXTI2_DC_IRQ: 	            rt = "DC"; 	        break;
		case EXTI2_THSENS_IRQ: 	        rt = "THSENS"; 	    break;
		case EXTI2_GPIO_0_3_IRQ: 	    rt = "GPIO_0_3"; 	break;
		case EXTI2_GPIO_4_7_IRQ: 	    rt = "GPIO_4_7"; 	break;
		case EXTI2_GPIO_8_11_IRQ: 	    rt = "GPIO_8_11"; 	break;
		case EXTI2_GPIO_12_15_IRQ: 	    rt = "GPIO_12_15"; 	break;
		case EXTI2_GPIO_16_19_IRQ: 	    rt = "GPIO_16_19"; 	break;
		case EXTI2_GPIO_20_23_IRQ: 	    rt = "GPIO_20_23"; 	break;
		case EXTI2_GPIO_24_27_IRQ: 	    rt = "GPIO_24_27"; 	break;
		case EXTI2_GPIO_28_31_IRQ: 	    rt = "GPIO_28_31"; 	break;
		case EXTI2_GPIO_32_35_IRQ: 	    rt = "GPIO_32_35"; 	break;
		case EXTI2_GPIO_36_39_IRQ: 	    rt = "GPIO_36_39"; 	break;
		case EXTI2_GPIO_40_43_IRQ: 	    rt = "GPIO_40_43"; 	break;
		case EXTI2_GPIO_44_47_IRQ: 	    rt = "GPIO_44_47"; 	break;
		case EXTI2_GPIO_48_51_IRQ: 	    rt = "GPIO_48_51"; 	break;
		case EXTI2_GPIO_52_55_IRQ: 	    rt = "GPIO_52_55"; 	break;
		case EXTI2_GPIO_56_59_IRQ: 	    rt = "GPIO_56_59"; 	break;
		case EXTI2_GPIO_60_63_IRQ: 	    rt = "GPIO_60_63"; 	break;
		case EXTI2_GPIO_64_67_IRQ: 	    rt = "GPIO_64_67"; 	break;

		case EXTI3_GPIO_68_71_IRQ: 	    rt = "GPIO_68_71"; 	break;
		case EXTI3_GPIO_72_75_IRQ: 	    rt = "GPIO_72_75"; 	break;
		case EXTI3_GPIO_76_79_IRQ: 	    rt = "GPIO_76_79"; 	break;
		case EXTI3_GPIO_80_83_IRQ: 	    rt = "GPIO_80_83"; 	break;
		case EXTI3_GPIO_84_87_IRQ: 	    rt = "GPIO_84_87"; 	break;
		case EXTI3_GPIO_88_91_IRQ: 	    rt = "GPIO_88_91"; 	break;
		case EXTI3_GPIO_92_95_IRQ: 	    rt = "GPIO_92_95"; 	break;
		case EXTI3_GPIO_96_99_IRQ:      rt = "GPIO_96_99"; 	break;
		case EXTI3_GPIO_100_103_IRQ: 	rt = "GPIO_100_103"; break;
		case EXTI3_GPIO_104_105_IRQ: 	rt = "GPIO_104_105"; break;
		case EXTI3_DDR_ECC0_IRQ: 	    rt = "DDR_ECC0"; 	break;
		case EXTI3_DDR_ECC1_IRQ: 	    rt = "DDR_ECC1"; 	break;

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

    if (vector < LS2K300_IRQ_SW0)
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
 
