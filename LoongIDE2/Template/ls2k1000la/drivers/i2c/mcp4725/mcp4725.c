/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * mcp4725a0.c
 *
 *  Created on: 2015-11-5
 *      Author: Bian
 */

#include "bsp.h"

#if MCP4725_DRV

#include <stdio.h>
#include <errno.h>
#include <stdint.h>

#include "ls2k_drv_io.h"
#include "ls2k_i2c_bus.h"
#include "../ls2k_i2c_hw.h"

#if defined(LS2K500)
#include "ls2k500.h"
#elif defined(LS2K1000LA)
#include "ls2k1000.h"
#else
#error "No Loongson1x SoC defined."
#endif

#include "i2c/mcp4725.h"

//-----------------------------------------------------------------------------

/**********************************************************
 * mcp4725 label: APXX == A2/A1 == 0b01
 */
#define MCP4725_ADDRESS			0x62	    /* mcp4725 address 1100 010(7bits) */

#define MCP4725_BAUDRATE        1000000

//-----------------------------------------------------------------------------

#define CHECK_DONE(rt) \
	do {               \
        if (0 != rt)   \
            goto lbl_done; \
    } while (0);

/******************************************************************************
 * mcp4725 utility
 ******************************************************************************/
/*
 * buffer bytes:
 *
 * 1th:
 * 		bit 7: 		EEPROM Write Status Indicate Bit, (1: Completed, 0: Incomplete)
 *		bit 6: 		Power On Reset. 						--
 *		bit 2:1: 	PD1 PD0 Function						  |
 *					0 	0 	Normal Mode						  |
 *					0 	1 	1 k次 resistor to ground (1)		  |--> Current Settings
 *					1 	0 	100 k次 resistor to ground (1)	  |	   in DAC Register
 *					1 	1 	500 k次 resistor to ground (1)	--
 *
 *		DAC register Data
 * 2th:	D11 D10 D9 D8 D7 D6 D5 D4
 * 3th: D3  D2  D1 D0 X  X  X  X
 *
 *		EEPROM Data
 * 4th: X  PD1 PD0 X  D11 D10 D9 D8
 * 5th: D7 D6  D5  D4 D3  D2  D1 D0
 *
 */
static void print_mcp4725_config_reg(unsigned char *buf, int nlen)
{
	if (nlen != 5)
		return;

	printk("read mcp4725 result: \r\n");
	printk("  EEPROM Write Status = %s\r\n", buf[0] & 0x80 ? "Completed" : "Incomplete");
	printk("  Power On Reset      = %s\r\n", buf[0] & 0x40 ? "ON" : "OFF");
	printk("  Power Down Mode     = ");
	switch ((buf[0] & 0x06) >> 1)
	{
		case 0:	printk("Normal Mode\r\n"); break;
		case 1:	printk("1 k次 resistor to ground\r\n"); break;
		case 2:	printk("100 k次 resistor to ground\r\n"); break;
		case 3:	printk("500 k次 resistor to ground\r\n"); break;
	}

	printk("  DAC register Data   = 0x%04X\r\n",
			((unsigned int)buf[1] << 4) + ((unsigned int)buf[2] >> 4));

	printk("  EEPROM Data: \r\n");
	printk("    Power Down Mode   = ");
	switch ((buf[3] & 0x60) >> 1)
	{
		case 0:	printk("Normal Mode\r\n"); break;
		case 1:	printk("1 k次 resistor to ground\r\n"); break;
		case 2:	printk("100 k次 resistor to ground\r\n"); break;
		case 3:	printk("500 k次 resistor to ground\r\n"); break;
	}

	printk("    Saved DAC Data    = 0x%04X\r\n",
			(((unsigned int)buf[3] << 8) + ((unsigned int)buf[4])) & 0x0FFF);
}

/******************************************************************************
 * driver routine
 ******************************************************************************/

STATIC_DRV int MCP4725_write(const void *bus, void *buf, int size, void *arg)
{
	int rt = 0;
	uint16_t *pVal;
	unsigned char data[2];

    if ((bus == NULL) || (buf == NULL))
    {
        return -1;
    }
        
	pVal = (uint16_t *)buf;

	data[0] = ((*pVal >> 8) & 0x0F);
	data[1] = (*pVal) & 0xFF;

	/* start transfer */
	rt = ls2k_i2c_send_start(bus, MCP4725_ADDRESS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls2k_i2c_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, (void *)MCP4725_BAUDRATE);
	CHECK_DONE(rt);

	/* address device, FALSE(0) for write */
	rt = ls2k_i2c_send_addr(bus, MCP4725_ADDRESS, 0);
	CHECK_DONE(rt);

	/*
	 * Write DAC Register using Fast Mode Write Command: (C2, C1) = (0, 0)
	 *
	 * 1st byte                    2nd byte
	 * 0 0 PD1 PD0 D11 D10 D9 D8   D7 D6 D5 D4 D3 D2 D1 D0
	 * | |  |   |   |          |
	 * | |  |   |    ------------> DAC Register Data (12 bits)
	 * | |   --------------------> Power Down Select
	 *  -------------------------> Fast Mode Command (C2, C1 = 0, 0)
	 */

	rt = ls2k_i2c_write_bytes(bus, data, 2);

lbl_done:
	/* terminate transfer */
	ls2k_i2c_send_stop(bus, MCP4725_ADDRESS);

	return rt;
}

