/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_fb_utils.c
 *
 *  Created on: 2014-11-10
 *      Author: Bian
 */

#include "bsp.h"

#if BSP_USE_DC

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <larchintrin.h>

#if defined(LS2K300)
#include "ls2k300.h"
#elif defined(LS2K500)
#include "ls2k500.h"
#elif defined(LS2K1000LA)
#include "ls2k1000.h"
#else
#error "No Loongson SoC defined."
#endif

#include "cpu.h"
#include "fb.h"

#include "ls2k_dc.h"
#include "font/font_desc.h"

/******************************************************************************
 * Defined Color, already RGB565
 */
#define _BLUE      		(0x14 << 0)
#define _GREEN      	(0x14 << 6)
#define _RED        	(0x14 << 11)

#define _BLACK    		(0)
#define _WHITE    		(_RED | _GREEN | _BLUE)

#define _HALF_BLUE    	(0x0A << 0)
#define _HALF_GREEN    	(0x0A << 6)
#define _HALF_RED    	(0x0A << 11)

#define _BRT_BLUE     	(0x1E << 0)
#define _BRT_GREEN    	(0x1E << 6)
#define _BRT_RED      	(0x1E << 11)

#define LU_BLACK    	(0)
#define LU_BLUE      	(_BLUE)
#define LU_GREEN    	(_GREEN)
#define LU_CYAN      	(_GREEN | _BLUE)
#define LU_RED      	(_RED)
#define LU_VIOLET    	(_RED | _BLUE)
#define LU_YELLOW    	(_RED | _GREEN)
#define LU_WHITE    	(_RED | _GREEN | _BLUE)
#define LU_GREY      	(_HALF_RED | _HALF_GREEN | _HALF_BLUE)
#define LU_BRT_BLUE		(_HALF_RED | _HALF_GREEN | _BRT_BLUE)
#define LU_BRT_GREEN  	(_HALF_RED | _BRT_GREEN | _HALF_BLUE)
#define LU_BRT_CYAN		(_HALF_RED | _BRT_GREEN | _BRT_BLUE)
#define LU_BRT_RED    	(_BRT_RED | _HALF_GREEN | _HALF_BLUE)
#define LU_BRT_VIOLET	(_BRT_RED | _HALF_GREEN | _BRT_BLUE)
#define LU_BRT_YELLOW	(_BRT_RED | _BRT_GREEN | _HALF_BLUE)
#define LU_BRT_WHITE  	(_BRT_RED | _BRT_GREEN | _BRT_BLUE)

#define LU_BTNFACE		((0x10 << 0) | (0x10 << 6) | (0x10 << 11))
#define LU_SILVER		((0x18 << 0) | (0x18 << 6) | (0x18 << 11))

/******************************************************************************
 * 颜色表
 */
static unsigned short m_color_map[256] =
{
	LU_BLACK,        	/* 0 */
	LU_BLUE,        	/* 1 */
	LU_GREEN,        	/* 2 */
	LU_CYAN,        	/* 3 */
	LU_RED,          	/* 4 */
	LU_VIOLET,       	/* 5 */
	LU_YELLOW,        	/* 6 */
	LU_WHITE,        	/* 7 */
	LU_GREY,        	/* 8 */
	LU_BRT_BLUE,      	/* 9 */
	LU_BRT_GREEN,      	/* 10 */
	LU_BRT_CYAN,      	/* 11 */
	LU_BRT_RED,        	/* 12 */
	LU_BRT_VIOLET,      /* 13 */
	LU_BRT_YELLOW,      /* 14 */
	LU_BRT_WHITE,      	/* 15 */
	LU_BTNFACE,	      	/* 16 */
	LU_SILVER,      	/* 17 */
};

/******************************************************************************
 * Console 参数
 */
#define COL_GAP				0		/* 列字符间距 */
#define ROW_GAP				0		/* 行字符间距 */

#define CONS_FONT_WIDTH		8
#define CONS_FONT_HEIGHT	16

#define LEFT_MARGIN			1
#define TOP_MARGIN			3
#define RIGHT_MARGIN		1
#define BOTTOM_MARGIN		2

/******************************************************************************
 * Local useful type & variables
 */

typedef struct
{
	void  *dc;                              /* display control device */

	struct fb_fix_screeninfo fixInfo;       // dc->fb_fix
	struct fb_var_screeninfo varInfo;       // dc->fb_var

	int    bytes_per_pixel;                 /* 每像素字节数 */

	int    fg_coloridx;                     /* 前景色 WHITE */
	int    bg_coloridx;                     /* 背景色 BLACK */

	unsigned int Rows;                      /* 屏幕总行数 */
	unsigned int Cols;                      /* 屏幕总列数 */

	unsigned int curCol;                    /* 当前列, 0 to fb->Cols - 1 */
	unsigned int curRow;                    /* 当前行, 0 to fb->Rows - 1 */

	unsigned char **lineAddr;               /* 屏幕行首内存地址 */

} FB_SOFTC_t;

