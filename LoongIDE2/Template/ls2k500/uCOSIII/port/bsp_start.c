/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * bsp_start.c
 *
 * created: 2022-02-18
 *  author: Bian
 */

/*
 * 1.安装中断向量
 * 2.初始化 Tick
 * 3.初始化 Console
 */

#include <stdint.h>
#include <string.h>
#include <larchintrin.h>

#include "cpu.h"
#include "ls2k500.h"

#include "os.h"

extern void machine_error_entry(void);
extern void exception_common_entry(void);
extern void tlbrefill_error_entry(void);

extern void loongarch_init_isr_table(void);
extern void Clock_initialize(void);

extern void console_init(unsigned int baudrate);
extern int main(void);

//-------------------------------------------------------------------------------------------------
// 2K0500时钟信号说明
//-------------------------------------------------------------------------------------------------
//
// 信号名称 	            频率(MHz) 	    类型 	  描述
// SYSCLK 	                100 	        I/O 	  外接系统参考时钟晶振
// RTC_XI/RTC_XO 	        32.768K 	    I/O 	  RTC参考时钟晶体
// EJTAG_TCK 	            33 	            I 	      JTAG时钟
// PCI_CLOCK 	            33~66 	        I 	      PCI总线接口时钟输入(PCI/LPC)
// TEST_CLOCK 	            100 	        I 	      TEST测试时钟输入
// PCIE_REFCLKp/n_I 	    100 	        DIFF IN   PCIE参考时钟输入, 如果使用内部100M时钟，则可悬空。
// SATA0~1_REFCLKp/n_I 	    100 	        DIFF IN   SATA两路参考时钟输入, 如果使用内部100M时钟，则可悬空。
// USB0~3_REFCLKp/n_I 	    25 	            DIFF IN   USB2.0四路参考时钟输入, 如果使用内部参考时钟，则可悬空。
// U3_REFCLKp/n_I 	        100 	        DIFF IN   USB3.0参考时钟输入, 如果使用内部参考时钟，则可悬空。
// DDR_CKp[1:0]
// DDR_CKn[1:0] 	        400 	        DIFF OUT  DDR3 SDRAM 差分时钟输出
// PCIE_REFCLKp0/1
// PCIE_REFCLKn0/1 	        100 	        DIFF OUT  PCIE两路参考差分时钟输出
// LCD_CLK 	                150 	        O 	      DVO显示时钟输出
// SPI_CLK 	                50 	            O 	      SPI总线时钟输出
// SDIO_CLK 	            50 	            O 	      SDIO总线时钟输出
// GMAC_RX_CLK              125 	        I         GMAC网络接收、发送时钟
// GMAC_TX_CLK 	            125             I/O 	  GMAC网络接收、发送时钟
// AC97_BCLK_I              12              I         AC97 数据流时钟
// HDA_BCLK_O               24              O         HDA 数据流时钟
// NODE_CLOCK 	            500~800 	    G 	      NODE模块时钟，供处理器核、SCACHE、IODMA、L1-XBAR模块使用
// DDR_CLOCK 	            400~600 	    G 	      DDR控制器时钟，供DDR3控制器使用
// NETWORK_CLOCK 	        200~400 	    G 	      供NETWORK互联结构使用
// PRINT_CLOCK 	            100~400 	    G 	      供PRINT打印接口控制器使用
// HDA_CLOCK 	            24 	            G 	      供HDA接口数据流时钟使用
// PIX0/1_CLOCK 	        100~200 	    G 	      供两路DVO显示时钟使用
// GPU_CLOCK 	            200~300 	    G 	      供GPU功能模块使用
// GMAC_CLOCK 	            125 	        G 	      供GMAC功能模块使用
// SB_CLOCK 	            100~200 	    G 	      供HDA、BOOT及CONFBUS模块内部控制器逻辑使用
// SATA_CLOCK 	            100~200 	    G 	      供SATA功能模块使用
// USB_CLOCK 	            100~200 	    G 	      USB、USB3.0功能模块控制器时钟
// APB_CLOCK 	            80~150 	        G 	      供APB各个设备使用
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// PMON 配置的芯片频率参数
//-------------------------------------------------------------------------------------------------

