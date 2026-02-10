/*
 * Copyright (C) 2020-2023 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 */

#include <string.h>
#include <larchintrin.h>

#include "bsp.h"
#include "cpu.h"
#include "ls1c103.h"
#include "clock.h"

//-----------------------------------------------------------------------------

extern void exception_common_entry(void);
extern void console_init(unsigned int baudrate);
extern int main(void);

//-----------------------------------------------------------------------------
// global Variable
//-----------------------------------------------------------------------------

unsigned int cpu_frequency = 8000000;     // CPU 工作频率
unsigned int bus_frequency = 8000000;     // BUS 工作频率

//-----------------------------------------------------------------------------
// 全局控制器
//-----------------------------------------------------------------------------

HW_PMU_t *g_pmu = (HW_PMU_t *)LS1C103_PMU_BASE;

//-----------------------------------------------------------------------------
// 使用外部 8M 时钟配置
//-----------------------------------------------------------------------------

#define USER_OUTER_OSC8M    0

#if USER_OUTER_OSC8M

static void OSC_init_outer(void)
{
    int nDelay = 2*8000;   		// 2ms

    g_pmu->ChipCtrl &= ~CHIPCTRL_8M_SEL;
    g_pmu->ChipCtrl |= CHIPCTRL_8M_EN;

    /*
     * 不加延时，睡眠起来后会时钟失效? 用loop 计数循环
     */
    while (nDelay-- > 0);

    while (g_pmu->CmdSts & CMDSR_8M_FAIL)
    {
        g_pmu->ChipCtrl &= ~CHIPCTRL_8M_SEL;
    }

    g_pmu->ChipCtrl |= CHIPCTRL_8M_SEL;
}

#endif

//-----------------------------------------------------------------------------

static void get_frequency(void)
{
    unsigned int val = g_pmu->CmdSts;

    if (!(val & CMDSR_8M_SEL)) 		/* 没有使用外部 8M 晶振*/
    {
#if 0
        /*
         * XXX 103: 这个内置晶振频率在FLASH 0xBF0201B0 中?
         */
        val = (*(volatile unsigned int *)0xBF0201B0) * 1000;
        if (val > 0)
        {
            cpu_frequency = val;
            bus_frequency = val;
        }
#endif

#if !USER_OUTER_OSC8M
        /*
         * 内部 32MHZ 晶振？
         */
        val = g_pmu->ChipCtrl;
        
        if (val & CHIPCTRL_FASTEN)      // 3 分频
        {
        	cpu_frequency = cpu_frequency * 4 / 3;
        	bus_frequency = bus_frequency * 4 / 3;
        }
#endif
    }
}

/****************************************************************************** 
 * ls1c103 bsp start
 */

extern void ls1c103_init_isr_table(void);

void bsp_start(void)
{
    unsigned int eentry;

    loongarch_interrupt_disable();

    /**
     * 把中断代码复制到0x10000000
     */
    eentry = __csrrd_w(LA_CSR_EBASE);
    memcpy((void *)eentry, (void *)exception_common_entry, 32);

    g_pmu->CommandW = 0;            // CMDSR_SLEEP_EN;

#if USER_OUTER_OSC8M
    OSC_init_outer();               // 设置为外部时钟 TODO delay_ms() 不运行
#else

#endif

    get_frequency();                /* 获取频率配置 */

    ls1c103_init_isr_table();       /* 初始化中断向量表 */

    console_init(115200);           /* initialize Console */

    Clock_initialize();             /* initialize system ticker */

    loongarch_interrupt_enable();   /* Enable all interrupts */

    //-------------------------------------------------------------------------
    // goto main function
    //-------------------------------------------------------------------------

    /**
     * 使用汇编跳转, 不使用 main();
     */
    volatile register unsigned int mainAddr = (unsigned int)main;

    asm volatile( "move $r20, %0 ; " : "=r"(mainAddr) );
    asm volatile( "jirl $r1, $r20, 0 ;" );

    /*
     * XXX never goto HERE!
     */
    for (;;) ;

    return;
}

//-----------------------------------------------------------------------------

/*
 * @@ END
 */