static FB_SOFTC_t ls2k_fb, *fb = &ls2k_fb;

/******************************************************************************
 * FrameBuffer Variables Initialize
 ******************************************************************************/

/*
 * macro for r/w buffer
 */
#define IN_BOUND(x, y)	    (((x)>=0) && ((x)<fb->varInfo.xres) && ((y)>=0) && ((y)<fb->varInfo.yres))

#define FB_ADDR_OFF(x, y)	(((x)+(y)*fb->varInfo.xres)*fb->bytes_per_pixel)

#define RD_FB16(offset)     (*((volatile unsigned short *)(fb->fixInfo.smem_start+(offset))))
#define WR_FB16(offset, v)  (*((volatile unsigned short *)(fb->fixInfo.smem_start+(offset)))=(v))

#define WR_FB32(offset, v)  (*((volatile unsigned int *)(fb->fixInfo.smem_start+(offset)))=(v))
#define WR_FB64(offset, v)  (*((volatile unsigned long *)(fb->fixInfo.smem_start+(offset)))=(v))

int fb_get_pixelsx(void)
{
	if (fb->dc != NULL)
	{
    	return fb->varInfo.xres;
    }

	return -1;
}

int fb_get_pixelsy(void)
{
	if (fb->dc != NULL)
	{
    	return fb->varInfo.yres;
    }

	return -1;
}

/******************************************************************************
 * FrameBuffer Draw Text
 ******************************************************************************/

/*
 * 显示一个字符
 */
void fb_draw_ascii_char(int x, int y, unsigned char *chr)
{
	unsigned int   mem_off;
	unsigned char *font_buf;
	int		       buflen;

	if (chr == NULL)
	{
    	return;
    }

	font_desc_t *font = get_font_desc((const unsigned char *)chr, false);
	if (font == NULL)
	{
    	return;
    }

	font_buf = (unsigned char *)font->get_font_data((const unsigned char *)chr, &buflen);
	if (font_buf == NULL)
	{
    	return;
    }

	unsigned int xormode  = fb->fg_coloridx &  XORMODE;
	unsigned int coloridx = fb->fg_coloridx & ~XORMODE;

	if (IN_BOUND(x, y) && IN_BOUND(x+font->width-1, y+font->height-1))
	{
		unsigned short wr16, rd16;
		int font_row, font_col;

		for (font_row = 0; font_row < font->height; font_row++)
		{
			mem_off = FB_ADDR_OFF(x, (y+font_row));
			unsigned int offset =  mem_off;
			unsigned char data  = *font_buf;

			for (font_col = 0; font_col < font->width; font_col++)
			{
				if (data & 0x80)
				{
					if (xormode)
					{
						rd16 = RD_FB16(offset);
						wr16 = m_color_map[coloridx] ^ rd16;
					}
					else
					{
                    	wr16 = m_color_map[coloridx];
                    }
					WR_FB16(offset, wr16);
				}

				offset += 2;
				data <<= 1;
			}

			font_buf++;
		}
	}
}

/*
 * 输出显示一个汉字
 */
void fb_draw_gb2312_char(int x, int y, unsigned char *str)
{
	unsigned int    mem_off;
	unsigned char  *temp_buf;
	unsigned short *font_buf;
	int             buflen;

	if (str == NULL)
	{
    	return;
    }

	font_desc_t *font = get_font_desc((const unsigned char *)str, false);
	if (font == NULL)
	{
    	return;
    }

	temp_buf = (unsigned char *)font->get_font_data((const unsigned char *)str, &buflen);
	if (temp_buf == NULL)
	{
    	return;
    }

	unsigned int xormode = fb->fg_coloridx &  XORMODE;
	unsigned int coloridx  = fb->fg_coloridx & ~XORMODE;

	font_buf = (unsigned short *)temp_buf;

	if (IN_BOUND(x, y) && IN_BOUND(x+font->width-1, y+font->height-1))
	{
		unsigned short wr16, rd16;
		int font_row, font_col;

		for (font_row = 0; font_row < font->height; font_row++)
		{
			mem_off             = FB_ADDR_OFF(x, (y+font_row));
			unsigned int offset = mem_off;
			unsigned short data = (*(unsigned char *)font_buf << 8) |
                                  (*((unsigned char *)font_buf + 1));

			for (font_col = 0; font_col < font->width; font_col++)
			{
				if (data & 0x8000)
				{
					if (xormode)
					{
						rd16 = RD_FB16(offset);
						wr16 = m_color_map[coloridx] ^ rd16;
					}
					else
					{
                    	wr16 = m_color_map[coloridx];
                    }
					WR_FB16(offset, wr16);
				}

				offset += 2;
				data <<= 1;
			}

			font_buf++;
		}
	}
}

