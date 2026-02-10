/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * st7701s.c
 *
 * created: 2022-3-25
 *  author: Bian
 */
 
/*
 * st7701s 采用3线制通讯, 连接在SPI1, 需要使用GPIO模拟SPI时序来实现读写
 *
 * SPI1_CS:     GPIO47
 * SPI1_CLK:    GPIO44
 * SPI1_MOSI:   GPIO46
 *
 */

/*
 * CPOL=0, CPHA=0: 空闲时CLK=0 上升沿采样DA
 */

#include "bsp.h"

#if USE_ST7701S

#include <stdio.h>

#include "cpu.h"
#include "ls2k500.h"
#include "ls2k500_gpio.h"

#define CS_LINE     47
#define DA_LINE     46
#define CLK_LINE    44

#define RESET_LINE  34
#define LIGHT_LINE  84

#define SPI_CS_0    gpio_write(CS_LINE, 0)
#define SPI_CS_1    gpio_write(CS_LINE, 1)
#define SPI_DA_0    gpio_write(DA_LINE, 0)
#define SPI_DA_1    gpio_write(DA_LINE, 1)
#define SPI_CLK_0   gpio_write(CLK_LINE, 0)
#define SPI_CLK_1   gpio_write(CLK_LINE, 1)

static void spi_write_data(unsigned char data)
{
	unsigned char n;

	for (n=0; n<8; n++)
	{
		SPI_CLK_0;
		if (data & 0x80)
		{
			SPI_DA_1;
		}
		else
		{
			SPI_DA_0;
		}

		delay_us(1);
		SPI_CLK_1;
		delay_us(1);
		
		data <<= 1;
	}
}

static unsigned char spi_read_data(unsigned char addr)
{
	unsigned char i, tmp=0;

	SPI_CS_1;
	SPI_CLK_1;
	delay_us(1);
	SPI_CS_0;
	SPI_CLK_0;
	SPI_DA_0;
	delay_us(1);
	SPI_CLK_1;
	delay_us(1);

	spi_write_data(addr);

    gpio_enable(DA_LINE, GPIO_IN);

	for (i=0; i<8; i++)
	{
		SPI_CLK_0;
		tmp <<= 1;

		if (gpio_read(DA_LINE))
		{
        	tmp |= 1;
        }
			
		delay_us(1);
		SPI_CLK_1;
		delay_us(1);
	}

	delay_us(1);
	SPI_CLK_0;
	SPI_CS_1;

    gpio_enable(DA_LINE, GPIO_OUT);

	return tmp;
}

static void SPI_WriteComm(unsigned char data)
{
	SPI_CS_0;
	SPI_CLK_0;
	
	SPI_DA_0;
	delay_us(1);
	SPI_CLK_1;
	delay_us(1);
	
	spi_write_data(data);

    SPI_CS_1;
}

static void SPI_WriteData(unsigned char data)
{
	SPI_CS_0;
	SPI_CLK_0;
	
	SPI_DA_1;
	delay_us(1);
	SPI_CLK_1;
	delay_us(1);

    spi_write_data(data);

    SPI_CS_1;
}

