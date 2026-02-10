/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_spi_bus.c
 *
 * created: 2022-02-24
 *  author: Bian
 */

/*
 * TODO 区分 SPI2~5
 */

#include "bsp.h"

#if (BSP_USE_SPI)

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "ls2k500.h"
#include "ls2k500_irq.h"

#include "osal.h"

#include "ls2k_drv_io.h"
#include "drv_os_priority.h"

#include "ls2k_spi_hw.h"
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
    HW_SPI_t *hwSPI;                /* pointer to HW registers */
    /* hardware parameters */
    unsigned int base_frq;          /* input frq for baud rate divider */
    unsigned int chipsel_nums;      /* total chip select numbers */
    unsigned int chipsel_high;      /* cs high level - XXX: value from tfr-mode */
    unsigned int dummy_char;        /* this character will be continuously transmitted in read only functions */
    unsigned int irqNum;            /* interrupt num */
    /* mutex */
    osal_mutex_t p_mutex;

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

/*
 * 是否是全功能的 SPI 设备
 */
static inline int SPI_is_0_or_1(SPI_bus_t *pSPI)
{
#if (BSP_USE_SPI0 || BSP_USE_SPI1)
  #if (BSP_USE_SPI0 && BSP_USE_SPI1)
	if ((pSPI == busSPI0) || (pSPI == busSPI1))
  #elif BSP_USE_SPI0
	if (pSPI == busSPI0)
  #elif BSP_USE_SPI1
	if (pSPI == busSPI1)
  #endif
	{
	    return (1);
    }
#endif

    return (0);
}

static int SPI_baud_to_mode(unsigned int baudrate, unsigned int base_freq,
					        unsigned char *spre, unsigned char *spr)
{
    int i;
    unsigned int divider;

    if (baudrate == 0)
        return -1;
        
    divider = base_freq / baudrate;
    if ((base_freq % baudrate) > 0)
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

static int SPI_wait_tx_done(SPI_bus_t *pSPI)
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

/******************************************************************************
 * SPI driver Implement
 ******************************************************************************/

extern int ls2k_spi_init_hook(const void *bus);

STATIC_DRV int SPI_initialize(const void *bus)
{
    SPI_bus_t *pSPI = (SPI_bus_t *)bus;

    if (bus == NULL)
        return -1;

    if (pSPI->initialized)
        return 0;

    ls2k_spi_init_hook(bus);

    pSPI->p_mutex = osal_mutex_create(pSPI->dev_name, 0);
    if (pSPI->p_mutex == NULL)
    {
        printk("create mutex for % fail\r\n", pSPI->dev_name);
    	return -1;
    }

	/* initialize with bsp bus frequency
	 */
	pSPI->base_frq = apb_frequency;

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
           (VA_TO_PHYS(pSPI->hwSPI) == SPI1_BASE) ? 1 : \
           (VA_TO_PHYS(pSPI->hwSPI) == SPI2_BASE) ? 2 : \
           (VA_TO_PHYS(pSPI->hwSPI) == SPI3_BASE) ? 3 : \
           (VA_TO_PHYS(pSPI->hwSPI) == SPI4_BASE) ? 4 : \
           (VA_TO_PHYS(pSPI->hwSPI) == SPI5_BASE) ? 5 : -1);

	return 0;
}

static int SPI_read_write_bytes(SPI_bus_t *pSPI,
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
			pSPI->hwSPI->data = pSPI->dummy_char;
		}
		else
		{
			pSPI->hwSPI->data = *(unsigned char *)txbuf;
			txbuf++;
		}

		/*
		 * wait until end of transfer
		 */
		rt = SPI_wait_tx_done(pSPI);

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

STATIC_DRV int SPI_read_bytes(const void *bus, unsigned char *buf, int len)
{
    SPI_bus_t *pSPI = (SPI_bus_t *)bus;

    if ((bus == NULL) || (buf == NULL))
    {
        return -1;
    }

	return SPI_read_write_bytes(pSPI, buf, NULL, len);
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

    if ((bus == NULL) || (buf == NULL))
    {
        return -1;
    }

	return SPI_read_write_bytes(pSPI, NULL, buf, len);
}

