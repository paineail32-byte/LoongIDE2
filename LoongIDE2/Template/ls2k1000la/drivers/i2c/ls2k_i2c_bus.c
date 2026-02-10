/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_i2c_bus.c
 *
 * created: 2022-02-24
 *  author: Bian
 */

#include "bsp.h"

#if IS24C16_DRV

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ls2k1000.h"
#include "ls2k1000_irq.h"

#include "osal.h"

#include "ls2k_drv_io.h"
#include "drv_os_priority.h"

#include "ls2k_i2c_hw.h"
#include "ls2k_i2c_bus.h"

//-----------------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------------

#define LOCK(p)     osal_mutex_obtain(p->p_mutex, OSAL_WAIT_FOREVER)
#define UNLOCK(p)   osal_mutex_release(p->p_mutex)

//-----------------------------------------------------------------------------
// device soft control
//-----------------------------------------------------------------------------

typedef struct
{
    HW_I2C_t *hwI2C;                /* pointer to HW registers */
    unsigned int base_frq;          /* input frq for baud rate divider */
    unsigned int baudrate;          /* work baud rate */
    unsigned int dummy_char;        /* dummy char */
    /* interrupt support*/
    unsigned int irqNum;            /* interrupt num */
    /* mutex */
    osal_mutex_t p_mutex;
    int  initialized;
    char dev_name[16];
} I2C_bus_t;

//-----------------------------------------------------------------------------
// IIC bus device table
//-----------------------------------------------------------------------------

#if BSP_USE_I2C0
static I2C_bus_t ls2k_I2C0 =
{
	.hwI2C       = (HW_I2C_t *)PHYS_TO_UNCACHED(0x1fe21000),
	.irqNum      = INTC0_I2C0_IRQ,
	.base_frq    = 0,
	.baudrate    = 100000,
	.dummy_char  = 0,
    .initialized = 0,
    .dev_name    = "i2c0",
};

const void *busI2C0 = &ls2k_I2C0;
#endif

#if BSP_USE_I2C1
static I2C_bus_t ls2k_I2C1 =
{
	.hwI2C       = (HW_I2C_t *)PHYS_TO_UNCACHED(0x1fe21800),
	.irqNum      = INTC0_I2C1_IRQ,
	.base_frq    = 0,
	.baudrate    = 100000,
	.dummy_char  = 0,
    .initialized = 0,
    .dev_name    = "i2c1",
};

const void *busI2C1 = &ls2k_I2C1;
#endif

static void ls2k1000_i2c_init_io_base(I2C_bus_t *bus)
{
    unsigned long apb_address0;

    apb_address0 = READ_REG64(APB_CFG_HEAD_BASE + 0x10) & ~0x0Ful;

#if (BSP_USE_I2C0)
    if (bus == &ls2k_I2C0)
        ls2k_I2C0.hwI2C = (HW_I2C_t *)PHYS_TO_UNCACHED(apb_address0 + 0x1000);
#endif
#if (BSP_USE_I2C1)
    if (bus == &ls2k_I2C1)
        ls2k_I2C1.hwI2C = (HW_I2C_t *)PHYS_TO_UNCACHED(apb_address0 + 0x1800);
#endif
}

//-----------------------------------------------------------------------------

/*
 * time-out, xxx how long is fit ?
 */
#define DEFAULT_TIMEOUT     10000

/******************************************************************************
 * I2C hardware
 ******************************************************************************/