void lcd_init(void)
{
	printk("lcd_init\n");

	/* gpio44, gpio46~47(spi1), output high level */
	gpio_enable(CS_LINE, GPIO_OUT);
	gpio_write(CS_LINE, 1);
	gpio_enable(DA_LINE, GPIO_OUT);
	gpio_write(DA_LINE, 1);
	gpio_enable(CLK_LINE, GPIO_OUT);
	gpio_write(CLK_LINE, 1);
	
	/* reset */
	/* gpio34 output */
	gpio_enable(RESET_LINE, GPIO_OUT);
	gpio_write(RESET_LINE, 1);
	delay_us(10000);
	gpio_write(RESET_LINE, 0);
	delay_us(200000);
	gpio_write(RESET_LINE, 1);
	delay_us(50000);

#if 0
    {
        unsigned char id1, id2, id3;

        id1 = spi_read_data(0xDA);      // ID1
        id2 = spi_read_data(0xDB);      // ID2
        id3 = spi_read_data(0xDC);      // ID3

        printk("st7701s ID: %02X %02X %02X\r\n", id1, id2 ,id3);
    }
#endif

	/*blacklight*/
	/* gpio84(pwm0) output high level */
	gpio_enable(LIGHT_LINE, GPIO_OUT);
	gpio_write(LIGHT_LINE, 1);

	SPI_WriteComm(0x11);
	delay_us(60000);
	delay_us(60000);

	SPI_WriteComm(0xFF);
	SPI_WriteData(0x77);
	SPI_WriteData(0x01);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x10);

	SPI_WriteComm(0xC0);
	SPI_WriteData(0x63);       // line setting: (0x63+1)*8 = 800
	SPI_WriteData(0x00);

	SPI_WriteComm(0xC1);
	SPI_WriteData(0x11);       // VBP[7:0]: Back-Porch = 17
	SPI_WriteData(0x02);       // VFP[7:0]: Front-Porch = 2

	SPI_WriteComm(0xC2);
	SPI_WriteData(0x31);
	SPI_WriteData(0x08);

	SPI_WriteComm(0xCC);
	SPI_WriteData(0x10);

	/* Gamma Cluster Setting */
	SPI_WriteComm(0xB0);
	SPI_WriteData(0x40);
	SPI_WriteData(0x01);
	SPI_WriteData(0x46);
	SPI_WriteData(0x0D);
	SPI_WriteData(0x13);
	SPI_WriteData(0x09);
	SPI_WriteData(0x05);
	SPI_WriteData(0x09);
	SPI_WriteData(0x09);
	SPI_WriteData(0x1B);
	SPI_WriteData(0x07);
	SPI_WriteData(0x15);
	SPI_WriteData(0x12);
	SPI_WriteData(0x4C);
	SPI_WriteData(0x10);
	SPI_WriteData(0xC8);

	SPI_WriteComm(0xB1);
	SPI_WriteData(0x40);
	SPI_WriteData(0x02);
	SPI_WriteData(0x86);
	SPI_WriteData(0x0D);
	SPI_WriteData(0x13);
	SPI_WriteData(0x09);
	SPI_WriteData(0x05);
	SPI_WriteData(0x09);
	SPI_WriteData(0x09);
	SPI_WriteData(0x1F);
	SPI_WriteData(0x07);
	SPI_WriteData(0x15);
	SPI_WriteData(0x12);
	SPI_WriteData(0x15);
	SPI_WriteData(0x19);
	SPI_WriteData(0x08);

	/* End Gamma Setting */
	/* End Display Control setting */
	/* Bank0 Setting End */
	/* Bank1 Setting */
	/* Power Control Registers Initial  */
	SPI_WriteComm(0xFF);
	SPI_WriteData(0x77);
	SPI_WriteData(0x01);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x11);

	SPI_WriteComm(0xB0);
	SPI_WriteData(0x50);

	/* Vcom Setting */
	SPI_WriteComm(0xB1);
	SPI_WriteData(0x68);

	/* End Vcom Setting */
	SPI_WriteComm(0xB2);
	SPI_WriteData(0x07);

	SPI_WriteComm(0xB3);
	SPI_WriteData(0x80);

	SPI_WriteComm(0xB5);
	SPI_WriteData(0x47);

	SPI_WriteComm(0xB7);
	SPI_WriteData(0x85);

	SPI_WriteComm(0xB8);
	SPI_WriteData(0x21);

	SPI_WriteComm(0xB9);
	SPI_WriteData(0x10);

	SPI_WriteComm(0xC1);
	SPI_WriteData(0x78);

	SPI_WriteComm(0xC2);
	SPI_WriteData(0x78);

	SPI_WriteComm(0xD0);
	SPI_WriteData(0x88);

	/* End Power Control Registers Initial  */
	delay_us(10000);

	SPI_WriteComm(0xE0);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x02);

	SPI_WriteComm(0xE1);
	SPI_WriteData(0x08);
	SPI_WriteData(0x00);
	SPI_WriteData(0x0A);
	SPI_WriteData(0x00);
	SPI_WriteData(0x07);
	SPI_WriteData(0x00);
	SPI_WriteData(0x09);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x33);
	SPI_WriteData(0x33);

	SPI_WriteComm(0xE2);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);

	SPI_WriteComm(0xE3);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x33);
	SPI_WriteData(0x33);

	SPI_WriteComm(0xE4);
	SPI_WriteData(0x44);
	SPI_WriteData(0x44);

	SPI_WriteComm(0xE5);
	SPI_WriteData(0x0E);
	SPI_WriteData(0x2D);
	SPI_WriteData(0xA0);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x10);
	SPI_WriteData(0x2D);
	SPI_WriteData(0xA0);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x0A);
	SPI_WriteData(0x2D);
	SPI_WriteData(0xA0);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x0C);
	SPI_WriteData(0x2D);
	SPI_WriteData(0xA0);
	SPI_WriteData(0xA0);

	SPI_WriteComm(0xE6);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x33);
	SPI_WriteData(0x33);

	SPI_WriteComm(0xE7);
	SPI_WriteData(0x44);
	SPI_WriteData(0x44);

	SPI_WriteComm(0xE8);
	SPI_WriteData(0x0D);
	SPI_WriteData(0x2D);
	SPI_WriteData(0xA0);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x0F);
	SPI_WriteData(0x2D);
	SPI_WriteData(0xA0);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x09);
	SPI_WriteData(0x2D);
	SPI_WriteData(0xA0);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x0B);
	SPI_WriteData(0x2D);
	SPI_WriteData(0xA0);
	SPI_WriteData(0xA0);

	SPI_WriteComm(0xEB);
	SPI_WriteData(0x02);
	SPI_WriteData(0x01);
	SPI_WriteData(0xE4);
	SPI_WriteData(0xE4);
	SPI_WriteData(0x44);
	SPI_WriteData(0x00);
	SPI_WriteData(0x40);

	SPI_WriteComm(0xEC);
	SPI_WriteData(0x02);
	SPI_WriteData(0x01);

	SPI_WriteComm(0xED);
	SPI_WriteData(0xAB);
	SPI_WriteData(0x89);
	SPI_WriteData(0x76);
	SPI_WriteData(0x54);
	SPI_WriteData(0x01);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xFF);
	SPI_WriteData(0x10);
	SPI_WriteData(0x45);
	SPI_WriteData(0x67);
	SPI_WriteData(0x98);
	SPI_WriteData(0xBA);

	/* End GIP Setting */
	/* Power Control Registers Initial End */
	/* Bank1 Setting */
	SPI_WriteComm(0xFF);
	SPI_WriteData(0x77);
	SPI_WriteData(0x01);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	delay_us(2000);

    SPI_WriteComm(0x3A);
    SPI_WriteData(0x55);       // 0x55: RGB565; 0x77: RGB888
	
	SPI_WriteComm(0x20);

	SPI_WriteComm(0x29);

	delay_us(20000);
}

#endif // #if USE_ST7701S


