/*
 * Copyright (C) 2021-2023 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * exception.c
 *
 * created: 2023-05-01
 *  author: Bian
 */

#include <stdio.h>
#include <larchintrin.h>

#include "rtthread.h"
#include "rthw.h"

#include "cpu.h"
#include "regdef.h"

#include "ls1c103.h"
#include "ls1c103_irq.h"

//-----------------------------------------------------------------------------

char *get_irq_description(int vector)
{
	char *rt = NULL;

	switch (vector)
	{
        /* CPU IP Interrupts
         */
		case LS1C103_IRQ_SW0: 		rt = "Software0"; 	break;
		case LS1C103_IRQ_SW1: 		rt = "Software1"; 	break;
		case LS1C103_IRQ_ATIM: 	    rt = "ATIM"; 	    break;
		case LS1C103_IRQ_GTIM: 	    rt = "GTIM"; 	    break;
		case LS1C103_IRQ_ADC: 	    rt = "ADC"; 	    break;
		case LS1C103_IRQ_TICKER: 	rt = "Ticker"; 	    break;

        /* IP5 Interrupts from PMU
         */
		case LS1C103_IRQ_BTIM:      rt = "BTIM";        break;
		case LS1C103_IRQ_I2C:       rt = "I2C";         break;
		case LS1C103_IRQ_UART1:     rt = "UART1";       break;
		case LS1C103_IRQ_UART0:     rt = "UART0";       break;
		case LS1C103_IRQ_FLASH:     rt = "Flash";       break;
		case LS1C103_IRQ_SPI:       rt = "SPI";         break;
		case LS1C103_IRQ_DMA0:      rt = "DMA0";        break;
		case LS1C103_IRQ_DMA1:      rt = "DMA1";        break;
		case LS1C103_IRQ_DMA2:      rt = "DMA2";        break;
		case LS1C103_IRQ_DMA3:      rt = "DMA3";        break;

        /* IP6 Interrupts from INTC
         */
		case LS1C103_IRQ_WAKE:      rt = "Wake";        break;
		case LS1C103_IRQ_BATFAIL:   rt = "BatFail";     break;
		case LS1C103_IRQ_32KFAIL:   rt = "32KFail";     break;
		case LS1C103_IRQ_8MFAIL:    rt = "8MFail";      break;
		case LS1C103_IRQ_RTC:       rt = "RTC";         break;

        /* IP7 Interrupts from ExtINT
         */
		case LS1C103_IRQ_GPIO0:     rt = "gpio0";   break;
		case LS1C103_IRQ_GPIO1:     rt = "gpio1";   break;
		case LS1C103_IRQ_GPIO2:     rt = "gpio2";   break;
		case LS1C103_IRQ_GPIO3:     rt = "gpio3";   break;
		case LS1C103_IRQ_GPIO4:     rt = "gpio4";   break;
		case LS1C103_IRQ_GPIO5:     rt = "gpio5";   break;
		case LS1C103_IRQ_GPIO6:     rt = "gpio6";   break;
		case LS1C103_IRQ_GPIO7:     rt = "gpio7";   break;
		case LS1C103_IRQ_GPIO8:     rt = "gpio8";   break;
		case LS1C103_IRQ_GPIO9:     rt = "gpio9";   break;
		case LS1C103_IRQ_GPIO10:    rt = "gpio10";  break;
		case LS1C103_IRQ_GPIO11:    rt = "gpio11";  break;
		case LS1C103_IRQ_GPIO12:    rt = "gpio12";  break;
		case LS1C103_IRQ_GPIO13:    rt = "gpio13";  break;
		case LS1C103_IRQ_GPIO14:    rt = "gpio14";  break;
		case LS1C103_IRQ_GPIO15:    rt = "gpio15";  break;
		case LS1C103_IRQ_GPIO16:    rt = "gpio16";  break;
		case LS1C103_IRQ_GPIO17:    rt = "gpio17";  break;
		case LS1C103_IRQ_GPIO18:    rt = "gpio18";  break;
		case LS1C103_IRQ_GPIO19:    rt = "gpio19";  break;
		case LS1C103_IRQ_GPIO20:    rt = "gpio20";  break;
		case LS1C103_IRQ_GPIO21:    rt = "gpio21";  break;
		case LS1C103_IRQ_GPIO22:    rt = "gpio22";  break;
		case LS1C103_IRQ_GPIO23:    rt = "gpio23";  break;
		case LS1C103_IRQ_GPIO24:    rt = "gpio24";  break;
		case LS1C103_IRQ_GPIO25:    rt = "gpio25";  break;
		case LS1C103_IRQ_GPIO26:    rt = "gpio26";  break;
		case LS1C103_IRQ_GPIO27:    rt = "gpio27";  break;
		case LS1C103_IRQ_GPIO28:    rt = "gpio28";  break;
		default:	                rt = "unknow";	break;
	}

	return rt;
}

//-----------------------------------------------------------------------------