/*
 * 输出文本
 */
void fb_textout(int x, int y, char *str)
{
	char *pch = str;
	int   dx = 0;

	if (fb->dc == NULL)
	{
		return;
	}

	while (*pch)
	{
		font_desc_t *ft_desc = get_font_desc((const unsigned char *)pch, false);

		if (ft_desc != NULL)
		{
			ft_desc->draw_font(x + dx, y, (const unsigned char *)pch);

			dx  += ft_desc->width + COL_GAP;
			pch += ft_desc->get_char_size(NULL);
		}
		else
		{
			dx  += CONS_FONT_WIDTH + COL_GAP;
			pch += 1;
		}
	}
}

/******************************************************************************
 * FrameBuffer As Console Output
 ******************************************************************************/

#define FB_ADDR_ASC(r, c) ((((r)*fb->varInfo.xres*(CONS_FONT_HEIGHT+ROW_GAP))+ \
						   ((c)*(CONS_FONT_WIDTH+COL_GAP)))*fb->bytes_per_pixel)

/*
 * clear a row of framebuffer
 */
static void fb_cons_clear_row(int row)
{
	unsigned long mem_off, wr64;
	unsigned short wr16;
	int i, count;

	/* clear the desired row */
	mem_off = FB_ADDR_ASC(row, 0);
	wr16 = m_color_map[fb->bg_coloridx];

#ifdef __loongarch64
    wr64  = ((unsigned long)wr16 << 48) |
            ((unsigned long)wr16 << 32) |
            ((unsigned long)wr16 << 16) |
             (unsigned long)wr16;
    count = ((fb->varInfo.xres * (CONS_FONT_HEIGHT+ROW_GAP)) * 2) / sizeof(unsigned long);

	for (i = 0; i < count; i++)
	{
		WR_FB64(mem_off, wr64);
		mem_off += sizeof(unsigned long);
	}

#else
    count = ((fb->varInfo.xres * (CONS_FONT_HEIGHT+ROW_GAP)) * 2) / sizeof(unsigned short);

	for (i = 0; i < count; i++)
	{
		WR_FB16(mem_off, wr16);
		mem_off += sizeof(unsigned short);
	}
#endif
}

/*
 * 滚屏: 直接搬移frame buffer数据
 */
static void fb_cons_scroll(void)
{
	/*
	 * XXX: 必须确认地址是 8 字节对齐的!
	 */
	unsigned int count;
	size_t *pdst, *psrc;

	fb->curRow++;

	if (fb->curRow > (fb->Rows - 1))
	{
		pdst  = (unsigned long *)fb->fixInfo.smem_start;
		psrc  = (unsigned long *)(fb->fixInfo.smem_start + FB_ADDR_ASC(1, 0));
		count = (unsigned int)(fb->varInfo.xres*(fb->varInfo.yres-(CONS_FONT_HEIGHT+ROW_GAP))*2/sizeof(size_t));

		for (int i=0; i<count; i++)
		{
			*pdst++ = *psrc++;
		}

		/* 行号不变 */
		fb->curCol = 0;
		fb->curRow--;
		fb_cons_clear_row(fb->curRow);
	}
}

/*
 * console 输出一个字符
 */
void fb_cons_putc(char chr)
{
	int x, y;
	font_desc_t *ft_desc;

	/* First parse the character to see if it printable or an acceptable
	 * control character. */
	switch (chr)
	{
		case '\r':
			fb->curCol = 0;
			return;

		case '\n':
			fb->curCol = 0;
			fb_cons_scroll();
			return;

		case '\b':
			fb->curCol--;
			if (fb->curCol < 0)
			{
				fb->curRow--;
				if (fb->curRow < 0)
				{
                	fb->curRow = 0;
                }
				fb->curCol = fb->Cols- 1;
			}

			/* erase the character, write a SPACE */
			chr = 20;
			x = fb->curCol*(CONS_FONT_WIDTH+COL_GAP);
			y = fb->curRow*(CONS_FONT_HEIGHT+ROW_GAP);

			ft_desc = get_font_desc((const unsigned char *)&chr, false);
			if (ft_desc != NULL)
			{
				ft_desc->draw_font(x, y, (const unsigned char *)&chr);
			}

			break;

		default:
			/* drop anything we can't print */
			ft_desc = get_font_desc((const unsigned char *)&chr, false);
			if (ft_desc == NULL)
			{
            	return;
            }

			/* get aligned to the first printable character */
			x = fb->curCol*(CONS_FONT_WIDTH+COL_GAP);
			y = fb->curRow*(CONS_FONT_HEIGHT+ROW_GAP);
			ft_desc->draw_font(x, y, (const unsigned char *)&chr);

			/* advance to next column */
			fb->curCol++;
			if (fb->curCol == fb->Cols)
			{
				fb->curCol = 0;
				fb_cons_scroll();
			}

			break;
	}
}

