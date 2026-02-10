/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * norflash.c
 *
 * created: 2024-06-23
 *  author: Bian
 */

#include "bsp.h"

#if NORFLASH_DRV

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "ls2k300.h"
#include "ls2k_spi_bus.h"
#include "../ls2k_spi_bus_hw.h"

#include "spi/norflash.h"
#include "norflash_hw.h"

#include "osal.h"

//-----------------------------------------------------------------------------

#define FLASH_BOOT_BASE			0x800000001C000000ULL

/**
 * 保留尾部 0x10000 = 64K 用来编程测试
 */
#define FLASH_BOOT_LIMIT       (NORFLASH_CHIP_SIZE - 0x10000)

#define SPI_MEM_CMD_RDSR		NORFLASH_CMD_RDSR		// 0x05
#define SPI_MEM_CMD_WREN		NORFLASH_CMD_WREN		// 0x06
#define SPI_MEM_CMD_PP			NORFLASH_CMD_PP			// 0x02
#define SPI_MEM_CMD_READ		NORFLASH_CMD_READ		// 0x03

#define SPI_MEM_CMD_RDID		NORFLASH_CMD_RDMDID		// 0x90
#define SPI_MEM_CMD_RDJEDECID	NORFLASH_CMD_RDJEDECID	// 0x9F
#define SPI_MEM_CMD_RDUNIQEID	NORFLASH_CMD_RDUNIQEID	// 0x4B

#define SPI_MEM_CMD_SE			NORFLASH_CMD_SE4K		// 0x20  // sector erase
#define SPI_MEM_CMD_SE4			SPI_MEM_CMD_SE
#define SPI_MEM_CMD_SE32		NORFLASH_CMD_BE32K		// 0x52	 // block erase, 32KB
#define SPI_MEM_CMD_SE64		NORFLASH_CMD_BE64K		// 0xD8	 // block erase, 64KB
#define SPI_MEM_CMD_BE			NORFLASH_CMD_CE			// 0xC7  // bulk erase

#define SPI_FLASH_BAUDRATE		NORFLASH_BAUDRATE
#define SPI_FLASH_BITSPERCHAR	NORFLASH_BITSPERCHAR
#define SPI_FLASH_PAGE_SIZE		NORFLASH_PAGE_SIZE
#define SPI_FLASH_BLOCK_SIZE	NORFLASH_BLOCK_SIZE_64
#define SPI_FLASH_CHIP_SIZE		NORFLASH_CHIP_SIZE

#define SPI_FLASH_SR_BUSY_BIT	norflash_sr_busy

//-----------------------------------------------------------------------------

#define CHECK_DONE(rt) \
	do {               \
        if (0 != rt)   \
            goto lbl_done; \
    } while (0);

//-----------------------------------------------------------------------------
// device
//-----------------------------------------------------------------------------

/*
 * norflash 片选
 */
#define NORFLASH_CS           0

/*
 * norflash 芯片参数
 */
static NORFLASH_param_t m_chipParam =
{
    .baudrate             = SPI_FLASH_BAUDRATE,         /* 速率受SPI 限制 */
    .erase_before_program = true,
    .empty_state          = 0xff,
    .page_size            = SPI_FLASH_PAGE_SIZE,        /* programming page size in byte */
    .sector_size          = SPI_FLASH_BLOCK_SIZE,       /* 64K erase sector size in byte */
    .mem_size             = SPI_FLASH_CHIP_SIZE,        /* 4M total capacity in byte */
};

/*
 * norflash 通信模式
 */
static SPI_mode_t m_devMode =
{
	.baudrate = 10000000,                       /* 实际用的速率: 10M */
	.bits_per_char = SPI_FLASH_BITSPERCHAR,     /* how many bits per byte/word/longword? */
	.lsb_first = false,                         /* true: send LSB first */
	.clock_pha = true,                          /* clock phase    - spi mode */
	.clock_pol = true,                          /* clock polarity - spi mode */
	.clock_inv = false,                         /* low active - true: inverted clock (high active) */
	.clock_phs = false,                         /* true: clock starts toggling at start of data tfr - interface mode */
};

//-----------------------------------------------------------------------------

/*
 * read NORFLASHBV manufacturer / device ID
 */
static int Norflash_read_id(const void *bus, unsigned int *id)
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
	rt = ls2k_spi_send_start(bus, NORFLASH_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* address device */
	rt = ls2k_spi_send_addr(bus, NORFLASH_CS, true);
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
	ls2k_spi_send_stop(bus, NORFLASH_CS);

	return rt;
}

