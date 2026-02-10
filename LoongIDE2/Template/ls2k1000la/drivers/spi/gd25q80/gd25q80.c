/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * gd25q80.c
 *
 * created: 2022-10-12
 *  author: Bian
 */

#include "bsp.h"

#if GD25Q80_DRV

#include <strings.h>
#include <stdio.h>
#include <errno.h>

#include "osal.h"

#include "ls2k1000.h"
#include "ls2k_spi_bus.h"
#include "../ls2k_spi_hw.h"

#include "spi/gd25q80.h"
#include "gd25q80_hw.h"

//-----------------------------------------------------------------------------

#define FLASH_BOOT_BASE			0x800000001C000000ULL
#define FLASH_BOOT_LIMIT        0x100000

#define SPI_MEM_CMD_RDSR		GD25Q80_CMD_RDSR		// 0x05
#define SPI_MEM_CMD_WREN		GD25Q80_CMD_WREN		// 0x06
#define SPI_MEM_CMD_PP			GD25Q80_CMD_PP			// 0x02
#define SPI_MEM_CMD_READ		GD25Q80_CMD_READ		// 0x03
#define SPI_MEM_CMD_RDID		GD25Q80_CMD_RDMDID		// 0x90
#define SPI_MEM_CMD_SE			GD25Q80_CMD_SE4K		// 0x20  // sector erase
#define SPI_MEM_CMD_SE4			SPI_MEM_CMD_SE
#define SPI_MEM_CMD_SE32		GD25Q80_CMD_BE32K		// 0x52	 // block erase, 32KB
#define SPI_MEM_CMD_SE64		GD25Q80_CMD_BE64K		// 0xD8	 // block erase, 64KB
#define SPI_MEM_CMD_BE			GD25Q80_CMD_CE			// 0xC7  // bulk erase

#define SPI_FLASH_BAUDRATE		GD25Q80_BAUDRATE
#define SPI_FLASH_BITSPERCHAR	GD25Q80_BITSPERCHAR
#define SPI_FLASH_PAGE_SIZE		GD25Q80_PAGE_SIZE
#define SPI_FLASH_BLOCK_SIZE	GD25Q80_BLOCK_SIZE_64
#define SPI_FLASH_CHIP_SIZE		GD25Q80_CHIP_SIZE

#define SPI_FLASH_SR_BUSY_BIT	gd25q80_sr_busy

//-----------------------------------------------------------------------------

#define CHECK_DONE(rt) \
	do {               \
        if (0 != rt)   \
            goto lbl_done; \
    } while (0);

//-----------------------------------------------------------------------------

extern int fls(int x);

//-----------------------------------------------------------------------------
// device
//-----------------------------------------------------------------------------

/*
 * gd25q80 片选
 */
#define GD25Q80_CS           0

/*
 * gd25q80 芯片参数
 */
static GD25Q80_param_t m_chipParam =
{
    .baudrate             = SPI_FLASH_BAUDRATE,         /* 速率受SPI 限制 */
    .erase_before_program = true,
    .empty_state          = 0xff,
    .page_size            = SPI_FLASH_PAGE_SIZE,        /* programming page size in byte */
    .sector_size          = SPI_FLASH_BLOCK_SIZE,       /* 64K erase sector size in byte */
    .mem_size             = SPI_FLASH_CHIP_SIZE,        /* 8M total capacity in byte */
};

/*
 * gd25q80 通信模式
 */
static SPI_mode_t m_devMode =
{
	.baudrate = 5000000,                        /* 实际用的速率: 10M */
	.bits_per_char = SPI_FLASH_BITSPERCHAR,     /* how many bits per byte/word/longword? */
	.lsb_first = false,                         /* true: send LSB first */
	.clock_pha = true,                          /* clock phase    - spi mode */
	.clock_pol = true,                          /* clock polarity - spi mode */
	.clock_inv = true,                          /* true: inverted clock (low active) - cs high or low */
	.clock_phs = false,                         /* true: clock starts toggling at start of data tfr - interface mode */
};

