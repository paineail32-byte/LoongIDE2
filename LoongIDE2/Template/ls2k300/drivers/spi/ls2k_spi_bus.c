/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_spi_bus.c
 *
 * created: 2024-06-23
 *  author: Bian
 */

#include "bsp.h"

#if (BSP_USE_SPI)

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "ls2k300.h"
#include "ls2k300_irq.h"

#include "osal.h"

#include "ls2k_drv_io.h"
#include "drv_os_priority.h"

#include "ls2k_spi_bus_hw.h"
#include "ls2k_spi_bus.h"

//-----------------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------------

#define LOCK(p)     osal_mutex_obtain(p->p_mutex, OSAL_WAIT_FOREVER)
#define UNLOCK(p)   osal_mutex_release(p->p_mutex)

//-----------------------------------------------------------------------------
// device soft control
//-----------------------------------------------------------------------------

/*
 * SPI bus
 */
typedef struct
{
    HW_SPI_t   *hwSPI;              /* pointer to HW registers */
    SPI_mode_t _mode;
    /* hardware parameters */
    unsigned int chipsel_nums;      /* total chip select numbers */
    unsigned int chipsel_high;      /* cs high level - XXX: value from tfr-mode */
    unsigned int irqNum;            /* interrupt num */
    
    osal_mutex_t p_mutex;           /* mutex */

    int  initialized;
    char dev_name[16];
} SPI_bus_t;

//-----------------------------------------------------------------------------
// 分频系数
//-----------------------------------------------------------------------------

typedef struct
{
	unsigned char	spre;
	unsigned char	spr;
	unsigned int    sprate;
} CLKDIV_table_t;

const static CLKDIV_table_t clk_table[12] =
{
    { .spre = 0b00, .spr = 0b00, .sprate = 2,    },
    { .spre = 0b00, .spr = 0b01, .sprate = 4,    },
    { .spre = 0b01, .spr = 0b00, .sprate = 8,    },
    { .spre = 0b00, .spr = 0b10, .sprate = 16,   },
    { .spre = 0b00, .spr = 0b11, .sprate = 32,   },
    { .spre = 0b01, .spr = 0b01, .sprate = 64,   },
    { .spre = 0b01, .spr = 0b10, .sprate = 128,  },
    { .spre = 0b01, .spr = 0b11, .sprate = 256,  },
    { .spre = 0b10, .spr = 0b00, .sprate = 512,  },
    { .spre = 0b10, .spr = 0b01, .sprate = 1024, },
    { .spre = 0b10, .spr = 0b10, .sprate = 2048, },
    { .spre = 0b10, .spr = 0b11, .sprate = 4096, },
};

/*
 * time-out, xxx how long is fit ?
 */
#define DEFAULT_TIMEOUT 	1000

/******************************************************************************
 * SPI hardware
 ******************************************************************************/
 
#define BAUDRATE_MAX    10000000        /* 10M */
#define BAUDRATE_MIN    50000           /* 50K */

static int ls2k_spi_baud_to_mode(unsigned int baudrate,
					             unsigned char *spre,
                                 unsigned char *spr)
{
    int i;
    unsigned int divider;

    if (baudrate < BAUDRATE_MIN)
        baudrate = BAUDRATE_MIN;
    else if (baudrate > BAUDRATE_MAX)
        baudrate = BAUDRATE_MAX;
        
    divider = apb_frequency / baudrate;
    if ((apb_frequency % baudrate) > 0)
    {
        divider += 1;
    }

	if (divider < 2)
	{
    	divider = 2;
    }

	if (divider > 4096)
	{
    	divider = 4096;
    }

    *spre = 0;
    *spr  = 0;

    for (i=0; i<12; i++)
    {
        if (clk_table[i].sprate >= divider)
        {
            *spre = clk_table[i].spre;
            *spr  = clk_table[i].spr;
            break;
        }
    }

	return 0;
}