unsigned int cpu_frequency;             // 芯片主频
unsigned int ddr_frequency;
unsigned int hda_frequency;
unsigned int net_frequency;
unsigned int gmac_frequency;
unsigned int sb_frequency;
unsigned int gpu_frequency;

unsigned int pix0_frequency;
unsigned int pix1_frequency;

unsigned int lsu_frequency;
unsigned int print_frequency;
unsigned int apb_frequency;             // APB 总线频率
unsigned int usb_frequency;
unsigned int sata_frequency;

/**
 * 晶振频率
 */
#define OSC_FREQ    100000000

static void fetch_frequency_setting(void)
{
    unsigned int clk_sel, r0, r1;
    unsigned int odiv, loopc, refc;

    clk_sel = READ_REG32(CHIP_SAMP0_BASE);
    clk_sel = (clk_sel >> SAMP0_CLK_SEL_SHIFT) & SAMP0_CLK_SEL_MASK;

    switch (clk_sel)
    {
        case 0:         // 硬件低频时钟配置模式
            cpu_frequency  = 500000000;
            ddr_frequency  = 480000000;
            net_frequency  = 320000000;
            gpu_frequency  = 200000000;
            hda_frequency  = 24000000;
            pix0_frequency = 100000000;
            pix1_frequency = 100000000;
            gmac_frequency = 0;
            apb_frequency  = 100000000;
            usb_frequency  = 100000000;
            sata_frequency = 100000000;
            break;

        case 1:         // 硬件高频时钟配置模式
            cpu_frequency  = 800000000;
            ddr_frequency  = 600000000;
            net_frequency  = 400000000;
            gpu_frequency  = 300000000;
            hda_frequency  = 24000000;
            pix0_frequency = 200000000;
            pix1_frequency = 200000000;
            gmac_frequency = 0;
            apb_frequency  = 150000000;
            usb_frequency  = 150000000;
            sata_frequency = 150000000;
            break;

        case 2:         // 软件配置模式
        {
            r0 = READ_REG32(PLL_NODE0_BASE);
            loopc = (r0 >> NODE0_DIV_LOOPC_SHIFT) & NODE0_DIV_LOOPC_MASK;
            refc  = (r0 >> NODE0_DIV_REFC_SHIFT)  & NODE0_DIV_REFC_MASK;
            
            if (r0 & NODE0_PLL_SEL)
            {
                odiv = (r0 >> NODE0_DIV_SHIFT) & NODE0_DIV_MASK;
                cpu_frequency = OSC_FREQ / odiv * loopc / refc;     // 500MHZ
            }

            r0 = READ_REG32(PLL_DDR0_BASE);
            r1 = READ_REG32(PLL_DDR1_BASE);
            loopc = (r0 >> DDR0_DIV_LOOPC_SHIFT) & DDR0_DIV_LOOPC_MASK;
            refc  = (r0 >> DDR0_DIV_REFC_SHIFT)  & DDR0_DIV_REFC_MASK;

            if (r0 & DDR0_PLL_SEL_DDR)
            {
                odiv = (r0 >> DDR0_DIV_SHIFT) & DDR0_DIV_MASK;
                ddr_frequency = OSC_FREQ / odiv * loopc / refc;     // 300MHZ
            }

            if (r0 & DDR0_PLL_SEL_HDA)
            {
                odiv = (r1 >> DDR1_DIV_HDA_SHIFT) & DDR1_DIV_HDA_MASK;
                hda_frequency = OSC_FREQ / odiv * loopc / refc;     // 23.077MHZ
            }

            if (r0 & DDR0_PLL_SEL_NETWORK)
            {
                odiv = r1 & DDR1_DIV_NETWORK_MASK;
                net_frequency = OSC_FREQ / odiv * loopc / refc;     // 300MHZ
            }
 
            r0 = READ_REG32(PLL_SOC0_BASE);
            r1 = READ_REG32(PLL_SOC1_BASE);
            loopc = (r0 >> SOC0_DIV_LOOPC_SHIFT) & SOC0_DIV_LOOPC_MASK;
            refc  = (r0 >> SOC0_DIV_REFC_SHIFT)  & SOC0_DIV_REFC_MASK;
                
            if (r0 & SOC0_PLL_SEL_GPU)
            {
                odiv = (r0 >> SOC0_DIV_GPU_SHIFT) & SOC0_DIV_GPU_MASK;
                gpu_frequency = OSC_FREQ / odiv * loopc / refc;     // 200MHZ
            }

            if (r0 & SOC0_PLL_SEL_GMAC)
            {
                odiv = (r1 >> SOC1_DIV_GMAC_SHIFT) & SOC1_DIV_GMAC_MASK;
                gmac_frequency = OSC_FREQ / odiv * loopc / refc;    // 125MHZ
            }

            if (r0 & SOC0_PLL_SEL_SB)
            {
                odiv = r1 & SOC1_DIV_SB_MASK;
                sb_frequency = OSC_FREQ / odiv * loopc / refc;      // 100MHZ
            }

            r0 = READ_REG32(PLL_PIX0_BASE);
            loopc = (r0 >> PIX0_DIV_LOOPC_SHIFT) & PIX0_DIV_LOOPC_MASK;
            refc  = (r0 >> PIX0_DIV_REFC_SHIFT)  & PIX0_DIV_REFC_MASK;
            
            if (r0 & PIX0_PLL_SEL)
            {
                odiv = (r0 >> PIX0_DIV_SHIFT) & PIX0_DIV_MASK;
                pix0_frequency = OSC_FREQ / odiv * loopc / refc;    // 108.93MHZ
            }

            r1 = READ_REG32(PLL_PIX1_BASE);
            loopc = (r1 >> PIX1_DIV_LOOPC_SHIFT) & PIX1_DIV_LOOPC_MASK;
            refc  = (r1 >> PIX1_DIV_REFC_SHIFT)  & PIX1_DIV_REFC_MASK;

            if (r1 & PIX1_PLL_SEL)
            {
                odiv = (r1 >> PIX1_DIV_SHIFT) & PIX1_DIV_MASK;
                pix1_frequency = OSC_FREQ / odiv * loopc / refc;    // 108.93MHZ
            }

            break;
        }

        case 3:         // 硬件bypass模式, PLL输出时钟全部使用外部输入系统时钟.
        default:
            /*
             * 根据硬件参数设置各个频率
             */
            gmac_frequency = 0;
            break;
    }

    if (gmac_frequency == 0)    // 硬件配置
    {
        return;
    }

    /*
     * 设备时钟分频配置寄存器, 分频计算公式为: fout=fin*(freqscale+1)/8.
     *
     * lsu_freqdiv分频系数除外
     */
    r0 = READ_REG32(PLL_FREQSCALE_BASE);

    /*
     * XXX 这三个输出频率细化?
     */
    sb_frequency    = (unsigned int)((unsigned long)sb_frequency * (((r0 >> FREQSCALE_SB_SHIFT) & FREQSCALE_SB_MASK) + 1) / 8);
    gpu_frequency   = (unsigned int)((unsigned long)gpu_frequency * (((r0 >> FREQSCALE_GPU_SHIFT) & FREQSCALE_GPU_MASK) + 1) / 8);
    cpu_frequency   = (unsigned int)((unsigned long)cpu_frequency * ((r0 & FREQSCALE_NODE_MASK) + 1) / 8);
    
    /*
     * 基准频率 net_frequency
     *
     * lsu   = 9.68MHZ
     * print = 300MHZ
     */
    lsu_frequency   = (unsigned int)((unsigned long)net_frequency / ((r0 >> FREQSCALE_LSU_SHIFT) & FREQSCALE_LSU_MASK));
    print_frequency = (unsigned int)((unsigned long)net_frequency * (((r0 >> FREQSCALE_PRINT_SHIFT) & FREQSCALE_PRINT_MASK) + 1) / 8);
    
    /*
     * 基准频率 sb_frequency
     *
     * apb  = 100MHZ
     * usb  = 100MHZ
     * sata = 100MHZ
     */
    apb_frequency   = (unsigned int)((unsigned long)sb_frequency * (((r0 >> FREQSCALE_APB_SHIFT) & FREQSCALE_APB_MASK) + 1) / 8);
    usb_frequency   = (unsigned int)((unsigned long)sb_frequency * (((r0 >> FREQSCALE_USB_SHIFT) & FREQSCALE_USB_MASK) + 1) / 8);
    sata_frequency  = (unsigned int)((unsigned long)sb_frequency * (((r0 >> FREQSCALE_SATA_SHIFT) & FREQSCALE_SATA_MASK) + 1) / 8);

    return;
}