//-----------------------------------------------------------------------------

/*
 * read GD25Q80BV manufacturer / device ID
 */
static int GD25Q80_read_id(const void *bus, unsigned int *id)
{
	unsigned char cmd[4];
	unsigned char val[2];
	unsigned int memory_en;
	int	rt, ret_cnt = 0;

	/******************************************************************
	 * check spi-flash is set to fast read mode
	 ******************************************************************/

    rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_GET_FAST_READ_MODE, &memory_en);
	CHECK_DONE(rt);

	/*
	 * change to not fast read mode.
	 */
	if (SPI_PARAM_MEMORY_EN & memory_en)
	{
		rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_FAST_READ_DISABLE, NULL);
        CHECK_DONE(rt);
	}

	/******************************************************************
	 * begin to read id
	 ******************************************************************/

	/* start transfer */
	rt = ls2k_spi_send_start(bus, GD25Q80_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* address device */
	rt = ls2k_spi_send_addr(bus, GD25Q80_CS, true);
	CHECK_DONE(rt);

	/*
	 * send "read md/id" command and address
	 */
	cmd[0] = SPI_MEM_CMD_RDID;
	cmd[1] = 0;
	cmd[2] = 0;
	cmd[3] = 0;

	ret_cnt = ls2k_spi_write_bytes(bus, cmd, 4);
	if (ret_cnt < 0)
	{
    	rt = ret_cnt;
    }
	CHECK_DONE(rt);

	/*
	 * fetch read data
	 */
	ret_cnt = ls2k_spi_read_bytes(bus, val, 2);
	if (ret_cnt < 0)
	{
    	rt = ret_cnt;
    }
	CHECK_DONE(rt);

	/*
	 * manufacturer id: bit15-bit8
	 * device id:       bit7-bit0
	 */
	*id = (val[0] << 8) | val[1];

	/*
	 * change back to fast read mode.
	 */
	if (SPI_PARAM_MEMORY_EN & memory_en)
	{
		rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_FAST_READ_ENABLE, NULL);
		CHECK_DONE(rt);
	}

lbl_done:
	/* terminate transfer */
	ls2k_spi_send_stop(bus, GD25Q80_CS);

	return rt;
}

/*
 * read GD25Q80BV status register
 */