static int ls2k_spi_set_tfr_mode(SPI_bus_t *pSPI, SPI_mode_t *new_mode)
{
	unsigned char spre, spr, val;
    SPI_mode_t *mode = &pSPI->_mode;
    
    if ((mode->baudrate == new_mode->baudrate) &&
        (mode->lsb_first == new_mode->lsb_first) &&
        (mode->clock_pha == new_mode->clock_pha) &&
        (mode->clock_pol == new_mode->clock_pol) &&
        (mode->clock_inv == new_mode->clock_inv) &&
        (mode->clock_phs == new_mode->clock_phs))
    {
        return 0;
    }

    pSPI->_mode = *new_mode;

    if (mode->bits_per_char != 8)
    {
        mode->bits_per_char = 8;
    }

	/*
	 * according baudrate, calculate the proper frequency
	 */
	ls2k_spi_baud_to_mode(mode->baudrate, &spre, &spr);

	pSPI->hwSPI->ctrl &= ~SPI_CTRL_EN;  	    /* disable spi */

	/* set spi-er register */
	val = spre & SPI_ER_SPRE_MASK;
	if (!mode->clock_phs)						/* = true: 同步发送 */
	{
		val |= SPI_ER_MODE;
	}
	pSPI->hwSPI->er = val;

	/* set spi-param register */
	val = pSPI->hwSPI->param;
    val &= ~SPI_PARAM_CLKDIV_MASK;
    val |= (spre << 6) | (spr << 4);
    pSPI->hwSPI->param = val;

	/* set spi-control register - spi mode */
	val = SPI_CTRL_MASTER | (spr & SPI_CTRL_SPR_MASK);
	if (mode->clock_pha)
	{
		val |= SPI_CTRL_CPHA;
	}
	if (mode->clock_pol)
	{
		val |= SPI_CTRL_CPOL;
	}

	/* set new control value and re-enable spi */
	pSPI->hwSPI->ctrl = val | SPI_CTRL_EN;

	/* set spi-sr register, clear flag */
	pSPI->hwSPI->sr = SPI_SR_IFLAG | SPI_SR_WOVERFLOW;

	/* chip select mode */
	pSPI->chipsel_high = mode->clock_inv;

	return 0;
}