void dump_registers(unsigned int *regs)
{
	rt_kprintf("Dump Registers:\r\n");

	rt_kprintf("zero = 0x%08x\r\n", regs[R_ZERO]);
	rt_kprintf("  ra = 0x%08x\r\n", regs[R_RA]);
	rt_kprintf("  tp = 0x%08x\r\n", regs[R_TP]);
	rt_kprintf("  sp = 0x%08x\r\n", regs[R_SP]);
	rt_kprintf("  a0 = 0x%08x\r\n", regs[R_A0]);
	rt_kprintf("  a1 = 0x%08x\r\n", regs[R_A1]);
	rt_kprintf("  a2 = 0x%08x\r\n", regs[R_A2]);
	rt_kprintf("  a3 = 0x%08x\r\n", regs[R_A3]);
	rt_kprintf("  a4 = 0x%08x\r\n", regs[R_A4]);
	rt_kprintf("  a5 = 0x%08x\r\n", regs[R_A5]);
	rt_kprintf("  a6 = 0x%08x\r\n", regs[R_A6]);
	rt_kprintf("  a7 = 0x%08x\r\n", regs[R_A7]);
	rt_kprintf("  t0 = 0x%08x\r\n", regs[R_T0]);
	rt_kprintf("  t1 = 0x%08x\r\n", regs[R_T1]);
	rt_kprintf("  t2 = 0x%08x\r\n", regs[R_T2]);
	rt_kprintf("  t3 = 0x%08x\r\n", regs[R_T3]);
	rt_kprintf("  t4 = 0x%08x\r\n", regs[R_T4]);
	rt_kprintf("  t5 = 0x%08x\r\n", regs[R_T5]);
	rt_kprintf("  t6 = 0x%08x\r\n", regs[R_T6]);
	rt_kprintf("  t7 = 0x%08x\r\n", regs[R_T7]);
	rt_kprintf("  t8 = 0x%08x\r\n", regs[R_T8]);
	rt_kprintf("  x  = 0x%08x\r\n", regs[R_X ]);
	rt_kprintf("  fp = 0x%08x\r\n", regs[R_FP]);
	rt_kprintf("  s0 = 0x%08x\r\n", regs[R_S0]);
	rt_kprintf("  s1 = 0x%08x\r\n", regs[R_S1]);
	rt_kprintf("  s2 = 0x%08x\r\n", regs[R_S2]);
	rt_kprintf("  s3 = 0x%08x\r\n", regs[R_S3]);
	rt_kprintf("  s4 = 0x%08x\r\n", regs[R_S4]);
	rt_kprintf("  s5 = 0x%08x\r\n", regs[R_S5]);
	rt_kprintf("  s6 = 0x%08x\r\n", regs[R_S6]);
	rt_kprintf("  s7 = 0x%08x\r\n", regs[R_S7]);
	rt_kprintf("  s8 = 0x%08x\r\n", regs[R_S8]);
	rt_kprintf("prmd = 0x%08x\r\n", regs[R_PRMD]);
	rt_kprintf("ecfg = 0x%08x\r\n", regs[R_ECFG]);
	rt_kprintf("estat= 0x%08x\r\n", regs[R_ESTAT]);
	rt_kprintf("epc  = 0x%08x\r\n", regs[R_ERA]);

	rt_kprintf("\r\n");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static char *get_exception_description(unsigned int execcode)
{
	char *rt = NULL;

	switch (execcode)
	{
		case CSR_ESTAT_ECODE_PIL:   rt = "Invalid exception for Load operation exception."; break;
		case CSR_ESTAT_ECODE_PIS:   rt = "Invalid exception for Store operation exception."; break;
		case CSR_ESTAT_ECODE_PIF:   rt = "Invalid exception for Fetch operation exception."; break;
		case CSR_ESTAT_ECODE_PME:   rt = "Page Modification Exception."; break;
		case CSR_ESTAT_ECODE_PPI:   rt = "Page Privilege level Illegal exception."; break;
		case CSR_ESTAT_ECODE_ADEF:  rt = "Address error Exception for Memory access instructions exception."; break;
		case CSR_ESTAT_ECODE_ADEM:  rt = "Address error Exception for Fetching instructions exception."; break;
		case CSR_ESTAT_ECODE_ALE:   rt = "Address alignment fault Exception."; break;
		case CSR_ESTAT_ECODE_SYS:   rt = "System call exception."; break;
		case CSR_ESTAT_ECODE_BREAK: rt = "Breakpoint exception."; break;
		case CSR_ESTAT_ECODE_INE:   rt = "Instruction Non-defined Exception."; break;
		case CSR_ESTAT_ECODE_IPE:   rt = "Instruction Privilege error Exception."; break;
		case CSR_ESTAT_ECODE_FPD:   rt = "Floating-Point instruction Disable exception."; break;
		case CSR_ESTAT_ECODE_FPE:   rt = "Floating-Point error Exception."; break;
        case CSR_ESTAT_ECODE_TLBR:  rt = "TLB-Refill error Exception."; break;
        default:                    rt = "Unknown Exception."; break;
	}

	return rt;
}

//-----------------------------------------------------------------------------

void c_exception_handler(unsigned int *regs)
{
    volatile unsigned int execcode;

    loongarch_interrupt_disable();

    execcode  = regs[R_ESTAT];
    execcode &= CSR_ESTAT_ESUBCODE_MASK | CSR_ESTAT_ECODE_MASK;
    
    rt_kprintf("Unhandled exception: %s\r\n", get_exception_description(execcode));

    execcode = __csrrd_w(LA_CSR_BADV);
    rt_kprintf("badvaddr == 0x%08x\r\n", execcode);

    dump_registers(regs);

    /*
     * dead loop
     */
    for (;;);

    /*
     * NEVER GO HERE!!!
     */
}

//-----------------------------------------------------------------------------

/*
 * @@END
 */



