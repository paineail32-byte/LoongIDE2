/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_dc.c
 *
 * created: 2022-03-16
 *  author: Bian
 */

#include "bsp.h"

#if BSP_USE_DC

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "ls2k300.h"
#include "ls2k_gpio.h"

#include "cpu.h"
#include "fb.h"

#include "ls2k_drv_io.h"
#include "drv_os_priority.h"

#include "ls2k_dc_hw.h"
#include "ls2k_dc.h"

#include "osal.h"

//-----------------------------------------------------------------------------
// 使用双缓冲
//-----------------------------------------------------------------------------

#define DC_DOUBLE_BUFFER    1

//-----------------------------------------------------------------------------
// 使用指定内存地址
//-----------------------------------------------------------------------------

#define FIXED_DC_MEMADDR    0               // !BSP_USE_LWMEM

/******************************************************************************
 * framebuffer device
 ******************************************************************************/

typedef struct 
{
	HW_DC_t *hwDC;                          /* framebuffer control */

	osal_mutex_t p_mutex;                   /* mutex */
	
	struct fb_fix_screeninfo fb_fix;        /* framebuffer standard device */
	struct fb_var_screeninfo fb_var;        /* framebuffer standard device */

#if DC_DOUBLE_BUFFER
    char *fb_1st_buffer;
	char *fb_2nd_buffer;
#endif

	int initialized;				        /* 是否初始化 */
	int started;				            /* 是否启动 */
} DC_softc_t;

/*
 * soft control of Display-Control
 */
static DC_softc_t  ls2k_DC0;

const void *devDC0 = &ls2k_DC0;               // extern variable

//-----------------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------------

#if 1
#define LOCK(p)     osal_mutex_obtain(p->p_mutex, OSAL_WAIT_FOREVER)
#define UNLOCK(p)   osal_mutex_release(p->p_mutex)
#else
#define LOCK(p)
#define UNLOCK(p)
#endif

/******************************************************************************
 * framebuffer display control routine
 ******************************************************************************/

typedef struct 
{
	unsigned int pclk, refresh;
	unsigned int hr, hss, hse, hfl;
	unsigned int vr, vss, vse, vfl;
	unsigned int pan_config;
	unsigned int hvsync_polarity;
} vga_struc_t;