/*
 * read 5 bytes of mcp4725 status
 */
static int MCP4725_read_config(const void *bus, unsigned char *buf)
{
	int rt;

	/* start transfer */
	rt = ls2k_i2c_send_start(bus, MCP4725_ADDRESS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls2k_i2c_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, (void *)MCP4725_BAUDRATE);
	CHECK_DONE(rt);

	/* address device */
	rt = ls2k_i2c_send_addr(bus, MCP4725_ADDRESS, 1);
	CHECK_DONE(rt);

	/*
	 * fetch read data
	 */
    rt = ls2k_i2c_read_bytes(bus, buf, 5);

lbl_done:

	/* terminate transfer */
	ls2k_i2c_send_stop(bus, MCP4725_ADDRESS);

	return rt;
}

static int MCP4725_write_eeprom(const void *bus, uint16_t Val)
{
    int rt;
    unsigned char buf[3];

	/*
	 * (A) Write DAC Register:            (C2, C1, C0) = (0,1,0) or
	 * (B) Write DAC Register and EEPROM: (C2, C1, C0) = (0,1,1)
	 *
	 *  2nd byte
	 *  C2 C1 C0 X X PD1 PD0 X
	 *  |      |      |   |
	 *  |      |       ----------> Power Down Selection
	 *   ------------------------> Write Command Type:
	 *                             Write DAC Register: (C2 = 0, C1 = 1, C0 = 0)
	 *                             Write DAC Register and EEPROM: (C2 = 0, C1 = 1, C0 = 1).
	 *
	 *  3rd byte                 4th byte
	 *  D11 D10 D9 D8 D7 D6 D5   D4 D2 D1 D0 X X X X
	 */
	buf[0] = 0x60 | ((Val & MCP4725_PD_MASK) >> (MCP4725_PD_SHIFT - 1));
	buf[1] = Val >> 8;
	buf[2] = (Val << 4) & 0xF0;

	/* start transfer */
	rt = ls2k_i2c_send_start(bus, MCP4725_ADDRESS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls2k_i2c_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, (void *)MCP4725_BAUDRATE);
	CHECK_DONE(rt);

	/* address device, FALSE(0) for write */
	rt = ls2k_i2c_send_addr(bus, MCP4725_ADDRESS, 0);
	CHECK_DONE(rt);

    rt = ls2k_i2c_write_bytes(bus, buf, 3);

lbl_done:
	/* terminate transfer */
	ls2k_i2c_send_stop(bus, MCP4725_ADDRESS);

	return rt;
}

STATIC_DRV int MCP4725_ioctl(const void *bus, int cmd, void *arg)
{
    int rt;

    if (bus == NULL)
    {
        return -1;
    }
        
	switch (cmd)
	{
		case IOCTL_MCP4725_WRTIE_EEPROM:
		{
			rt = MCP4725_write_eeprom(bus, (uint16_t)((uintptr_t)arg));
			break;
		}

		case IOCTL_MCP4725_DISP_CONFIG_REG:
		{
			unsigned char buf[5];
			rt = MCP4725_read_config(bus, buf);
			if (rt >= 0)
			{
				print_mcp4725_config_reg(buf, 5);
			}
			break;
		}

		case IOCTL_MCP4725_READ_DAC:
		{
			unsigned char buf[5];
			rt = MCP4725_read_config(bus, buf);
			if (rt >= 0)
			{
				uint16_t *val = (uint16_t *)arg;
				*val = ((uint16_t)(buf[1] << 4) + (buf[2] >> 4)) & 0x0FFF;
			}
			break;
		}

		case IOCTL_MCP4725_READ_EEPROM:
		{
			unsigned char buf[5];
			rt = MCP4725_read_config(bus, buf);
			if (rt >= 0)
			{
				uint16_t *val = (uint16_t *)arg;
				*val = ((uint16_t)(buf[3] << 8) + buf[4]) & 0x0FFF;
			}
			break;
		}

		default:
			rt = -1;
			break;
	}

	return rt;
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * MCP4725 driver operators
 */
static const driver_ops_t MCP4725_drv_ops =
{
    .init_entry  = NULL,
    .open_entry  = NULL,
    .close_entry = NULL,
    .read_entry  = NULL,
    .write_entry = MCP4725_write,
    .ioctl_entry = MCP4725_ioctl,
};

const driver_ops_t *mcp4725_drv_ops = &MCP4725_drv_ops;
#endif

/******************************************************************************
 * user api
 */
int set_mcp4725_dac(const void *bus, uint16_t dacVal)
{
    return MCP4725_write(bus, (void *)&dacVal, 2, NULL);
}

#endif // #if MCP4725_DRV