//-----------------------------------------------------------------------------
// LS2K500 bsp start
//-----------------------------------------------------------------------------

extern int bsp_start_hook1(void);
extern int bsp_start_hook2(void);

void bsp_start(void)
{
	OS_ERR   uErr;
    uint64_t eentry;

    loongarch_interrupt_disable();

    eentry = PHYS_TO_CACHED(__csrrd_d(LA_CSR_MERREBASE));
    memcpy((void *)PHYS_TO_UNCACHED(eentry), (void *)machine_error_entry, 32);

    eentry = __csrrd_d(LA_CSR_EBASE);
    memcpy((void *)PHYS_TO_UNCACHED(eentry), (void *)exception_common_entry, 32);

    eentry = PHYS_TO_CACHED(__csrrd_d(LA_CSR_TLBREBASE));
    memcpy((void *)PHYS_TO_UNCACHED(eentry), (void *)tlbrefill_error_entry, 32);

    asm volatile( "dbar 0; " );
    asm volatile( "ibar 0; " );

    loongarch_init_isr_table();         /* initialize isr table */

    fetch_frequency_setting();

    /**
     * Initialize uC/OS-III
     */
    OSInit(&uErr);

    /**
     * 初始化内存管理
     */
    #if 1
    {
    	extern unsigned char _end;
        extern int heap_add_region(void *addr, size_t size);
        extern void malloc_create_oslock(void);

        size_t heap_start = ((size_t)&_end + 0x10) & ~0x0FULL;
        size_t heap_end   = PHYS_TO_CACHED(get_memory_size() - 8);
        heap_end -= 4*1024*1024;					/* 保留后 4M 内存 */

        if (heap_start + 1*1024*1024 < heap_end)	/* 至少有 1M 空间 */
        {
            heap_add_region((void *)heap_start, heap_end - heap_start);
            malloc_create_oslock();
        }
        else
        {
            for ( ; ; );
        }
    }
    #endif

    bsp_start_hook1();					/* hook1: 实现文件系统初始化等 */

    console_init(115200);               /* initialize console */

    Clock_initialize();                 /* initialize ticker */

    loongarch_interrupt_enable();		/* Enable CPU Interrept */

    bsp_start_hook2();					/* hook2: 实现 EMMC, USB 初始化等 */

    //-------------------------------------------------------------------------
    // goto main function
    //-------------------------------------------------------------------------

    /*
     * 使用汇编跳转, 不再使用 main();
     */
    volatile register unsigned long mainAddr = (unsigned long)main;

    asm volatile( "move $r20, %0 ; " : "=r"(mainAddr) );
    asm volatile( "jirl $r1, $r20, 0 ;" );

    /*
     * NEVER go here!
     */
    while (1)
        ;
}

//-------------------------------------------------------------------------------------------------

/*
 * @@ END
 */


