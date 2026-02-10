/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * at24c02.c
 *
 * created: 2024-7-12
 *  author: 
 */

#include "bsp.h"

#if AT24C02_DRV

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "osal.h"

#include "ls2k_i2c_bus.h"

#include "i2c/at24c02.h"

// #define AT24_DEBUG

//-----------------------------------------------------------------------------

#define AT24C02_ADDRESS         0x50        /* 0b1010 000 */

#define AT24C02_BAUDRATE		400000		/* 400kHz */

#define AT24C02_PAGE_BYTES      8
#define AT24C02_PAGES           32
#define AT24C02_CAPACITY        (AT24C02_PAGES*AT24C02_PAGE_BYTES)

#define PROGRAM_DELAY           5           /* 手册上写延时 5ms */

//-----------------------------------------------------------------------------

#define CHECK_DONE(rt) \
	do {               \
        if (0 != rt)   \
            goto lbl_done; \
    } while (0);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/**
 * bus:  busI2C
 * buf:  unsigned char *
 * size: length of buf
 * arg:  integer, address of eeprom
 */
STATIC_DRV int AT24C02_read(const void *bus, void *buf, int size, void *arg)
{
    int rt = 0;
    int off = (long)arg;
    int remain_bytes = size;
    unsigned char *pch = buf, eeprom_addr;

    if ((bus == NULL) || (buf == NULL))
    {
        return -1;
    }

    if ((size <= 0) || (off >= AT24C02_CAPACITY))
    {
        return -2;
    }
	
	if (size + off > AT24C02_CAPACITY)
	{
	    size = AT24C02_CAPACITY - off;
	}

    /**********************************
     * 8 字节对齐, 循环读
     */

    while (remain_bytes > 0)
    {
        int this_bytes, rw_cnt;

        this_bytes = off & (AT24C02_PAGE_BYTES - 1);
        this_bytes = (0 == this_bytes) ? AT24C02_PAGE_BYTES
                                       : AT24C02_PAGE_BYTES - this_bytes;
        this_bytes = (this_bytes < remain_bytes) ? this_bytes : remain_bytes;

    #ifdef AT24_DEBUG
        PRINTF("RD: off=0x%08x, count=%i\n", (int)off, this_bytes);
    #endif

	    /* start transfer */
	    rt = ls2k_i2c_send_start(bus, 0);
	    CHECK_DONE(rt);

	    /* set transfer mode */
        rt = ls2k_i2c_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, (void *)AT24C02_BAUDRATE);
	    CHECK_DONE(rt);

	    /* address device, FALSE(0) for WRITE */
	    rt = ls2k_i2c_send_addr(bus, AT24C02_ADDRESS, false);
	    CHECK_DONE(rt);
	
	    /* write start addr - offset */
	    eeprom_addr = off;
	    rw_cnt = ls2k_i2c_write_bytes(bus, &eeprom_addr, 1);
	    if (rw_cnt < 0)
		    rt = rw_cnt;
	    CHECK_DONE(rt);

	    /*
	     * restart - address device, TRUE(1) for READ
	     */
	    rt = ls2k_i2c_send_addr(bus, AT24C02_ADDRESS, true);
	    CHECK_DONE(rt);

	    /* read out data */
	    rw_cnt = ls2k_i2c_read_bytes(bus, pch, this_bytes);
        if (rw_cnt < 0)
		    rt = rw_cnt;
		CHECK_DONE(rt);

        off += this_bytes;
        pch += this_bytes;
        remain_bytes -= this_bytes;
        
	    /* terminate transfer */
	    ls2k_i2c_send_stop(bus, 0);
    }

lbl_done:
	/* terminate transfer */
    ls2k_i2c_send_stop(bus, 0);

	return (int)((long)pch - (long)buf);
}

/**
 * bus:  busI2C
 * buf:  unsigned char *
 * size: length of buf
 * arg:  integer, address of eeprom
 */
STATIC_DRV int AT24C02_write(const void *bus, void *buf, int size, void *arg)
{
    int rt = 0;
    int off = (long)arg;
    int remain_bytes = size;
    unsigned char *pch = buf;

    if ((bus == NULL) || (buf == NULL))
    {
        return -1;
    }

    if ((size <= 0) || (off >= AT24C02_CAPACITY))
    {
        return -2;
    }

	if (size + off > AT24C02_CAPACITY)
	{
	    size = AT24C02_CAPACITY - off;
	}

    /**********************************
     * 8 字节对齐, 循环写
     */

    while (remain_bytes > 0)
    {
        int this_bytes, rw_cnt;
#if I2C_DEVICE_NEW
        unsigned char dataBuf[AT24C02_PAGE_BYTES+2];    // 1 pagesize + 1
#endif

        this_bytes = off & (AT24C02_PAGE_BYTES - 1);
        this_bytes = (0 == this_bytes) ? AT24C02_PAGE_BYTES
                                       : AT24C02_PAGE_BYTES - this_bytes;
        this_bytes = (this_bytes < remain_bytes) ? this_bytes : remain_bytes;

    #ifdef AT24_DEBUG
        PRINTF("WR: off=0x%08x, count=%i\n", (int)off, this_bytes);
    #endif

	    /* start transfer */
	    rt = ls2k_i2c_send_start(bus, 0);
	    CHECK_DONE(rt);

	    /* set transfer mode */
        rt = ls2k_i2c_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, (void *)AT24C02_BAUDRATE);
	    CHECK_DONE(rt);

	    /* address device, FALSE(0) for write */
	    rt = ls2k_i2c_send_addr(bus, AT24C02_ADDRESS, false);
	    CHECK_DONE(rt);

#if I2C_DEVICE_NEW
        /*
         * LS2K300 使用新的 I2C 控制器
         */
        dataBuf[0] = off;
        memcpy(&dataBuf[1], pch, this_bytes);

	    rw_cnt = ls2k_i2c_write_bytes(bus, dataBuf, this_bytes + 1);
	    if (rw_cnt < 0)
		    rt = rw_cnt;
	    CHECK_DONE(rt);

#else
	    /* write start addr - offset */
	    eeprom_addr = off;
	    rw_cnt = ls2k_i2c_write_bytes(bus, &eeprom_addr, 1);
	    if (rw_cnt < 0)
		    rt = rw_cnt;
	    CHECK_DONE(rt);

	    /* write data */
	    rw_cnt = ls2k_i2c_write_bytes(bus, pch, this_bytes);
	    if (rw_cnt < 0)
		    rt = rw_cnt;
	    CHECK_DONE(rt);

#endif

        off += this_bytes;
        pch += this_bytes;
        remain_bytes -= this_bytes;

        /* terminate transfer */
        ls2k_i2c_send_stop(bus, 0);

        /*
         * delay wait program done. 
         */
        osal_msleep(PROGRAM_DELAY);
    }

lbl_done:
	/* terminate transfer */
    ls2k_i2c_send_stop(bus, 0);

	return (int)((long)pch - (long)buf);
}

/******************************************************************************
 * driver table
 ******************************************************************************/

#if (PACK_DRV_OPS)
/******************************************************************************
 * AT24C02 driver operators
 */
static const driver_ops_t _AT24C02_drv_ops =
{
    .init_entry  = NULL,
    .open_entry  = NULL,
    .close_entry = NULL,
    .read_entry  = AT24C02_read,
    .write_entry = AT24C02_write,
    .ioctl_entry = NULL,
};

const driver_ops_t *at24c02_drv_ops = &_AT24C02_drv_ops;
#endif

#endif // #if AT24C02_DRV

//-----------------------------------------------------------------------------

/*
 * @@ END
 */
 