static int GD25Q80_read_sr(const void *bus, unsigned int *sr)
{
	unsigned char cmd, val;
	int rt, ret_cnt = 0;

	/* start transfer */
	rt = ls2k_spi_send_start(bus, GD25Q80_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* select device - chipsel */
	rt = ls2k_spi_send_addr(bus, GD25Q80_CS, true);
	CHECK_DONE(rt);

	/*
	 * send read status register command
	 */
	cmd = SPI_MEM_CMD_RDSR;
	ret_cnt = ls2k_spi_write_bytes(bus, &cmd, 1);
	if (ret_cnt < 0)
	{
    	rt = ret_cnt;
    }
	CHECK_DONE(rt);

	/*
	 * fetch read data
	 */
	ret_cnt = ls2k_spi_read_bytes(bus, &val, 1);
	if (ret_cnt < 0)
	{
    	rt = ret_cnt;
    }
	CHECK_DONE(rt);

	*sr = val;

lbl_done:
	/* terminate transfer */
	ls2k_spi_send_stop(bus, GD25Q80_CS);

	return rt;
}

static int GD25Q80_is_busy(const void *bus)
{
	unsigned int sr;

	if (GD25Q80_read_sr(bus, &sr) != 0)
	{
		return -1;
	}

	return (sr & GD25Q80_SR_BUSY);
}

/*
 * wait for x ms for spi-flash work done.
 */
static int GD25Q80_wait_ms(const void *bus, unsigned int ms)
{
	while (ms-- > 0)
	{
		osal_msleep(1);     /* delay 1 ms per loop, if done, exit immediately */
		if (0 == GD25Q80_is_busy(bus)) // TODO test dead lock ?
		{
            return 0;
        }
	}

	return -ETIMEDOUT;
}

/*
 * set spi-flash writable.
 */
static int GD25Q80_set_write_en(const void *bus)
{
	unsigned char cmd;
	int rt, ret_cnt = 0;

	/* start transfer */
	rt = ls2k_spi_send_start(bus, GD25Q80_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* select device - chipsel */
	rt = ls2k_spi_send_addr(bus, GD25Q80_CS, true);
	CHECK_DONE(rt);

	/*
	 * send write_enable command
	 */
	cmd = SPI_MEM_CMD_WREN;
	ret_cnt = ls2k_spi_write_bytes(bus, &cmd, 1);
	if (ret_cnt < 0)
	{
    	rt = ret_cnt;
    }

lbl_done:
	/* terminate transfer */
	ls2k_spi_send_stop(bus, GD25Q80_CS);

	return rt;
}

/*
 * spi-flash erase 1 sector.
 */
static int GD25Q80_erase_1_sector(const void *bus, unsigned char cmd, unsigned int addr)
{
    unsigned char cmdbuf[4];
    unsigned int memory_en, ms;
	int rt, cmd_size, ret_cnt = 0;

	/******************************************************************
	 * check spi-flash is set to fast read mode
	 ******************************************************************/

	rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_GET_FAST_READ_MODE, &memory_en);
	CHECK_DONE(rt);

	/*
	 * change to not fast read mode.
	 */
	if (SPI_PARAM_MEMORY_EN & memory_en)
	{
		rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_FAST_READ_DISABLE, NULL);
		CHECK_DONE(rt);
	}

	/******************************************************************
	 * after diasble spi-flash fast read mode...
	 ******************************************************************/

	/* check arguments */
	if (addr > m_chipParam.mem_size)
	{
    	rt = -EADDRNOTAVAIL;
    }
	CHECK_DONE(rt);

	/******************************************************************
	 * begin to erase sector.
	 ******************************************************************/

	/**************************************************
	 * First we must set flash write enable
	 * Here also has set the transfer mode.
	 **************************************************/

	rt = GD25Q80_set_write_en(bus);
	CHECK_DONE(rt);

	/**************************************************
	 * Second we can do page-program
	 **************************************************/

	/* start transfer */
	rt = ls2k_spi_send_start(bus, GD25Q80_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = -ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* address device */
	rt = ls2k_spi_send_addr(bus, GD25Q80_CS, true);
	CHECK_DONE(rt);

	/*
	 * send command and address,
	 * command may be:
	 * GD25Q80_CMD_SE		0x20	 sector erase, 4KB,  CMD, A23-A16, A15-A8, A7-A0
	 * GD25Q80_CMD_BE32		0x52	 block erase, 32KB,  CMD, A23-A16, A15-A8, A7-A0
	 * GD25Q80_CMD_BE64		0xD8	 block erase, 64KB,  CMD, A23-A16, A15-A8, A7-A0
	 */
	cmdbuf[0] = cmd;
	cmdbuf[1] = (addr >> 16) & 0xff;
	cmdbuf[2] = (addr >>  8) & 0xff;
	cmdbuf[3] = (addr >>  0) & 0xff;
	cmd_size  = 4;

	ret_cnt = ls2k_spi_write_bytes(bus, cmdbuf, cmd_size);
	if (ret_cnt < 0)
	{
    	rt = ret_cnt;
    }
	CHECK_DONE(rt);

	/* terminate transfer */
	ls2k_spi_send_stop(bus, GD25Q80_CS);

	/**************************************************
	 * Delay for Erase done
	 **************************************************/

	switch (cmd)
	{
		case SPI_MEM_CMD_SE4:	ms = 200;	break;
		case SPI_MEM_CMD_SE32:	ms = 800;	break;
		case SPI_MEM_CMD_SE64:
		default:				ms = 1000;	break;
	}

	/* poll flash sr-busy flag, until device is finished */
	rt = GD25Q80_wait_ms(bus, ms);
	CHECK_DONE(rt);

	/*
	 * change back to fast read mode.
	 */
	if (SPI_PARAM_MEMORY_EN & memory_en)
	{
		rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_FAST_READ_ENABLE, NULL);
		CHECK_DONE(rt);
	}

lbl_done:
	/* terminate transfer */
	ls2k_spi_send_stop(bus, GD25Q80_CS);

	return rt;
}