static vga_struc_t vga_modes[] =
{
/****************************************************************************************
 *     pclk        hr   hss   hse   hfl    vr   vss   vse   vfl  pan_config
 *      |  refresh |     |     |     |     |     |     |     |       |   hvsync_polarity
 *      |     |    |     |     |     |     |     |     |     |       |           |
 *      V     V    V     V     V     V     V     V     V     V       V           V
 */
#if 0
	{  6429, 70,  240,  250,  260,  280,  320,  324,  326,  328, 0x00000301, 0x40000000}, /*"240x320_70.00"*/ /* ili9341 DE mode */
	{  7154, 60,  320,  332,  364,  432,  240,  248,  254,  276, 0x00000103, 0xc0000000}, /*"320x240_60.00"*/ /* HX8238-D */
	{ 12908, 60,  320,  360,  364,  432,  480,  488,  490,  498, 0x00000101, 0xc0000000}, /*"320x480_60.00"*/ /* NT35310 */
	{  9009, 60,  480,  488,  489,  531,  272,  276,  282,  288, 0x00000101, 0xc0000000}, /*"480x272_60.00"*/ /* LT043A-02AT */
	{ 20217, 60,  480,  488,  496,  520,  640,  642,  644,  648, 0x00000101, 0xc0000000}, /*"480x640_60.00"*/ /* jbt6k74 */
	{ 25200, 60,  640,  656,  666,  800,  480,  512,  514,  525, 0x00000301, 0xc0000000}, /*"640x480_60.00"*/ /* AT056TN53 */
	{ 33100, 60,  640,  672,  736,  832,  640,  641,  644,  663, 0x00000101, 0xc0000000}, /*"640x640_60.00"*/
	{ 39690, 60,  640,  672,  736,  832,  768,  769,  772,  795, 0x00000101, 0xc0000000}, /*"640x768_60.00"*/
	{ 42130, 60,  640,  680,  744,  848,  800,  801,  804,  828, 0x00000101, 0xc0000000}, /*"640x800_60.00"*/
#endif

    { 32000, 60,  480,  488,  490,  498,  800,  817,  819,  832, 0x00000301, 0xc0000000}, /*"480x800_60.00"*/ /* ST7701s */
//  { 33000, 60,  480,  488,  490,  498,  800,  832,  912, 1024, 0x00000301, 0xc0000000}, /*"480x800_60.00"*/ /* WKS43178 */

//  { 33000, 60,  800,    0,    0,  928,  480,    0,    0,  525, 0x00000101, 0x00000000}, /*"800x480_60.00"*/ /* AT070TN83 V1 */
//  { 29232, 60,  800,    0,    0,  928,  480,    0,    0,  525, 0x00000101, 0x00000000}, /*"800x480_60.00"*/ /* AT070TN92 */

#if 0
    { 49500, 75,  800,  816,  896, 1056,  600,  601,  604,  625, 0x00000101, 0xc0000000}, /*"800x600_75.00"*/
	{ 40730, 60,  800,  832,  912, 1024,  640,  641,  644,  663, 0x00000101, 0xc0000000}, /*"800x640_60.00"*/
	{ 40010, 60,  832,  864,  952, 1072,  600,  601,  604,  622, 0x00000101, 0xc0000000}, /*"832x600_60.00"*/
	{ 40520, 60,  832,  864,  952, 1072,  608,  609,  612,  630, 0x00000101, 0xc0000000}, /*"832x608_60.00"*/
	{ 38170, 60, 1024, 1048, 1152, 1280,  480,  481,  484,  497, 0x00000101, 0xc0000000}, /*"1024x480_60.00"*/
#endif

    /* 飞凌 */

    /*
     * 米联客5寸: 实际使用只有 800*480? PMON 下也是这样
     */

	{ 51200, 60, 1024, 1044, 1184, 1344,  600,  603,  620,  635, 0x00000101, 0x00000000}, /*"1024x600_60.00"*/ 

//	{ 51200, 60, 1024,    0,    0, 1344,  600,    0,    0,  635, 0x00000101, 0x00000000}, /*"1024x600_60.00"*/

#if 0
	{ 52830, 60, 1024, 1072, 1176, 1328,  640,  641,  644,  663, 0x00000101, 0xc0000000}, /*"1024x640_60.00"*/
	{ 65000, 60, 1024, 1048, 1184, 1344,  768,  771,  777,  806, 0x00000101, 0xc0000000}, /*"1024x768_60.00"*/
	{ 71380, 60, 1152, 1208, 1328, 1504,  764,  765,  768,  791, 0x00000101, 0xc0000000}, /*"1152x764_60.00"*/
	{ 83460, 60, 1280, 1344, 1480, 1680,  800,  801,  804,  828, 0x00000101, 0xc0000000}, /*"1280x800_60.00"*/
	{135000, 75, 1280, 1296, 1440, 1688, 1024, 1025, 1028, 1066, 0x00000101, 0xc0000000}, /*"1280x1024_75.00"*/
	{ 85500, 60, 1360, 1424, 1536, 1792,  768,  771,  777,  795, 0x00000101, 0xc0000000}, /*"1360x768_60.00"*/
	{121750, 60, 1440, 1528, 1672, 1904, 1050, 1053, 1057, 1089, 0x00000101, 0xc0000000}, /*"1440x1050_60.00"*/
	{136750, 75, 1440, 1536, 1688, 1936,  900,  903,  909,  942, 0x00000101, 0xc0000000}, /*"1440x900_75.00"*/
	{148500, 60, 1920, 2008, 2052, 2200, 1080, 1084, 1089, 1125, 0x00000101, 0xc0000000}, /*"1920x1080_60.00"*/
#endif
};

static int vgamode_count = sizeof(vga_modes) / sizeof(vga_struc_t);

/*******************************************************************************
 * parse the string display mode
 */

extern char LCD_display_mode[];

