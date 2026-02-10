
/*
 * Copyright (C) 2021-2023 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * irq.c
 *
 * created: 2023-06-10
 *  author: Bian
 */

#include <stdio.h>
#include <larchintrin.h>

#include "cpu.h"
#include "regdef.h"

#include "ls1c103.h"
#include "ls1c103_irq.h"

extern int printk(const char* format, ...);

//-----------------------------------------------------------------------------
// 中断向量表
//-----------------------------------------------------------------------------

typedef struct
{
    irq_handler_t   isr;
    void           *arg;
} ISR_Table_t;

static ISR_Table_t isr_table[LS1C103_IRQ_COUNT];    // 在内存中

//-----------------------------------------------------------------------------
// 初始化中断向量表
//-----------------------------------------------------------------------------

static void ls1c103_default_isr(int vector, void *arg);

void ls1c103_init_isr_table(void)
{
    int i;
    
    for (i=0; i<LS1C103_IRQ_COUNT; i++)
    {
        isr_table[i].isr = ls1c103_default_isr;
    }
}

//-----------------------------------------------------------------------------
// 默认中断
//-----------------------------------------------------------------------------

extern char *get_irq_description(int vector);

static void ls1c103_default_isr(int vector, void *arg)
{
    unsigned int *stack = (unsigned int *)arg;
	unsigned int ecfg  = stack[R_ECFG];
	unsigned int estat = stack[R_ESTAT];
	
    char *irqname = get_irq_description(vector);
    
	printk("Unhandled isr %s:\r\n  vector=%i, ecfg=0x%08X, estat=0x%08X\n",
            irqname, vector, ecfg, estat);

	return;
}

//-----------------------------------------------------------------------------
// 中断分发
//-----------------------------------------------------------------------------

static void ls1c103_dispatch_isr(int vector, unsigned int *stack)
{
    irq_handler_t isr;
    void *arg;
    
    if ((vector >= 0) && (vector < LS1C103_IRQ_COUNT))
    {
        isr = isr_table[vector].isr;
        arg = isr_table[vector].arg;
        
        if (NULL == isr) isr = ls1c103_default_isr;
        if (NULL == arg) arg = (void *)stack;
        
        isr(vector, arg);

    }
}

//-----------------------------------------------------------------------------
// 中断响应 
//-----------------------------------------------------------------------------

extern HW_PMU_t *g_pmu;