static int ls2k_spi_wait_tx_done(SPI_bus_t *pSPI)
{
	register unsigned int tmo = 0;

	/*
	 * Wait for SPI to terminate
	 */
	while ((pSPI->hwSPI->sr & SPI_SR_IFLAG) == 0)
	{
		if (tmo++ > DEFAULT_TIMEOUT)
		{
			printk("SPI tmo!\r\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int ls2k_spi_read_write_bytes(SPI_bus_t *pSPI,
						             unsigned char *rxbuf,
						             const unsigned char *txbuf,
						             int len)
{
	int rt, rw_cnt = 0;
	unsigned char rx_val;

	/* Clear interrupt and txoverflow flag */
	pSPI->hwSPI->sr |= (SPI_SR_IFLAG | SPI_SR_WOVERFLOW);

	/* Allow interrupt */
	pSPI->hwSPI->ctrl |= SPI_CTRL_IEN;

	while (len-- > 0)
	{
		if (txbuf == NULL)
		{
			/* write dummy char while read. */
			pSPI->hwSPI->data = 0x5A;
		}
		else
		{
			pSPI->hwSPI->data = *(unsigned char *)txbuf;
			txbuf++;
		}

		/*
		 * wait until end of transfer
		 */
		rt = ls2k_spi_wait_tx_done(pSPI);

		if (0 != rt)
		{
			printk("SPI rw tmo.\r\n");
			return -rt;
		}

		rx_val = pSPI->hwSPI->data;
		if (rxbuf != NULL)
		{
			(*(unsigned char *)rxbuf) = rx_val;
			rxbuf++;
		}

		/*
		 * Clear Interrupt and txoverflow flag
		 */
		pSPI->hwSPI->sr |= (SPI_SR_IFLAG | SPI_SR_WOVERFLOW);

		rw_cnt++;
	}

	/* Disable interrupt */
	pSPI->hwSPI->ctrl &= ~SPI_CTRL_IEN;

	return rw_cnt;
}

/******************************************************************************
 * SPI driver Implement
 ******************************************************************************/

extern int ls2k_spi_init_hook(const void *bus);

#if BSP_USE_SPI0 && BSP_USE_SPI1
#define VALID_ARG_BUS(bus)  if (!((bus == busSPI0) || (bus == busSPI1))) \
                            { errno = EINVAL; return -1; }
#elif BSP_USE_SPI0
#define VALID_ARG_BUS(bus)  if (!(bus == busSPI0)) { errno = EINVAL; return -1; }
#elif BSP_USE_SPI1          
#define VALID_ARG_BUS(bus)  if (!(bus == busSPI1)} { errno = EINVAL; return -1; }
#else
#define VALID_ARG_BUS(bus)
#endif

STATIC_DRV int SPI_initialize(const void *bus)
{
    SPI_bus_t *pSPI = (SPI_bus_t *)bus;

    VALID_ARG_BUS(bus);

    if (pSPI->initialized)
    {
        return 0;
    }

    ls2k_spi_init_hook(bus);
    
    pSPI->p_mutex = osal_mutex_create(pSPI->dev_name, OSAL_OPT_FIFO);
    if (pSPI->p_mutex == NULL)
    {
        printk("create mutex for % fail\r\n", pSPI->dev_name);
    	return -1;
    }

	/*
	 * init HW registers:
	 */

	/* disable interrupt output */
	pSPI->hwSPI->ctrl &= ~SPI_CTRL_IEN;

	/* set spi-sr register, clear flag */
	pSPI->hwSPI->sr = SPI_SR_BUSY | SPI_SR_WOVERFLOW;

	/* disable SPI interrupt
	 */
    ls2k_interrupt_disable(pSPI->irqNum);

    pSPI->initialized = 1;

    printk("SPI%i controller initialized.\r\n", \
           (VA_TO_PHYS(pSPI->hwSPI) == SPI0_BASE) ? 0 : \
           (VA_TO_PHYS(pSPI->hwSPI) == SPI1_BASE) ? 1 : -1);

	return 0;
}

STATIC_DRV int SPI_read_bytes(const void *bus, unsigned char *buf, int len)
{
    SPI_bus_t *pSPI = (SPI_bus_t *)bus;

    VALID_ARG_BUS(bus);

	return ls2k_spi_read_write_bytes(pSPI, buf, NULL, len);
}

/*
 * send some bytes to SPI device
 * Input Parameters:	bus specifier structure
 * 						buffer to send
 * 						number of bytes to send
 */
STATIC_DRV int SPI_write_bytes(const void *bus, unsigned char *buf, int len)
{
    SPI_bus_t *pSPI = (SPI_bus_t *)bus;

    VALID_ARG_BUS(bus);

	return ls2k_spi_read_write_bytes(pSPI, NULL, buf, len);
}

/*
 * according chip-select to send begin r/w op
 */
STATIC_DRV int SPI_send_start(const void *bus, unsigned int Addr)
{
	unsigned char chip_sel;
    SPI_bus_t *pSPI = (SPI_bus_t *)bus;

    VALID_ARG_BUS(bus);

	LOCK(pSPI);
	
	Addr &= 0x03;							/* (sc_ptr->chipsel_nums - 1); */
	chip_sel = 1 << Addr;					/* set chip_sel enable */

	if (!pSPI->chipsel_high)				/* De-select chip */
	{
    	chip_sel |= 1 << (Addr + 4);
    }

	pSPI->hwSPI->softcs = chip_sel;

    return 0;
}

/*
 * spi chip select
 */
STATIC_DRV int SPI_send_addr(const void *bus, unsigned int Addr, int rw)
{
	unsigned char chip_sel;
    SPI_bus_t *pSPI = (SPI_bus_t *)bus;

    VALID_ARG_BUS(bus);

	Addr &= 0x03;						    /* address range 0~3 */
	chip_sel = pSPI->hwSPI->softcs & 0x0F;

	if (pSPI->chipsel_high)				    /* Select chip */
	{
    	chip_sel |= 1 << (Addr + 4);
    }
	else
	{
    	chip_sel &= ~(1 << (Addr + 4));
    }

    pSPI->hwSPI->softcs = chip_sel;

	return 0;
}

/*
 * according chip-select to send finish r/w op
 */
STATIC_DRV int SPI_send_stop(const void *bus, unsigned int Addr)
{
	unsigned char chip_sel;
    SPI_bus_t *pSPI = (SPI_bus_t *)bus;

    VALID_ARG_BUS(bus);

    Addr &= 0x03;							/* (sc_ptr->chipsel_nums - 1); */
    chip_sel = pSPI->hwSPI->softcs & 0x0F;

    if (pSPI->chipsel_high)					/* De-select chip */
    {
    	chip_sel &= ~(1 << (Addr + 4));
    }
    else
    {
    	chip_sel |= 1 << (Addr + 4);
    }

    pSPI->hwSPI->softcs = chip_sel;

    UNLOCK(pSPI);
	return 0;
}

STATIC_DRV int SPI_ioctl(const void *bus, int cmd, void *arg)
{
	SPI_bus_t *pSPI = (SPI_bus_t *)bus;
    unsigned int val;
	int rt = 0;

    VALID_ARG_BUS(bus);

	switch (cmd)
	{
		case IOCTL_SPI_I2C_SET_TFRMODE:
		    if (arg)
			    rt = -ls2k_spi_set_tfr_mode(pSPI, (SPI_mode_t *)arg);
            else
                rt = -1;
			break;

		case IOCTL_FLASH_FAST_READ_ENABLE:
        /*
			pSPI->hwSPI->timing = spi_timing_tcsh_2;
			pSPI->hwSPI->param |= spi_param_fast_read |
					              spi_param_burst_en |
								  spi_param_memory_en;
         */
            pSPI->hwSPI->param |= SPI_PARAM_MEMORY_EN;
			break;

		case IOCTL_FLASH_FAST_READ_DISABLE:
		    pSPI->hwSPI->param &= ~SPI_PARAM_MEMORY_EN;
			break;

		case IOCTL_FLASH_GET_FAST_READ_MODE:
            val  = (unsigned)pSPI->hwSPI->param;
            val &= SPI_PARAM_MEMORY_EN;
			*(unsigned int *)arg = val;
			break;

		default:
			rt = -1;
			break;
	}

	return rt;
}

//-----------------------------------------------------------------------------
// SPI bus driver ops
//-----------------------------------------------------------------------------

#if (PACK_DRV_OPS)
static const libspi_ops_t ls2k_spi_drv_ops =
{
    .init        = SPI_initialize,
    .send_start  = SPI_send_start,
    .send_stop   = SPI_send_stop,
    .send_addr   = SPI_send_addr,
    .read_bytes  = SPI_read_bytes,
    .write_bytes = SPI_write_bytes,
    .ioctl       = SPI_ioctl,
};
const libspi_ops_t *spi_drv_ops = &ls2k_spi_drv_ops;
#endif

//-----------------------------------------------------------------------------
// SPI bus device table
//-----------------------------------------------------------------------------

#if BSP_USE_SPI0
static SPI_bus_t ls2k_SPI0 =
{
    .hwSPI        = (HW_SPI_t *)PHYS_TO_UNCACHED(SPI0_BASE),
#if USE_EXTINT
    .irqNum       = EXTI2_SPI0_IRQ,
#else
    .irqNum       = INTC1_SPI0_IRQ,
#endif
    .chipsel_nums = 4,
	.chipsel_high = 0,
    .initialized  = 0,
    .dev_name     = "spi0",
};
const void *busSPI0 = &ls2k_SPI0;
#endif

#if BSP_USE_SPI1
static SPI_bus_t ls2k_SPI1 =
{
    .hwSPI        = (HW_SPI_t *)PHYS_TO_UNCACHED(SPI1_BASE),
#if USE_EXTINT
    .irqNum       = EXTI2_SPI1_IRQ,
#else
    .irqNum       = INTC1_SPI1_IRQ,
#endif
    .chipsel_nums = 4,
	.chipsel_high = 0,
    .initialized  = 0,
    .dev_name     = "spi1",
};
const void *busSPI1 = &ls2k_SPI1;
#endif

/******************************************************************************
 * for external enable/disable spiflash fastread mode.
 */
int ls2k_spiflash_fastread_enable(const void *bus)
{
    SPI_bus_t *pSPI = (SPI_bus_t *)bus;
    
    LOCK(pSPI);
	pSPI->hwSPI->timing = SPI_TIMING_tCSH_2T;
	pSPI->hwSPI->param |= SPI_PARAM_FAST_READ | SPI_PARAM_BURST_EN | SPI_PARAM_MEMORY_EN;
    UNLOCK(pSPI);

	return 0;
}

int ls2k_spiflash_fastread_disable(const void *bus)
{
    SPI_bus_t *pSPI = (SPI_bus_t *)bus;
    
    LOCK(pSPI);
	pSPI->hwSPI->param &= ~SPI_PARAM_MEMORY_EN;
    UNLOCK(pSPI);

	return 0;
}

#endif

//-----------------------------------------------------------------------------

/*
 * @@ END
 */


