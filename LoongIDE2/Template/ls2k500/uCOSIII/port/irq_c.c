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

#include "ls2k500.h"
#include "ls2k500_irq.h"

extern void printk(const char *fmt, ...);

#if (!USE_EXTINT)
#define BSP_INTERRUPT_VECTOR_MAX    LS2K500_IRQ_COUNT
#else
#define BSP_INTERRUPT_VECTOR_MAX    LS2K500_EXTIRQ_COUNT
#endif

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
 * 需要二次分发的GPIO中断向量表
 */
static isr_tbl_t gpio_isr_table[GPIO_INT_COUNT];

static void ls2k500_gpio_shared_irq_handler(int vector, void *arg);

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

    for (i=0; i<GPIO_INT_COUNT; i++)
    {
		gpio_isr_table[i].isr = loongarch_default_isr;
		gpio_isr_table[i].arg = 0;
	}
	
    for (i=0; i<4; i++)
    {
        WRITE_REG32(GPIO_INTENx_ADDR(i), 0);
    }

#if USE_EXTINT

    OR_REG32(CHIP_CTRL0_BASE, CTRL0_EXTIOINT_EN);   /* 使能全局 EXTIOINT */

    WRITE_REG8(EXTINT_MAP0, EXTINT_ROUTE_IP2);      /* 第 0 组中断路由到 IP2 */
    WRITE_REG8(EXTINT_MAP1, EXTINT_ROUTE_IP3);      /* 第 1 组中断路由到 IP3 */
    WRITE_REG8(EXTINT_MAP2, EXTINT_ROUTE_IP4);      /* 第 2 组中断路由到 IP4 */
    WRITE_REG8(EXTINT_MAP3, EXTINT_ROUTE_IP5);      /* 第 3 组中断路由到 IP5 */

    for (i=EXTINTC2_GPIO0_2_IRQ; i<=EXTINTC3_GPIO120_122_IRQ; i++)
    {
        ls2k_install_irq_handler(i, ls2k500_gpio_shared_irq_handler, NULL);
    }

#else

    AND_REG32(CHIP_CTRL0_BASE, ~CTRL0_EXTIOINT_EN); /* 禁止全局 EXTIOINT */
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
    for (i=INTC1_GPIO0_31_IRQ; i<=INTC1_GPIO96_127_IRQ; i++)
    {
        ls2k_install_irq_handler(i, ls2k500_gpio_shared_irq_handler, NULL);
        //ls2k_set_irq_triggermode(i, INT_TRIGGER_LEVEL);
    }

#endif
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
        bsp_irq_handler_dispatch(LS2K500_IRQ_IPI, (void *)stack);
    }

    if (estat & ECFGF_TIMER)
    {
        __csrwr_d(1, LA_CSR_TINTCLR);    /* 清除 Timer 中断 */
        bsp_irq_handler_dispatch(LS2K500_IRQ_TIMER, (void *)stack);
    }
    
    if (estat & ECFGF_PC)
    {
        bsp_irq_handler_dispatch(LS2K500_IRQ_PERF, (void *)stack);
    }

    if (estat & ECFGF_IP7)
    {
        bsp_irq_handler_dispatch(LS2K500_IRQ_HW7, (void *)stack);   // 未使用
    }
    
    if (estat & ECFGF_IP6)
    {
        bsp_irq_handler_dispatch(LS2K500_IRQ_HW6, (void *)stack);   // 未使用
    }
    
    if (estat & ECFGF_IP5)
    {
        bsp_irq_handler_dispatch(LS2K500_IRQ_HW5, (void *)stack);   // 未使用
    }
    
    if (estat & ECFGF_IP4)
    {
        bsp_irq_handler_dispatch(LS2K500_IRQ_HW4, (void *)stack);   // 未使用
    }

    if (estat & ECFGF_IP3)
    {
#if USE_EXTINT
        /*
         * 特别处理: GMAC0/GMAC1 interrupt
         */
        unsigned int en, sr;

        sr = READ_REG32(0x1fe11040);
        en = READ_REG32(0x1fe11424);
        sr &= en;

        if (sr & (1 << 12))                             /* GMAC0 */
        {
            WRITE_REG32(0x1fe1142c, (1 << 12));
            bsp_irq_handler_dispatch(EXTINTC3_GMAC0_IRQ, (void *)stack);
            WRITE_REG32(0x1fe11428, (1 << 12));
        }
        else if (sr & (1 << 14))                        /* GMAC1 */
        {
            WRITE_REG32(0x1fe1142c, (1 << 14));
            bsp_irq_handler_dispatch(EXTINTC3_GMAC1_IRQ, (void *)stack);
            WRITE_REG32(0x1fe11428, (1 << 14));
        }

#endif

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
        bsp_irq_handler_dispatch(LS2K500_IRQ_SW1, (void *)stack);
    }

    if (estat & ECFGF_SIP0)
    {
        clear_csr_estat(ECFGF_SIP0);
        bsp_irq_handler_dispatch(LS2K500_IRQ_SW0, (void *)stack);
    }
    
    return;
}