static int ls2k_dc_parse_vgamode(char *vgamode,
                                 int  *xres,
                                 int  *yres,
                                 int  *refreshrate,
                                 int  *colordepth)
{
	int   i;
	char *p, *end, *pmode;

	if (NULL == vgamode)
	{
    	return -EINVAL;
    }

	pmode = vgamode;

	/* find first digit charactor */
	for (i=0; i<20; i++)
	{
    	if (isdigit((int)*((pmode+i))))
		{
        	break;
        }
	}

	if (i >= 20)
	{
    	return -EINVAL;
    }

	/* x-y resolution */
	*xres =	strtol(pmode+i, &end, 10);
	*yres = strtol(end+1, NULL, 10);

	if ((*xres<=0 || *xres>2048)||(*yres<=0 || *yres>2048))
	{
    	return -EINVAL;
    }

	/* find the display mode is supported */
	for (i=0; i<vgamode_count; i++)
	{
    	if (vga_modes[i].hr == *xres && vga_modes[i].vr == *yres)
		{
        	break;
        }
    }

	if (i >= vgamode_count)
	{
    	return -ENOTSUP;
    }

	/* refresh rate */
	p = strchr(pmode, '@');
	if (p != NULL)
	{
    	*refreshrate = strtol(p+1, NULL, 0);
    }

	/* color depth */
	p = strchr(pmode, '-');
	if (p != NULL)
	{
    	*colordepth = strtol(p+1, NULL, 0);
    }

    return 0;
}

/*******************************************************************************
 * Display Control wait enable done
 */
static int ls2k_dc_wait_enable(DC_softc_t *dc)
{
	unsigned int val;
	int timeout = 204800;

	val = dc->hwDC->config;
	do
	{
		dc->hwDC->config = val | DC_CFG_OUTPUT_EN;
		val = dc->hwDC->config;
	} while (((val & DC_CFG_OUTPUT_EN) == 0) && (timeout-- > 0));

	if (timeout <= 0)
	{
		printk("Enable framebuffer timeout!\r\n");
		return -1;
	}

	return 0;
}

/*******************************************************************************
 * initialize framebuffer hardware
 */
/*
    1. 将对应的PLL的PD信号设置为1;
    2. 设置寄存器除了sel_pll_*及soft_set_pll之外的其它寄存器, 即这两个寄存器在设置的过程中写为0;
    3. 将对应的PLL的PD信号设置为0;
    4. 其他寄存器值不变, 将soft_set_pll设置为1;
    5. 等待寄存器中的锁定信号locked_*为1;
    6. 设置sel_pll_*为1, 此时对应的时钟频率将切换为软件设置的频率.
    如果后期有需要修改PLL参数, 则要先切换时钟为参考时钟, 然后按上述步骤再配一遍.

*/
#define DC_DELAY_US     200

static int config_pix_pll(unsigned int pix_pll_base,
                          unsigned int div,
                          unsigned int loopc,
                          unsigned int refc)
{

	unsigned int out, tmo=0;

    out = (div << PIX_PLL0_ODIV_SHIFT) |
          (loopc << PIX_PLL0_LOOPC_SHIFT) |
          (refc << PIX_PLL0_REFC_SHIFT);

_again:

    WRITE_REG32(pix_pll_base, PIX_PLL0_PD);
    OR_REG32(pix_pll_base, PIX_PLL0_PD);
	OR_REG32(pix_pll_base, PIX_PLL0_PD);

    OR_REG32(pix_pll_base, out);

    AND_REG32(pix_pll_base, ~PIX_PLL0_PD);
    OR_REG32(pix_pll_base, PIX_PLL0_SOFT_SET);

	while (!(READ_REG32(pix_pll_base) & PIX_PLL0_LOCKED))
    {
        if (tmo++ > 100)
        {
            printk("config pix pll, try again...\r\n");
            goto _again;
        }

        delay_us(1);
    }

    OR_REG32(pix_pll_base, PIX_PLL0_SEL_PIX);
    delay_us(1);
    
    return 0;
}

/**
 * 晶振频率
 */
extern unsigned int osc_frequency;

/**
 * 频率计算
 */
