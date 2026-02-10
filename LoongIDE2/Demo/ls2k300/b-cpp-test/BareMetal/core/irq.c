/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * irq.c
 *
 * created: 2024-06-18
 *  author: Bian
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <larchintrin.h>

#include "bsp.h"

#include "cpu.h"
#include "regdef.h"

#include "ls2k300.h"
#include "ls2k300_irq.h"

extern void printk(const char *fmt, ...);

#if (!USE_EXTINT)
#define BSP_INTERRUPT_VECTOR_MAX    LS2K300_IRQ_COUNT
#else
#define BSP_INTERRUPT_VECTOR_MAX    LS2K300_EXTIRQ_COUNT
#endif

//-----------------------------------------------------------------------------
// 全局标志, 运行在中断处理程序中
//-----------------------------------------------------------------------------

unsigned int RunningInsideISR;

//-----------------------------------------------------------------------------
// 初始化中断
//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

/**
 * 需要二次分发的GPIO中断向量表
 */
static isr_tbl_t gpio_isr_table[GPIO_COUNT];

static void ls2k300_gpio_common_isr(int vector, void *arg);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

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

    for (i=0; i<GPIO_COUNT; i++)
    {
		gpio_isr_table[i].isr = loongarch_default_isr;
		gpio_isr_table[i].arg = 0;
	}

#if USE_EXTINT

    OR_REG32(CHIP_CTRL0_BASE, CTRL0_EXTIOINT_EN);   /* 使能全局 EXTIOINT */

    /*
     * 第 3 组中断路由到 IP0
     * 第 2 组中断路由到 IP1
     * 第 1 组中断路由到 IP2
     * 第 0 组中断路由到 IP3
     */
    WRITE_REG32(EXTIOI_MAP_BASE, 0x01020408);

    for (i=EXTI2_GPIO_0_3_IRQ; i<=EXTI3_GPIO_104_105_IRQ; i++)
    {
        ls2k_install_irq_handler(i, ls2k300_gpio_common_isr, NULL);
    }

#else

    AND_REG32(CHIP_CTRL0_BASE, ~CTRL0_EXTIOINT_EN);     /* 禁止全局 EXTIOINT */
    WRITE_REG32(INTC0_CLR_BASE, ~0);
    WRITE_REG32(INTC1_CLR_BASE, ~0);

    /*
     * 所有中断默认低电平触发
     */
    WRITE_REG32(INTC0_EDGE_BASE, 0);
    WRITE_REG32(INTC0_POL_BASE,  0);
    WRITE_REG32(INTC1_EDGE_BASE, 0);
    WRITE_REG32(INTC1_POL_BASE,  0);

    /*
     * 安装 GPIO 共享中断
     */
    for (i=INTC1_GPIO_0_15_IRQ; i<=INTC1_GPIO_96_105_IRQ; i++)
    {
        ls2k_install_irq_handler(i, ls2k300_gpio_common_isr, NULL);
        ls2k_set_irq_routeip(i, INT_ROUTE_IP3); /* 默认路由 */
    }

#endif
}

//-----------------------------------------------------------------------------
// 中断分发
//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

#if USE_EXTINT
__attribute__((weak))
void ls2k300_irq_dispatch_IP2(void (*dispatch)(int, void*), uint64_t *stack)
{
    return;
}
#endif