//-------------------------------------------------------------------------------------------------

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
            bsp_irq_handler_dispatch(LS2K500_IRQ0_BASE + index, (void *)stack);

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
        
        if (READ_REG8(EXTINT_MAP0 + map_num) != route_ip)
            continue;

        /*
         * 找到路由的中断组
         */
        en = READ_REG32(EXTINTC0_EN_BASE + map_num * 4);
        sr = READ_REG32(CORE_EXTI0_SR_BASE + map_num * 4);
   
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
                WRITE_REG32(EXTINTC0_CLR_BASE + map_num * 4, 1 << index);

                /*
                 * include EXTINT0~3
                 */
                bsp_irq_handler_dispatch(LS2K500_EXTIRQ0_BASE + map_num * 32 + index, (void *)stack);
            }

            index++;

            /* shift, and make sure MSB is clear */
            sr = (sr >> 1) & 0x7ffffffful;
        }

        return;  /* DONE */
    }

#endif
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

#if (!USE_EXTINT)
static inline unsigned int get_int_route_entry_reg(unsigned int vector)
{
    if ((vector>= INTC0_UART0_IRQ) && (vector <= INTC0_SDIO0_1_IRQ))
    {
        return INTC_ENTRY0_3 + (vector - INTC0_UART0_IRQ);
    }
    else if ((vector>= INTC1_PCIE0_IRQ) && (vector <= INTC1_UART7_9_IRQ))
    {
        return INTC_ENTRY32_35 + (vector - INTC1_PCIE0_IRQ);
    }

    return 0;
}

/*
 * 设置中断 route
 */