static unsigned int cal_freq(unsigned int pix_hz,
                             unsigned int *p_div,
                             unsigned int *p_loopc,
                             unsigned int *p_refc)
{
	unsigned int odiv, loopc, refc;
	unsigned long bias, bias_min = 1000;

    *p_div = 0;
    *p_loopc = 0;
    *p_refc = 0;
    
    for (refc = 1; refc < 0x80; refc++)
    {
    	unsigned long calc_hz1;

        /*
         * 保证可配分频器的输出 refclk / div_ref 在 20~40MHz 范围内
         */
        calc_hz1 = osc_frequency / refc;
        if ((calc_hz1 < 20000000) || (calc_hz1 > 40000000))
        {
            continue;
        }

        for (loopc = 1; loopc < 0x200; loopc++)
        {
        	unsigned long calc_hz2;

            /*
             * PLL 倍频值 refclk/div_ref*div_loopc 需要在1GHz~3.2GHz
             */
            calc_hz2 = calc_hz1 * loopc;
            if ((calc_hz2 < 1000000000) || (calc_hz2 > 3200000000))
            {
                continue;
            }

            calc_hz2 /= 1000;	/* 硬件内部处理? */

            for (odiv = 1; odiv < 0x80; odiv++)
            {
            	unsigned long calc_hz3;

                calc_hz3 = calc_hz2 / odiv;

                bias = (calc_hz3 > pix_hz) ? (calc_hz3 - pix_hz) : (pix_hz - calc_hz3);

                if (bias < bias_min)
                {
					*p_div   = odiv;
					*p_loopc = loopc;
					*p_refc  = refc;

					bias_min = bias;    // 寻找最优设置
				}
			}
		}
	}
	
	if (*p_div > 0)
	{
        return 0;
	}
	
	printk("calculate dc frequency error!!!\n");
	return -1;
}

static int ls2k_dc_hw_initialize(DC_softc_t *dc)
{
	int i, mode = -1;

    /*
     * PAD 复用
     */

	/*
	 * framebuffer disable output
	 */
	dc->hwDC->config &= ~DC_CFG_OUTPUT_EN;
	dc->hwDC->config &= ~DC_CFG_OUTPUT_EN;
	delay_us(DC_DELAY_US);

	/* find the fit vgamode - whether supported
	 */
	for (i=0; i<vgamode_count; i++)
	{
	    unsigned int div, loopc, refc;
	    
		mode = i;

		if ((vga_modes[i].hr != dc->fb_var.xres) ||
			(vga_modes[i].vr != dc->fb_var.yres))
		{
        	continue;
        }

        if (cal_freq(vga_modes[i].pclk, &div, &loopc, &refc) == 0)
        {
            config_pix_pll(PIX_PLL0_BASE, div, loopc, refc);

		    break;
	    }

	    mode = -1;
	}

	if (mode < 0)
	{
		printk("\r\n\nunsupported framebuffer resolution, choose from bellow:\n");
		for (i=0; i<vgamode_count; i++)
		{
        	printk("%dx%d, ", vga_modes[i].hr, vga_modes[i].vr);
        }
		printk("\r\n");
		return -1;
	}

	/* Frame Buffer Memory Address.
	 */
	dc->hwDC->buf_addr     = VA_TO_PHYS(dc->fb_fix.smem_start);
	dc->hwDC->dbl_buf_addr = VA_TO_PHYS(dc->fb_fix.smem_start);

	/* panel_config
	 */
	dc->hwDC->pan_config = 0x80001111 | vga_modes[mode].pan_config; 

	dc->hwDC->hdisplay = (vga_modes[mode].hfl << DC_HDISP_TOTAL_SHIFT) | vga_modes[mode].hr;

	dc->hwDC->hsync    = (vga_modes[mode].hse << DC_HSYNC_END_SHIFT) |
						  vga_modes[mode].hvsync_polarity | vga_modes[mode].hss;

	dc->hwDC->vdisplay = (vga_modes[mode].vfl << DC_VDISP_TOTAL_SHIFT) | vga_modes[mode].vr;

    dc->hwDC->vsync    = (vga_modes[mode].vse << DC_VSYNC_END_SHIFT) |
						  vga_modes[mode].hvsync_polarity | vga_modes[mode].vss;

	/* set configure register 16bpp
	 */
    switch (dc->fb_var.bits_per_pixel)
    {
        case 32:
            dc->hwDC->config = DC_CFG_RESET | DC_CFG_COLOR_R8G8B8;
            
        case 16:
            dc->hwDC->config = DC_CFG_RESET | DC_CFG_COLOR_R5G6B5;
            break;

        default:
            return -1;
    }
	delay_us(DC_DELAY_US);

	dc->hwDC->stride = (dc->fb_var.xres * 2 + DC_BURST_SIZE) & ~DC_BURST_SIZE;
	dc->hwDC->origin = 0;
	delay_us(10);

	/* wait for enable done.
	 */

	/* flag hardware has initialized
	 */
	dc->initialized = 1;

	return 0;
}

