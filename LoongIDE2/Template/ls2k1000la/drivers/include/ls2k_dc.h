/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_dc.h
 *
 * created: 2022-03-16
 *  author: Bian
 */

#ifndef _LS2K_DC_H
#define _LS2K_DC_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// FrameBuffer devices
//-----------------------------------------------------------------------------

#if BSP_USE_DC
extern const void *devDC0;
#endif

//-----------------------------------------------------------------------------
// ioctl command
//-----------------------------------------------------------------------------

#define IOCTRL_DC_CLEAR_BUFFER	0x46B1
#define IOCTRL_LCD_POWERON		0x46B2
#define IOCTRL_LCD_POWEROFF		0x46B3

//-----------------------------------------------------------------------------
// FrameBuffer driver operators
//-----------------------------------------------------------------------------

#include "ls2k_drv_io.h"

#if (PACK_DRV_OPS)
extern const driver_ops_t *dc_drv_ops;

#define ls2k_dc_init(dc, arg)             dc_drv_ops->init_entry(dc, arg)
#define ls2k_dc_open(dc, arg)             dc_drv_ops->open_entry(dc, arg)
#define ls2k_dc_close(dc, arg)            dc_drv_ops->close_entry(dc, arg)
#define ls2k_dc_read(dc, buf, size, arg)  dc_drv_ops->read_entry(dc, buf, size, arg)
#define ls2k_dc_write(dc, buf, size, arg) dc_drv_ops->write_entry(dc, buf, size, arg)
#define ls2k_dc_ioctl(dc, cmd, arg)       dc_drv_ops->ioctl_entry(dc, cmd, arg)

#else

int DC_initialize(const void *dev, void *arg);
int DC_open(const void *dev, void *arg);
int DC_close(const void *dev, void *arg);
int DC_read(const void *dev, void *buf, int size, void *arg);
int DC_write(const void *dev, void *buf, int size, void *arg);
int DC_ioctl(const void *dev, int cmd, void *arg);

#define ls2k_dc_init(dc, arg)             DC_initialize(dc, arg)
#define ls2k_dc_open(dc, arg)             DC_open(dc, arg)
#define ls2k_dc_close(dc, arg)            DC_close(dc, arg)
#define ls2k_dc_read(dc, buf, size, arg)  DC_read(dc, buf, size, arg)
#define ls2k_dc_write(dc, buf, size, arg) DC_write(dc, buf, size, arg)
#define ls2k_dc_ioctl(dc, cmd, arg)       DC_ioctl(dc, cmd, arg)

#endif

/*
 * user api
 */
int ls2k_dc_initialized(void);          /* return 1 if initialized */
int ls2k_dc_started(void);              /* return 1 if started */

/**
 * for RT-Thread
 */
#ifdef OS_RTTHREAD
#define FB_DEVICE_NAME      "fb0"
#endif

/******************************************************************************
 * LCD resolution
 ******************************************************************************/

/*
 * LCD suported vgamode
 */
#define LCD_480x272		"480x272-16@60"		/* Fit: 4" LCD */
#define LCD_480x800		"480x800-16@60"		/* Fit: 4.3" Vertical */
#define LCD_800x480		"800x480-16@60"		/* Fit: 7" inch LCD */

/*
 * 打开前调用.
 */
extern char LCD_display_mode[];             /* likely LCD_480x272 */

/******************************************************************************
 *
 * Framebuffer Applicaton Interface Functions, in "ls2k_fb_utils.c"
 *
 ******************************************************************************/

extern int fb_open(void);                   /* 初始化并打开framebuffer驱动 */
extern void fb_close(void);                 /* 关闭framebuffer驱动  */

extern int fb_get_pixelsx(void);            /* 返回LCD的X分辨率 */
extern int fb_get_pixelsy(void);            /* 返回LCD的Y分辨率 */

/* 设置颜色索引表
 */
extern void fb_set_color(unsigned coloridx, unsigned value);    /* 设定颜色索引表colidx处的颜色 */
extern unsigned fb_get_color(unsigned coloridx);                /* 读取颜色索引表colidx处的颜色 */

/* 设置字符前后背景色
 */
extern void fb_set_bgcolor(unsigned coloridx, unsigned value);  /* 设置字符输出使用的背景色 */
extern void fb_set_fgcolor(unsigned coloridx, unsigned value);  /* 设置字符输出使用的前景色 */

/* 控制台
 */
extern void fb_cons_putc(char chr);                             /* 在LCD控制台输出一个字符 */
extern void fb_cons_puts(char *str);                            /* 在LCD控制台输出一个字符串 */
extern void fb_cons_clear(void);                                /* 执行LCD控制台清屏 */

/* 文本输出
 */
extern void fb_textout(int x, int y, char *str);                /* 在LCD[x,y]处打印字符串 */