void c_interrupt_handler(unsigned int *stack)
{
    unsigned int i_en, i_sr, tmp;
	unsigned int ecfg  = stack[R_ECFG];
	unsigned int estat = stack[R_ESTAT];

    estat &= ecfg & CSR_ESTAT_IS_MASK;
    if (estat == 0)                                 /* 会出现吗? tlbrerr/merr? */
    {
        return;
    }

    /**
     * IP11 定时器中断
     */
    if (estat & CSR_ECFG_TI)
    {
        ls1c103_dispatch_isr(LS1C103_IRQ_TICKER, stack);
        __csrwr_w(1, LA_CSR_TINTCLR);               /* 清除 Timer 中断 */
    }

    /**
     * HW-IP5 External Interrupt: GPIO
     */
	if (estat & CSR_ECFG_HWI5)
	{
	    int i;

        i_en = READ_REG32(EXTI_EN_ADDR);
        i_sr = READ_REG32(EXTI_SRC_ADDR);
	    tmp  = i_en & i_sr;
	    
	    for (i=0; i<28; i++)
	    {
            if (tmp & bit(i))
            {
                ls1c103_dispatch_isr(LS1C103_IRQ_GPIO_BASE + i, stack);
                OR_REG32(EXTI_CLR_ADDR, bit(i));    /* clear int flag*/
            }
        }
	}

	/**
	 * HW-IP4 PMU Interrupt
	 */
	if (estat & CSR_ECFG_HWI4)
	{
	    if (g_pmu->CmdSts & (CMDSR_INTSRC_RTC | CMDSR_INTEN_RTC))
            ls1c103_dispatch_isr(LS1C103_IRQ_RTC, stack);

	    if (g_pmu->CmdSts & (CMDSR_INTSRC_8MFAIL | CMDSR_INTEN_8MFAIL))
            ls1c103_dispatch_isr(LS1C103_IRQ_8MFAIL, stack);

	    if (g_pmu->CmdSts & (CMDSR_INTSRC_32KFAIL | CMDSR_INTEN_32KFAIL))
            ls1c103_dispatch_isr(LS1C103_IRQ_32KFAIL, stack);

	    if (g_pmu->CmdSts & (CMDSR_INTSRC_BATFAIL | CMDSR_INTEN_BATFAIL))
            ls1c103_dispatch_isr(LS1C103_IRQ_BATFAIL, stack);
            
	    if (g_pmu->CmdSts & (CMDSR_INTSRC_WAKE | CMDSR_INTEN_WAKE))
            ls1c103_dispatch_isr(LS1C103_IRQ_WAKE, stack);

        g_pmu->CommandW |= CMDSR_INTSRC_RTC |
                           CMDSR_INTSRC_8MFAIL |
                           CMDSR_INTSRC_32KFAIL |
                           CMDSR_INTSRC_BATFAIL |
                           CMDSR_INTSRC_WAKE;
    }

	/**
	 * HW-IP3 INTC Interrupt
	 */
	if (estat & CSR_ECFG_HWI3)
	{
        i_en = READ_REG32(INTC_EN_BASE);
        i_sr = READ_REG32(INTC_SR_BASE);
	    tmp  = i_en & i_sr;

	    if (tmp & INTC_DMA3_BIT)
            ls1c103_dispatch_isr(LS1C103_IRQ_DMA3, stack);
            
	    if (tmp & INTC_DMA2_BIT)
            ls1c103_dispatch_isr(LS1C103_IRQ_DMA2, stack);
            
	    if (tmp & INTC_DMA1_BIT)
            ls1c103_dispatch_isr(LS1C103_IRQ_DMA1, stack);
            
	    if (tmp & INTC_DMA0_BIT)
            ls1c103_dispatch_isr(LS1C103_IRQ_DMA0, stack);

	    if (tmp & INTC_SPI_BIT)
            ls1c103_dispatch_isr(LS1C103_IRQ_SPI, stack);
    
        if (tmp & INTC_FLASH_BIT)
            ls1c103_dispatch_isr(LS1C103_IRQ_FLASH, stack);

        if (tmp & INTC_UART0_BIT)
            ls1c103_dispatch_isr(LS1C103_IRQ_UART0, stack);

        if (tmp & INTC_UART1_BIT)
            ls1c103_dispatch_isr(LS1C103_IRQ_UART1, stack);

        if (tmp & INTC_I2C_BIT)
            ls1c103_dispatch_isr(LS1C103_IRQ_I2C, stack);

        if (tmp & INTC_BTIM_BIT)
            ls1c103_dispatch_isr(LS1C103_IRQ_BTIM, stack);

        OR_REG32(INTC_SR_BASE, 0xFFFF);     /* clear int flag*/
	}

	/**
	 * HW-IP2 ADC Interrupt
	 */
	if (estat & CSR_ECFG_HWI2)
	{
	    ls1c103_dispatch_isr(LS1C103_IRQ_ADC, stack);
	}

	/**
	 * HW-IP1 GTIM Interrupt
	 */
	if (estat & CSR_ECFG_HWI1)
	{
	    ls1c103_dispatch_isr(LS1C103_IRQ_GTIM, stack);
	}
	
	/**
	 * HW-IP0 ATIM Interrupt
	 */
	if (estat & CSR_ECFG_HWI0)
	{
	    ls1c103_dispatch_isr(LS1C103_IRQ_ATIM, stack);
	}

	/**
	 * SW-IP0 Interrupt
	 */
	if (estat & CSR_ECFG_SWI1)
	{
	    clear_csr_estat(CSR_ECFG_SWI1);
	    ls1c103_dispatch_isr(LS1C103_IRQ_SW1, stack);
	}

	/**
	 * SW-IP0 Interrupt
	 */
	if (estat & CSR_ECFG_SWI0)
	{
	    clear_csr_estat(CSR_ECFG_SWI0);
	    ls1c103_dispatch_isr(LS1C103_IRQ_SW0, stack);
	}

    return;
}