static int SPI_set_tfr_mode(SPI_bus_t *pSPI, SPI_mode_t *pMODE)
{
	int rt;
	unsigned char spre, spr, val;

	/*
	 * according baudrate, calculate the proper frequency
	 */
	rt = SPI_baud_to_mode(pMODE->baudrate, pSPI->base_frq, &spre, &spr);
	if (0 != rt)
	{
		return rt;
	}

	pSPI->hwSPI->ctrl &= ~SPI_CTRL_EN;  	    /* disable spi */

	/* set spi-er register */
	val = spre & SPI_ER_SPRE_MASK;
	if (!pMODE->clock_phs)						/* = true: 同步发送 */
	{
		val |= SPI_ER_MODE;
	}
	pSPI->hwSPI->er = val;

	/* set spi-param register */
	if (SPI_is_0_or_1(pSPI))
	{
	    val = pSPI->hwSPI->R4.param;
    	val &= ~SPI_PARAM_CLKDIV_MASK;
    	val |= (spre << 6) | (spr << 4);
    	pSPI->hwSPI->R4.param = val;
    }

	/* set spi-control register - spi mode */
	val = SPI_CTRL_MASTER | (spr & SPI_CTRL_SPR_MASK);
	if (pMODE->clock_pha)
	{
		val |= SPI_CTRL_CPHA;
	}
	if (pMODE->clock_pol)
	{
		val |= SPI_CTRL_CPOL;
	}

	/* set new control value and re-enable spi */
	pSPI->hwSPI->ctrl = val | SPI_CTRL_EN;

	/* set spi-sr register, clear flag */
	pSPI->hwSPI->sr = SPI_SR_IFLAG | SPI_SR_WOVERFLOW;

	/* set idle character */
	pSPI->dummy_char = pSPI->dummy_char & 0xFF;

	/* chip select mode */
	pSPI->chipsel_high = !pMODE->clock_inv;

	return 0;
}

STATIC_DRV int SPI_ioctl(const void *bus, int cmd, void *arg)
{
	SPI_bus_t *pSPI = (SPI_bus_t *)bus;
    SPI_mode_t *pMODE;
    unsigned int val;
	int rt = 0;

    if (bus == NULL)
    {
        return -1;
    }

	switch (cmd)
	{
		case IOCTL_SPI_I2C_SET_TFRMODE:
		    pMODE = (SPI_mode_t *)arg;
			rt = -SPI_set_tfr_mode(pSPI, pMODE);
			break;

		case IOCTL_FLASH_FAST_READ_ENABLE:
        /*
			pSPI->hwSPI->timing = spi_timing_tcsh_2;
			pSPI->hwSPI->param |= spi_param_fast_read |
					              spi_param_burst_en |
								  spi_param_memory_en;
         */
            if (SPI_is_0_or_1(pSPI))
            {
                pSPI->hwSPI->R4.param |= SPI_PARAM_MEMORY_EN;
            }
			break;

		case IOCTL_FLASH_FAST_READ_DISABLE:
		    if (SPI_is_0_or_1(pSPI))
		    {
			    pSPI->hwSPI->R4.param &= ~SPI_PARAM_MEMORY_EN;
			}
			break;

		case IOCTL_FLASH_GET_FAST_READ_MODE:
            if (SPI_is_0_or_1(pSPI))
            {
		        val  = (unsigned)pSPI->hwSPI->R4.param;
                val &= SPI_PARAM_MEMORY_EN;
			    *(unsigned int *)arg = val;
            }
            else
            {
                *(unsigned int *)arg = 0;
            }
			break;

		default:
			rt = -1;
			break;
	}

	return rt;
}

/*
 * according chip-select to send begin r/w op
 */
STATIC_DRV int SPI_send_start(const void *bus, unsigned int Addr)
{
	unsigned char chip_sel;
    SPI_bus_t *pSPI = (SPI_bus_t *)bus;

    if (bus == NULL)
    {
        return -1;
    }

	LOCK(pSPI);
	
	if (SPI_is_0_or_1(pSPI))
	{
    	Addr &= 0x03;							/* (sc_ptr->chipsel_nums - 1); */
	    chip_sel = 1 << Addr;					/* set chip_sel enable */

	    if (!pSPI->chipsel_high)				/* XXX de-select chip */
	    {
    	    chip_sel |= 1 << (Addr + 4);
        }

	    pSPI->hwSPI->softcs = chip_sel;
    }
    else // if busSPI2~5
    {
        pSPI->hwSPI->R4.cs &= ~SPI_CSn_EN;
    }

    return 0;
}

/*
 * spi chip select
 */
STATIC_DRV int SPI_send_addr(const void *bus, unsigned int Addr, int rw)
{
	unsigned char chip_sel;
    SPI_bus_t *pSPI = (SPI_bus_t *)bus;

    if (bus == NULL)
    {
        return -1;
    }

	if (SPI_is_0_or_1(pSPI))
	{
        Addr &= 0x03;						    /* address range 0~3 */
	    chip_sel = pSPI->hwSPI->softcs & 0x0F;

	    if (pSPI->chipsel_high)				    /* select chip */
	    {
    	    chip_sel |= 1 << (Addr + 4);
        }
	    else
	    {
    	    chip_sel &= ~(1 << (Addr + 4));
        }

    	pSPI->hwSPI->softcs = chip_sel;
    }
    else // if busSPI2~5
    {
        pSPI->hwSPI->R4.cs &= ~SPI_CSn;
    }

	return 0;
}

/*
 * according chip-select to send finish r/w op
 */
