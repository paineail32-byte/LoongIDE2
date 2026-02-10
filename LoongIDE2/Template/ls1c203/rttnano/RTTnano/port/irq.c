
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

#include "ls1c203.h"
#include "ls1c203_irq.h"

//-----------------------------------------------------------------------------
// 中断向量表
//-----------------------------------------------------------------------------

struct rt_irq_desc __ISR_tbl[LS1C203_IRQ_COUNT];

//-----------------------------------------------------------------------------
// 初始化中断向量表
//-----------------------------------------------------------------------------

void __ISR_default(int vector, void *arg);

void __ISR_tbl_initialize(void)
{
    int i;
    
    for (i=0; i<LS1C203_IRQ_COUNT; i++)
    {
		__ISR_tbl[i].handler = __ISR_default;
		__ISR_tbl[i].param = 0;
    }
}

//-----------------------------------------------------------------------------
// 默认中断
//-----------------------------------------------------------------------------

extern char *get_irq_description(int vector);

void __ISR_default(int vector, void *arg)
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

__WEAK void __ISR_dispatch(int vector, unsigned int *stack)
{
    rt_isr_handler_t isr;
    void *arg;
    
    if ((vector >= 0) && (vector < LS1C203_IRQ_COUNT))
    {
        isr = __ISR_tbl[vector].handler;
        arg = __ISR_tbl[vector].param;
        
        if (NULL == isr) isr = __ISR_default;
        if (NULL == arg) arg = (void *)stack;

        isr(vector, arg);

    }
    else
    {
        __ISR_default(vector, stack);
    }
}

//-----------------------------------------------------------------------------
// 中断响应 
//-----------------------------------------------------------------------------

extern HW_CONF_t *g_conf;
extern HW_PMU_t  *g_pmu;

