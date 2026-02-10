/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * irq_c.c
 *
 * created: 2022-02-18
 *  author: Bian
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <larchintrin.h>

#include "cpu.h"
#include "regdef.h"

#include "ls2k1000.h"
#include "ls2k1000_irq.h"

#define BSP_INTERRUPT_VECTOR_MAX    LS2K1000LA_IRQ_COUNT

//-------------------------------------------------------------------------------------------------
// 初始化中断
//-------------------------------------------------------------------------------------------------

/**
 * 中断向量
 */
typedef struct _isr_tbl
{
	irq_handler_t isr;       // 中断句柄
	uint64_t arg;            // 参数
} isr_tbl_t;

/**
 * 中断向量表
 */
static isr_tbl_t isr_table[BSP_INTERRUPT_VECTOR_MAX];

/**
 * 当前使用的核, INT_ROUTE_CORE0 or INT_ROUTE_CORE1
 */
extern unsigned int __cpunum;       /* CPU numeber, in bsp_start.c */

//-------------------------------------------------------------------------------------------------
// functions
//-------------------------------------------------------------------------------------------------

extern void dump_exception_info_then_dead(int vector, uint64_t *stack);

/**
 * 默认中断
 */
static void loongarch_default_isr(int vector, void *p)
{
    dump_exception_info_then_dead(vector, (uint64_t *)p);
}

/**
 * 初始化
 */
void loongarch_init_isr_table(void)
{
	unsigned int i;

	for (i=0; i<BSP_INTERRUPT_VECTOR_MAX; i++)
    {
		isr_table[i].isr = loongarch_default_isr;
		isr_table[i].arg = 0;
	}
}

//-------------------------------------------------------------------------------------------------
// 中断分发
//-------------------------------------------------------------------------------------------------

static void bsp_irq_handler_dispatch(int vector, void *arg)
{
	if ((vector >= 0) && (vector < BSP_INTERRUPT_VECTOR_MAX))
	{
	    if (isr_table[vector].isr)
        {
            if (0 != isr_table[vector].arg)
                isr_table[vector].isr(vector, (void *)isr_table[vector].arg);
            else
                isr_table[vector].isr(vector, arg);   // 传递 stack 作为参数
        }
        else
        {
            loongarch_default_isr(vector, arg);
        }
	}
	else
	{
		loongarch_default_isr(vector, arg);
	}
}

//-------------------------------------------------------------------------------------------------

static void call_vectored_isr(unsigned int ipflag, uint64_t *stack);

void c_interrupt_handler(uint64_t *stack)
{
	unsigned int ecfg  = (unsigned int)stack[R_ECFG];
	unsigned int estat = (unsigned int)stack[R_ESTAT];

    estat &= ecfg & CSR_ESTAT_IS_MASK;
    if (estat == 0)                         /* 会出现吗? tlbrerr/merr? */
    {
        return;
    }

    /**
     * real 中断
     */
    if (estat & ECFGF_IPI)
    {
        bsp_irq_handler_dispatch(LS2K1000LA_IRQ_IPI, (void *)stack);
    }

    if (estat & ECFGF_TIMER)
    {
         __csrwr_d(1, LA_CSR_TINTCLR);    /* 清除 Timer 中断 */
        bsp_irq_handler_dispatch(LS2K1000LA_IRQ_TIMER, (void *)stack);
    }

    if (estat & ECFGF_PC)
    {
        bsp_irq_handler_dispatch(LS2K1000LA_IRQ_PERF, (void *)stack);
    }

    if (estat & ECFGF_IP7)
    {
        bsp_irq_handler_dispatch(LS2K1000LA_IRQ_HW7, (void *)stack);   // 未使用
    }

    if (estat & ECFGF_IP6)
    {
        bsp_irq_handler_dispatch(LS2K1000LA_IRQ_HW6, (void *)stack);   // 未使用
    }

    if (estat & ECFGF_IP5)
    {
        bsp_irq_handler_dispatch(LS2K1000LA_IRQ_HW5, (void *)stack);   // 未使用
    }

    if (estat & ECFGF_IP4)
    {
        bsp_irq_handler_dispatch(LS2K1000LA_IRQ_HW4, (void *)stack);   // TODO 核间中断
    }

    if (estat & ECFGF_IP3)
    {
        call_vectored_isr(ECFGF_IP3, stack);
    }

    if (estat & ECFGF_IP2)
    {
        call_vectored_isr(ECFGF_IP2, stack);
    }

    if (estat & ECFGF_IP1)
    {
        call_vectored_isr(ECFGF_IP1, stack);
    }

    if (estat & ECFGF_IP0)
    {
        call_vectored_isr(ECFGF_IP0, stack);
    }

    if (estat & ECFGF_SIP1)
    {
        clear_csr_estat(ECFGF_SIP1);
        bsp_irq_handler_dispatch(LS2K1000LA_IRQ_SW1, (void *)stack);
    }

    if (estat & ECFGF_SIP0)
    {
        clear_csr_estat(ECFGF_SIP0);
        bsp_irq_handler_dispatch(LS2K1000LA_IRQ_SW0, (void *)stack);
    }

    return;
}