void ls2k_set_irq_routeip(int vector, int route_ip)
{
    if ((vector >= INTC0_UART0_IRQ) || (vector <= INTC1_UART7_9_IRQ))
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
    if ((vector >= INTC0_UART0_IRQ) || (vector <= INTC1_UART7_9_IRQ))
    {
        int intc_index, bit_index;
        unsigned int bit_val;

        vector -= LS2K500_IRQ0_BASE;
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

#if 0 // (!USE_EXTINT)
        /*
         * clear route
         */
        if ((vector >= INTC0_UART0_IRQ) || (vector <= INTC1_UART7_9_IRQ))
        {
            unsigned int route_reg;
            
            route_reg = get_int_route_entry_reg(vector);
            if (route_reg > 0)
            {
                WRITE_REG8(route_reg, 0);       // 直接操作字节
            }
        }
#endif

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

#if (!USE_EXTINT)
        vector -= LS2K500_IRQ0_BASE;
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
        vector -= LS2K500_EXTIRQ0_BASE;
        intc_index = vector / 32;
        bit_index  = vector % 32;

        if ((intc_index >= 0) && (intc_index <= 3))
        {
            bit_val = 1 << bit_index;
            WRITE_REG32(EXTINTC0_CLR_BASE + intc_index * 4, bit_val);
            OR_REG32(EXTINTC0_EN_BASE + intc_index * 4, bit_val);
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
        vector -= LS2K500_IRQ0_BASE;
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
        vector -= LS2K500_EXTIRQ0_BASE;
        intc_index = vector / 32;
        bit_index  = vector % 32;

        if ((intc_index >= 0) && (intc_index <= 3))
        {
            bit_val = 1 << bit_index;
            AND_REG32(EXTINTC0_EN_BASE + intc_index * 4, ~bit_val);
        }
#endif
    }
}

//-------------------------------------------------------------------------------------------------
// GPIO 中断处理
//-------------------------------------------------------------------------------------------------

static void ls2k500_gpio_shared_irq_handler(int vector, void *arg)
{
#if (!USE_EXTINT)

    int index, from;
    unsigned int in_val, int_en, out_en, changed;

    index = vector - INTC1_GPIO0_31_IRQ;
    from  = index << 5;

    in_val = READ_REG32(GPIO_IN_ADDR(index));
    out_en = READ_REG32(GPIO_OEN_ADDR(index));
    int_en = READ_REG32(GPIO_INTENx_ADDR(index));
    changed = in_val & out_en & int_en;
    
#else

    /*
     * XXX 实测结果: 响应 0==>1 的变化
     */
    int index, from, maskshift;
    unsigned int in_val, int_en, out_en, changed;

    index = vector - EXTINTC2_GPIO0_2_IRQ;
    from  = index << 2;
    maskshift = (index % 32) / 4 * 4;
    index >>= 3;
    
    in_val = READ_REG32(GPIO_IN_ADDR(index));
    in_val &= 0x7 << maskshift; 
    out_en = READ_REG32(GPIO_OEN_ADDR(index));
    out_en &= 0x7 << maskshift;
    int_en = READ_REG32(GPIO_INTENx_ADDR(index));
    int_en &= 0x7 << maskshift;

    changed = in_val & out_en & int_en;
    changed >>= maskshift;

#endif

    index = 0;

    /*
     * 检测变化的位
     */
    while (changed)
    {
        /* check LSB
         */
        if (changed & 1)
        {
            int gpionum = from + index;             /* 发生数据变化的GPIO NUM */

            if (gpio_isr_table[gpionum].isr)
            {
                if (0 != gpio_isr_table[gpionum].arg)
                {
                    gpio_isr_table[gpionum].isr(gpionum, (void *)gpio_isr_table[gpionum].arg);
                }
                else
                {
                    gpio_isr_table[gpionum].isr(gpionum, arg);  // 传递 arg 作为参数
                }
            }
            else
            {
                loongarch_default_isr(gpionum, arg);
            }
        }

        index++;

        /* shift, and make sure MSB is clear */
        changed = (changed >> 1) & 0x7fffffffull;
    }
}

//-------------------------------------------------------------------------------------------------

int ls2k500_gpio_interrupt_enable(int gpionum, int trigger_mode)
{
    if ((gpionum >= 0) && (gpionum < GPIO_INT_COUNT))
    {
        int register regval;
        int register regindex = gpionum / 32;
        int register bitshift = gpionum % 32;
    
        regval = READ_REG32(GPIO_INTENx_ADDR(regindex));
        regval |= 1 << bitshift;
        WRITE_REG32(GPIO_INTENx_ADDR(regindex), regval);

#if (!USE_EXTINT)

        /*
         * 设置共享中断的触发方式
         */
        regval = READ_REG32(INTC1_EN_BASE);
        bitshift = regindex + 26;   // INTC1_GPIO0_31_BIT
     
        switch (trigger_mode)
        {
            case 0x01:              /* GPIO_INT_TRIG_EDGE_UP 上升沿触发 gpio 中断 */
                AND_REG32(INTC1_EDGE_BASE, ~(1 << bitshift));
                OR_REG32(INTC1_POL_BASE, 1 << bitshift);
                break;

            case 0x02:              /* GPIO_INT_TRIG_EDGE_DOWN 下降沿触发 gpio 中断 */
                AND_REG32(INTC1_EDGE_BASE, ~(1 << bitshift));
                AND_REG32(INTC1_POL_BASE, ~(1 << bitshift));
                break;

            case 0x04:              /* GPIO_INT_TRIG_LEVEL_HIGH 高电平触发 gpio 中断 */
                OR_REG32(INTC1_EDGE_BASE, 1 << bitshift);
                OR_REG32(INTC1_POL_BASE, 1 << bitshift);
                break;

            case 0x08:              /* GPIO_INT_TRIG_LEVEL_LOW 低电平触发 gpio 中断 */
                OR_REG32(INTC1_EDGE_BASE, 1 << bitshift);
                AND_REG32(INTC1_POL_BASE, ~(1 << bitshift));
                break;
        }

#else

        if (gpionum < 36)
        {
            regval = READ_REG32(EXTINTC2_EN_BASE);
            bitshift = gpionum / 4 + 23;                // from EXTINTC2_GPIO0_2_BIT

            OR_REG32(EXTINTC2_EN_BASE, 1 << bitshift);
        }
        else
        {
            regval = READ_REG32(EXTINTC3_EN_BASE);
            bitshift = (gpionum - 36) / 4;

            OR_REG32(EXTINTC3_EN_BASE, 1 << bitshift);
        }

#endif

        return 0;
    }

    return -1;
}

int ls2k500_gpio_interrupt_disable(int gpionum)
{
    if ((gpionum >= 0) && (gpionum < GPIO_INT_COUNT))
    {
        int register regval;
        int register regindex = gpionum / 32;
        int register bitshift = gpionum % 32;

        regval = READ_REG32(GPIO_INTENx_ADDR(regindex));
        regval &= ~(1 << bitshift);
        WRITE_REG32(GPIO_INTENx_ADDR(regindex), regval);

        /*
         * 自动禁止共享中断
         */
#if (!USE_EXTINT)

        if (regval == 0)
        {
            bitshift = regindex + 26;   // INTC1_GPIO0_31_BIT
            WRITE_REG32(INTC1_CLR_BASE, 1 << bitshift);
        }
        
#else

        bitshift = (bitshift / 4) * 4;      /* 4个一组共享中断 */

        if ((regval & (0x0F << bitshift)) == 0)
        {
            if (gpionum < 36)
            {
                bitshift = gpionum / 4 + 23;
                AND_REG32(EXTINTC2_EN_BASE, ~(1 << bitshift));
            }
            else
            {
                bitshift = (gpionum - 36) / 4;
                AND_REG32(EXTINTC3_EN_BASE, ~(1 << bitshift));
            }
        }

#endif

        return 0;
    }

    return -1;
}

/**
 * TODO trigger_mode
 */
int ls2k500_gpio_isr_install(int gpionum, void (*isr)(int, void *), void *arg)
{
    if ((gpionum >= 0) && (gpionum < GPIO_INT_COUNT))
    {
#if USE_EXTINT
        if ((gpionum % 4) == 3)
        {
            printk("the gpio[%i] can't use irq, because (%i mod 4)=3.\n", gpionum, gpionum);
            return -1;
        }
#endif

        loongarch_critical_enter();

        /*
         * install interrupt handler
         */
        gpio_isr_table[gpionum].isr = isr;
        gpio_isr_table[gpionum].arg = (uint64_t)arg;

        //ls2k500_gpio_interrupt_enable(gpionum);

        loongarch_critical_exit();

        return 0;
    }

    return -1;
}

int ls2k500_gpio_isr_remove(int gpionum)
{
    if ((gpionum >= 0) && (gpionum < GPIO_INT_COUNT))
    {
    	loongarch_critical_enter();

        ls2k500_gpio_interrupt_disable(gpionum);

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

/*
 * @@ END
 */