STATIC_DRV int SPI_send_stop(const void *bus, unsigned int Addr)
{
	unsigned char chip_sel;
    SPI_bus_t *pSPI = (SPI_bus_t *)bus;

    if (bus == NULL)
    {
        return -1;
    }

    if (SPI_is_0_or_1(pSPI))
    {
	    Addr &= 0x03;							/* (sc_ptr->chipsel_nums - 1); */
    	chip_sel = pSPI->hwSPI->softcs & 0x0F;

    	if (pSPI->chipsel_high)				    /* XXX de-select chip */
    	{
    		chip_sel &= ~(1 << (Addr + 4));
    	}
    	else
    	{
    		chip_sel |= 1 << (Addr + 4);
    	}

    	pSPI->hwSPI->softcs = chip_sel;
    }
    else // if busSPI2~5
    {
        pSPI->hwSPI->R4.cs |= SPI_CSn_EN | SPI_CSn;
    }
    
    UNLOCK(pSPI);
	return 0;
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
    .irqNum       = EXTINTC2_SPI0_IRQ,
#else
    .irqNum       = INTC1_SPI0_IRQ,
#endif
    .base_frq     = 0,
    .chipsel_nums = 4,
	.chipsel_high = 0,
    .dummy_char   = 0,
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
    .irqNum       = EXTINTC2_SPI1_IRQ,
#else
    .irqNum       = INTC1_SPI1_IRQ,
#endif
    .base_frq     = 0,
    .chipsel_nums = 4,
	.chipsel_high = 0,
    .dummy_char   = 0,
    .initialized  = 0,
    .dev_name     = "spi1",
};

const void *busSPI1 = &ls2k_SPI1;
#endif

#if BSP_USE_SPI2
static SPI_bus_t ls2k_SPI2 =
{
    .hwSPI        = (HW_SPI_t *)PHYS_TO_UNCACHED(SPI2_BASE),
#if USE_EXTINT
    .irqNum       = EXTINTC0_SPI2_IRQ,
#else
    .irqNum       = INTC1_SPI2_3_IRQ,
#endif
    .base_frq     = 0,
    .chipsel_nums = 4,
	.chipsel_high = 0,
    .dummy_char   = 0,
    .initialized  = 0,
    .dev_name     = "spi2",
};

const void *busSPI2 = &ls2k_SPI2;
#endif

#if BSP_USE_SPI3
static SPI_bus_t ls2k_SPI3 =
{
    .hwSPI        = (HW_SPI_t *)PHYS_TO_UNCACHED(SPI3_BASE),
#if USE_EXTINT
    .irqNum       = EXTINTC0_SPI3_IRQ,
#else
    .irqNum       = INTC1_SPI2_3_IRQ,
#endif
    .base_frq     = 0,
    .chipsel_nums = 4,
	.chipsel_high = 0,
    .dummy_char   = 0,
    .initialized  = 0,
    .dev_name     = "spi3",
};

const void *busSPI3 = &ls2k_SPI3;
#endif

#if BSP_USE_SPI4
static SPI_bus_t ls2k_SPI4 =
{
    .hwSPI        = (HW_SPI_t *)PHYS_TO_UNCACHED(SPI4_BASE),
#if USE_EXTINT
    .irqNum       = EXTINTC0_SPI4_IRQ,
#else
    .irqNum       = INTC1_SPI4_5_IRQ,
#endif
    .base_frq     = 0,
    .chipsel_nums = 4,
	.chipsel_high = 0,
    .dummy_char   = 0,
    .initialized  = 0,
    .dev_name     = "spi4",
};

const void *busSPI4 = &ls2k_SPI4;
#endif

#if BSP_USE_SPI5
static SPI_bus_t ls2k_SPI5 =
{
    .hwSPI        = (HW_SPI_t *)PHYS_TO_UNCACHED(SPI5_BASE),
#if USE_EXTINT
    .irqNum       = EXTINTC0_SPI5_IRQ,
#else
    .irqNum       = INTC1_SPI4_5_IRQ,
#endif
    .base_frq     = 0,
    .chipsel_nums = 4,
	.chipsel_high = 0,
    .dummy_char   = 0,
    .initialized  = 0,
    .dev_name     = "spi5",
};

const void *busSPI5 = &ls2k_SPI5;
#endif

/******************************************************************************
 * for external enable/disable spiflash fastread mode.
 */
int ls2k_spiflash_fastread_enable(const void *bus)
{
    SPI_bus_t *pSPI = (SPI_bus_t *)bus;
    
    if (SPI_is_0_or_1(pSPI))
	{
        LOCK(pSPI);
	    pSPI->hwSPI->timing = SPI_TIMING_tCSH_2T;
	    pSPI->hwSPI->R4.param |= SPI_PARAM_FAST_READ | SPI_PARAM_BURST_EN | SPI_PARAM_MEMORY_EN;
        UNLOCK(pSPI);

	    return 0;
    }

    return -1;
}

int ls2k_spiflash_fastread_disable(const void *bus)
{
    SPI_bus_t *pSPI = (SPI_bus_t *)bus;
    
    if (SPI_is_0_or_1(pSPI))
	{
        LOCK(pSPI);
	    pSPI->hwSPI->R4.param &= ~SPI_PARAM_MEMORY_EN;
        UNLOCK(pSPI);

	    return 0;
    }

    return -1;
}

#endif


