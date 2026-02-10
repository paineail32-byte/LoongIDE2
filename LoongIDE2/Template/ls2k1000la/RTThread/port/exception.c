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
#include "ls2k1000.h"
#include "ls2k1000_irq.h"

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

		case LS2K1000LA_IRQ_SW0: 	rt = "Software0"; 	break;
		case LS2K1000LA_IRQ_SW1: 	rt = "Software1"; 	break;
		case LS2K1000LA_IRQ_HW0: 	rt = "Hardware0"; 	break;
		case LS2K1000LA_IRQ_HW1: 	rt = "Hardware1"; 	break;
		case LS2K1000LA_IRQ_HW2: 	rt = "Hardware2"; 	break;
		case LS2K1000LA_IRQ_HW3: 	rt = "Hardware3"; 	break;
		case LS2K1000LA_IRQ_HW4: 	rt = "Hardware4"; 	break;
		case LS2K1000LA_IRQ_HW5: 	rt = "Hardware5"; 	break;
		case LS2K1000LA_IRQ_HW6: 	rt = "Hardware6"; 	break;
		case LS2K1000LA_IRQ_HW7: 	rt = "Hardware7"; 	break;
		case LS2K1000LA_IRQ_PERF: 	rt = "Performance counter overflow"; break;
		case LS2K1000LA_IRQ_TIMER: 	rt = "Timer"; 			break;
		case LS2K1000LA_IRQ_IPI: 	rt = "Inter-processor"; break;

		case INTC0_UART0_3_IRQ: 	rt = "UART0_3"; 	break;
		case INTC0_UART4_7_IRQ: 	rt = "UART4_7"; 	break;
		case INTC0_UART8_11_IRQ: 	rt = "UART8_11"; 	break;
		case INTC0_E1_IRQ:          rt = "E1"; 		    break;
		case INTC0_HDA_IRQ: 		rt = "HDA"; 		break;
		case INTC0_I2S_IRQ: 		rt = "I2S"; 		break;
		case INTC0_THSENS_IRQ: 		rt = "THSENS"; 		break;
		case INTC0_TOY_TICK_IRQ: 	rt = "TOY_TICK"; 	break;
		case INTC0_RTC_TICK_IRQ: 	rt = "RTC_TICK"; 	break;
		case INTC0_CAMERA_IRQ: 		rt = "CAMERA"; 		break;
		case INTC0_GMAC0_SBD_IRQ: 	rt = "GMAC0_SBD"; 	break;
		case INTC0_GMAC0_PMT_IRQ: 	rt = "GMAC0_PMT"; 	break;
		case INTC0_GMAC1_SBD_IRQ: 	rt = "GMAC1_SBD"; 	break;
		case INTC0_GMAC1_PMT_IRQ: 	rt = "GMAC1_PMT"; 	break;
		case INTC0_CAN0_IRQ: 		rt = "CAN0"; 		break;
		case INTC0_CAN1_IRQ: 		rt = "CAN1"; 		break;
		case INTC0_SPI_IRQ: 		rt = "SPI"; 		break;
		case INTC0_SATA_IRQ: 		rt = "SATA"; 		break;
		case INTC0_NAND_IRQ: 		rt = "NAND"; 		break;
		case INTC0_HPET0_IRQ: 		rt = "HPET0"; 		break;
		case INTC0_I2C0_IRQ: 		rt = "I2C0"; 		break;
		case INTC0_I2C1_IRQ: 		rt = "I2C1"; 		break;
		case INTC0_PWM0_IRQ: 		rt = "PWM0"; 		break;
		case INTC0_PWM1_IRQ: 		rt = "PWM1"; 		break;
		case INTC0_PWM2_IRQ: 		rt = "PWM2"; 		break;
		case INTC0_PWM3_IRQ: 		rt = "PWM3"; 		break;
		case INTC0_DC_IRQ: 		    rt = "DC"; 		    break;
		case INTC0_GPU_IRQ: 		rt = "GPU"; 		break;
		case INTC0_VPU_IRQ: 		rt = "VPU"; 		break;
		case INTC0_SDIO_IRQ: 		rt = "SDIO"; 		break;

		case INTC1_PCIE00_IRQ: 		rt = "PCIE0-0"; 	break;
		case INTC1_PCIE01_IRQ: 		rt = "PCIE0-1"; 	break;
		case INTC1_PCIE02_IRQ: 		rt = "PCIE0-2"; 	break;
		case INTC1_PCIE03_IRQ: 		rt = "PCIE0-3"; 	break;
		case INTC1_PCIE10_IRQ: 		rt = "PCIE1-0"; 	break;
		case INTC1_PCIE11_IRQ: 		rt = "PCIE1-1"; 	break;
		case INTC1_HPET1_IRQ: 		rt = "HPET1"; 		break;
		case INTC1_HPET2_IRQ: 		rt = "HPET2"; 		break;
		case INTC1_TOY0_IRQ: 		rt = "TOY0"; 		break;
		case INTC1_TOY1_IRQ: 		rt = "TOY1"; 		break;
		case INTC1_TOY2_IRQ: 		rt = "TOY2"; 		break;
		case INTC1_TOY3_IRQ: 		rt = "TOY3"; 		break;
		case INTC1_DMA0_IRQ: 		rt = "DMA0"; 		break;
		case INTC1_DMA1_IRQ: 		rt = "DMA1"; 		break;
		case INTC1_DMA2_IRQ: 		rt = "DMA2"; 		break;
		case INTC1_DMA3_IRQ: 		rt = "DMA3"; 		break;
		case INTC1_DMA4_IRQ: 		rt = "DMA4"; 		break;
		case INTC1_OTG_IRQ: 		rt = "OTG"; 		break;
		case INTC1_EHCI_IRQ: 		rt = "EHCI"; 		break;
		case INTC1_OHCI_IRQ: 		rt = "OHCI"; 		break;
		case INTC1_RTC0_IRQ: 		rt = "RTC0"; 		break;
		case INTC1_RTC1_IRQ: 		rt = "RTC1"; 		break;
		case INTC1_RTC2_IRQ: 		rt = "RTC2"; 		break;
		case INTC1_RSA_IRQ: 		rt = "RSA"; 		break;
		case INTC1_AES_IRQ: 		rt = "AES"; 		break;
		case INTC1_DES_IRQ: 		rt = "DES"; 		break;
		case INTC1_GPIO4_31_IRQ: 	rt = "GPIO4_31"; 	break;
		case INTC1_GPIO32_63_IRQ: 	rt = "GPIO32_63"; 	break;
		case INTC1_GPIO0_IRQ: 		rt = "GPIO0"; 		break;
		case INTC1_GPIO1_IRQ: 		rt = "GPIO1"; 		break;
		case INTC1_GPIO2_IRQ: 		rt = "GPIO2"; 		break;
		case INTC1_GPIO3_IRQ: 		rt = "GPIO3"; 		break;

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

    if (vector < LS2K1000LA_IRQ_SW0)
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
         * exception ´ý´¦Àí
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
 