__WEAK void c_interrupt_handler(unsigned int *stack)
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
        __csrwr_w(1, LA_CSR_TINTCLR);               /* 清除 Timer 中断 */
        __ISR_dispatch(LS1C203_IRQ_TICKER, stack);
    }

    /**
     * HW-IP5 External Interrupt: GPIO
     */
	if ((estat & CSR_ECFG_HWI5) && (g_pmu->CmdSts & CMDSR_INTSRC_EXTINT))
	{
	    int i;

        i_en = READ_REG32(EXTI_EN_ADDR);
        i_sr = READ_REG32(EXTI_SRC_ADDR);
	    tmp  = i_en & i_sr;

	    for (i=0; i<28; i++)
	    {
            if (tmp & bit(i))
            {
                __ISR_dispatch(LS1C203_IRQ_GPIO_BASE + i, stack);
                OR_REG32(EXTI_CLR_ADDR, bit(i));    /* clear int flag*/
            }
        }
	}

	/**
	 * HW-IP4 PMU Interrupt
	 */
	if (estat & CSR_ECFG_HWI4)
	{
	    tmp = g_pmu->CmdSts;

	    if (tmp & (CMDSR_INTSRC_RTC | CMDSR_INTEN_RTC))
            __ISR_dispatch(LS1C203_IRQ_RTC, stack);

	    if (tmp & (CMDSR_INTSRC_8MFAIL | CMDSR_INTEN_8MFAIL))
            __ISR_dispatch(LS1C203_IRQ_8MFAIL, stack);

	    if (tmp & (CMDSR_INTSRC_32KFAIL | CMDSR_INTEN_32KFAIL))
            __ISR_dispatch(LS1C203_IRQ_32KFAIL, stack);

	    if (tmp & (CMDSR_INTSRC_BATFAIL | CMDSR_INTEN_BATFAIL))
            __ISR_dispatch(LS1C203_IRQ_BATFAIL, stack);

	    if (tmp & (CMDSR_INTSRC_WAKE | CMDSR_INTEN_WAKE))
            __ISR_dispatch(LS1C203_IRQ_WAKE, stack);

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
        i_en = g_conf->ien;
        i_sr = g_conf->iout;
	    tmp  = i_en & i_sr;

	    if (tmp & INTC_ACOMP_BIT)
            __ISR_dispatch(LS1C203_IRQ_ACOMP, stack);

	    if (tmp & INTC_CAN_BIT)
            __ISR_dispatch(LS1C203_IRQ_CAN, stack);

	    if (tmp & INTC_DMA3_BIT)
            __ISR_dispatch(LS1C203_IRQ_DMA3, stack);

	    if (tmp & INTC_DMA2_BIT)
            __ISR_dispatch(LS1C203_IRQ_DMA2, stack);

	    if (tmp & INTC_DMA1_BIT)
            __ISR_dispatch(LS1C203_IRQ_DMA1, stack);

	    if (tmp & INTC_DMA0_BIT)
            __ISR_dispatch(LS1C203_IRQ_DMA0, stack);

	    if (tmp & INTC_SPI_BIT)
            __ISR_dispatch(LS1C203_IRQ_SPI, stack);

        if (tmp & INTC_FLASH_BIT)
            __ISR_dispatch(LS1C203_IRQ_FLASH, stack);

        if (tmp & INTC_UART0_BIT)
            __ISR_dispatch(LS1C203_IRQ_UART0, stack);

        if (tmp & INTC_UART1_BIT)
            __ISR_dispatch(LS1C203_IRQ_UART1, stack);

        if (tmp & INTC_I2C_BIT)
            __ISR_dispatch(LS1C203_IRQ_I2C, stack);

        if (tmp & INTC_BTIM_BIT)
            __ISR_dispatch(LS1C203_IRQ_BTIM, stack);

        // OR_REG32(INTC_SR_BASE, 0xFFFF);     /* clear int flag*/
	}

	/**
	 * HW-IP2 ADC Interrupt
	 */
	if (estat & CSR_ECFG_HWI2)
	{
	    __ISR_dispatch(LS1C203_IRQ_ADC, stack);
	}

	/**
	 * HW-IP1 GTIM Interrupt
	 */
	if (estat & CSR_ECFG_HWI1)
	{
	    __ISR_dispatch(LS1C203_IRQ_GTIM, stack);
	}

	/**
	 * HW-IP0 ATIM Interrupt
	 */
	if (estat & CSR_ECFG_HWI0)
	{
	    __ISR_dispatch(LS1C203_IRQ_ATIM, stack);
	}

	/**
	 * SW-IP0 Interrupt
	 */
	if (estat & CSR_ECFG_SWI1)
	{
	    clear_csr_estat(CSR_ECFG_SWI1);
	    __ISR_dispatch(LS1C203_IRQ_SW1, stack);
	}

	/**
	 * SW-IP0 Interrupt
	 */
	if (estat & CSR_ECFG_SWI0)
	{
	    clear_csr_estat(CSR_ECFG_SWI0);
	    __ISR_dispatch(LS1C203_IRQ_SW0, stack);
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

__WEAK int ls1c203_install_isr(int vector, irq_handler_t isr, void *arg)
{
    if ((vector >= 0) && (vector < LS1C203_IRQ_COUNT))
    {
        if (isr)
            __ISR_tbl[vector].handler = isr;
        else
            __ISR_tbl[vector].handler = __ISR_default;

        __ISR_tbl[vector].param = arg;
        
        return vector;
    }
    
	return -1;
}

//-----------------------------------------------------------------------------
// 移除中断
//-----------------------------------------------------------------------------

int ls1c203_remove_isr(int vector)
{
    if ((vector >= 0) && (vector < LS1C203_IRQ_COUNT))
    {
        __ISR_tbl[vector].handler = __ISR_default;
        __ISR_tbl[vector].param = NULL;

        return vector;
    }

	return -1;
}

//-----------------------------------------------------------------------------
// 根据中断向量号开关中断
//-----------------------------------------------------------------------------

int ls1c203_interrupt_enable(int vector)
{
    switch (vector)
    {
        case LS1C203_IRQ_ATIM:      __csrxchg_w(CSR_ECFG_HWI0, CSR_ECFG_HWI0, LA_CSR_ECFG); break;
        case LS1C203_IRQ_GTIM:      __csrxchg_w(CSR_ECFG_HWI1, CSR_ECFG_HWI1, LA_CSR_ECFG); break;
        case LS1C203_IRQ_ADC:       __csrxchg_w(CSR_ECFG_HWI2, CSR_ECFG_HWI2, LA_CSR_ECFG); break;

        case LS1C203_IRQ_BTIM:      g_conf->ien |= INTC_BTIM_BIT;  break;
        case LS1C203_IRQ_I2C:       g_conf->ien |= INTC_I2C_BIT;   break;
        case LS1C203_IRQ_UART1:     g_conf->ien |= INTC_UART1_BIT; break;
        case LS1C203_IRQ_UART0:     g_conf->ien |= INTC_UART0_BIT; break;
        case LS1C203_IRQ_FLASH:     g_conf->ien |= INTC_FLASH_BIT; break;
        case LS1C203_IRQ_SPI:       g_conf->ien |= INTC_SPI_BIT;   break;
        case LS1C203_IRQ_DMA0:      g_conf->ien |= INTC_DMA0_BIT;  break;
        case LS1C203_IRQ_DMA1:      g_conf->ien |= INTC_DMA1_BIT;  break;
        case LS1C203_IRQ_DMA2:      g_conf->ien |= INTC_DMA2_BIT;  break;
        case LS1C203_IRQ_DMA3:      g_conf->ien |= INTC_DMA3_BIT;  break;
        case LS1C203_IRQ_CAN:       g_conf->ien |= INTC_CAN_BIT;   break;
        case LS1C203_IRQ_ACOMP:     g_conf->ien |= INTC_ACOMP_BIT; break;

        case LS1C203_IRQ_WAKE:      g_pmu->CmdSts |= CMDSR_INTEN_WAKE;    break;
        case LS1C203_IRQ_BATFAIL:   g_pmu->CmdSts |= CMDSR_INTEN_BATFAIL; break;
        case LS1C203_IRQ_32KFAIL:   g_pmu->CmdSts |= CMDSR_INTEN_32KFAIL; break;
        case LS1C203_IRQ_8MFAIL:    g_pmu->CmdSts |= CMDSR_INTEN_8MFAIL;  break;
        case LS1C203_IRQ_RTC:       g_pmu->CmdSts |= CMDSR_INTEN_RTC;     break;

        default:
            if ((vector >= LS1C203_IRQ_GPIO0) && (vector <= LS1C203_IRQ_GPIO28))
            {
                EXTI_EN(vector - LS1C203_IRQ_GPIO_BASE);
                g_pmu->CmdSts |= CMDSR_EXTINT_EN;
                break;
            }
            else
                return -1;
    }

    return 0;
}

int ls1c203_interrupt_disable(int vector)
{
    switch (vector)
    {
        case LS1C203_IRQ_ATIM:      __csrxchg_w(0, CSR_ECFG_HWI0, LA_CSR_ECFG); break;
        case LS1C203_IRQ_GTIM:      __csrxchg_w(0, CSR_ECFG_HWI1, LA_CSR_ECFG); break;
        case LS1C203_IRQ_ADC:       __csrxchg_w(0, CSR_ECFG_HWI2, LA_CSR_ECFG); break;

        case LS1C203_IRQ_BTIM:      g_conf->ien &= ~INTC_BTIM_BIT;  break;
        case LS1C203_IRQ_I2C:       g_conf->ien &= ~INTC_I2C_BIT;   break;
        case LS1C203_IRQ_UART1:     g_conf->ien &= ~INTC_UART1_BIT; break;
        case LS1C203_IRQ_UART0:     g_conf->ien &= ~INTC_UART0_BIT; break;
        case LS1C203_IRQ_FLASH:     g_conf->ien &= ~INTC_FLASH_BIT; break;
        case LS1C203_IRQ_SPI:       g_conf->ien &= ~INTC_SPI_BIT;   break;
        case LS1C203_IRQ_DMA0:      g_conf->ien &= ~INTC_DMA0_BIT;  break;
        case LS1C203_IRQ_DMA1:      g_conf->ien &= ~INTC_DMA1_BIT;  break;
        case LS1C203_IRQ_DMA2:      g_conf->ien &= ~INTC_DMA2_BIT;  break;
        case LS1C203_IRQ_DMA3:      g_conf->ien &= ~INTC_DMA3_BIT;  break;
        case LS1C203_IRQ_CAN:       g_conf->ien &= ~INTC_CAN_BIT;   break;
        case LS1C203_IRQ_ACOMP:     g_conf->ien &= ~INTC_ACOMP_BIT; break;

        case LS1C203_IRQ_WAKE:      g_pmu->CmdSts &= ~CMDSR_INTEN_WAKE;    break;
        case LS1C203_IRQ_BATFAIL:   g_pmu->CmdSts &= ~CMDSR_INTEN_BATFAIL; break;
        case LS1C203_IRQ_32KFAIL:   g_pmu->CmdSts &= ~CMDSR_INTEN_32KFAIL; break;
        case LS1C203_IRQ_8MFAIL:    g_pmu->CmdSts &= ~CMDSR_INTEN_8MFAIL;  break;
        case LS1C203_IRQ_RTC:       g_pmu->CmdSts &= ~CMDSR_INTEN_RTC;     break;

        default:
            if ((vector >= LS1C203_IRQ_GPIO0) && (vector <= LS1C203_IRQ_GPIO28))
            {
                EXTI_DIS(vector - LS1C203_IRQ_GPIO_BASE);
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
    ls1c203_interrupt_disable(vector);
}

void rt_hw_interrupt_umask(int vector)
{
    ls1c203_interrupt_enable(vector);
}

//-------------------------------------------------------------------------------------------------

rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler,
                                         void *param, const char *name)
{
    rt_isr_handler_t old_handler = RT_NULL;

    if (vector >= 0 && vector < LS1C203_IRQ_COUNT)
    {
        old_handler = __ISR_tbl[vector].handler;

        ls1c203_install_isr(vector, handler, param);
    }

    return old_handler;

}

//-------------------------------------------------------------------------------------------------

/*
 * @@ END
 */