/******************************************************************************
 * start framebuffer
 */
static int ls2k_dc_start(DC_softc_t *dc)
{
	/*
	 * wait for framebuffer enable done
	 */
	if (ls2k_dc_wait_enable(dc) < 0)
	{
    	return -1;
    }

	dc->started = 1;

	return 0;
}

/******************************************************************************
 * stop famebuffer
 */
static void ls2k_dc_stop(DC_softc_t *dc)
{
	dc->hwDC->config &= ~DC_CFG_OUTPUT_EN;
	delay_us(100);

	dc->started = 0;
}

/******************************************************************************
 * clear the memory buffer
 */

static void ls2k_clear_fb_buffer(DC_softc_t *dc, unsigned int color)
{
	size_t *addr, size;
	int i;

	/*
	 * 清除内存 - 8 字节对齐
	 */
	addr = (size_t *)dc->fb_fix.smem_start;
	size = (dc->fb_var.xres*dc->fb_var.yres*((dc->fb_var.bits_per_pixel+7)/8)+3)/(sizeof(size_t));
	for (i=0; i<size; i++)
	{
    	*addr++ = color;
    }
}

/******************************************************************************
 * framebuffer driver routine
 ******************************************************************************/

static unsigned short red16[] =
{
	0x0000, 0x0000, 0x0000, 0x0000, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0x5555, 0x5555, 0x5555, 0x5555, 0xffff, 0xffff, 0xffff, 0xffff
};

static unsigned short green16[] =
{
	0x0000, 0x0000, 0xaaaa, 0xaaaa, 0x0000, 0x0000, 0x5555, 0xaaaa,
	0x5555, 0x5555, 0xffff, 0xffff, 0x5555, 0x5555, 0xffff, 0xffff
};

static unsigned short blue16[] =
{
	0x0000, 0xaaaa, 0x0000, 0xaaaa, 0x0000, 0xaaaa, 0x0000, 0xaaaa,
	0x5555, 0xffff, 0x5555, 0xffff, 0x5555, 0xffff, 0x5555, 0xffff
};

static int get_fix_screen_info(DC_softc_t *dc, struct fb_fix_screeninfo *info)
{
	if (NULL == info)
	{
    	return -1;
    }

	info->smem_start  = dc->fb_fix.smem_start;
	info->smem_len    = dc->fb_fix.smem_len;
	info->type        = dc->fb_fix.type;
	info->visual      = dc->fb_fix.visual;
	info->line_length = dc->fb_fix.line_length;

	return 0;
}

static int get_var_screen_info(DC_softc_t *dc, struct fb_var_screeninfo *info)
{
	if (NULL == info)
	{
    	return -1;
    }

	info->xres             = dc->fb_var.xres;
	info->yres             = dc->fb_var.yres;
	info->bits_per_pixel   = dc->fb_var.bits_per_pixel;

	info->red.offset       = dc->fb_var.red.offset;
	info->red.length       = dc->fb_var.red.length;
	info->red.msb_right    = dc->fb_var.red.msb_right;

	info->green.offset     = dc->fb_var.green.offset;
	info->green.length     = dc->fb_var.green.length;
	info->green.msb_right  = dc->fb_var.green.msb_right;

	info->blue.offset      = dc->fb_var.blue.offset;
	info->blue.length      = dc->fb_var.blue.length;
	info->blue.msb_right   = dc->fb_var.blue.msb_right;

	info->transp.offset    = dc->fb_var.transp.offset;
	info->transp.length    = dc->fb_var.transp.length;
	info->transp.msb_right = dc->fb_var.transp.msb_right;

	return 0;
}

/******************************************************************************
 * framebuffer driver implement
 ******************************************************************************/

extern void *aligned_malloc(size_t size, unsigned int align);
extern int ls2k_dc_init_hook(const void *dev);

