/*
 * Copyright (C) 2021-2025 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 */

#include <string.h>
#include <larchintrin.h>

#include "bsp.h"
#include "cpu.h"
#include "ls1c203.h"
#include "clock.h"

#define TRY_OUTER_OSC8M   0

//-----------------------------------------------------------------------------

extern void exception_common_entry(void);
extern void console_init(unsigned int baudrate);
extern int main(void);

//-----------------------------------------------------------------------------

/*
 * global Variable
 */
unsigned int cpu_frequency;         // CPU  工作频率
unsigned int bus_frequency;         // BUS  片上总线时钟
unsigned int eapb_frequency;        // EAPB 常开域总线时钟

//-----------------------------------------------------------------------------
// 全局控制器
//-----------------------------------------------------------------------------

HW_CONF_t *g_conf = (HW_CONF_t *)CONF_BASE;

HW_PMU_t *g_pmu = (HW_PMU_t *)PMU_BASE;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/**
 * 配置 PLL
 *
 * PLLout = PLLref * M / N;
 *
 *  其中:   2.5M <= PLLref <= 25M;
 *          100M <= PLLout <= 200M
 *          M >= 4; N >= 1
 */
int ls1c203_pll_init(unsigned int osc, unsigned int out)
{
    unsigned char m, n, best_m=0, best_n=0;
    unsigned int bias = 1000000;

    for (m = PLL_M_MIN; m <= PLL_M_MAX; m++)
    {
        for (n = PLL_N_MIN; n <= PLL_N_MAX; n++)
        {
            unsigned int calc_out, this_bias;

            calc_out = osc * m / n;
            this_bias = (calc_out > out) ? (calc_out - out) : (out - calc_out);

            if (this_bias < bias)
            {
                best_m = m;
                best_n = n;
                bias = this_bias;
                
                if (bias == 0) break;
            }
        }

        if (bias == 0) break;
    }

    if ((best_m == 0) || (best_n == 0))
    {
        g_conf->pllctrl  = PLL_PD | PLL_BYPASS;
        g_conf->pllctrl |= PLL_PD;

        return -1;
    }

    /*
     * Config PLLCtrl
     */
_again:

    g_conf->pllctrl  = PLL_PD | PLL_BYPASS;
    g_conf->pllctrl |= PLL_PD;

    g_conf->pllctrl |= (best_m << PLL_M_SHIFT) | (best_n << PLL_N_SHIFT);
    g_conf->pllctrl &= ~(PLL_PD | PLL_BYPASS);

    bias = 1000000;
    
	while (!(g_conf->pllctrl & PLL_LOCK))
    {
        if (bias-- <= 0)
        {
            goto _again;
        }
    }

    g_conf->pllctrl |= PLL_SEL;

    return 0;
}

#if TRY_OUTER_OSC8M
/**
 * 使用外部 8M 时钟配置
 */
void try_set_outer_osc(void)
{
    int clkup_delay = g_conf->cpuctrl & CPU_CLKUP_DLY_MASK;

    switch (clkup_delay)
    {
        case CPU_CLKUP_DLY_2440US: clkup_delay = 2440000; break;
        case CPU_CLKUP_DLY_1460US: clkup_delay = 1460000; break;
        case CPU_CLKUP_DLY_480US:  clkup_delay =  480000; break;
        case CPU_CLKUP_DLY_5140US: 
        default:                   clkup_delay = 5140000; break;
    }

    g_conf->cpuctrl &= ~CPU_8M_SEL;
    g_conf->cpuctrl |= CPU_OSC8M_EN;

    /*
     * 等待外部时钟有效
     */
    clkup_delay /= 4;
    while (g_conf->cpuctrl & CPU_CLK8M_FAIL)
    {
        g_conf->cpuctrl |= CPU_8M_SEL;
        if (clkup_delay-- <= 0)
            break;
    }

    /*
     * 配置外部时钟
     */
    if (!(g_conf->cpuctrl & CPU_CLK8M_FAIL))    // CPU_CLK8M_SEL
    {
        ls1c203_pll_init(8000000, 160000000);
    }
    else
    {
        g_conf->pllctrl  = PLL_PD | PLL_BYPASS;
    }
}
#endif

void get_chip_frequency(void)
{
    unsigned int osc_freq;
    unsigned char m, n;

#if TRY_OUTER_OSC8M
    if (g_conf->cpuctrl & CPU_CLK8M_SEL)        /* 使用外部 8M 晶振 */
    {
        osc_freq = 8000000;
    }
    else                                        /* 使用内部 16M 晶振 */
#endif
    {
        osc_freq = 16000000;
    }

    if (!(g_conf->pllctrl & PLL_SEL))
    {
        /*
         * always use pll?
         */
        ls1c203_pll_init(osc_freq, 160000000);
    }

    if (g_conf->pllctrl & PLL_SEL)              /* 使用 PLL */
    {
        m = (g_conf->pllctrl & PLL_M_MASK) >> PLL_M_SHIFT;
        n = (g_conf->pllctrl & PLL_N_MASK) >> PLL_N_SHIFT;

        cpu_frequency = osc_freq * m / n;
        bus_frequency = cpu_frequency / 4;

        if (!(g_conf->cpuctrl & CPU_TURBOEN))
            cpu_frequency = bus_frequency;
    }
    else
    {
        cpu_frequency = osc_freq;
        bus_frequency = osc_freq;
    }

    eapb_frequency = osc_freq / 4;
}

/****************************************************************************** 
 * ls1c203 bsp start
 */

extern void __ISR_tbl_initialize(void);

void bsp_start(void)
{
    loongarch_interrupt_disable();

    /**
     * 把中断代码复制到0x10000000
     */
    unsigned int eentry = __csrrd_w(LA_CSR_EBASE);
    memcpy((void *)eentry, (void *)exception_common_entry, 32);

#if 0
    g_pmu->ChipCtrl |= CHIPCTRL_INPUT_HOLD;
    g_pmu->CommandW = 0;            // CMDSR_SLEEP_EN;
#endif

#if TRY_OUTER_OSC8M
    try_set_outer_osc();            /* 设置为外部时钟 */
#endif

    get_chip_frequency();           /* 获取频率配置 */

    __ISR_tbl_initialize();         /* 初始化中断向量表 */

    console_init(115200);           /* initialize Console */

    Clock_initialize();             /* initialize system ticker */

    loongarch_interrupt_enable();   /* Enable all interrupts */

    //-------------------------------------------------------------------------
    // goto main function
    //-------------------------------------------------------------------------

    /*
     * 使用汇编跳转, 不使用 main();
     */
    volatile register unsigned int mainAddr = (unsigned int)main;

    asm volatile( "move $r20, %0 ; " : "=r"(mainAddr) );
    asm volatile( "jirl $r1, $r20, 0 ;" );

    /*
     * NEVER go here!
     */
    while (1)
        ;

    return;
}