/* 显示 BMP
 */
extern int fb_showbmp(int x, int y, char *bmpfilename);         /* 在LCD[x,y]处显示bmp图像 */

/* LCD 绘图函数
 */
extern void fb_put_cross(int x, int y, unsigned coloridx);                    /* 在LCD[x,y]处画”＋”符号 */
extern void fb_put_string(int x, int y, char *str, unsigned coloridx);        /* 在LCD[x,y]处用指定颜色打印字符串 */
extern void fb_put_string_center(int x, int y, char *str, unsigned coloridx); /* 在LCD上以[x,y]为中心用指定颜色打印字符串 */
extern void fb_drawpixel(int x, int y, unsigned coloridx);                    /* 在LCD[x,y]处用指定颜色画像素 */
extern void fb_drawpoint(int x, int y, int thickness, unsigned coloridx);     /* 在LCD[x,y]处用指定颜色、宽度画点 */
extern void fb_drawline(int x1, int y1, int x2, int y2, unsigned coloridx);   /* 在LCD[x1,y1]～[x2,y2]处用指定颜色画线 */
extern void fb_drawrect(int x1, int y1, int x2, int y2, unsigned coloridx);   /* 在LCD[x1,y1]～[x2,y2]处用指定颜色画矩形框 */
extern void fb_fillrect(int x1, int y1, int x2, int y2, unsigned coloridx);   /* 在LCD[x1,y1]～[x2,y2]处用指定颜色填充矩形框 */
extern void fb_copyrect(int x1, int y1, int x2, int y2, int px, int py);	  /* 把LCD[x1,y1]～[x2,y2]处的图像相对于[x1,y1移动到[px, py]的位置 */

/*
 * shorten name
 */
#define SetColor            fb_set_color
#define GetColor            fb_get_color
#define SetBGColor          fb_set_bgcolor
#define SetFGColor          fb_set_fgcolor

#define TextOut             fb_textout
#define PutString           fb_put_string
#define PutStringCenter     fb_put_string_center

#define DrawPixel           fb_drawpixel
#define DrawPoint           fb_drawpoint
#define DrawLine            fb_drawline
#define DrawRect            fb_drawrect
#define FillRect            fb_fillrect
#define CopyRect            fb_copyrect

/**********************************************************************
 * 字库和显示
 **********************************************************************/

#define XORMODE				0x80000000

/*
 * 颜色表 RGB888, 使用 set_color() 设置
 */
#define clBLACK				0x00

#define clRED				(0xA0 << 16)
#define clGREEN				(0xA0 << 8)
#define clBLUE				(0xA0 << 0)

#define clCYAN				(clBLUE | clGREEN)
#define clVIOLET			(clRED  | clBLUE)
#define clYELLOW			(clRED  | clGREEN)
#define clWHITE				(clRED  | clGREEN | clBLUE)

/* half brightness */
#define clhRED				(0x50 << 16)
#define clhGREEN			(0x50 << 8)
#define clhBLUE				(0x50 << 0)
/* more brightness */
#define clbRED      		(0xF0 << 16)
#define clbGREEN    		(0xF0 << 8)
#define clbBLUE     		(0xF0 << 0)

#define clGREY				(clhRED | clhGREEN | clhBLUE)
#define clBRTBLUE			(clhRED | clhGREEN | clbBLUE)
#define clBRTGREEN			(clhRED | clbGREEN | clhBLUE)
#define clBRTCYAN			(clhRED | clbGREEN | clbBLUE)
#define clBRTRED			(clbRED | clhGREEN | clhBLUE)
#define clBRTVIOLET			(clbRED | clhGREEN | clbBLUE)
#define clBRTYELLOW			(clbRED | clbGREEN | clhBLUE)
#define clBRTWHITE			(clhRED | clhGREEN | clhBLUE)

#define clBTNFACE			0x00808080
#define clSILVER			0x00C0C0C0
#define clHINT				0x00E4F0F0

/*
 * 颜色索引 RGB565, 可通过 get_color() 获取.
 */
#define cidxBLACK			0
#define cidxBLUE			1
#define cidxGREEN			2
#define cidxCYAN			3
#define cidxRED				4
#define cidxVIOLET			5
#define cidxYELLOW			6
#define cidxWHITE			7
#define cidxGREY			8
#define cidxBRTBLUE			9
#define cidxBRTGREEN		10
#define cidxBRTCYAN			11
#define cidxBRTRED			12
#define cidxBRTVIOLET		13
#define cidxBRTYELLOW		14
#define cidxBRTWHITE		15

#define cidxBTNFACE			16
#define cidxSILVER			17

/**********************************************************
 * XXX how to use font, please reference "font_desc.h"
 **********************************************************/

#ifdef __cplusplus
}
#endif

#endif // _LS2K_DC_H