STATIC_DRV int DC_initialize(const void *dev, void *arg)
{
	DC_softc_t *pDC = (DC_softc_t *)devDC0;
	int xres, yres, refreshrate=60, colordepth=16;
	unsigned int mem_len;

#if !FIXED_DC_MEMADDR
	char *fb_buffer = NULL;
#endif
#if DC_DOUBLE_BUFFER
	char *fb_dbl_buffer = NULL;
#endif

    if (pDC->initialized)
    {
        return 0;
    }

    ls2k_dc_init_hook(devDC0);

    memset(pDC, 0, sizeof(DC_softc_t));    // clear all

    pDC->p_mutex = osal_mutex_create("DCMutex", OSAL_OPT_FIFO);
    if (pDC->p_mutex == NULL)
    {
        printk("DC create mutex fail!\n");
    	return -1;
    }

    /*
     * XXX use FB0 as framebuffer controller
     */
    pDC->hwDC = (HW_DC_t *)PHYS_TO_UNCACHED(DC_BASE);

	pDC->fb_fix.type   = FB_TYPE_PACKED_PIXELS;
	pDC->fb_fix.visual = FB_VISUAL_TRUECOLOR;

	if (ls2k_dc_parse_vgamode(LCD_display_mode, &xres, &yres, &refreshrate, &colordepth) < 0)
	{
		printk("display control vga mode %s is not supported!\n", LCD_display_mode);
		return -1;
	}

	mem_len = xres * yres * colordepth / 8;
	pDC->fb_var.xres = xres;
	pDC->fb_var.yres = yres;
	pDC->fb_var.bits_per_pixel = colordepth;

	pDC->fb_fix.line_length = xres * colordepth / 8;
	pDC->fb_fix.smem_len = mem_len;

#if FIXED_DC_MEMADDR
	/*
	 * 使用指定内存地址
	 */
	pDC->fb_fix.smem_start = (char *)DC_MEMORY_ADDRESS;

#else
	/*
	 * FIXME alloc framebuffer memory dynamic.
	 */
	fb_buffer = (char *)aligned_malloc(pDC->fb_fix.smem_len, 0x400);
	if (fb_buffer == NULL)
	{
	    printk("framebuffer alloc memory fail!\n");
		return -1;
	}

  #if DC_DOUBLE_BUFFER
  	fb_dbl_buffer = (char *)aligned_malloc(pDC->fb_fix.smem_len, 0x400);

    pDC->fb_1st_buffer = fb_buffer;
    pDC->fb_2nd_buffer = fb_dbl_buffer;
  #endif

	/*
	 * 256 byte aligned
	 */
	pDC->fb_fix.smem_start = fb_buffer;

#endif

	/*
	 * clear the memory buffer
	 */
    memset((void *)pDC->fb_fix.smem_start, 0, pDC->fb_fix.smem_len);

#if DC_DOUBLE_BUFFER
    if (fb_dbl_buffer)
	    memset((void *)fb_dbl_buffer, 0, pDC->fb_fix.smem_len);
#endif

	/*
	 * TODO only 16 color mode of R5G6B5
	 */
    switch (pDC->fb_var.bits_per_pixel)
    {
        case 32:
	        pDC->fb_var.red.length   = 8;
	        pDC->fb_var.red.offset   = 16;
	        pDC->fb_var.green.length = 8;
	        pDC->fb_var.green.offset = 8;
	        pDC->fb_var.blue.length  = 8;
	        pDC->fb_var.blue.offset  = 0;
            break;
            
        case 16:
	        pDC->fb_var.red.length   = 5;
	        pDC->fb_var.red.offset   = 11;
	        pDC->fb_var.green.length = 5;
	        pDC->fb_var.green.offset = 6;
	        pDC->fb_var.blue.length  = 5;
	        pDC->fb_var.blue.offset  = 0;
            break;

        default:
            printk("not supported rgb format, colordepth=%i!\n", colordepth);
            return -1;
    }

	if (ls2k_dc_hw_initialize(pDC) < 0)
	{
		printk("display control initialize fail!\n");
		return -1;
	}

	pDC->initialized = 1;

    DBG_OUT("display control controller initialized.\r\n");

	return 0;
}

STATIC_DRV int DC_open(const void *dev, void *arg)
{
	DC_softc_t *pDC = (DC_softc_t *)devDC0;

	if (!pDC->initialized)
    {
        if (DC_initialize(dev, arg) < 0)
        {
            return -1;
        }
    }

	if (!pDC->started)
    {
    	if (ls2k_dc_start(pDC) < 0)
		{
        	return -1;
        }
    }

	return 0;
}

STATIC_DRV int DC_close(const void *dev, void *arg)
{
	DC_softc_t *pDC = (DC_softc_t *)devDC0;

	ls2k_dc_stop(pDC);

	return 0;
}

/*
 * arg is/as offset
 */