//-------------------------------------------------------------------------------------------------

static void call_vectored_isr(unsigned int ipflag, uint64_t *stack)
{
    unsigned long int sr, en;
    int index;

/*
    sr  = ((uint64_t)READ_REG32(INTC1_SR_BASE)) << 32;  // 高 32 位状态
    en  = ((uint64_t)READ_REG32(INTC1_EN_BASE)) << 32;  // 高 32 位状态
    sr |= READ_REG32(INTC0_SR_BASE);                    // 低 32 位使能
    en |= READ_REG32(INTC0_EN_BASE);                    // 低 32 位使能
 */

    switch (__cpunum)
    {
        case 0:
            sr  = ((uint64_t)READ_REG32(CORE0_INTISR1)) << 32;  // 高 32 位状态
            en  = ((uint64_t)READ_REG32(INTC1_EN_BASE)) << 32;  // 高 32 位状态
            sr |= READ_REG32(CORE0_INTISR0);                    // 低 32 位使能
            en |= READ_REG32(INTC0_EN_BASE);                    // 低 32 位使能
            break;

        case 1:
            sr  = ((uint64_t)READ_REG32(CORE1_INTISR1)) << 32;  // 高 32 位状态
            en  = ((uint64_t)READ_REG32(INTC1_EN_BASE)) << 32;  // 高 32 位状态
            sr |= READ_REG32(CORE1_INTISR0);                    // 低 32 位使能
            en |= READ_REG32(INTC0_EN_BASE);                    // 低 32 位使能
            break;

        default:
            sr = 0;
            en = 0;
            break;
    }

    sr &= en;
    index = 0;

    while (sr)
    {
        /* check LSB
         */
        if (sr & 1)
        {
            /* clear interrupt flag, this will disable the interrupt together
             */
            if (index < 32)
            {
                WRITE_REG32(INTC0_CLR_BASE, 1 << index);
            }
            else
            {
                WRITE_REG32(INTC1_CLR_BASE, 1 << (index - 32));
            }

            /*
             * include INTC0/INTC1
             */
            bsp_irq_handler_dispatch(LS2K1000LA_IRQ0_BASE + index, (void *)stack);

            /* enable this interrupt because disable just.
             */
            if (index < 32)
            {
                OR_REG32(INTC0_SET_BASE, 1 << index);
            }
            else
            {
                OR_REG32(INTC1_SET_BASE, 1 << (index - 32));
            }
        }

        index++;

        /* shift, and make sure MSB is clear */
        sr = (sr >> 1) & 0x7fffffffffffffffull;
    }
}

//-------------------------------------------------------------------------------------------------

/* Generate a software interrupt */
int assert_sw_irq(unsigned int irqnum)
{
    if (irqnum == ECFGB_SIP0)
        set_csr_estat(ECFGF_SIP0);
    else if (irqnum == ECFGB_SIP1)
        set_csr_estat(ECFGF_SIP1);
    else
        return -1;

    return irqnum;
}

/* Clear a software interrupt */
int negate_sw_irq(unsigned int irqnum)
{
    if (irqnum == ECFGF_SIP0)
        clear_csr_estat(ECFGF_SIP0);
    else if (irqnum == ECFGF_SIP1)
        clear_csr_estat(ECFGF_SIP1);
    else
        return -1;

    return irqnum;
}

//-------------------------------------------------------------------------------------------------

void ls2k_install_irq_handler(int vector, irq_handler_t isr, void *arg)
{
    if ((vector >= 0) && (vector < BSP_INTERRUPT_VECTOR_MAX))
    {
        loongarch_critical_enter();

        /*
         * install interrupt handler
         */
        isr_table[vector].isr = isr;
        isr_table[vector].arg = (uint64_t)arg;

        loongarch_critical_exit();
    }
}