/*
 * 输出一个字符串
 */
void fb_cons_puts(char *str)
{
	int   x, y;
	char *pch = str;

	if (fb->dc == NULL)
	{
    	return;
    }

	while (*pch)
	{
		if (!(*pch & 0x80))
		{
			fb_cons_putc(*pch++);
		}

		/**************************************************
		 * gb2312 charset display.
		 */
		else
		{
			font_desc_t *ft_desc = get_font_desc((const unsigned char *)pch, false);
			if (ft_desc != NULL)
			{
				if ((fb->Cols - fb->curCol) < 2)
				{
					fb->curCol = 0;
					fb_cons_scroll();
				}

				/* 显示一个汉字 */
				x = fb->curCol*(CONS_FONT_WIDTH+COL_GAP);
				y = fb->curRow*(CONS_FONT_HEIGHT+ROW_GAP);
				ft_desc->draw_font(x, y, (const unsigned char *)pch);

				fb->curCol += 2;
				if (fb->curCol == fb->Cols)
				{
					fb->curCol = 0;
					fb_cons_scroll();
				}
			}
			else
			{
				fb_cons_putc('?');
				fb_cons_putc('?');
			}

			pch += 2;
		}
	}
}

/*
 * 清除屏幕
 */
void fb_cons_clear(void)
{
	int i, fbsize;
	unsigned short wr16;
	unsigned long wr64;

	fbsize = fb->varInfo.xres*fb->varInfo.yres*fb->bytes_per_pixel;
	wr16 = m_color_map[fb->bg_coloridx];

#ifdef __loongarch64
    wr64  = ((unsigned long)wr16 << 48) |
            ((unsigned long)wr16 << 32) |
            ((unsigned long)wr16 << 16) |
             (unsigned long)wr16;

	for (i = 0; i < fbsize; i += sizeof(unsigned long))
	{
    	WR_FB64(i, wr64);
    }

#else
	for (i = 0; i < fbsize; i += sizeof(unsigned short))
	{
    	WR_FB16(i, wr16);
    }

#endif
}

/******************************************************************************
 * Frame Simple Graphics Functions
 ******************************************************************************/

union multiptr
{
	unsigned char  *p8;
	unsigned short *p16;
	unsigned int   *p32;
};

int fb_is_opened(void)
{
    return (fb->dc != NULL) ? 1 : 0;
}

/*
 * open the framebuffer
 */
int fb_open(void)
{
	unsigned y, off;

	/* already open? */
	if (fb->dc != NULL)
	{
    	return 0;
    }

	/* not Initialized? */
	if (ls2k_dc_init(devDC0, NULL) != 0)
    {
        return -1;
    }

    if (ls2k_dc_open(devDC0, NULL) != 0)
    {
        return -1;
    }

    fb->dc = (void *)devDC0;

    ls2k_dc_ioctl(devDC0, FBIOGET_FSCREENINFO, &fb->fixInfo);
    ls2k_dc_ioctl(devDC0, FBIOGET_VSCREENINFO, &fb->varInfo);

	fb->fg_coloridx = 15;
	fb->bytes_per_pixel = (int)(fb->varInfo.bits_per_pixel + 7) / 8;
	fb->Rows = (CONS_FONT_HEIGHT>0) ? (fb->varInfo.yres/(CONS_FONT_HEIGHT+ROW_GAP)) : 8;
	fb->Cols = (CONS_FONT_WIDTH>0)  ? (fb->varInfo.xres/(CONS_FONT_WIDTH+COL_GAP))  : 8;

	fb->lineAddr = malloc(sizeof(uintptr_t) * fb->varInfo.yres);

	if (fb->lineAddr == NULL)
	{
		fb_close();
		return -1;
	}

	off = 0;
	for (y = 0; y < fb->varInfo.yres; y++, off += fb->fixInfo.line_length)
	{
    	fb->lineAddr[y] = (void *)fb->fixInfo.smem_start + off;
    }

    DBG_OUT("framebuffer open successful.\r\n");

	return 0;
}

void fb_close(void)
{
	if (fb->dc != NULL)
	{
		ls2k_dc_close(fb->dc, NULL);

        free(fb->lineAddr);

		fb->dc = NULL;
	}
}

/*
 * draw on framebuffer
 */