STATIC_DRV int DC_read(const void *dev, void *buf, int size, void *arg)
{
    int rdBytes;
	DC_softc_t *pDC = (DC_softc_t *)devDC0;
	unsigned int offset = (unsigned long)arg;

    if (buf == NULL)
    {
        return -1;
    }

    LOCK(pDC);

	rdBytes = ((offset + size) > pDC->fb_fix.smem_len) ? (pDC->fb_fix.smem_len - offset) : size;

	memcpy(buf, (const void *)(pDC->fb_fix.smem_start + offset), rdBytes);

    UNLOCK(pDC);

	return rdBytes;
}

/*
 * arg is/as offset
 */
STATIC_DRV int DC_write(const void *dev, void *buf, int size, void *arg)
{
    int wrBytes;
	DC_softc_t *pDC = (DC_softc_t *)devDC0;
	unsigned int offset = (unsigned long)arg;

    if (buf == NULL)
    {
        return -1;
    }

    LOCK(pDC);

	wrBytes = ((offset + size) > pDC->fb_fix.smem_len) ? (pDC->fb_fix.smem_len - offset) : size;

	memcpy((void *)(pDC->fb_fix.smem_start + offset), buf, wrBytes);

    UNLOCK(pDC);

	return wrBytes;
}

static int get_palette(struct fb_cmap *cmap)
{
	unsigned int i;

	if (cmap->start + cmap->len >= 16)
	{
    	return 1;
    }

	for (i = 0; i < cmap->len; i++)
	{
		cmap->red[cmap->start + i]   = red16[cmap->start + i];
		cmap->green[cmap->start + i] = green16[cmap->start + i];
		cmap->blue[cmap->start + i]  = blue16[cmap->start + i];
	}

	return 0;
}

static int set_palette(struct fb_cmap *cmap)
{
	unsigned int i;

	if (cmap->start + cmap->len >= 16)
	{
    	return 1;
    }

	for (i = 0; i < cmap->len; i++)
	{
		red16[cmap->start + i]   = cmap->red[cmap->start + i];
		green16[cmap->start + i] = cmap->green[cmap->start + i];
		blue16[cmap->start + i]  = cmap->blue[cmap->start + i];
	}

	return 0;
}

STATIC_DRV int DC_ioctl(const void *dev, int cmd, void *arg)
{
    int rt = 0;
	DC_softc_t *pDC = (DC_softc_t *)devDC0;

	switch (cmd)
    {
		case FBIOGET_FSCREENINFO:
		    rt = get_fix_screen_info(pDC, (struct fb_fix_screeninfo *)arg);
			break;

		case FBIOGET_VSCREENINFO:
			rt = get_var_screen_info(pDC, (struct fb_var_screeninfo *)arg);
			break;

		case FBIOPUT_VSCREENINFO:    /* not implemented yet */
			rt = -1;
		    break;

		case FBIOGETCMAP:
			rt = get_palette((struct fb_cmap *)arg);
			break;

		case FBIOPUTCMAP:
			rt = set_palette((struct fb_cmap *)arg);
			break;

		case IOCTRL_DC_CLEAR_BUFFER:
		{
			unsigned int clr = (unsigned long)arg;
			clr = (clr << 16) | clr;
			ls2k_clear_fb_buffer(pDC, clr);
			break;
		}

		case IOCTRL_LCD_POWERON:
			break;

		case IOCTRL_LCD_POWEROFF:
			break;

		default:
			break;
    }

	return rt;
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * diaplay control driver operators
 */
static const driver_ops_t ls2k_dc_drv_ops =
{
    .init_entry  = DC_initialize,
    .open_entry  = DC_open,
    .close_entry = DC_close,
    .read_entry  = DC_read,
    .write_entry = DC_write,
    .ioctl_entry = DC_ioctl,
};

const driver_ops_t *dc_drv_ops = &ls2k_dc_drv_ops;
#endif

//-----------------------------------------------------------------------------
// 是否初始化
//-----------------------------------------------------------------------------

int ls2k_dc_initialized(void)
{
	return ls2k_DC0.initialized;
}

//-----------------------------------------------------------------------------
// 是否启动
//-----------------------------------------------------------------------------

int ls2k_dc_started(void)
{
	return ls2k_DC0.started;
}

#endif // #ifdef BSP_USE_FB

/*
 * @@ END
 */