//-----------------------------------------------------------------------------
// SW0/SW1 Interrupt Support
//-----------------------------------------------------------------------------

/*
 * Generate a software interrupt
 */
int assert_sw_irq(unsigned int irqnum)
{
    if ((irqnum == CSR_ECFG_SWI0) || (irqnum == CSR_ECFG_SWI1))
    {
        set_csr_estat(irqnum);
        return irqnum;
    }

    return -1;
}

/*
 * Clear a software interrupt
 */
int negate_sw_irq(unsigned int irqnum)
{
    if ((irqnum == CSR_ECFG_SWI0) || (irqnum == CSR_ECFG_SWI1))
    {
        clear_csr_estat(irqnum);
        return irqnum;
    }
    
    return -1;
}

//-----------------------------------------------------------------------------
// 安装中断
//-----------------------------------------------------------------------------

int ls1c103_install_isr(int vector, irq_handler_t isr, void *arg)
{
    if ((vector >= 0) && (vector < LS1C103_IRQ_COUNT))
    {
        if (isr)
            isr_table[vector].isr = isr;
        else
            isr_table[vector].isr = ls1c103_default_isr;
        isr_table[vector].arg = arg;
        
        return vector;
    }
    
	return -1;
}

//-----------------------------------------------------------------------------
// 移除中断
//-----------------------------------------------------------------------------

int ls1c103_remove_isr(int vector)
{
    if ((vector >= 0) && (vector < LS1C103_IRQ_COUNT))
    {
        isr_table[vector].isr = ls1c103_default_isr;
        isr_table[vector].arg = NULL;

        return vector;
    }

	return -1;
}

//-----------------------------------------------------------------------------
// 根据中断向量号开关中断
//-----------------------------------------------------------------------------

int ls1c103_interrupt_enable(int vector)
{
    switch (vector)
    {
        case LS1C103_IRQ_ATIM:      __csrxchg_w(CSR_ECFG_HWI0, CSR_ECFG_HWI0, LA_CSR_ECFG); break;
        case LS1C103_IRQ_GTIM:      __csrxchg_w(CSR_ECFG_HWI1, CSR_ECFG_HWI1, LA_CSR_ECFG); break;
        case LS1C103_IRQ_ADC:       __csrxchg_w(CSR_ECFG_HWI2, CSR_ECFG_HWI2, LA_CSR_ECFG); break;

        case LS1C103_IRQ_BTIM:      OR_REG32(INTC_EN_BASE, INTC_BTIM_BIT);  break;
        case LS1C103_IRQ_I2C:       OR_REG32(INTC_EN_BASE, INTC_I2C_BIT);   break;
        case LS1C103_IRQ_UART1:     OR_REG32(INTC_EN_BASE, INTC_UART1_BIT); break;
        case LS1C103_IRQ_UART0:     OR_REG32(INTC_EN_BASE, INTC_UART0_BIT); break;
        case LS1C103_IRQ_FLASH:     OR_REG32(INTC_EN_BASE, INTC_FLASH_BIT); break;
        case LS1C103_IRQ_SPI:       OR_REG32(INTC_EN_BASE, INTC_SPI_BIT);   break;
        case LS1C103_IRQ_DMA0:      OR_REG32(INTC_EN_BASE, INTC_DMA0_BIT);  break;
        case LS1C103_IRQ_DMA1:      OR_REG32(INTC_EN_BASE, INTC_DMA1_BIT);  break;
        case LS1C103_IRQ_DMA2:      OR_REG32(INTC_EN_BASE, INTC_DMA2_BIT);  break;
        case LS1C103_IRQ_DMA3:      OR_REG32(INTC_EN_BASE, INTC_DMA3_BIT);  break;

        case LS1C103_IRQ_WAKE:      g_pmu->CmdSts |= CMDSR_INTEN_WAKE;    break;
        case LS1C103_IRQ_BATFAIL:   g_pmu->CmdSts |= CMDSR_INTEN_BATFAIL; break;
        case LS1C103_IRQ_32KFAIL:   g_pmu->CmdSts |= CMDSR_INTEN_32KFAIL; break;
        case LS1C103_IRQ_8MFAIL:    g_pmu->CmdSts |= CMDSR_INTEN_8MFAIL;  break;
        case LS1C103_IRQ_RTC:       g_pmu->CmdSts |= CMDSR_INTEN_RTC;     break;

        default:
            if ((vector >= LS1C103_IRQ_GPIO0) && (vector <= LS1C103_IRQ_GPIO28))
            {
                EXTI_ENABLE(vector - LS1C103_IRQ_GPIO_BASE);
                g_pmu->CmdSts |= CMDSR_EXTINT_EN;
                break;
            }
            else
                return -1;
    }

    return 0;
}