/*
 * spi-flash erase chip.
 */
static int GD25Q80_erase_chip(const void *bus)
{
    unsigned char cmd;
	unsigned int  memory_en;
	int rt, ret_cnt = 0;

	/******************************************************************
	 * check spi-flash is set to fast read mode
	 ******************************************************************/

	rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_GET_FAST_READ_MODE, &memory_en);
	CHECK_DONE(rt);

	/*
	 * change to not fast read mode.
	 */
	if (SPI_PARAM_MEMORY_EN & memory_en)
	{
		rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_FAST_READ_DISABLE, NULL);
		CHECK_DONE(rt);
	}

	/******************************************************************
	 * begin to erase chip.
	 * First we must set flash write enable
	 ******************************************************************/

	rt = GD25Q80_set_write_en(bus);
	CHECK_DONE(rt);

	/**************************************************
	 * Second we can do erase
	 **************************************************/

	/* start transfer */
	rt = ls2k_spi_send_start(bus, GD25Q80_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = -ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* address device */
	rt = ls2k_spi_send_addr(bus, GD25Q80_CS, true);
	CHECK_DONE(rt);

	/*
	 * send command.
	 */
	cmd = SPI_MEM_CMD_BE;
	ret_cnt = ls2k_spi_write_bytes(bus, &cmd, 1);
	if (ret_cnt < 0)
	{
    	rt = ret_cnt;
    }
	CHECK_DONE(rt);

	/* terminate transfer */
	ls2k_spi_send_stop(bus, GD25Q80_CS);

	/**************************************************
	 * Delay for Erase done
	 **************************************************/

	/* poll flash sr-busy flag, until device is finished */
	rt = GD25Q80_wait_ms(bus, 5000); // 4000
	CHECK_DONE(rt);

	/*
	 * change back to fast read mode.
	 */
	if (SPI_PARAM_MEMORY_EN & memory_en)
	{
		rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_FAST_READ_ENABLE, NULL);
		CHECK_DONE(rt);
	}

lbl_done:
	/* terminate transfer */
	ls2k_spi_send_stop(bus, GD25Q80_CS);

	return rt;
}

/*
 * check special spi-mem is blank
 */
static int GD25Q80_check_isblank(const void *bus, unsigned int addr, int *size)
{
    unsigned char *ptr;
    unsigned int  bblank = 1;
	int rt, off, bit, cnt;

	rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_FAST_READ_ENABLE, NULL);
	CHECK_DONE(rt);

	cnt = *size;
	bit = fls(cnt) - 1;

	off = (0xFFFFFFFF << bit) & addr;
	if ((cnt < 0) || (off > (m_chipParam.mem_size - cnt)))
	{
    	rt = -EADDRNOTAVAIL;
    }
	CHECK_DONE(rt);

	/*
	 * byte -> int -> byte to fast
	 */
	ptr = (void *)(unsigned long)(off + FLASH_BOOT_BASE);
	while (cnt-- > 0)
	{
		if ((unsigned char)*ptr != 0xFF)
		{
			bblank = 0;
			break;
		}
		ptr++;
	}

	*size = bblank;

lbl_done:
	return rt;
}

/**********************************************************************
 * spi flash driver impelememt
 **********************************************************************/

/**********************************************************************
 * Purpose: initialize
 * Input Parameters:
 * Return Value:		0 = ok or error code
 **********************************************************************/

