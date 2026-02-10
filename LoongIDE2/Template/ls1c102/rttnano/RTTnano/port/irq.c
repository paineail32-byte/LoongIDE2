
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

#include "rtthread.h"
#include "rthw.h"

#include "cpu.h"
#include "regdef.h"

#include "ls1c102.h"
#include "ls1c102_irq.h"

//-----------------------------------------------------------------------------
// 中断向量表
//-----------------------------------------------------------------------------

/**
 * 中断向量表
 */
static struct rt_irq_desc isr_table[LS1C102_IRQ_COUNT];

//-----------------------------------------------------------------------------
// 初始化中断向量表
//-----------------------------------------------------------------------------

static void ls1c102_default_isr(int vector, void *arg);

void ls1c102_init_isr_table(void)
{
    int i;
    
    for (i=0; i<LS1C102_IRQ_COUNT; i++)
    {
		isr_table[i].handler = ls1c102_default_isr;
		isr_table[i].param = 0;
    }
}

//-----------------------------------------------------------------------------
// 默认中断
//-----------------------------------------------------------------------------

extern char *get_irq_description(int vector);

static void ls1c102_default_isr(int vector, void *arg)
{
    unsigned int *stack = (unsigned int *)arg;
	unsigned int ecfg  = stack[R_ECFG];
	unsigned int estat = stack[R_ESTAT];
	
    char *irqname = get_irq_description(vector);
    
	rt_kprintf("Unhandled isr %s:\r\n  vector=%i, ecfg=0x%08X, estat=0x%08X\n",
                irqname, vector, ecfg, estat);

	return;
}

//-----------------------------------------------------------------------------
// 中断分发
//-----------------------------------------------------------------------------

static void ls1c102_dispatch_isr(int vector, unsigned int *stack)
{
    rt_isr_handler_t isr;
    void *arg;
    
    if ((vector >= 0) && (vector < LS1C102_IRQ_COUNT))
    {
        isr = isr_table[vector].handler;
        arg = isr_table[vector].param;
        
        if (NULL == isr) isr = ls1c102_default_isr;
        if (NULL == arg) arg = (void *)stack;
        
        isr(vector, arg);

    }
    else
    {
        ls1c102_default_isr(vector, stack);
    }
}

//-----------------------------------------------------------------------------
// 中断响应 
//-----------------------------------------------------------------------------

extern HW_PMU_t  *g_pmu;
extern HW_INTC_t *g_intc;