void fb_put_cross(int x, int y, unsigned coloridx)
{
	fb_drawline(x - 10, y, x - 2, y, coloridx);
	fb_drawline(x + 2, y, x + 10, y, coloridx);
	fb_drawline(x, y - 10, x, y - 2, coloridx);
	fb_drawline(x, y + 2, x, y + 10, coloridx);
	fb_drawline(x - 6, y - 9, x - 9, y - 9, coloridx + 1);
	fb_drawline(x - 9, y - 8, x - 9, y - 6, coloridx + 1);
	fb_drawline(x - 9, y + 6, x - 9, y + 9, coloridx + 1);
	fb_drawline(x - 8, y + 9, x - 6, y + 9, coloridx + 1);
	fb_drawline(x + 6, y + 9, x + 9, y + 9, coloridx + 1);
	fb_drawline(x + 9, y + 8, x + 9, y + 6, coloridx + 1);
	fb_drawline(x + 9, y - 6, x + 9, y - 9, coloridx + 1);
	fb_drawline(x + 8, y - 9, x + 6, y - 9, coloridx + 1);
}

void fb_put_string(int x, int y, char *str, unsigned coloridx)
{
	unsigned int saved_coloridx = fb->fg_coloridx;
	fb->fg_coloridx = coloridx;
	fb_textout(x, y, str);
	fb->fg_coloridx = saved_coloridx;
}

void fb_put_string_center(int x, int y, char *str, unsigned coloridx)
{
	x -= strlen(str) * CONS_FONT_WIDTH  / 2;
	y -= CONS_FONT_HEIGHT / 2;
	fb_put_string(x, y, str, coloridx);
}

void fb_set_color(unsigned coloridx, unsigned value)
{
	unsigned int res;
	unsigned short red, green, blue;
	struct fb_cmap cmap;

	switch (fb->bytes_per_pixel)
	{
		default:
		case 1:
			res   = coloridx;
			red   = (value >> 8) & 0xFF00;
			green = value & 0xFF00;
			blue  = (value << 8) & 0xFF00;
			cmap.start  = coloridx;
			cmap.len    = 1;
			cmap.red    = &red;
			cmap.green  = &green;
			cmap.blue   = &blue;
			cmap.transp = NULL;
			ls2k_dc_ioctl(fb->dc, FBIOPUTCMAP, &cmap);
			break;

		case 2:
		case 3:
		case 4:
			red   = (value >> 16) & 0xFF;
			green = (value >>  8) & 0xFF;
			blue  = value & 0xFF;
			res   = ((red   >> (8 - fb->varInfo.red.length))   << fb->varInfo.red.offset)   |
				    ((green >> (8 - fb->varInfo.green.length)) << fb->varInfo.green.offset) |
				    ((blue  >> (8 - fb->varInfo.blue.length))  << fb->varInfo.blue.offset);
			break;
	}

	m_color_map[coloridx] = res;
}

unsigned fb_get_color(unsigned coloridx)
{
	if (coloridx < 255)
	{
    	return m_color_map[coloridx];
    }

    return 0;
}

void fb_set_bgcolor(unsigned coloridx, unsigned value)
{
	fb->bg_coloridx = coloridx;
	fb_set_color(coloridx & ~XORMODE, value);
}

void fb_set_fgcolor(unsigned coloridx, unsigned value)
{
	fb->fg_coloridx = coloridx;
	fb_set_color(coloridx & ~XORMODE, value);
}

static void fb_set_pixel_internal(union multiptr loc, unsigned xormode, unsigned color)
{
	switch (fb->bytes_per_pixel)
	{
		case 1:
		default:
			if (xormode)
            	*loc.p8 ^= color;
			else
            	*loc.p8  = color;
			break;

		case 2:
			if (xormode)
            	*loc.p16 ^= color;
			else
            	*loc.p16  = color;
			break;

		case 3:
			if (xormode)
			{
				*loc.p8++ ^= (color >> 16) & 0xFF;
				*loc.p8++ ^= (color >>  8) & 0xFF;
				*loc.p8   ^= color & 0xFF;
			}
			else
			{
				*loc.p8++ = (color >> 16) & 0xFF;
				*loc.p8++ = (color >>  8) & 0xFF;
				*loc.p8   = color & 0xFF;
			}
			break;

		case 4:
			if (xormode)
            	*loc.p32 ^= color;
			else
            	*loc.p32  = color;
			break;
	}
}

void fb_drawpixel(int x, int y, unsigned coloridx)
{
	unsigned xormode = 0;
	union multiptr loc;

	if ((x < 0) || ((unsigned int)x >= fb->varInfo.xres) ||
	    (y < 0) || ((unsigned int)y >= fb->varInfo.yres))
	{
    	return;
    }

	xormode = coloridx & XORMODE;
	coloridx &= ~XORMODE;

	loc.p8 = fb->lineAddr[y] + x * fb->bytes_per_pixel;
	fb_set_pixel_internal(loc, xormode, m_color_map[coloridx]);
}