static int I2C_wait_done(I2C_bus_t *pIIC)
{
	register unsigned int tmo = 0;

	/* wait for i2c to terminate */
	while (pIIC->hwI2C->CMDSR.sr & I2C_SR_TXIP)
	{
		if (tmo++ > DEFAULT_TIMEOUT)
		{
			printk("I2C tmo!\r\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int I2C_set_baudrate(I2C_bus_t *pIIC, unsigned int baudrate)
{
	unsigned int fdr_val;
	unsigned char ctrl;

//  if (pIIC->working_baudrate == baudrate)
//      return 0;

	/* set frequency divider, default to 100kHz */
	fdr_val = baudrate < pIIC->baudrate ? baudrate : pIIC->baudrate;
	fdr_val = fdr_val <= 400000 ? fdr_val : 400000;    /* limit max 400K */

	/**********************************************************************************************
     * LS2K1000
     *
     * 假设分频锁存器的值为prescale, 从LPB总线PCLK时钟输入的频率为clock_a,
     * SCL总线的输出频率为clock_s, 则应满足如下关系:
     *
     * clock_s = clock_a / 4 * (prcescale + 1)
     *
     **********************************************************************************************/
     
	fdr_val = pIIC->base_frq / (4 * fdr_val) - 1;

	/* write the clock div register */
	ctrl  = pIIC->hwI2C->ctrl;
	ctrl &= ~(I2C_CTRL_EN | I2C_CTRL_IEN);
	pIIC->hwI2C->ctrl   = ctrl;
	pIIC->hwI2C->prerlo = fdr_val & 0xFF;
	pIIC->hwI2C->prerhi = (fdr_val >> 8) & 0xFF;

	/* set control register to module enable */
	pIIC->hwI2C->CMDSR.cmd = 0x00;
	ctrl |= I2C_CTRL_MASTER | I2C_CTRL_EN;
	pIIC->hwI2C->ctrl = ctrl;

//  pIIC->working_baudrate = baudrate;

	return 0;
}

/******************************************************************************
 * I2C driver Implement
 ******************************************************************************/

extern int ls2k_i2c_init_hook(const void *bus);

STATIC_DRV int I2C_initialize(const void *bus)
{
	I2C_bus_t *pIIC = (I2C_bus_t *)bus;

    if (bus == NULL)
        return -1;

    if (pIIC->initialized)
        return 0;

    ls2k1000_i2c_init_io_base(pIIC);

    ls2k_i2c_init_hook(bus);

    pIIC->p_mutex = osal_mutex_create(pIIC->dev_name, 0);
    if (pIIC->p_mutex == NULL)
    {
        printk("create mutex for %s fail\r\n", pIIC->dev_name);
    	return -1;
    }

	/* initialize with bsp bus frequency */
	pIIC->base_frq = apb_frequency;

	/* initialize the baudrate */
	I2C_set_baudrate(pIIC, pIIC->baudrate);

    pIIC->initialized = 1;

    printk("I2C%i controller initialized.\r\n",
#if BSP_USE_I2C0
           (bus == busI2C0) ? 0 :
#endif
#if BSP_USE_I2C1
           (bus == busI2C1) ? 1 :
#endif
           -1);

	return 0;
}

STATIC_DRV int I2C_send_start(const void *bus, unsigned int Addr)
{
	unsigned int tmo = 0;
	I2C_bus_t *pIIC = (I2C_bus_t *)bus;

    if (bus == NULL)
    {
        return -1;
    }

	LOCK(pIIC);

	/* wait for bus idle */
	while (pIIC->hwI2C->CMDSR.sr & I2C_SR_BUSY)
	{
		if (tmo++ > DEFAULT_TIMEOUT)
			return -ETIMEDOUT;
		delay_us(1);
	}

	/* "start" command to handle the bus */
	pIIC->hwI2C->CMDSR.cmd = I2C_CMD_START;

	return 0;
}

STATIC_DRV int I2C_send_stop(const void *bus, unsigned int Addr)
{
	I2C_bus_t *pIIC = (I2C_bus_t *)bus;

    if (bus == NULL)
    {
        return -1;
    }

	/* "stop" command to release the bus */
	pIIC->hwI2C->CMDSR.cmd = I2C_CMD_STOP;

	delay_us(1);

    UNLOCK(pIIC);
	return 0;
}

STATIC_DRV int I2C_send_addr(const void *bus, unsigned int Addr, int rw)
{
	int rt;
	I2C_bus_t *pIIC = (I2C_bus_t *)bus;

    if (bus == NULL)
    {
        return -1;
    }

	/* set then slave address, 1: read, 0: write. */
	pIIC->hwI2C->data = ((unsigned char)Addr << 1) | ((rw) ? 1 : 0);

	/* "start" "write" command to send addr */
	pIIC->hwI2C->CMDSR.cmd = I2C_CMD_WRITE | I2C_CMD_START;

	/* wait for successful transfer */
	rt = I2C_wait_done(pIIC);

	/* slave is no ack */
	if (0 == rt)
	{
		if (pIIC->hwI2C->CMDSR.sr & I2C_SR_RXNACK)
		{
			I2C_send_stop(pIIC, 0);
			rt = -2;
		}
	}
	else
	{
		I2C_send_stop(pIIC, 0);
	}

	return rt;
}

STATIC_DRV int I2C_read_bytes(const void *bus, unsigned char *buf, int len)
{
	int rt;
	unsigned char *p = buf;
	I2C_bus_t *pIIC = (I2C_bus_t *)bus;

    if ((bus == NULL) || (buf == NULL))
    {
        return -1;
    }

	while (len-- > 0)
	{
		/* Read command */
		if (len == 0)
		{
			/* last byte is not acknowledged */
			pIIC->hwI2C->CMDSR.cmd = I2C_CMD_READ | I2C_CMD_NACK;
		}
		else
		{
			pIIC->hwI2C->CMDSR.cmd = I2C_CMD_READ | I2C_CMD_ACK;
		}

		/* wait until end of transfer */
		rt = I2C_wait_done(pIIC);
		if (0 != rt)
		{
			I2C_send_stop(pIIC, 0);
			return -rt;
		}

		/* Read data */
		*p++ = pIIC->hwI2C->data;
	}

	return p - buf;
}

STATIC_DRV int I2C_write_bytes(const void *bus, unsigned char *buf, int len)
{
	int rt;
	unsigned char *p = buf;
	I2C_bus_t *pIIC = (I2C_bus_t *)bus;

    if ((bus == NULL) || (buf == NULL))
    {
        return -1;
    }

	while (len-- > 0)
	{
		/* Write data */
		pIIC->hwI2C->data = *p++;

		/* Write command */
		pIIC->hwI2C->CMDSR.cmd = I2C_CMD_WRITE; // XXX | I2C_CMD_ACK

		/* Wait until end of transfer */
		rt = I2C_wait_done(pIIC);
		if (0 != rt)
		{
			I2C_send_stop(pIIC, 0);
			return -rt;
		}

		/* Slave is no ack */
		if (pIIC->hwI2C->CMDSR.sr & I2C_SR_RXNACK)
		{
			I2C_send_stop(pIIC, 0);
			return p - buf;
		}
	}

	return p - buf;
}

STATIC_DRV int I2C_ioctl(const void *bus, int cmd, void *arg)
{
	int rt = -1;
	I2C_bus_t *pIIC = (I2C_bus_t *)bus;

    if (bus == NULL)
    {
        return -1;
    }

	switch (cmd)
	{
		case IOCTL_SPI_I2C_SET_TFRMODE:
			rt = -I2C_set_baudrate(pIIC, (unsigned int)(uint64_t)arg);
			break;

		default:
			rt = -1;
			break;
	}

	return rt;
}

//-----------------------------------------------------------------------------
// IIC bus driver ops
//-----------------------------------------------------------------------------

#if (PACK_DRV_OPS)
static const libi2c_ops_t ls2k_i2c_drv_ops =
{
    .init        = I2C_initialize,
    .send_start  = I2C_send_start,
    .send_stop   = I2C_send_stop,
    .send_addr   = I2C_send_addr,
    .read_bytes  = I2C_read_bytes,
    .write_bytes = I2C_write_bytes,
    .ioctl       = I2C_ioctl,
};

const libi2c_ops_t *i2c_drv_ops = &ls2k_i2c_drv_ops;
#endif

#endif // #if (BSP_USE_I2C)

/*
 * @@ END
 */
 