STATIC_DRV int GD25Q80_init(const void *bus, void *arg)
{
    return 0;
}

/**********************************************************************
 * Purpose: write a block of data to flash
 * Input Parameters:
 * Return Value:		0 = ok or error code
 **********************************************************************/

STATIC_DRV int GD25Q80_write(const void *bus, void *buf, int size, void *arg)
{
	int rt, cmd_size;
	int curr_cnt, bytes_sent = 0, ret_cnt = 0;
	unsigned char cmdbuf[4], *pchbuf;
	unsigned int  memory_en, offset;

    if ((bus == NULL) || (buf == NULL) || (arg == NULL))
    {
        return -1;
    }

    offset = *(unsigned int *)arg;
    pchbuf = (unsigned char *)buf;

	/******************************************************************
	 * check spi-flash is set to fast read mode
	 ******************************************************************/

	rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_GET_FAST_READ_MODE, &memory_en);
	CHECK_DONE(rt);

	/*
	 * change to not fast read mode.
	 */
	if (SPI_PARAM_MEMORY_EN & memory_en)
	{
		rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_FAST_READ_DISABLE, NULL);
		CHECK_DONE(rt);
	}

	/* check arguments */
	if ((size <= 0) ||
		(size > m_chipParam.mem_size) ||
		(offset > (m_chipParam.mem_size - size)))
	{
		rt = -EFAULT;
	}
	else if (pchbuf == NULL)
	{
		rt = -EADDRNOTAVAIL;
	}
	CHECK_DONE(rt);

	/******************************************************************
	 * begin to write data, PP = 256 bytes.
	 ******************************************************************/

	while (size > bytes_sent)
	{
		/*
		 * mention: curr_cnt / page_size / off.
		 */
		curr_cnt = size - bytes_sent;
		if ((m_chipParam.page_size > 0) &&
			(offset / m_chipParam.page_size) !=
			((offset+curr_cnt+1) / m_chipParam.page_size))
		{
			curr_cnt = m_chipParam.page_size - (offset % m_chipParam.page_size);
		}

		/**************************************************
		 * First we must set flash write enable
		 * Here also has set the transfer mode.
		 **************************************************/

		rt = GD25Q80_set_write_en(bus);
		CHECK_DONE(rt);

		/**************************************************
		 * Second we can do page-program
		 **************************************************/

		/* start transfer */
		rt = ls2k_spi_send_start(bus, GD25Q80_CS);
		CHECK_DONE(rt);

		/* set transfer mode */
		rt = -ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
		CHECK_DONE(rt);

		/* address device */
		rt = ls2k_spi_send_addr(bus, GD25Q80_CS, true);
		CHECK_DONE(rt);

		/*
		 * send "page program" command and address
		 * remove the mem_size check
		 */
		cmdbuf[0] = SPI_MEM_CMD_PP;
		cmdbuf[1] = (offset >> 16) & 0xff;
		cmdbuf[2] = (offset >>  8) & 0xff;
		cmdbuf[3] = (offset >>  0) & 0xff;
		cmd_size  = 4;

		ret_cnt = ls2k_spi_write_bytes(bus, cmdbuf, cmd_size);
		if (ret_cnt < 0)
		{
        	rt = ret_cnt;
        }
		CHECK_DONE(rt);

		/*
		 * send write data
		 */
		ret_cnt = ls2k_spi_write_bytes(bus, pchbuf, curr_cnt);
		if (ret_cnt < 0)
		{
        	rt = ret_cnt;
        }
		CHECK_DONE(rt);

		/* terminate transfer */
		rt = ls2k_spi_send_stop(bus, GD25Q80_CS);
		CHECK_DONE(rt);

		/**************************************************
		 * Delay for page-program done
		 **************************************************/

		/* poll flash sr-busy flag, until device is finished */
		rt = GD25Q80_wait_ms(bus, 5);
		CHECK_DONE(rt);

		/* adjust bytecount to be sent and pointers */
		bytes_sent += curr_cnt;
		offset     += curr_cnt;
		pchbuf     += curr_cnt;
	}

	/*
	 * change back to fast read mode.
	 */
	if (SPI_PARAM_MEMORY_EN & memory_en)
	{
		rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_FAST_READ_ENABLE, NULL);
		CHECK_DONE(rt);
	}