void fb_drawpoint(int x, int y, int thickness, unsigned coloridx)
{
	if (thickness == 1)
	{
		fb_drawpixel(x, y, coloridx);
	}
	else
	{
		fb_drawpixel(x, y, coloridx);
		fb_drawpixel(x, y - 1, coloridx);
		fb_drawpixel(x, y + 1, coloridx);
		fb_drawpixel(x - 1, y, coloridx);
		fb_drawpixel(x + 1, y, coloridx);
	}
}

void fb_drawline(int x1, int y1, int x2, int y2, unsigned coloridx)
{
	int tmp;
	int dx = x2 - x1;
	int dy = y2 - y1;

	if (abs(dx) < abs(dy))
	{
		if (y1 > y2)
		{
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		x1 <<= 16;

		/* dy is apriori >0 */
		dx = (dx << 16) / dy;
		while (y1 <= y2)
		{
			fb_drawpixel(x1 >> 16, y1, coloridx);
			x1 += dx;
			y1++;
		}
	}
	else
	{
		if (x1 > x2)
		{
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		y1 <<= 16;

		dy = dx ? (dy << 16) / dx : 0;
		while (x1 <= x2)
		{
			fb_drawpixel(x1, y1 >> 16, coloridx);
			y1 += dy;
			x1++;
		}
	}
}

void fb_drawrect(int x1, int y1, int x2, int y2, unsigned coloridx)
{
	fb_drawline(x1, y1, x2, y1, coloridx);
	fb_drawline(x2, y1, x2, y2, coloridx);
	fb_drawline(x2, y2, x1, y2, coloridx);
	fb_drawline(x1, y2, x1, y1, coloridx);
}

void fb_fillrect(int x1, int y1, int x2, int y2, unsigned coloridx)
{
	int tmp;
	unsigned xormode;
	union multiptr loc;

	/* Clipping and sanity checking */
	if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
	if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }
	if (x1 < 0) { x1 = 0; } if ((unsigned int)x1 >= fb->varInfo.xres) { x1 = fb->varInfo.xres - 1; }
	if (x2 < 0) { x2 = 0; } if ((unsigned int)x2 >= fb->varInfo.xres) { x2 = fb->varInfo.xres - 1; }
	if (y1 < 0) { y1 = 0; } if ((unsigned int)y1 >= fb->varInfo.yres) { y1 = fb->varInfo.yres - 1; }
	if (y2 < 0) { y2 = 0; } if ((unsigned int)y2 >= fb->varInfo.yres) { y2 = fb->varInfo.yres - 1; }

	if ((x1 > x2) || (y1 > y2))
	{
    	return;
    }

	xormode = coloridx & XORMODE;
	coloridx &= ~XORMODE;

	coloridx = m_color_map[coloridx];

	for ( ; y1 <= y2; y1++)
	{
		loc.p8 = fb->lineAddr[y1] + x1 * fb->bytes_per_pixel;
		for (tmp = x1; tmp <= x2; tmp++)
		{
			fb_set_pixel_internal(loc, xormode, coloridx);
			loc.p8 += fb->bytes_per_pixel;
		}
	}
}

/*
 * copy rectangle to point, destination is out of source
 */
static void fb_copyrect_internal(int x1, int y1, int x2, int y2, int px, int py)
{
	unsigned int src_off, dst_off;
	unsigned short color;
	int x, y, dx, dy;

	dx = px - x1;
	dy = py - y1;

	for (y=y1; y<=y2; y++)
	{
		if ((y < 0) || (y >= fb->varInfo.yres) || (y + dy < 0) || (y + dy >= fb->varInfo.yres))
		{
        	continue;
        }

		for (x=x1; x<=x2; x++)
		{
			if ((x < 0) || (x >= fb->varInfo.xres) || (x + dx < 0) || (x + dx >= fb->varInfo.xres))
			{
            	continue;
            }

			src_off = FB_ADDR_OFF(x, y);
			color   = RD_FB16(src_off);
			dst_off = FB_ADDR_OFF(x + dx, y + dy);
			WR_FB16(dst_off, color);
		}
	}
}

/*
 * copy rectangle to point
 */
void fb_copyrect(int x1, int y1, int x2, int y2, int px, int py)
{
    int _x2 = px + x2 - x1;
    int _y2 = py + y2 - y1;

    /* if same, do nothing */
    if ((px == x1) && (py == y1))
    {
        return;
    }

    /* -> vertical & down */
    else if ((px == x1) && (py > y1) && (py <= y2))
    {
        fb_copyrect_internal(x1, py, x2, y2,     px, py * 2 - y1);
		fb_copyrect_internal(x1, y1, x2, py - 1, px, py);
    }

    /* -> vertical & up */
    else if ((px == x1) && (py < y1) && (_y2 >= y1))
    {
        fb_copyrect_internal(x1, y1, x2, _y2,     px, py);
		fb_copyrect_internal(x1, _y2 - 1, x2, y2, px, py - y1 + _y2 - 1);
    }

    /* -> horizontal & right */
    else if ((py == y1) && (px > x1) && (px <= x2))
    {
        fb_copyrect_internal(px, y1, x2, y2,     px * 2 - x1, py);
		fb_copyrect_internal(x1, y1, px - 1, y2, px, py);
    }

    /* -> horizontal & left */
    else if ((py == y1) && (px < x1) && (_x2 >= x1))
    {
        fb_copyrect_internal(x1, y1, _x2, y2,     px, py);
		fb_copyrect_internal(_x2 + 1, y1, x2, y2, px - x1 + _x2 + 1, py);
    }

    /* -> right & down */
	else if (((px > x1 ) && (px <= x2)) && ((py > y1) && (py <= y2)))
    {
		fb_copyrect_internal(px, py, x2, y2,     px * 2 - x1, py * 2 - y1);
		fb_copyrect_internal(x1, py, px - 1, y2, px, py * 2 - y1);
		fb_copyrect_internal(x1, y1, x2, py - 1, px, py);
    }

    /* -> right & up */
	else if (((px > x1 ) && (px <= x2)) && ((_y2 >= y1) && (_y2 < y2)))
    {
		fb_copyrect_internal(px, y1, x2, _y2,     px * 2 - x1, py);
		fb_copyrect_internal(x1, y1, px - 1, _y2, px, py);
		fb_copyrect_internal(x1, _y2 + 1, x2, y2, px, py - y1 + _y2 + 1);
    }

    /* -> left & up */
	else if (((_x2 >= x1 ) && (_x2 < x2)) && ((_y2 >= y1) && (_y2 < y2)))
    {
		fb_copyrect_internal(x1, y1, _x2, _y2,     px, py);
		fb_copyrect_internal(_x2 + 1, y1, x2, _y2, px - x1 + _x2 + 1, py);
		fb_copyrect_internal(x1, _y2 + 1, x2, y2,  px, py - y1 + _y2 + 1);
    }

    /* -> left & down */
	else if (((_x2 >= x1 ) && (_x2 < x2)) && ((py > y1) && (py <= y2)))
    {
		fb_copyrect_internal(x1, py, _x2, y2,     px, py * 2 - y1);
		fb_copyrect_internal(_x2 + 1, py, x2, y2, px - x1 + _x2 + 1, py * 2 - y1);
		fb_copyrect_internal(x1, y1, x2, py - 1,  px, py);
    }

    else
    {
    	fb_copyrect_internal(x1, y1, x2, y2, px, py);
    }
}

//-------------------------------------------------------------------------------------------------

/******************************************************************************
 * 显示 BMP 图像
 ******************************************************************************/

/*
 * BMP 文件格式
 */

/* 文件头结构 - 14byte
 */
typedef struct
{
	char cfType[2];         	/* 文件类型, 必须为 "BM" (0x4D42)*/
	char cfSize[4];        		/* 文件的大小(字节) */
	char cfReserved[4];     	/* 保留, 必须为 0 */
	char cfoffBits[4];      	/* 位图阵列相对于文件头的偏移量(字节)*/
} __attribute__((packed)) BITMAPFILEHEADER;

/* 位图信息头结构 - 40byte
 */
typedef struct
{
	char ciSize[4];         	/* size of BITMAPINFOHEADER */
	char ciWidth[4];        	/* 位图宽度(像素) */
	char ciHeight[4];       	/* 位图高度(像素) */
	char ciPlanes[2];       	/* 目标设备的位平面数, 必须置为1 */
	char ciBitCount[2];     	/* 每个像素的位数, 1,4,8,16或24 */
	char ciCompress[4];     	/* 位图阵列的压缩方法,0=不压缩 */
	char ciSizeImage[4];    	/* 图像大小(字节) */
	char ciXPelsPerMeter[4];	/* 目标设备水平每米像素个数 */
	char ciYPelsPerMeter[4];	/* 目标设备垂直每米像素个数 */
	char ciClrUsed[4];      	/* 位图实际使用的颜色表的颜色数 */
	char ciClrImportant[4]; 	/* 重要颜色索引的个数 */
} __attribute__((packed)) BITMAPINFOHEADER;

/* 像素 - 5551
 */
typedef struct
{
	unsigned short blue:  5;
	unsigned short green: 5;
	unsigned short red:   5;
	unsigned short rev:   1;
} __attribute__((packed)) PIXEL;

/* 变量
 */
static BITMAPFILEHEADER	FileHead;
static BITMAPINFOHEADER	InfoHead;

/*
 * 字符转整数
 */
static long chartolong(char *str, int length)
{
	long number = 0;

	if ((length > 0) && (length <= 4))
	{
		memcpy((void *)&number, (void *)str, length);
	}

	return number;
}

/*
 * 从文件装入显示 BMP 图像
 */
static int fb_showbmp_internal(int x, int y, char *bmpfilename)
{
	FILE *fd;
	int rt, row, ciBitCount, ciWidth, ciHeight;
	unsigned int BytesPerLine;
    unsigned long addr;
	unsigned char *bmpbuf, *buf;

	/******************************************************
	 * 打开位图文件
	 ******************************************************/

	fd = fopen(bmpfilename, "r");
	if (fd == NULL)
	{
		printk("can't open file %s!\r\n", bmpfilename);
		return -1;
	}

	/* 读取位图文件头 */
    rt = fread((char *)&FileHead, 1, sizeof(BITMAPFILEHEADER), fd);
	if (rt != sizeof(BITMAPFILEHEADER))
	{
		printk("read BMP header error, %s!\r\n", bmpfilename);
        fclose(fd);
		return -2;
	}

	/* 判断位图的类型 */
	if (memcmp(FileHead.cfType, "BM", 2) != 0)
	{
		printk("%s is not a BMP file\r\n", bmpfilename);
        fclose(fd);
		return -3;
	}

	/* 读取位图信息头 */
	rt = fread((char *)&InfoHead, 1, sizeof(BITMAPINFOHEADER), fd);
	if (rt != sizeof(BITMAPINFOHEADER))
	{
		printk("read BMP infoheader error, %s!\r\n", bmpfilename);
        fclose(fd);
		return -4;
	}

	ciWidth    = (int)chartolong(InfoHead.ciWidth,    4);
	ciHeight   = (int)chartolong(InfoHead.ciHeight,   4);
	ciBitCount = (int)chartolong(InfoHead.ciBitCount, 2);

	if (16 != ciBitCount)
	{
		printk("bmp is not 16 bits per pixel. (%d)\r\n", ciBitCount);
        fclose(fd);
		return -5;
	}

    fseek(fd, (int)chartolong(FileHead.cfoffBits, 4), SEEK_SET);
	BytesPerLine = (ciWidth * ciBitCount + 31) / 32 * 4;

	/******************************************************
	 * 存储全部像素的内存 - 整个文件处理
	 ******************************************************/

	bmpbuf = (unsigned char *)malloc(BytesPerLine*ciHeight);
	if (NULL == bmpbuf)
	{
		printk("no memory to load bmp, size=%i.\r\n", (int)BytesPerLine*ciHeight);
        fclose(fd);
		return -6;
	}

	/* 读全部像素数据 */
    rt = fread(bmpbuf, 1, BytesPerLine*ciHeight, fd);
	if (rt < BytesPerLine*ciHeight)
	{
		printk("load bmp to memory error, size=%i.\r\n", (int)BytesPerLine*ciHeight);
		free(bmpbuf);
        fclose(fd);
		return -7;
	}

	/**************************************************************************
	 * 开始处理数据
	 **************************************************************************/

	buf = bmpbuf;

	for (row=ciHeight - 1; row >=0; row--)
	{
		PIXEL    *pix;
		unsigned short val;
		unsigned int col, datalen, colend;
		unsigned int curx, cury;

		/* 处理一行像素 */
		pix = (PIXEL *)buf;
		for (col=0; col<ciWidth; col++)
		{
			val = (pix->red << 11) | (pix->green << 6) | pix->blue;
			*((unsigned short *)pix) = val;
			pix++;
		}

		curx = x;
		cury = y + row;
		if ((curx>=0) && (curx<fb->varInfo.xres) && (cury>=0) && (cury<fb->varInfo.yres))
		{
			/* 计算数据长度和目的地址 */
			colend = ((fb->varInfo.xres-1) <= (curx+ciWidth)) ? (fb->varInfo.xres-1) : (curx+ciWidth);
			datalen = (colend - curx) * fb->bytes_per_pixel;
			addr = (unsigned long)fb->fixInfo.smem_start +
                   (curx + cury * fb->varInfo.xres) * fb->bytes_per_pixel;

			/* 复制一行数据 */
			memcpy((void *)addr, (void *)buf, datalen);
		}

		buf += BytesPerLine;
	}

	free(bmpbuf);

    fclose(fd);

	return 0;
}

int fb_showbmp(int x, int y, char *bmpfilename)
{
	if (fb->dc != NULL)
	{
    	return fb_showbmp_internal(x, y, bmpfilename);
    }
	else
	{
    	return -1;
    }
}

//-------------------------------------------------------------------------------------------------

#ifdef USE_LVGL

void ls2k_draw_rgb565_pixel(int x, int y, unsigned int color)
{
    uintptr_t fbAddr;

    fbAddr = (uintptr_t)fb->lineAddr[y] + x * fb->bytes_per_pixel;

    *((unsigned short *)fbAddr) = color;
}

#endif

#endif // #ifdef BSP_USE_FB