static inline unsigned int get_int_route_entry_reg(unsigned int vector)
{
    if ((vector>= INTC0_UART0_3_IRQ) && (vector <= INTC0_SDIO_IRQ))
    {
        return INTC0_ROUTE_BASE + (vector - INTC0_UART0_3_IRQ);
    }
    else if ((vector>= INTC1_PCIE00_IRQ) && (vector <= INTC1_GPIO3_IRQ))
    {
        return INTC1_ROUTE_BASE + (vector - INTC1_PCIE00_IRQ);
    }

    return 0;
}

/*
 * 设置中断 route
 */
void ls2k_set_irq_routeip(int vector, int route_ip)
{
    if ((vector >= INTC0_UART0_3_IRQ) && (vector <= INTC1_GPIO3_IRQ) &&
        (route_ip >= INT_ROUTE_IP0) && (route_ip <= INT_ROUTE_IP3))
    {
        unsigned int route_reg = get_int_route_entry_reg(vector);
        if (route_reg > 0)
        {
            loongarch_critical_enter();

            switch (__cpunum)
            {
                case 0:
                    WRITE_REG8(route_reg, route_ip | INT_ROUTE_CORE0);   // 直接操作字节
                    break;

                case 1:
                    WRITE_REG8(route_reg, route_ip | INT_ROUTE_CORE1);   // 直接操作字节
                    break;
            }

            loongarch_critical_exit();
        }
    }
}

/*
 * 设置中断触发方式
 */
void ls2k_set_irq_triggermode(int vector, int mode)
{
    if ((vector >= INTC0_UART0_3_IRQ) || (vector <= INTC1_GPIO3_IRQ))
    {
        int intc_index, bit_index;
        unsigned int edge_reg, bit_val;

        vector -= LS2K1000LA_IRQ0_BASE;
        intc_index = vector / 32;
        bit_index  = vector % 32;
        bit_val = 1 << bit_index;

        switch (intc_index)
        {
            case 0:
                edge_reg = INTC0_EDGE_BASE;
                break;

            case 1:
                edge_reg = INTC1_EDGE_BASE;
                break;

            default:
                return;
        }

        switch (mode)
        {
            case INT_TRIGGER_LEVEL:         /* 电平触发 */
                AND_REG32(edge_reg, ~bit_val);
                break;

            case INT_TRIGGER_PULSE:         /* 边沿触发 */
                OR_REG32(edge_reg, bit_val);
                break;
        }
    }
}

void ls2k_remove_irq_handler(int vector)
{
    if ((vector >= 0) && (vector < BSP_INTERRUPT_VECTOR_MAX))
    {
        loongarch_critical_enter();

        ls2k_interrupt_disable(vector);

        /*
         * remove isr
         */
        isr_table[vector].isr = loongarch_default_isr;
        isr_table[vector].arg = vector;

        loongarch_critical_exit();
    }
}

//-------------------------------------------------------------------------------------------------
// 根据中断向量来使能/禁止中断
//-------------------------------------------------------------------------------------------------

void ls2k_interrupt_enable(int vector)
{
    if ((vector >= 0) && (vector < BSP_INTERRUPT_VECTOR_MAX))
    {
        int intc_index, bit_index;
        unsigned int bit_val;

        vector -= LS2K1000LA_IRQ0_BASE;
        intc_index = vector / 32;
        bit_index  = vector % 32;
        bit_val = 1 << bit_index;

        switch (intc_index)
        {
            case 0:
                OR_REG32(INTC0_SET_BASE, bit_val);
                break;

            case 1:
                OR_REG32(INTC1_SET_BASE, bit_val);
                break;
        }
    }
}

void ls2k_interrupt_disable(int vector)
{
    if ((vector >= 0) && (vector < BSP_INTERRUPT_VECTOR_MAX))
    {
        int intc_index, bit_index;
        unsigned int bit_val;

        vector -= LS2K1000LA_IRQ0_BASE;
        intc_index = vector / 32;
        bit_index  = vector % 32;
        bit_val = 1 << bit_index;

        switch (intc_index)
        {
            case 0:
                WRITE_REG32(INTC0_CLR_BASE, bit_val);
                break;

            case 1:
                WRITE_REG32(INTC1_CLR_BASE, bit_val);
                break;
        }
    }
}

//-------------------------------------------------------------------------------------------------

/*
 * @@ END
 */