//  *(unsigned int *)arg = offset;
	rt = bytes_sent;

lbl_done:
	/* terminate transfer */
	ls2k_spi_send_stop(bus, GD25Q80_CS);

	return rt;
}

/**********************************************************************
 * Purpose: read a block of data from flash
 * Input Parameters:
 * Return Value: 		0 = ok or error code
 **********************************************************************/

STATIC_DRV int GD25Q80_read(const void *bus, void *buf, int size, void *arg)
{
	int rt=0, cmd_size, ret_cnt = 0;
	unsigned char cmdbuf[4], *pchbuf;
	unsigned int  memory_en, offset;

    if ((bus == NULL) || (buf == NULL) || (arg == NULL))
    {
        return -1;
    }

    offset = *(unsigned int *)arg;
    pchbuf = (unsigned char *)buf;

	/* check arguments */
	if ((size <= 0)
		|| (size > m_chipParam.mem_size)
		|| (offset > (m_chipParam.mem_size - size)))
	{
		rt = -EFAULT;
	}
	else if (pchbuf == NULL)
	{
		rt = -EADDRNOTAVAIL;
	}
	CHECK_DONE(rt);

	/******************************************************************
	 * check spi-flash is set to fast read mode
	 ******************************************************************/

    rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_GET_FAST_READ_MODE, &memory_en);
	CHECK_DONE(rt);

    if ((offset + size < FLASH_BOOT_LIMIT) &&   /* 1M 快速读空间 */
        (SPI_PARAM_MEMORY_EN & memory_en))      /* 允许快速读 */
    {
		unsigned char *ptr;
		ptr = (void *)(unsigned long)(offset + FLASH_BOOT_BASE);

    	while (size-- > 0)
    	{
    		*pchbuf++ = *ptr++;			/* is here need delay ? */
    		ret_cnt++;
    	}

    	return ret_cnt;
    }
    else if (memory_en)
    {
        ls2k_spi_ioctl(bus, IOCTL_FLASH_FAST_READ_DISABLE, NULL);
    }

	/******************************************************************
	 * if spi-flash engine is not set...
	 ******************************************************************/

	/* start transfer */
	rt = ls2k_spi_send_start(bus, GD25Q80_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* select device - chipsel */
	rt = ls2k_spi_send_addr(bus, GD25Q80_CS, true);
	CHECK_DONE(rt);

	if (offset >= m_chipParam.mem_size)
	{
		/*
		 * HACK: beyond size of memory array? then read status register instead
		 */
		/*
		 * send read status register command
		 */
		cmdbuf[0] = SPI_MEM_CMD_RDSR;
		ret_cnt = ls2k_spi_write_bytes(bus, cmdbuf, 1);
		if (ret_cnt < 0)
		{
        	rt = ret_cnt;
        }
	}
	else
	{
		/*
		 * send read command and address
		 * remove the mem_size check
		 */
		cmdbuf[0] = SPI_MEM_CMD_READ;
		cmdbuf[1] = (offset >> 16) & 0xff;
		cmdbuf[2] = (offset >>  8) & 0xff;
		cmdbuf[3] = (offset >>  0) & 0xff;
		cmd_size  = 4;

		/*
		 * get read data
		 */
		ret_cnt = ls2k_spi_write_bytes(bus, cmdbuf, cmd_size);

		if (ret_cnt < 0)
		{
        	rt = ret_cnt;
        }
	}
	CHECK_DONE(rt);

	/*
	 * fetch read data
	 */
	ret_cnt = ls2k_spi_read_bytes(bus, pchbuf, size);
	if (ret_cnt < 0)
	{
    	rt = ret_cnt;
    }
	CHECK_DONE(rt);

	rt = ret_cnt;

lbl_done:
	/* terminate transfer */
	ls2k_spi_send_stop(bus, GD25Q80_CS);

    if (memory_en)
    {
        ls2k_spi_ioctl(bus, IOCTL_FLASH_FAST_READ_ENABLE, NULL);
    }
    
	return rt;
}