static int Norflash_read_jdecid(const void *bus, unsigned int *id)
{
	unsigned char cmd;
	unsigned char val[3];
	int	rt, ret_cnt = 0;

	/******************************************************************
	 * begin to read id
	 ******************************************************************/

	/* start transfer */
	rt = ls2k_spi_send_start(bus, NORFLASH_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* address device */
	rt = ls2k_spi_send_addr(bus, NORFLASH_CS, true);
	CHECK_DONE(rt);

	/*
	 * send "read md/id" command and address
	 */
	cmd = SPI_MEM_CMD_RDJEDECID;

	ret_cnt = ls2k_spi_write_bytes(bus, &cmd, 1);
	if (ret_cnt < 0)
	{
    	rt = ret_cnt;
    }
	CHECK_DONE(rt);

	/*
	 * fetch read data
	 */
	ret_cnt = ls2k_spi_read_bytes(bus, val, 3);
	if (ret_cnt < 0)
	{
    	rt = ret_cnt;
    }
	CHECK_DONE(rt);

	*id = (val[0] << 16) | (val[1] << 8) | val[2];

lbl_done:
	/* terminate transfer */
	ls2k_spi_send_stop(bus, NORFLASH_CS);

	return rt;
}

static int Norflash_read_uniqueid(const void *bus, unsigned long *id)
{
	unsigned char cmd[5];
	unsigned char val[8];
	int	rt, ret_cnt = 0;

	/******************************************************************
	 * begin to read id
	 ******************************************************************/

	/* start transfer */
	rt = ls2k_spi_send_start(bus, NORFLASH_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* address device */
	rt = ls2k_spi_send_addr(bus, NORFLASH_CS, true);
	CHECK_DONE(rt);

	/*
	 * send "read md/id" command and address
	 */
	cmd[0] = SPI_MEM_CMD_RDUNIQEID;
	cmd[1] = 0;
	cmd[2] = 0;
	cmd[3] = 0;
	cmd[4] = 0;

	ret_cnt = ls2k_spi_write_bytes(bus, cmd, 5);
	if (ret_cnt < 0)
	{
    	rt = ret_cnt;
    }
	CHECK_DONE(rt);

	/*
	 * fetch read data
	 */
	ret_cnt = ls2k_spi_read_bytes(bus, val, 8);
	if (ret_cnt < 0)
	{
    	rt = ret_cnt;
    }
	CHECK_DONE(rt);

    *id = ((unsigned long)(val[0] & 0xFF) << 56) |
    	  ((unsigned long)(val[1] & 0xFF) << 48) |
		  ((unsigned long)(val[2] & 0xFF) << 40) |
		  ((unsigned long)(val[3] & 0xFF) << 32) |
		  ((unsigned long)(val[4] & 0xFF) << 24) |
		  ((unsigned long)(val[5] & 0xFF) << 16) |
		  ((unsigned long)(val[6] & 0xFF) << 8)  |
		  ((unsigned long)(val[7] & 0xFF) << 0);

lbl_done:
	/* terminate transfer */
	ls2k_spi_send_stop(bus, NORFLASH_CS);

	return rt;
}

/*
 * read NORFLASHBV status register
 */
#define RDSR_CMD_BYTES  0

static int Norflash_read_sr(const void *bus, unsigned int *sr)
{
#if RDSR_CMD_BYTES == 4
	unsigned char cmd[4], val[4];
#else
	unsigned char cmd, val;
#endif
	int rt, ret_cnt = 0;

	/* start transfer */
	rt = ls2k_spi_send_start(bus, NORFLASH_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* select device - chipsel */
	rt = ls2k_spi_send_addr(bus, NORFLASH_CS, true);
	CHECK_DONE(rt);

	/*
	 * send read status register command
	 */
#if RDSR_CMD_BYTES == 4
	cmd[0] = cmd[1] = cmd[2] = cmd[3] = SPI_MEM_CMD_RDSR;
	ret_cnt = ls2k_spi_write_bytes(bus, cmd, 4);
#else
	cmd = SPI_MEM_CMD_RDSR;
	ret_cnt = ls2k_spi_write_bytes(bus, &cmd, 1);
#endif
	if (ret_cnt < 0)
	{
    	rt = ret_cnt;
    }
	CHECK_DONE(rt);

	/*
	 * fetch read data
	 */
#if RDSR_CMD_BYTES == 4
	ret_cnt = ls2k_spi_read_bytes(bus, val, 4);
#else
	ret_cnt = ls2k_spi_read_bytes(bus, &val, 1);
#endif
	if (ret_cnt < 0)
	{
    	rt = ret_cnt;
    }
	CHECK_DONE(rt);

#if RDSR_CMD_BYTES == 4
	*sr = val[0] | (val[1] << 8) | (val[2] << 16) | (val[3] << 24);
#else
	*sr = val;
#endif

lbl_done:
	/* terminate transfer */
	ls2k_spi_send_stop(bus, NORFLASH_CS);

	return rt;
}

static int Norflash_is_busy(const void *bus)
{
	unsigned int sr;

	if (Norflash_read_sr(bus, &sr) != 0)
	{
		return -1;
	}

	return (sr & NORFLASH_SR_BUSY);
}

/*
 * wait for x ms for spi-flash work done.
 */
static int Norflash_wait_ms(const void *bus, unsigned int ms)
{
	while (ms-- > 0)
	{
		osal_msleep(1);     /* delay 1 ms per loop, if done, exit immediately */

		if (0 == Norflash_is_busy(bus)) // TODO test dead lock ?
		{
            return 0;
        }
	}

	return -ETIMEDOUT;
}

/*
 * set spi-flash writable.
 */
static int Norflash_set_write_en(const void *bus)
{
	unsigned char cmd;
	int rt, ret_cnt = 0;

	/* start transfer */
	rt = ls2k_spi_send_start(bus, NORFLASH_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* select device - chipsel */
	rt = ls2k_spi_send_addr(bus, NORFLASH_CS, true);
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
	ls2k_spi_send_stop(bus, NORFLASH_CS);

	return rt;
}

/*
 * spi-flash erase 1 sector.
 */
static int Norflash_erase_1_sector(const void *bus, unsigned char cmd, unsigned int addr)
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

	rt = Norflash_set_write_en(bus);
	CHECK_DONE(rt);

	/**************************************************
	 * Second we can do page-program
	 **************************************************/

	/* start transfer */
	rt = ls2k_spi_send_start(bus, NORFLASH_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = -ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* address device */
	rt = ls2k_spi_send_addr(bus, NORFLASH_CS, true);
	CHECK_DONE(rt);

	/*
	 * send command and address,
	 * command may be:
	 * NORFLASH_CMD_SE		0x20	 sector erase, 4KB,  CMD, A23-A16, A15-A8, A7-A0
	 * NORFLASH_CMD_BE32		0x52	 block erase, 32KB,  CMD, A23-A16, A15-A8, A7-A0
	 * NORFLASH_CMD_BE64		0xD8	 block erase, 64KB,  CMD, A23-A16, A15-A8, A7-A0
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
	ls2k_spi_send_stop(bus, NORFLASH_CS);

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
	rt = Norflash_wait_ms(bus, ms);
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
	ls2k_spi_send_stop(bus, NORFLASH_CS);

	return rt;
}

/*
 * spi-flash erase chip.
 */
static int Norflash_erase_chip(const void *bus)
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

	rt = Norflash_set_write_en(bus);
	CHECK_DONE(rt);

	/**************************************************
	 * Second we can do erase
	 **************************************************/

	/* start transfer */
	rt = ls2k_spi_send_start(bus, NORFLASH_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = -ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* address device */
	rt = ls2k_spi_send_addr(bus, NORFLASH_CS, true);
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
	ls2k_spi_send_stop(bus, NORFLASH_CS);

	/**************************************************
	 * Delay for Erase done
	 **************************************************/

	/* poll flash sr-busy flag, until device is finished */
	rt = Norflash_wait_ms(bus, 4000);
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
	ls2k_spi_send_stop(bus, NORFLASH_CS);

	return rt;
}

/*
 * check special spi-mem is blank
 */
extern int fls(int x);

static int Norflash_check_isblank(const void *bus, unsigned int addr, int *size)
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

STATIC_DRV int NORFLASH_initialize(const void *bus, void *arg)
{
    return 0;
}

/**********************************************************************
 * Purpose: write a block of data to flash
 * Input Parameters:
 * Return Value:		0 = ok or error code
 **********************************************************************/

STATIC_DRV int NORFLASH_write(const void *bus, void *buf, int size, void *arg)
{
	int rt=0, cmd_size;
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

		rt = Norflash_set_write_en(bus);
		CHECK_DONE(rt);

		/**************************************************
		 * Second we can do page-program
		 **************************************************/

		/* start transfer */
		rt = ls2k_spi_send_start(bus, NORFLASH_CS);
		CHECK_DONE(rt);

		/* set transfer mode */
		rt = -ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
		CHECK_DONE(rt);

		/* address device */
		rt = ls2k_spi_send_addr(bus, NORFLASH_CS, true);
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
		rt = ls2k_spi_send_stop(bus, NORFLASH_CS);
		CHECK_DONE(rt);

		/**************************************************
		 * Delay for page-program done
		 **************************************************/

		/* poll flash sr-busy flag, until device is finished */
		rt = Norflash_wait_ms(bus, 5);
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
	ls2k_spi_send_stop(bus, NORFLASH_CS);

	return rt;
}

/**********************************************************************
 * Purpose: read a block of data from flash
 * Input Parameters:
 * Return Value: 		0 = ok or error code
 **********************************************************************/

STATIC_DRV int NORFLASH_read(const void *bus, void *buf, int size, void *arg)
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

    if ((offset + size < FLASH_BOOT_LIMIT) &&   /* XXX bytes 快速读空间 */
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
	rt = ls2k_spi_send_start(bus, NORFLASH_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls2k_spi_ioctl(bus, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* select device - chipsel */
	rt = ls2k_spi_send_addr(bus, NORFLASH_CS, true);
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
	ls2k_spi_send_stop(bus, NORFLASH_CS);

    if (memory_en)
    {
        ls2k_spi_ioctl(bus, IOCTL_FLASH_FAST_READ_ENABLE, NULL);
    }
    
	return rt;
}

STATIC_DRV int NORFLASH_open(const void *bus, void *arg)
{
	return 0;
}

STATIC_DRV int NORFLASH_close(const void *bus, void *arg)
{
	return 0;
}

#define ARG_NULL_BREAK(arg)     if (NULL==arg) { errno = EINVAL; break; }

STATIC_DRV int NORFLASH_ioctl(const void *bus, int cmd, void *arg)
{
	int rt = 0;
	unsigned int val32 = 0;
    unsigned long val64 = 0;
    
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

		case IOCTL_NORFLASH_READ_ID:
		    ARG_NULL_BREAK(arg);
			rt = Norflash_read_id(bus, &val32);
			*(unsigned int *)arg = val32;
			break;

		case IOCTL_NORFLASH_READ_JDECID:
		    ARG_NULL_BREAK(arg);
			rt = Norflash_read_jdecid(bus, &val32);
			*(unsigned int *)arg = val32;
			break;

		case IOCTL_NORFLASH_READ_UNIQUEID:
		    ARG_NULL_BREAK(arg);
			rt = Norflash_read_uniqueid(bus, &val64);
			*(unsigned long *)arg = val64;
			break;

		case IOCTL_NORFLASH_ERASE_4K:
			val32 = (unsigned long)arg;
			rt = Norflash_erase_1_sector(bus, SPI_MEM_CMD_SE4, val32);
			break;

		case IOCTL_NORFLASH_ERASE_32K:
			val32 = (unsigned long)arg;
			rt = Norflash_erase_1_sector(bus, SPI_MEM_CMD_SE32, val32);
			break;

		case IOCTL_NORFLASH_ERASE_64K:
			val32 = (unsigned long)arg;
			rt = Norflash_erase_1_sector(bus, SPI_MEM_CMD_SE64, val32);
			break;

		case IOCTL_NORFLASH_SECTOR_ERASE:
			val32 = (unsigned long)arg;
			rt = Norflash_erase_1_sector(bus, SPI_MEM_CMD_SE, val32);
			break;

		case IOCTL_NORFLASH_BULK_ERASE:
			rt = Norflash_erase_chip(bus);
			break;

		case IOCTL_NORFLASH_WRITE_PROTECT:
			rt = -ENOTSUP;
			break;

		default:
			if (cmd & IOCTL_NORFLASH_IS_BLANK)
			{
				int size;
				
				ARG_NULL_BREAK(arg);

				if (cmd & IOCTL_NORFLASH_ERASE_64K)
					size = 64 * 1024;
				else if (cmd & IOCTL_NORFLASH_ERASE_32K)
					size = 32 * 1024;
				else 					// default is IOCTL_NORFLASH_ERASE_4K
					size = 4 * 1024;

				val32 = *(unsigned int *)arg;
				rt = Norflash_check_isblank(bus, val32, &size);
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
 * SPI0-NORFLASH driver operators
 */
static const driver_ops_t ls2k_norflash_drv_ops =
{
    .init_entry  = NORFLASH_initialize,
    .open_entry  = NORFLASH_open,
    .close_entry = NORFLASH_close,
    .read_entry  = NORFLASH_read,
    .write_entry = NORFLASH_write,
    .ioctl_entry = NORFLASH_ioctl,
};

const driver_ops_t *norflash_drv_ops = &ls2k_norflash_drv_ops;
#endif

//-----------------------------------------------------------------------------
// User API
//-----------------------------------------------------------------------------

#endif // #if NORFLASH_DRV

//-----------------------------------------------------------------------------

/*
 * @@ END
 */


