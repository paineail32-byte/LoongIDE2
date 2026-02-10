/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * is24c16.c
 *
 * created: 2022-10-11
 *  author: Bian
 */

#include "bsp.h"

#ifdef IS24C16_DRV

#include <stdio.h>
#include <errno.h>

#include "osal.h"

#include "ls2k_i2c_bus.h"

#include "i2c/is24c16.h"

//-----------------------------------------------------------------------------

#define IS24C16_ADDRESS         0x50            // is24c16 地址

#define IS24C16_BAUDRATE        100000          // is24c16 通信速率

#define BLOCK_SIZE              0x100           // 256 bytes/block
#define CHIP_BLOCKS             8               // total 8 blocks
#define CHIP_SIZE               0x800           // 4K

#define WR_PAGE_SIZE            16              // 一次连续写16字节

#define WR_PAGE_DELAY_MS		12				// 写一次的延时, 10ms 不够

//-----------------------------------------------------------------------------

#define CHECK_DONE(rt) \
	do {               \
        if (0 != rt)   \
            goto lbl_done; \
    } while (0);

/******************************************************************************
 * driver implement
 ******************************************************************************/

/*
 * arg: unsigned int, 读地址
 */
STATIC_DRV int IS24C16_read(const void *bus, void *buf, int size, void *arg)
{
    unsigned char B210, *pbuf = (unsigned char *)buf;
    unsigned int addr = (unsigned int)(unsigned long)arg;
    int rt = 0, rd_cnt = 0;
    
    if ((bus == NULL) || (buf == NULL))
        return -1;

    if ((size <= 0) || (addr >= CHIP_SIZE))
        return 0;

    /* read to CHIP_SIZE once limited */
    rd_cnt = (addr + size) < CHIP_SIZE ? size : CHIP_SIZE - addr;
 
    /* eeprom address HIGH 3 BITs */
    B210 = (unsigned char)(addr >> 8);
    
 	/* start transfer */
	rt = ls2k_i2c_send_start(bus, IS24C16_ADDRESS);
	CHECK_DONE(rt);
	
	/* set transfer mode */
	rt = ls2k_i2c_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, (void *)IS24C16_BAUDRATE);
	CHECK_DONE(rt);

	/* address device, FALSE(0) for WRTIE */
	rt = ls2k_i2c_send_addr(bus, IS24C16_ADDRESS | B210, 0);   // with B210
	CHECK_DONE(rt);

	/* eeprom address LOW BYTE to read */
	rt = ls2k_i2c_write_bytes(bus, (unsigned char *)&addr, 1);
	if (rt == 1) rt = 0;
	CHECK_DONE(rt);

	/* restart - address device, TRUE(1) for READ */
	rt = ls2k_i2c_send_addr(bus, IS24C16_ADDRESS, 1);
	CHECK_DONE(rt);

	/* read out data */
	rt = ls2k_i2c_read_bytes(bus, pbuf, rd_cnt);
	if (rt == rd_cnt) rt = 0;
	CHECK_DONE(rt);

	rt = rd_cnt;

lbl_done:

	/* terminate transfer */
	ls2k_i2c_send_stop(bus, IS24C16_ADDRESS);

	return rt;
}

/*
 * arg: unsigned int 写地址
 */
STATIC_DRV int IS24C16_write(const void *bus, void *buf, int size, void *arg)
{
    unsigned char *pbuf = (unsigned char *)buf;
    unsigned int addr = (unsigned int)(unsigned long)arg;
    int rt = 0, wr_cnt = 0;

    if ((bus == NULL) || (buf == NULL))
        return -1;

    if ((size <= 0) || (addr >= CHIP_SIZE))
        return 0;

    while ((wr_cnt < size) && (addr < CHIP_SIZE))
    {
        int i, this_count;
        unsigned char B210, wr_buf[WR_PAGE_SIZE + 1];

        /* write WRITE_PAGE bytes once limited */
        this_count = (size - wr_cnt) <= WR_PAGE_SIZE ? (size - wr_cnt) : WR_PAGE_SIZE;

        /* exceed EEPROM capicity */
        if (this_count > (CHIP_SIZE - addr))
            this_count = CHIP_SIZE - addr;

        /* eeprom address HIGH 3 BITs */
        B210 = (unsigned char)(addr >> 8);
        
        /* eeprom write address LOW BYTE */
        wr_buf[0] = (unsigned char)addr;            // little-endian

        /* fill write data buffer */
        for (i=0; i<this_count; i++)
        {
        	wr_buf[i+1] = pbuf[i];
        }

	    /* start transfer */
	    rt = ls2k_i2c_send_start(bus, IS24C16_ADDRESS);
	    CHECK_DONE(rt);

        /* set transfer mode */
	    rt = ls2k_i2c_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, (void *)IS24C16_BAUDRATE);
	    CHECK_DONE(rt);

	    /* address device, FALSE(0) for WRTIE  */
	    rt = ls2k_i2c_send_addr(bus, IS24C16_ADDRESS | B210, 0);   // with B210
	    CHECK_DONE(rt);

        /* write address & data */
	    rt = ls2k_i2c_write_bytes(bus, wr_buf, this_count + 1);
	    if (rt == this_count + 1) rt = 0;
	    CHECK_DONE(rt);

	    /*
         * terminate a writting operation
         */
	    ls2k_i2c_send_stop(bus, IS24C16_ADDRESS);
	
        wr_cnt += this_count;
        pbuf   += this_count;
        addr   += this_count;
        
        osal_msleep(WR_PAGE_DELAY_MS);      /* TODO how long? write cycle time max */
    }

    rt = wr_cnt;

lbl_done:

	/* terminate transfer */
	ls2k_i2c_send_stop(bus, IS24C16_ADDRESS);

    return rt;
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * IS24C16 driver operators
 */
static const driver_ops_t _IS24C16_drv_ops =
{
    .init_entry  = NULL,
    .open_entry  = NULL,
    .close_entry = NULL,
    .read_entry  = IS24C16_read,
    .write_entry = IS24C16_write,
    .ioctl_entry = NULL,
};

const driver_ops_t *is24c16_drv_ops = &_IS24C16_drv_ops;
#endif

#endif // #ifdef IS24C16_DRV