//-----------------------------------------------------------------------------

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
        bsp_irq_handler_dispatch(LS2K300_IRQ_IPI, (void *)stack);
    }

    if (estat & ECFGF_TIMER)
    {
        __csrwr_d(1, LA_CSR_TINTCLR);    /* 清除 Timer 中断 */
        bsp_irq_handler_dispatch(LS2K300_IRQ_TIMER, (void *)stack);
    }

    if (estat & ECFGF_PC)
    {
        bsp_irq_handler_dispatch(LS2K300_IRQ_PERF, (void *)stack);
    }

    if (estat & ECFGF_IP7)
    {
        bsp_irq_handler_dispatch(LS2K300_IRQ_HW7, (void *)stack);   // 未使用
    }

    if (estat & ECFGF_IP6)
    {
        bsp_irq_handler_dispatch(LS2K300_IRQ_HW6, (void *)stack);   // 未使用
    }

    if (estat & ECFGF_IP5)
    {
        bsp_irq_handler_dispatch(LS2K300_IRQ_HW5, (void *)stack);   // 未使用
    }
    
    if (estat & ECFGF_IP4)
    {
        bsp_irq_handler_dispatch(LS2K300_IRQ_HW4, (void *)stack);   // 未使用
    }

    if (estat & ECFGF_IP3)
    {
        call_vectored_isr(ECFGF_IP3, stack);
    }

    if (estat & ECFGF_IP2)
    {
        call_vectored_isr(ECFGF_IP2, stack);
#if USE_EXTINT
        ls2k300_irq_dispatch_IP2(bsp_irq_handler_dispatch, stack);
#endif
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
        bsp_irq_handler_dispatch(LS2K300_IRQ_SW1, (void *)stack);
    }

    if (estat & ECFGF_SIP0)
    {
        clear_csr_estat(ECFGF_SIP0);
        bsp_irq_handler_dispatch(LS2K300_IRQ_SW0, (void *)stack);
    }
    
    return;
}

//-----------------------------------------------------------------------------

static void call_vectored_isr(unsigned int ipflag, uint64_t *stack)
{
#if (!USE_EXTINT)

    uint64_t sr, en;
    int index;

    sr  = ((uint64_t)READ_REG32(INTC_CORE_ISR1)) << 32; // 高 32 位状态
    en  = ((uint64_t)READ_REG32(INTC1_EN_BASE)) << 32;  // 高 32 位状态
    sr |= READ_REG32(INTC_CORE_ISR0);                   // 低 32 位使能
    en |= READ_REG32(INTC0_EN_BASE);                    // 低 32 位使能

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
            bsp_irq_handler_dispatch(LS2K300_IRQ0_BASE + index, (void *)stack);

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
    
#else

    int map_num;
    unsigned char route_ip = ipflag >> 2;       /* EXTINT_ROUTE_IP3~0 */
    
    /**
     * 遍历 4 个 MAP 寄存器, 找出哪组中断路由到了 route_ip
     */
    for (map_num=0; map_num<4; map_num++)
    {
        int index;
        unsigned int en, sr;
        
        if (READ_REG8(EXTIOI_MAP_BASE + map_num) != route_ip)
            continue;

        /*
         * 找到路由的中断组
         */
        en = READ_REG32(EXTIOI_IEN0_BASE + map_num * 4);
        sr = READ_REG32(EXTIOI_CORE_ISR0_BASE + map_num * 4);
   
        sr &= en;
        index = 0;

        while (sr)
        {
            /* check LSB
             */
            if (sr & 1)
            {
                /* clear interrupt flag
                 */
                WRITE_REG32(EXTIOI_ISR0_BASE + map_num * 4, 1 << index);

                /*
                 * include EXTINT0~3
                 */
                bsp_irq_handler_dispatch(LS2K300_EXTIRQ0_BASE + map_num * 32 + index, (void *)stack);
            }

            index++;

            /* shift, and make sure MSB is clear */
            sr = (sr >> 1) & 0x7ffffffful;
        }

        return;  /* DONE */
    }

#endif
}

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

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

#if (!USE_EXTINT)
static inline unsigned int get_int_route_entry_reg(unsigned int vector)
{
    if ((vector>= INTC0_UART0_IRQ) && (vector <= INTC0_SDIO0_IRQ))
    {
        return INTC_ENTRY_0_7 + (vector - INTC0_UART0_IRQ);
    }
    else if ((vector>= INTC1_SDIO1_IRQ) && (vector <= INTC1_DDR_ECC1_IRQ))
    {
        return INTC_ENTRY_32_39 + (vector - INTC1_SDIO1_IRQ);
    }

    return 0;
}

/*
 * 设置中断 route
 */
void ls2k_set_irq_routeip(int vector, int route_ip)
{
    if ((vector >= INTC0_UART0_IRQ) || (vector <= INTC1_DDR_ECC1_IRQ))
    {
        unsigned int route_reg = get_int_route_entry_reg(vector);
        if (route_reg > 0)
        {
        	loongarch_critical_enter();

            WRITE_REG8(route_reg, route_ip); 	// 直接操作字节

            loongarch_critical_exit();
        }
    }
}