void c_interrupt_handler(unsigned int *stack)
{
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
        ls1c102_dispatch_isr(LS1C102_IRQ_TICKER, stack);
        __csrwr_w(1, LA_CSR_TINTCLR);               /* 清除 Timer 中断 */
    }

    /**
     * HW-IP5 External Interrupt: GPIO
     */
	if (estat & CSR_ECFG_HWI5)
	{
	    int i;

	    for (i=0; i<32; i++)
	    {
            if (g_pmu->ExIntEn & g_pmu->ExIntSrc & bit(i))
            {
                ls1c102_dispatch_isr(LS1C102_IRQ_GPIO_BASE + i, stack);
            }
        }

        g_pmu->CommandW |= CMDSR_INTSRC_EXTINT;
	}

	/**
	 * HW-IP4 INTC Interrupt
	 */
	if (estat & CSR_ECFG_HWI4)
	{
	    if (g_intc->en & g_intc->out & INTC_DMA)
            ls1c102_dispatch_isr(LS1C102_IRQ_DMA, stack);
      
	    if (g_intc->en & g_intc->out & INTC_VPWM)
	        ls1c102_dispatch_isr(LS1C102_IRQ_VPWM, stack);

	    if (g_intc->en & g_intc->out & INTC_SPI)
            ls1c102_dispatch_isr(LS1C102_IRQ_SPI, stack);
    
        if (g_intc->en & g_intc->out & INTC_FLASH)
            ls1c102_dispatch_isr(LS1C102_IRQ_FLASH, stack);

        if (g_intc->en & g_intc->out & INTC_UART0)
            ls1c102_dispatch_isr(LS1C102_IRQ_UART0, stack);

        if (g_intc->en & g_intc->out & INTC_UART1)
            ls1c102_dispatch_isr(LS1C102_IRQ_UART1, stack);

        if (g_intc->en & g_intc->out & INTC_I2C)
            ls1c102_dispatch_isr(LS1C102_IRQ_I2C, stack);

        if (g_intc->en & g_intc->out & INTC_TIMER)
            ls1c102_dispatch_isr(LS1C102_IRQ_TIMER, stack);

        g_intc->clr = 0xFF;         // clear interrupt flag
	}

	/**
	 * HW-IP3 INTC Interrupt
	 */
	if (estat & CSR_ECFG_HWI3)
	{
	    if (g_pmu->CmdSts & (CMDSR_INTSRC_ADC | CMDSR_INTEN_ADC))
            ls1c102_dispatch_isr(LS1C102_IRQ_ADC, stack);

	    if (g_pmu->CmdSts & (CMDSR_INTSRC_RTC | CMDSR_INTEN_RTC))
            ls1c102_dispatch_isr(LS1C102_IRQ_RTC, stack);
  
	    if (g_pmu->CmdSts & (CMDSR_INTSRC_8MFAIL | CMDSR_INTEN_8MFAIL))
            ls1c102_dispatch_isr(LS1C102_IRQ_8MFAIL, stack);

	    if (g_pmu->CmdSts & (CMDSR_INTSRC_32KFAIL | CMDSR_INTEN_32KFAIL))
            ls1c102_dispatch_isr(LS1C102_IRQ_32KFAIL, stack);
    
	    if (g_pmu->CmdSts & (CMDSR_INTSRC_BATFAIL | CMDSR_INTEN_BATFAIL))
            ls1c102_dispatch_isr(LS1C102_IRQ_BATFAIL, stack);

        g_pmu->CommandW |= CMDSR_INTSRC_ADC |
                           CMDSR_INTSRC_RTC |
                           CMDSR_INTSRC_8MFAIL |
                           CMDSR_INTSRC_32KFAIL |
                           CMDSR_INTSRC_BATFAIL;
    }

	/**
	 * HW-IP2 UART2 Interrupt
	 */
	if (estat & CSR_ECFG_HWI2)
	{
	    ls1c102_dispatch_isr(LS1C102_IRQ_UART2, stack);
	    g_pmu->CommandW |= CMDSR_INTSRC_UART2;
	}
	
	/**
	 * HW-IP1 Touch Interrupt
	 */
	if (estat & CSR_ECFG_HWI1)
	{
	    ls1c102_dispatch_isr(LS1C102_IRQ_TOUCH, stack);
	    g_pmu->CommandW |= CMDSR_INTSRC_TOUCH;
	}
	
	/**
	 * HW-IP0 WAKEUP Interrupt
	 */
	if (estat & CSR_ECFG_HWI0)
	{
	    ls1c102_dispatch_isr(LS1C102_IRQ_WAKEUP, stack);
	    g_pmu->CommandW |= CMDSR_INTSRC_WAKE;
	}

	/**
	 * SW-IP0 Interrupt
	 */
	if (estat & CSR_ECFG_SWI1)
	{
	    ls1c102_dispatch_isr(LS1C102_IRQ_SW1, stack);
	    clear_csr_estat(CSR_ECFG_SWI1);
	}

	/**
	 * SW-IP0 Interrupt
	 */
	if (estat & CSR_ECFG_SWI0)
	{
	    ls1c102_dispatch_isr(LS1C102_IRQ_SW0, stack);
	    clear_csr_estat(CSR_ECFG_SWI0);
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

int ls1c102_install_isr(int vector, irq_handler_t isr, void *arg)
{
    if ((vector >= 0) && (vector < LS1C102_IRQ_COUNT))
    {
        if (isr)
            isr_table[vector].handler = isr;
        else
            isr_table[vector].handler = ls1c102_default_isr;
        isr_table[vector].param = arg;
        
        return vector;
    }
    
	return -1;
}

//-----------------------------------------------------------------------------
// 移除中断
//-----------------------------------------------------------------------------

int ls1c102_remove_isr(int vector)
{
    if ((vector >= 0) && (vector < LS1C102_IRQ_COUNT))
    {
        isr_table[vector].handler = ls1c102_default_isr;
        isr_table[vector].param = NULL;

        return vector;
    }

	return -1;
}

//-----------------------------------------------------------------------------
// 根据中断向量号开关中断
//-----------------------------------------------------------------------------

int ls1c102_interrupt_enable(int vector)
{
    switch (vector)
    {
        case LS1C102_IRQ_WAKEUP:    g_pmu->CmdSts |= CMDSR_INTEN_WAKE;    break;
        case LS1C102_IRQ_TOUCH:     g_pmu->CmdSts |= CMDSR_INTEN_TOUCH;   break;
        case LS1C102_IRQ_UART2:     g_pmu->CmdSts |= CMDSR_INTEN_UART2;   break;
        case LS1C102_IRQ_TICKER:    break;
        case LS1C102_IRQ_BATFAIL:   g_pmu->CmdSts |= CMDSR_INTEN_BATFAIL; break;
        case LS1C102_IRQ_32KFAIL:   g_pmu->CmdSts |= CMDSR_INTEN_32KFAIL; break;
        case LS1C102_IRQ_8MFAIL:    g_pmu->CmdSts |= CMDSR_INTEN_8MFAIL;  break;
        case LS1C102_IRQ_RTC:       g_pmu->CmdSts |= CMDSR_INTEN_RTC;     break;
        case LS1C102_IRQ_ADC:       g_pmu->CmdSts |= CMDSR_INTEN_ADC;     break;

        case LS1C102_IRQ_TIMER:     g_intc->en |= INTC_TIMER; break;
        case LS1C102_IRQ_I2C:       g_intc->en |= INTC_I2C;   break;
        case LS1C102_IRQ_UART1:     g_intc->en |= INTC_UART1; break;
        case LS1C102_IRQ_UART0:     g_intc->en |= INTC_UART0; break;
        case LS1C102_IRQ_FLASH:     g_intc->en |= INTC_FLASH; break;
        case LS1C102_IRQ_SPI:       g_intc->en |= INTC_SPI;   break;
        case LS1C102_IRQ_VPWM:      g_intc->en |= INTC_VPWM;  break;
        case LS1C102_IRQ_DMA:       g_intc->en |= INTC_DMA;   break;

        default:
            if ((vector >= LS1C102_IRQ_GPIO0) && (vector <= LS1C102_IRQ_GPIO55))
            {
                g_pmu->ExIntEn |= 1 << (vector - LS1C102_IRQ_GPIO_BASE);
                g_pmu->CmdSts  |= CMDSR_EXINTEN;
                break;
            }
            else
                return -1;
    }

    return 0;
}

int ls1c102_interrupt_disable(int vector)
{
    switch (vector)
    {
        case LS1C102_IRQ_WAKEUP:    g_pmu->CmdSts &= ~CMDSR_INTEN_WAKE;    break;
        case LS1C102_IRQ_TOUCH:     g_pmu->CmdSts &= ~CMDSR_INTEN_TOUCH;   break;
        case LS1C102_IRQ_UART2:     g_pmu->CmdSts &= ~CMDSR_INTEN_UART2;   break;
        case LS1C102_IRQ_TICKER:    break;
        case LS1C102_IRQ_BATFAIL:   g_pmu->CmdSts &= ~CMDSR_INTEN_BATFAIL; break;
        case LS1C102_IRQ_32KFAIL:   g_pmu->CmdSts &= ~CMDSR_INTEN_32KFAIL; break;
        case LS1C102_IRQ_8MFAIL:    g_pmu->CmdSts &= ~CMDSR_INTEN_8MFAIL;  break;
        case LS1C102_IRQ_RTC:       g_pmu->CmdSts &= ~CMDSR_INTEN_RTC;     break;
        case LS1C102_IRQ_ADC:       g_pmu->CmdSts &= ~CMDSR_INTEN_ADC;     break;

        case LS1C102_IRQ_TIMER:     g_intc->en &= ~INTC_TIMER; break;
        case LS1C102_IRQ_I2C:       g_intc->en &= ~INTC_I2C;   break;
        case LS1C102_IRQ_UART1:     g_intc->en &= ~INTC_UART1; break;
        case LS1C102_IRQ_UART0:     g_intc->en &= ~INTC_UART0; break;
        case LS1C102_IRQ_FLASH:     g_intc->en &= ~INTC_FLASH; break;
        case LS1C102_IRQ_SPI:       g_intc->en &= ~INTC_SPI;   break;
        case LS1C102_IRQ_VPWM:      g_intc->en &= ~INTC_VPWM;  break;
        case LS1C102_IRQ_DMA:       g_intc->en &= ~INTC_DMA;   break;

        default:
            if ((vector >= LS1C102_IRQ_GPIO0) && (vector <= LS1C102_IRQ_GPIO55))
            {
                g_pmu->ExIntEn &= ~(1 << (vector - LS1C102_IRQ_GPIO_BASE));
                if (0 == g_pmu->ExIntEn)
                {
                    g_pmu->CmdSts &= ~CMDSR_EXINTEN;
                }
                break;
            }
            else
                return -1;
    }

    return 0;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// RTThread Needed.
//-----------------------------------------------------------------------------

rt_base_t rt_hw_interrupt_disable(void)
{
    unsigned int crmd = __csrrd_w(LA_CSR_CRMD);
    __csrxchg_w(~CSR_CRMD_IE, CSR_CRMD_IE, LA_CSR_CRMD);
    return (rt_base_t)crmd;
}

void rt_hw_interrupt_enable(rt_base_t level)
{
    __csrwr_w((unsigned int)level, LA_CSR_CRMD);
}

//-------------------------------------------------------------------------------------------------

void rt_hw_interrupt_mask(int vector)
{
    ls1c102_interrupt_disable(vector);
}

void rt_hw_interrupt_umask(int vector)
{
    ls1c102_interrupt_enable(vector);
}

//-------------------------------------------------------------------------------------------------

rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler,
                                         void *param, const char *name)
{
    rt_isr_handler_t old_handler = RT_NULL;

    if (vector >= 0 && vector < LS1C102_IRQ_COUNT)
    {
        old_handler = isr_table[vector].handler;

        ls1c102_install_isr(vector, handler, param);
    }

    return old_handler;

}

//-------------------------------------------------------------------------------------------------

/*
 * @@ END
 */