STATIC_DRV int GD25Q80_open(const void *bus, void *arg)
{
	return 0;
}

STATIC_DRV int GD25Q80_close(const void *bus, void *arg)
{
	return 0;
}

STATIC_DRV int GD25Q80_ioctl(const void *bus, int cmd, void *arg)
{
	int rt = 0;
	unsigned int val = 0;

    if (bus == NULL)
    {
        return -1;
    }

	switch (cmd)
	{
		case IOCTL_FLASH_FAST_READ_ENABLE:
			rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_FAST_READ_ENABLE, NULL);
			break;

		case IOCTL_FLASH_FAST_READ_DISABLE:
			rt = -ls2k_spi_ioctl(bus, IOCTL_FLASH_FAST_READ_DISABLE, NULL);
			break;

		case IOCTL_GD25Q80_READ_ID:
			rt = GD25Q80_read_id(bus, &val);
			*(unsigned int *)arg = val;
			break;

		case IOCTL_GD25Q80_READ_JDECID:
		case IOCTL_GD25Q80_READ_UNIQUEID:
			rt = -ENOTSUP;
			break;

		case IOCTL_GD25Q80_ERASE_4K:
			val = (unsigned long)arg;
			rt = GD25Q80_erase_1_sector(bus, SPI_MEM_CMD_SE4, val);
			break;

		case IOCTL_GD25Q80_ERASE_32K:
			val = (unsigned long)arg;
			rt = GD25Q80_erase_1_sector(bus, SPI_MEM_CMD_SE32, val);
			break;

		case IOCTL_GD25Q80_ERASE_64K:
			val = (unsigned long)arg;
			rt = GD25Q80_erase_1_sector(bus, SPI_MEM_CMD_SE64, val);
			break;

		case IOCTL_GD25Q80_SECTOR_ERASE:
			val = (unsigned long)arg;
			rt = GD25Q80_erase_1_sector(bus, SPI_MEM_CMD_SE, val);
			break;

		case IOCTL_GD25Q80_BULK_ERASE:
			rt = GD25Q80_erase_chip(bus);
			break;

		case IOCTL_GD25Q80_WRITE_PROTECT:
			rt = -ENOTSUP;
			break;

		default:
			if (cmd & IOCTL_GD25Q80_IS_BLANK)
			{
				int size;

				if (cmd & IOCTL_GD25Q80_ERASE_64K)
					size = 64 * 1024;
				else if (cmd & IOCTL_GD25Q80_ERASE_32K)
					size = 32 * 1024;
				else 					// default is IOCTL_GD25Q80_ERASE_4K
					size = 4 * 1024;

				val = *(unsigned int *)arg;
				rt = GD25Q80_check_isblank(bus, val, &size);
				if (0 == rt)
				{
                	*((unsigned int *)arg) = (unsigned int)size;
                }
			}
			else
			{
            	return -ENOTSUP;
            }

			break;
	}

	return rt;
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * SPI0-GD25Q80 driver operators
 */
static const driver_ops_t _GD25Q80_drv_ops =
{
    .init_entry  = GD25Q80_init,
    .open_entry  = GD25Q80_open,
    .close_entry = GD25Q80_close,
    .read_entry  = GD25Q80_read,
    .write_entry = GD25Q80_write,
    .ioctl_entry = GD25Q80_ioctl,
};

const driver_ops_t *gd25q80_drv_ops = &_GD25Q80_drv_ops;
#endif

#endif // #if GD25Q80_DRV

/*
 * @@ END
 */
 