/*
 * 设置中断触发方式
 */
void ls2k_set_irq_triggermode(int vector, int mode)
{
    if ((vector >= INTC0_UART0_IRQ) || (vector <= INTC1_DDR_ECC1_IRQ))
    {
        int intc_index, bit_index;
        unsigned int bit_val;

        vector -= LS2K300_IRQ0_BASE;
        intc_index = vector / 32;
        bit_index  = vector % 32;
        bit_val = 1 << bit_index;

        if ((intc_index >= 0) && (intc_index <= 1))
        {
            switch (mode)
            {
                case INT_TRIGGER_LEVEL:         /* 电平触发 */
                    AND_REG32(INTC_EDGE(intc_index), ~bit_val);
                    break;

                case INT_TRIGGER_PULSE:         /* 边沿触发 */
                    OR_REG32(INTC_EDGE(intc_index), bit_val);
                    break;
            }
        }
    }
}
#endif

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

//-----------------------------------------------------------------------------
// 根据中断向量来使能/禁止中断
//-----------------------------------------------------------------------------

void ls2k_interrupt_enable(int vector)
{
    if ((vector >= 0) && (vector < BSP_INTERRUPT_VECTOR_MAX))
    {
        int intc_index, bit_index;
        unsigned int bit_val;

#if (!USE_EXTINT)
        vector -= LS2K300_IRQ0_BASE;
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
#else
        vector -= LS2K300_EXTIRQ0_BASE;
        intc_index = vector / 32;
        bit_index  = vector % 32;

        if ((intc_index >= 0) && (intc_index <= 3))
        {
            bit_val = 1 << bit_index;
            WRITE_REG32(EXTIOI_ISR0_BASE + intc_index * 4, bit_val);
            OR_REG32(EXTIOI_IEN0_BASE + intc_index * 4, bit_val);
        }
#endif
    }
}

void ls2k_interrupt_disable(int vector)
{
    if ((vector >= 0) && (vector < BSP_INTERRUPT_VECTOR_MAX))
    {
        int intc_index, bit_index;
        unsigned int bit_val;
        
#if (!USE_EXTINT)
        vector -= LS2K300_IRQ0_BASE;
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
#else
        vector -= LS2K300_EXTIRQ0_BASE;
        intc_index = vector / 32;
        bit_index  = vector % 32;

        if ((intc_index >= 0) && (intc_index <= 3))
        {
            bit_val = 1 << bit_index;
            AND_REG32(EXTIOI_IEN0_BASE + intc_index * 4, ~bit_val);
        }
#endif
    }
}

//-----------------------------------------------------------------------------
// GPIO 中断处理
//-----------------------------------------------------------------------------

static void ls2k300_gpio_common_isr(int vector, void *arg)
{
    int i, gpio1, gpio2;
    
    /*
     * 计算出中断号对应的 GPIO 编号
     */
#if (!USE_EXTINT)
    gpio1 = (vector - INTC1_GPIO_0_15_IRQ) * 16;
    gpio2 = gpio1 + 15;
#else
    gpio1 = (vector - EXTI2_GPIO_0_3_IRQ) * 4;
    gpio2 = gpio1 + 3;
#endif

    for (i=gpio1; i<=gpio2; i++)
    {
        /**
         * 处理 OEN & IEN & ISR
         */
        if (READ_REG8(GPIO_OEN_ADDR + i) &&    		// DIR_IN
            READ_REG8(GPIO_IEN_ADDR + i) &&     	// Int En
            READ_REG8(GPIO_ISR_ADDR + i))       	// Int Status
        {
        	WRITE_REG8(GPIO_ICLR_ADDR + i, 1);      // clear Int Status

            if (gpio_isr_table[i].isr)
            {
                if (gpio_isr_table[i].arg)
                {
                    gpio_isr_table[i].isr(i, (void *)gpio_isr_table[i].arg);
                }
                else
                {
                    gpio_isr_table[i].isr(i, arg);  // 传递 arg 作为参数
                }
            }
            else
            {
                loongarch_default_isr(i, arg);
            }
        }
    }
}