int ls1c103_interrupt_disable(int vector)
{
    switch (vector)
    {
        case LS1C103_IRQ_ATIM:      __csrxchg_w(0, CSR_ECFG_HWI0, LA_CSR_ECFG); break;
        case LS1C103_IRQ_GTIM:      __csrxchg_w(0, CSR_ECFG_HWI1, LA_CSR_ECFG); break;
        case LS1C103_IRQ_ADC:       __csrxchg_w(0, CSR_ECFG_HWI2, LA_CSR_ECFG); break;

        case LS1C103_IRQ_BTIM:      AND_REG32(INTC_EN_BASE, ~INTC_BTIM_BIT);  break;
        case LS1C103_IRQ_I2C:       AND_REG32(INTC_EN_BASE, ~INTC_I2C_BIT);   break;
        case LS1C103_IRQ_UART1:     AND_REG32(INTC_EN_BASE, ~INTC_UART1_BIT); break;
        case LS1C103_IRQ_UART0:     AND_REG32(INTC_EN_BASE, ~INTC_UART0_BIT); break;
        case LS1C103_IRQ_FLASH:     AND_REG32(INTC_EN_BASE, ~INTC_FLASH_BIT); break;
        case LS1C103_IRQ_SPI:       AND_REG32(INTC_EN_BASE, ~INTC_SPI_BIT);   break;
        case LS1C103_IRQ_DMA0:      AND_REG32(INTC_EN_BASE, ~INTC_DMA0_BIT);  break;
        case LS1C103_IRQ_DMA1:      AND_REG32(INTC_EN_BASE, ~INTC_DMA1_BIT);  break;
        case LS1C103_IRQ_DMA2:      AND_REG32(INTC_EN_BASE, ~INTC_DMA2_BIT);  break;
        case LS1C103_IRQ_DMA3:      AND_REG32(INTC_EN_BASE, ~INTC_DMA3_BIT);  break;

        case LS1C103_IRQ_WAKE:      g_pmu->CmdSts &= ~CMDSR_INTEN_WAKE;    break;
        case LS1C103_IRQ_BATFAIL:   g_pmu->CmdSts &= ~CMDSR_INTEN_BATFAIL; break;
        case LS1C103_IRQ_32KFAIL:   g_pmu->CmdSts &= ~CMDSR_INTEN_32KFAIL; break;
        case LS1C103_IRQ_8MFAIL:    g_pmu->CmdSts &= ~CMDSR_INTEN_8MFAIL;  break;
        case LS1C103_IRQ_RTC:       g_pmu->CmdSts &= ~CMDSR_INTEN_RTC;     break;

        default:
            if ((vector >= LS1C103_IRQ_GPIO0) && (vector <= LS1C103_IRQ_GPIO28))
            {
                EXTI_DISABLE(vector - LS1C103_IRQ_GPIO_BASE);
                if (0 == READ_REG32(EXTI_EN_ADDR))
                {
                    g_pmu->CmdSts &= ~CMDSR_EXTINT_EN;
                }
                break;
            }
            else
                return -1;
    }

    return 0;
}

//-----------------------------------------------------------------------------

/*
 * @@ END
 */