//-----------------------------------------------------------------------------

int ls2k300_gpio_interrupt_enable(int gpionum, int trigger_mode)
{
    if ((gpionum >= 0) && (gpionum < GPIO_COUNT))
    {
        /*
         * 必须是设置为 IN 的 GPIO
         */
        if (READ_REG8(GPIO_OEN_ADDR + gpionum) == 0)
            return -1;

        WRITE_REG8(GPIO_ICLR_ADDR  + gpionum, 1);
        WRITE_REG8(GPIO_IDUAL_ADDR + gpionum, 0);

        /*
         *  触发方式
         *
         * 	| POL	| EDGE	|	 描述			|
         * 	|-------|-------|---------------|
         *	|  0	|  0	| 低电平触发中断	|
         *	|  1	|  0	| 高电平触发中断	|
         * 	|  0	|  1	| 下降沿触发中断	|
         *	|  1	|  1	| 上升沿触发中断	|
         *
         */

        switch (trigger_mode)
        {
            case 0x01:          /* GPIO_INT_TRIG_EDGE_UP */
                WRITE_REG8(GPIO_IPOL_ADDR  + gpionum, 1);
                WRITE_REG8(GPIO_IEDGE_ADDR + gpionum, 1);
                break;
          
            case 0x02:          /* GPIO_INT_TRIG_EDGE_DOWN */
                WRITE_REG8(GPIO_IPOL_ADDR  + gpionum, 0);
                WRITE_REG8(GPIO_IEDGE_ADDR + gpionum, 1);
                break;
    
            case 0x04:          /* GPIO_INT_TRIG_LEVEL_HIGH */
                WRITE_REG8(GPIO_IPOL_ADDR  + gpionum, 1);
                WRITE_REG8(GPIO_IEDGE_ADDR + gpionum, 0);
                break;

            case 0x08:          /* GPIO_INT_TRIG_LEVEL_LOW */
                WRITE_REG8(GPIO_IPOL_ADDR  + gpionum, 0);
                WRITE_REG8(GPIO_IEDGE_ADDR + gpionum, 0);
                break;

            case 0x10:          /* GPIO_INT_TRIG_DUAL */
                WRITE_REG8(GPIO_IDUAL_ADDR + gpionum, 1);
                WRITE_REG8(GPIO_IEDGE_ADDR + gpionum, 1);
                break;

            default:
                WRITE_REG8(GPIO_IEN_ADDR  + gpionum, 0);
                return -1;
        }
        
        /*
         * 使能中断
         */
        WRITE_REG8(GPIO_ICLR_ADDR + gpionum, 1);
        WRITE_REG8(GPIO_IEN_ADDR  + gpionum, 1);

        return 0;
    }

    return -1;
}

int ls2k300_gpio_interrupt_disable(int gpionum)
{
    if ((gpionum >= 0) && (gpionum < GPIO_COUNT))
    {
        /*
         * 禁止中断
         */
        WRITE_REG8(GPIO_IEN_ADDR  + gpionum, 0);
        WRITE_REG8(GPIO_ICLR_ADDR + gpionum, 1);

        return 0;
    }

    return -1;
}

/**
 * 安装 GPIO 中断
 */
int ls2k300_gpio_isr_install(int gpionum, void (*isr)(int, void *), void *arg)
{
    if ((gpionum >= 0) && (gpionum < GPIO_COUNT))
    {
        loongarch_critical_enter();

        /*
         * install interrupt handler
         */
        gpio_isr_table[gpionum].isr = isr;
        gpio_isr_table[gpionum].arg = (uint64_t)arg;

        loongarch_critical_exit();
        
        return 0;
    }

    return -1;
}

int ls2k300_gpio_isr_remove(int gpionum)
{
    if ((gpionum >= 0) && (gpionum < GPIO_COUNT))
    {
    	loongarch_critical_enter();

        ls2k300_gpio_interrupt_disable(gpionum);

        /*
         * remove isr
         */
        gpio_isr_table[gpionum].isr = loongarch_default_isr;
        gpio_isr_table[gpionum].arg = gpionum;

        loongarch_critical_exit();
        
        return 0;
    }

    return -1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/*
 * @@ END
 */

