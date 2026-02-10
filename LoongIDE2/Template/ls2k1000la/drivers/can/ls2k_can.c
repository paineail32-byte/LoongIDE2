/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_can.c
 *
 * created: 2022-03-09
 *  author: Bian
 */

#include "bsp.h"

#if BSP_USE_CAN

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ls2k1000.h"
#include "ls2k1000_irq.h"

#include "osal.h"

#include "ls2k_drv_io.h"
#include "drv_os_priority.h"

#include "ls2k_can.h"
#include "ls2k_can_hw.h"

//-----------------------------------------------------------------------------

#define CAN_DEBUG       0

#define RX_FIFO_LEN		64
#define TX_FIFO_LEN		64

#define RESET_TIMEOUT 	100
#define MAX_TSEG2 		7
#define MAX_TSEG1 		15

//-----------------------------------------------------------------------------

/******************************************************************************
 * fifo interface
 */
typedef struct
{
	int			count;
	int			ovcount;			/* overwrite count */
	int			full;				/* 1 = base contain cnt CANMsgs, tail==head */
	CANMsg_t   *tail, *head;
	CANMsg_t   *base;
	char		fifoarea[0];
} CAN_fifo_t;

/******************************************************************************
 * ls2k can priv defination
 */
typedef struct CAN
{
	/* hardware shortcuts
	 */
	HW_CAN_t       *hwCAN;			/* CAN 硬件 */
	unsigned int 	irqNum;			/* 中断号 */

	CAN_speed_t	    timing;			/* btr0/btr1 */
	unsigned int 	afmode;			/* 单/双过滤模式 */
	unsigned int	coremode;		/* CAN core: 标准CAN2.0A、扩展CAN2.0B */
	unsigned int	workmode;		/* normal, selftest, listenonly */

	osal_event_t    p_event;

    int             timeout;        /* 收发超时 */
	int 		    started;		/* can device started */
	unsigned int 	status;
	CAN_stats_t		stats;

	/* rx and tx fifos
	 */
	CAN_fifo_t		*rxfifo;
	CAN_fifo_t		*txfifo;

	/* Config
	 */
	unsigned int	speed; 			/* speed in HZ */
	unsigned char	acode[4];
	unsigned char	amask[4];

    int             initialized;
    int             opened;
    char            dev_name[16];
} CAN_t;

//-------------------------------------------------------------------------------------------------
// Here is CAN interface defination
//-------------------------------------------------------------------------------------------------

#if BSP_USE_CAN0
static CAN_t ls2k_CAN0 =
{
	.hwCAN       = (HW_CAN_t *)PHYS_TO_UNCACHED(0x1fe20c00),
    .irqNum      = INTC0_CAN0_IRQ,
	.initialized = 0,
	.dev_name    = "can0",
};

const void *devCAN0 = (void *)&ls2k_CAN0;
#endif

#if BSP_USE_CAN1
static CAN_t ls2k_CAN1 =
{
	.hwCAN       = (HW_CAN_t *)PHYS_TO_UNCACHED(0x1fe20d00),
    .irqNum      = INTC0_CAN1_IRQ,
	.initialized = 0,
	.dev_name    = "can1",
};

const void *devCAN1 = (void *)&ls2k_CAN1;
#endif

/*
 * CAN IO base
 */
static void ls2k1000_can_init_io_base(CAN_t *pCAN)
{
    unsigned long apb_address0;

    apb_address0 = READ_REG64(APB_CFG_HEAD_BASE + 0x10) & ~0x0Ful;

#if (BSP_USE_CAN0)
    if (pCAN == &ls2k_CAN0)
        ls2k_CAN0.hwCAN = (HW_CAN_t *)PHYS_TO_UNCACHED(apb_address0 + 0xc00);
#endif
#if (BSP_USE_CAN1)
    if (pCAN == &ls2k_CAN1)
        ls2k_CAN1.hwCAN = (HW_CAN_t *)PHYS_TO_UNCACHED(apb_address0 + 0xd00);
#endif
}

/******************************************************************************
 * FIFO IMPLEMENTATION
 ******************************************************************************/

static CAN_fifo_t *can_fifo_create(int count)
{
	CAN_fifo_t *fifo;

	fifo = malloc(sizeof(CAN_fifo_t) + count * sizeof(CANMsg_t));
	if (fifo)
	{
		fifo->count = count;
		fifo->full = 0;
		fifo->ovcount = 0;
		fifo->base = (CANMsg_t *)&fifo->fifoarea[0];
		fifo->tail = fifo->head = fifo->base;

		/* clear CAN Messages
		 */
		memset(fifo->base, 0, count * sizeof(CANMsg_t));
	}

	return fifo;
}

static void can_fifo_free(CAN_fifo_t *fifo)
{
	if (fifo)
	{
		free(fifo);
		fifo = NULL;
	}
}

static inline int can_fifo_full(CAN_fifo_t *fifo)
{
	return fifo->full;
}

static inline int can_fifo_empty(CAN_fifo_t *fifo)
{
	return (!fifo->full) && (fifo->head == fifo->tail);
}

static void can_fifo_get(CAN_fifo_t *fifo)
{
	if (!fifo)
	{
    	return;
    }

	if (can_fifo_empty(fifo))
	{
    	return;
    }

	/*
     * increment indexes
	 */
	fifo->tail = (fifo->tail >= &fifo->base[fifo->count - 1]) ? fifo->base : fifo->tail + 1;
	fifo->full = 0;
}

/* Stage 1 - get buffer to fill (never fails if force!=0)
 */
static CANMsg_t *can_fifo_put_claim(CAN_fifo_t *fifo, int force)
{
	if (!fifo)
	{
    	return NULL;
    }

	if (can_fifo_full(fifo))
	{
		if (!force)
		{
        	return NULL;
        }

		/* all buffers already used ==> overwrite the oldest
		 */
		fifo->ovcount++;
		can_fifo_get(fifo);
	}

	return fifo->head;
}

/* Stage 2 - increment indexes
 */
static void can_fifo_put(CAN_fifo_t *fifo)
{
	if (can_fifo_full(fifo))
	{
    	return;
    }

	/*
     * wrap around the indexes
	 */
	fifo->head = (fifo->head >= &fifo->base[fifo->count - 1]) ? fifo->base : fifo->head + 1;
	if (fifo->head == fifo->tail)
	{
    	fifo->full = 1;
    }
}

static CANMsg_t *can_fifo_claim_get(CAN_fifo_t *fifo)
{
	if (can_fifo_empty(fifo))
	{
    	return NULL;
    }

	/* return oldest message
	 */
	return fifo->tail;
}

static void can_fifo_clear(CAN_fifo_t *fifo)
{
	fifo->full = 0;
	fifo->ovcount = 0;
	fifo->head = fifo->tail = fifo->base;
}

/******************************************************************************
 * Predefined Interface
 ******************************************************************************/

/* interrupt routine, use for PeliCAN
 */
static void ls2k_can_interrupt_handler(int vector, void *arg);
 
/******************************************************************************
 * Hardware Operating
 ******************************************************************************/

/* This function calculates BTR0 and BTR1 values for a given bitrate.
 *
 * Set communication parameters.
 * param: rate Requested baud rate in bits/second.
 * param: result Pointer to where resulting BTRs will be stored.
 * return: zero if successful to calculate a baud rate.
 *
 *
 * LS2K1000 baudrate:
 *
 * 1bit time = clock_time*((BRP+1)*2)*(1+(TESG2+1)+(TESG1+1))
 */
static int ls2k_can_calc_speedregs(unsigned int rate, CAN_speed_t *result)
{
	int best_error = 1000000000;
	int error;
	int best_tseg=0, best_brp, brp=0;
	int tseg=0, tseg1=0, tseg2=0;
	int sjw = 0;
	int sampl_pt = 65;

	int clock_hz = apb_frequency;

	if ((rate < 5000) || (rate > 1000000))
	{
    	return -1;
    }

	/****************************************************************
	 * find best match, return -2 if no good reg combination
	 * is available for this frequency
	 ****************************************************************/

	/* tseg even = round down, odd = round up
	 */
	for (tseg = (0 + 0 + 2) * 2;
	     tseg <= (MAX_TSEG2 + MAX_TSEG1 + 2) * 2 + 1;
	     tseg++)
	{
		brp = clock_hz / ((1 + tseg / 2) * rate) + tseg % 2;
		if ((brp == 0) || (brp > 64))
		{
        	continue;
        }

		error = rate - clock_hz / (brp * (1 + tseg / 2));
		if (error < 0)
		{
			error = -error;
		}

		if (error <= best_error)
		{
			best_error = error;
			best_tseg = tseg / 2;
			best_brp = brp - 1;
		}
	}

	if (best_error && (rate / best_error < 10))
	{
		printk("CAN Error: rate %i is not possible with %iHZ clock\r\n", (int)rate, (int)clock_hz);
		return -2;
	}
	else if (!result)
	{
		/* nothing to store result in, but a valid bitrate can be calculated
		 */
		return 0;
	}

	/* some heuristic specials
	 */
	if (best_tseg <= 10)
	{
    	sampl_pt = 80;
    }
	else if (best_tseg <= 15)
	{
    	sampl_pt = 72;
    }

	tseg2 = best_tseg - (sampl_pt * (best_tseg + 1)) / 100;

	if (tseg2 < 0)
	{
    	tseg2 = 0;
    }

	if (tseg2 > MAX_TSEG2)
	{
    	tseg2 = MAX_TSEG2;
    }

	tseg1 = best_tseg - tseg2 - 2;

	if (tseg1 > MAX_TSEG1)
	{
		tseg1 = MAX_TSEG1;
		tseg2 = best_tseg - tseg1 - 2;
	}

	if (tseg2 >= 3)
	{
    	sjw = 1;
    }

	if (rate > 250000)
	{
    	result->samples = 0;		/* 1 次采样 */
    }
	else
	{
    	result->samples = 1;		/* 3 次采样 */
    }

	result->btr0 = (sjw << CAN_BTR0_SJW_SHIFT) | (best_brp & CAN_BTR0_BRP_MASK);

	if (result->samples)
	{
    	result->btr1 = (1 << 7) | (tseg2 << CAN_BTR1_TSEG2_SHIFT) | tseg1;
    }
	else
	{
    	result->btr1 = (0 << 7) | (tseg2 << CAN_BTR1_TSEG2_SHIFT) | tseg1;
    }

#if 0
    int best_rate = apb_frequency/((best_brp+1)*2*(1+(tseg2+1)+(tseg1+1)));
    printk("needed rate=%i, result rate=%i\n", rate, best_rate);
#endif

	return 0;
}

static int ls2k_can_set_speedregs(CAN_t *pCAN, CAN_speed_t *timing)
{
	if (!timing || !pCAN || !pCAN->hwCAN)
	{
    	return -1;
    }

	pCAN->hwCAN->btr0 = timing->btr0;
	pCAN->hwCAN->btr1 = timing->btr1;

	return 0;
}

static void ls2k_can_set_accept(CAN_t *pCAN, unsigned char *acode, unsigned char *amask)
{
	if (pCAN->coremode == CAN_CORE_20A)
	{
		pCAN->hwCAN->R4.std.accode = acode[0];
		pCAN->hwCAN->R4.std.acmask = amask[0];
	}
	else
	{
		pCAN->hwCAN->R10.ext.msg.accept.accode[0] = acode[0];
		pCAN->hwCAN->R10.ext.msg.accept.accode[1] = acode[1];
		pCAN->hwCAN->R10.ext.msg.accept.accode[2] = acode[2];
		pCAN->hwCAN->R10.ext.msg.accept.accode[3] = acode[3];
		pCAN->hwCAN->R10.ext.msg.accept.acmask[0] = amask[0];
		pCAN->hwCAN->R10.ext.msg.accept.acmask[1] = amask[1];
		pCAN->hwCAN->R10.ext.msg.accept.acmask[2] = amask[2];
		pCAN->hwCAN->R10.ext.msg.accept.acmask[3] = amask[3];
	}
}

/*
 * put the CAN-Control to reset mode
 */
static void ls2k_can_hw_stop(CAN_t *pCAN)
{
	int i;
	unsigned char tmp;

	/* Disable CAN interrupt
	 */
	ls2k_interrupt_disable(pCAN->irqNum);

	/* disable can interrupts
	 */
	if (pCAN->coremode == CAN_CORE_20B)
	{
    	pCAN->hwCAN->R4.ext.inten = 0;
    }

	tmp = pCAN->hwCAN->intflags;				/* Read for clear */

	tmp = pCAN->hwCAN->ctrl;

	for (i=0; i<RESET_TIMEOUT; i++)
	{
		if (tmp & CAN_CTRL_RESET)				/* Check reset bit */
		{
		    pCAN->started = false;
			return;
		}

		pCAN->hwCAN->ctrl = CAN_CTRL_RESET;		/* Reset chip */

		delay_us(10);

		tmp = pCAN->hwCAN->ctrl;
	}

   	printk("Error: CAN Device reset timeout.\r\n");

}

/*
 * put the CAN-Control into workmode
 */
static int ls2k_can_hw_start(CAN_t *pCAN)
{
	unsigned char ctrl, tmp;
	int i;

	if (!pCAN->rxfifo || !pCAN->txfifo)
	{
    	return -1;
    }

	if (pCAN->started)
	{
    	ls2k_can_hw_stop(pCAN);
    }

	/* empty the TX fifo
	 */
	can_fifo_clear(pCAN->txfifo);

	pCAN->status = 0;

	ls2k_can_set_speedregs(pCAN, &pCAN->timing);
	ls2k_can_set_accept(pCAN, pCAN->acode, pCAN->amask);

	/*
	 * 必须在进入工作模式前先设置 CAN 模式
	 */
	if (pCAN->coremode == CAN_CORE_20A)
	{
		pCAN->hwCAN->cmd = CAN_CMD_STANDARD;		/* CAN2.0A */
		delay_us(10);

		/* Enable all CAN interrupt, and set core in workmode
		 */
		ctrl = CAN_STD_INTERRUPTS;
	}
	else
	{
		pCAN->hwCAN->cmd = CAN_CMD_EXTENDED;		/* CAN2.0B */
		delay_us(10);

		ctrl = CAN_CTRL_STANDWORK_EXT;				/* XXX CAN_STAND_MODE, Care of here! */

//		if (sc->afmode)
//			ctrl |= can_ctrl_afilter_ext;

		pCAN->hwCAN->R10.ext.rxerrcnt = 0;
		pCAN->hwCAN->R10.ext.txerrcnt = 0;

		tmp = pCAN->hwCAN->R10.ext.errcode;
	}

	/* clear CAN pending interrupts
	 */
	tmp = pCAN->hwCAN->intflags;

	tmp = pCAN->hwCAN->ctrl;

	for (i=0; i<RESET_TIMEOUT; i++)
	{
		if ((tmp & CAN_CTRL_RESET) == 0)		/* Exit reset mode, begin working. */
		{
			pCAN->started = true;

			if (pCAN->coremode == CAN_CORE_20B)
			{
				/* Enable all CAN interrupt
				 */
				pCAN->hwCAN->R4.ext.inten = CAN_IEN_ALL_EXT;
			}

			/* Enable CAN interrupt
			 */
			ls2k_interrupt_enable(pCAN->irqNum);

			return 0;
		}

		pCAN->hwCAN->ctrl = ctrl;

		delay_us(10);

		tmp = pCAN->hwCAN->ctrl;
	}

	printk("Error: CAN Device set work timeout.\n");

	return -1;
}

/* Try to send message "msg", if hardware txfifo is full, then -1 is returned.
 * Be sure to have disabled CAN interrupts when entering this function.
 */
static int ls2k_can_send_msg(CAN_t *pCAN, CANMsg_t *msg)
{
	unsigned char ch, cmd;

	/* is there room in send buffer?
	 */
	if (!(pCAN->hwCAN->status & CAN_STATUS_TXBUF))
	{
    	return -1;
    }

	/* Transmit command
	 */
	cmd = CAN_CMD_TXREQUEST;

	ch = msg->len & 0x0F;

	if (pCAN->coremode == CAN_CORE_20A)		/* Standard Frame */
	{
		if (msg->rtr)
		{
        	ch |= 0x10;
        }

		pCAN->hwCAN->R10.std.tx.id[0] = ((unsigned char)(msg->id >> 3) & 0xFF);
		pCAN->hwCAN->R10.std.tx.id[1] = ((unsigned char)(msg->id << 5) & 0xE0) | ch;
		ch = msg->len & 0x0F;
		while (ch--)
		{
			pCAN->hwCAN->R10.std.tx.data[ch] = msg->data[ch];
		}
	}
	else
	{
		if (msg->rtr)
		{
        	ch |= 0x40;
        }

		if (msg->extended)					/* Extended Frame */
		{
			pCAN->hwCAN->R10.ext.msg.tx.frameinfo = ch | 0x80;
			pCAN->hwCAN->R10.ext.msg.tx.ext.id[0] = (unsigned char)(msg->id >> (5+8+8)) & 0xFF;
			pCAN->hwCAN->R10.ext.msg.tx.ext.id[1] = (unsigned char)(msg->id >> (5+8)) & 0xFF;
			pCAN->hwCAN->R10.ext.msg.tx.ext.id[2] = (unsigned char)(msg->id >> (5)) & 0xFF;
			pCAN->hwCAN->R10.ext.msg.tx.ext.id[3] = (unsigned char)(msg->id << 3) & 0xF8;
			ch = msg->len & 0x0F;
			while (ch--)
			{
				pCAN->hwCAN->R10.ext.msg.tx.ext.data[ch] = msg->data[ch];
			}
		}
		else								/* Standard Frame */
		{
			pCAN->hwCAN->R10.ext.msg.tx.frameinfo = ch;
			pCAN->hwCAN->R10.ext.msg.tx.std.id[0] = (unsigned char)(msg->id >> 3) & 0xFF;
			pCAN->hwCAN->R10.ext.msg.tx.std.id[1] = (unsigned char)(msg->id << 5) & 0xE0;
			ch = msg->len & 0x0F;
			while (ch--)
			{
				pCAN->hwCAN->R10.ext.msg.tx.std.data[ch] = msg->data[ch];
			}
		}

		/* TODO Please care of here!
		 */
		if (pCAN->workmode == CAN_SELF_RECEIVE)
		{
        	cmd |= CAN_CMD_SELFRXREQUEST_EXT;
        }
	}

#if 0
	/* using with "can_cmd_txrequest" send once
	 */
	if (msg->sshot)
		cmd |= can_cmd_txabort;
#endif

	pCAN->hwCAN->cmd = cmd;

	return 0;
}

/******************************************************************************
 * CAN Driver Implement
 ******************************************************************************/

static void ls2k_can_clear_softc(CAN_t *pCAN)
{
    int i;

    memset((void *)&pCAN->stats,  0, sizeof(CAN_stats_t));

    pCAN->timing.btr0 = 0;
    pCAN->timing.btr1 = 0;
    pCAN->timing.samples = 0;

    pCAN->afmode   = 0;
    pCAN->coremode = 0;
    pCAN->workmode = 0;
    pCAN->timeout = 0;
    pCAN->started = 0;
    pCAN->status  = 0;
    pCAN->speed   = 0;

	can_fifo_free(pCAN->rxfifo);
	can_fifo_free(pCAN->txfifo);

    for (i=0; i<4; i++)
    {
	    pCAN->acode[i] = 0;
	    pCAN->amask[i] = 0;
    }
}

/******************************************************************************
 * CAN_init
 */

extern int ls2k_can_init_hook(const void *dev);

STATIC_DRV int CAN_init(const void *dev, void *arg)
{
    CAN_t *pCAN = (CAN_t *)dev;

	if (dev == NULL)
	{
    	return -1;
    }

    if (pCAN->initialized)
    {
        return 0;
    }

    ls2k1000_can_init_io_base(pCAN);

    ls2k_can_init_hook(pCAN);
    
    /*
     * 引脚复用
     */

    ls2k_can_clear_softc(pCAN);

    pCAN->p_event = osal_event_create(pCAN->dev_name, 0);
    if (NULL == pCAN->p_event)
    {
        printk("create CAN event fail.\r\n");
        return -1;
    }

	/*
	 * 设置 CAN 默认工作模式
	 */
	pCAN->coremode = CAN_CORE_20B;			/* Default is CAN2.0B */
	pCAN->workmode = CAN_STAND_MODE;		/* Normal */
	pCAN->speed    = CAN_SPEED_250K;
	pCAN->amask[0] = 0xFF;
	pCAN->amask[1] = 0xFF;
	pCAN->amask[2] = 0xFF;
	pCAN->amask[3] = 0xFF;
	pCAN->afmode   = 1; 					/* single acceptance filter */

	if (ls2k_can_calc_speedregs(pCAN->speed, &pCAN->timing))
	{
		/* calculate rate error
		 */
		pCAN->speed = CAN_SPEED_50K;
		ls2k_can_calc_speedregs(pCAN->speed, &pCAN->timing);
	}

	/* stop CAN, put core in reset mode
	 */
	ls2k_can_hw_stop(pCAN);

	/* set CAN with poll trigger mode
	 */
    //
    
	/* Setup interrupt handler
	 */
    ls2k_install_irq_handler(pCAN->irqNum, ls2k_can_interrupt_handler, (void *)pCAN);

    ls2k_set_irq_routeip(pCAN->irqNum, INT_ROUTE_IP0);
    // ls2k_set_irq_triggermode(pCAN->irqNum, INT_TRIGGER_LEVEL);

    printk("CAN%i controller initialized.\r\n",
#if BSP_USE_CAN0
    		pCAN == devCAN0 ? 0 :
#endif
#if BSP_USE_CAN1
    		pCAN == devCAN1 ? 1 :
#endif
    		-1);

    pCAN->initialized = 1;

	return 0;
}

/******************************************************************************
 * CAN_open
 */
STATIC_DRV int CAN_open(const void *dev, void *arg)
{
    CAN_t *pCAN = (CAN_t *)dev;

	if (dev == NULL)
	{
    	return -1;
    }

    if (pCAN->opened)
    {
        return 0;
    }

	/* already open
	 */
	if (pCAN->rxfifo && pCAN->txfifo)
	{
    	return 0;
    }

	/* allocate fifos
	 */
	pCAN->rxfifo = can_fifo_create(RX_FIFO_LEN);
	if (!pCAN->rxfifo)
    {
    	return -1;
    }

    pCAN->txfifo = can_fifo_create(TX_FIFO_LEN);
	if (!pCAN->txfifo)
	{
		can_fifo_free(pCAN->rxfifo);
		return -1;
	}

	/* clear stat counters
	 */
	memset(&pCAN->stats, 0, sizeof(CAN_stats_t));

	/* HW must be in reset-mode here (close and initializes resets core...)
	 * set default modes/speeds
	 */
	ls2k_can_hw_stop(pCAN);

    pCAN->opened = 1;
	return 0;
}

/******************************************************************************
 * CAN_close
 */
STATIC_DRV int CAN_close(const void *dev, void *arg)
{
    CAN_t *pCAN = (CAN_t *)dev;

	if (dev == NULL)
	{
    	return -1;
    }

	/* stop CAN device, and set to reset mode
	 */
	ls2k_can_hw_stop(pCAN);

    ls2k_can_clear_softc(pCAN);

    pCAN->opened = 0;
	return 0;
}

/******************************************************************************
 * CAN read
 */
STATIC_DRV int CAN_read(const void *dev, void *buf, int size, void *arg)
{
    int left = size;
    CAN_t *pCAN = (CAN_t *)dev;
    CANMsg_t *srcmsg, *dstmsg = (CANMsg_t *)buf;

	/* does at least one message fit ?
	 */
	if ((dev == NULL) || (buf == NULL) || (left < sizeof(CANMsg_t)))
	{
    	return -1;
    }

	if (!pCAN->started)
	{
		ls2k_can_hw_start(pCAN);
	}

	while (left >= sizeof(CANMsg_t))
	{
		/* A bus off interrupt may have occured after read
		 */
		if (pCAN->status & (CAN_STATUS_ERR_BUSOFF | CAN_STATUS_RESET))
		{
        	return -2;
        }

		srcmsg = can_fifo_claim_get(pCAN->rxfifo);

		if (!srcmsg)
		{
		    int tmo = pCAN->timeout;
		    unsigned int recv_event;

			/* No more messages in reception fifo. Wait for incoming packets
			 * return if no wait OR readed some messages.
			 */
			if ((tmo == 0) || (left != size))
            {
            	break;
            }

			/*
             * wait for incomming messages...
			 */
            recv_event = osal_event_receive(pCAN->p_event,
                                            CAN_RX_EVENT,
                                            OSAL_EVENT_FLAG_AND | OSAL_EVENT_FLAG_CLEAR,
                                            OSAL_WAIT_FOREVER );

            if (recv_event != CAN_RX_EVENT)
            {
                break;
            }

			/* did we get woken up by a BUS OFF error?
			 */
			if (pCAN->status & (CAN_STATUS_ERR_BUSOFF | CAN_STATUS_RESET))
			{
				/* At this point it should not matter how many messages we handled
				 */
                return -2;
			}

			/* no errors detected, it must be a message
			 */
			continue;
		}

		/* got message, copy it to userspace buffer
		 */
		*dstmsg = *srcmsg;

		/* Return borrowed message, RX interrupt can use it again
		 */
		can_fifo_get(pCAN->rxfifo);

		left -= sizeof(CANMsg_t);
		dstmsg++;
	}

    return size - left;
}

/******************************************************************************
 * CAN write
 */
STATIC_DRV int CAN_write(const void *dev, void *buf, int size, void *arg)
{
    int left = size;
    CAN_t *pCAN = (CAN_t *)dev;
    CANMsg_t *msg = (CANMsg_t *)buf, *fifo_msg;

	if ((dev == NULL) || (buf == NULL) || (left < sizeof(CANMsg_t)))
	{
    	return -1;
    }

	msg->len = (msg->len > 8) ? 8 : msg->len;

	/* A bus off interrupt may have occured before being send
	 */
	if (pCAN->status & (CAN_STATUS_ERR_BUSOFF | CAN_STATUS_RESET))
	{
    	return -2;
    }

	if (!pCAN->started)
	{
		ls2k_can_hw_start(pCAN);
	}

	/* If no messages in software tx fifo, we will try to send first message
	 * by putting it directly into the HW TX fifo.
	 */
	if (can_fifo_empty(pCAN->txfifo))
	{
		if (ls2k_can_send_msg(pCAN, msg) == 0)
		{
			/* First message put directly into HW TX fifo, This will turn TX interrupt on.
			 */
			left -= sizeof(CANMsg_t);
			msg++;

			pCAN->stats.tx_msgs++;
		}
	}

	/* Put messages into software fifo
	 */
	while (left >= sizeof(CANMsg_t))
	{
		msg->len = (msg->len > 8) ? 8 : msg->len;

		fifo_msg = can_fifo_put_claim(pCAN->txfifo, 0);

		if (!fifo_msg)
		{
		    int tmo = pCAN->timeout;
		    unsigned int recv_event;

            /* Waiting only if no messages previously sent.
             * return if no wait OR written some messages.
			 */
			if ((tmo == 0) || (left != size))
			{
            	break;
            }

			/*
             * wait for messages sent...
			 */
            recv_event = osal_event_receive(pCAN->p_event,
                                            CAN_TX_EVENT,
                                            OSAL_EVENT_FLAG_AND | OSAL_EVENT_FLAG_CLEAR,
                                            OSAL_WAIT_FOREVER );

            if (recv_event != CAN_TX_EVENT)
            {
                break;
            }

			/* did we get woken up by a BUS OFF error?
			 */
			if (pCAN->status & (CAN_STATUS_ERR_BUSOFF | CAN_STATUS_RESET))
			{
				/* At this point it should not matter how many messages we handled
				 */
				return -2;
			}

			if (can_fifo_empty(pCAN->txfifo))
			{
				if (!ls2k_can_send_msg(pCAN, msg))
				{
					/* First message put directly into HW TX fifo
					 * This will turn TX interrupt on.
					 */
					left -= sizeof(CANMsg_t);
					msg++;

					pCAN->stats.tx_msgs++;
				}
			}

			continue;
		}

		/* copy message into fifo area
		 */
		*fifo_msg = *msg;

		/* tell interrupt handler about the message
		 */
		can_fifo_put(pCAN->txfifo);

		/* Prepare insert of next message
		 */
		msg++;
		left -= sizeof(CANMsg_t);
	}

    return size - left;
}

/******************************************************************************
 * CAN control
 */
#define CAN_STARTED_BREAK   if (pCAN->started) { rt = -1; break; }
#define PTR_NULL_BREAK(ptr) if (ptr == NULL)   { rt = -1; break; }

STATIC_DRV int CAN_ioctl(const void *dev, int cmd, void *arg)
{
    int rt = 0;
    CAN_t *pCAN = (CAN_t *)dev;
	CAN_speed_t timing;
	CAN_afilter_t *afilter;
	CAN_stats_t *dststats;
	unsigned int speed, rxcnt, txcnt, val;

	if (dev == NULL)
	{
    	return -1;
    }

	switch (cmd)
	{
		case IOCTL_CAN_SET_SPEED:
			CAN_STARTED_BREAK;
			speed = (unsigned long)arg;
			rt = ls2k_can_calc_speedregs(speed, &timing);
			if (rt)
			{
				rt = -2;
				break;
			}
			ls2k_can_set_speedregs(pCAN, &timing);
			pCAN->speed = speed;
			pCAN->timing = timing;
			break;

		case IOCTL_CAN_SET_BTRS:
			CAN_STARTED_BREAK;
			pCAN->speed = 0;
			val = (unsigned long)arg;
			pCAN->timing.btr1 = val & 0xFF;
			pCAN->timing.btr0 = (val>>8) & 0xFF;
			ls2k_can_set_speedregs(pCAN, &pCAN->timing);
			break;

		case IOCTL_CAN_SPEED_AUTO:
			rt = 0;
			break;

		case IOCTL_CAN_SET_BUFLEN:
			CAN_STARTED_BREAK;
			val = (unsigned long)arg;
			rxcnt = val & 0x0000FFFF;
			txcnt = val >> 16;
			can_fifo_free(pCAN->rxfifo);
			can_fifo_free(pCAN->txfifo);
			pCAN->rxfifo = can_fifo_create(rxcnt);
			pCAN->txfifo = can_fifo_create(txcnt);
			if (!pCAN->rxfifo || !pCAN->txfifo)
			{
				rt = -2;
				break;
			}
			break;

		case IOCTL_CAN_GET_CONF:
			rt = 0;
			break;

		case IOCTL_CAN_GET_STATS:
			dststats = (CAN_stats_t *)arg;
			PTR_NULL_BREAK(dststats);
			if (pCAN->rxfifo)
				pCAN->stats.rx_sw_dovr = pCAN->rxfifo->ovcount;
			*dststats = pCAN->stats;
			break;

		case IOCTL_CAN_GET_STATUS:
			if (!arg)
			{
				rt = -1;
				break;
			}
			*(unsigned int *)arg = pCAN->status;
			break;

		case IOCTL_CAN_SET_LINK:
			rt = 0;
			break;

		case IOCTL_CAN_SET_FILTER:
			CAN_STARTED_BREAK;
			afilter = (CAN_afilter_t *)arg;
			if (!afilter)
			{
				rt = -2;
				break;
			}
			pCAN->acode[0] = afilter->code[0];
			pCAN->acode[1] = afilter->code[1];
			pCAN->acode[2] = afilter->code[2];
			pCAN->acode[3] = afilter->code[3];
			pCAN->amask[0] = afilter->mask[0];
			pCAN->amask[1] = afilter->mask[1];
			pCAN->amask[2] = afilter->mask[2];
			pCAN->amask[3] = afilter->mask[3];
			pCAN->afmode   = afilter->afmode;
			ls2k_can_set_accept(pCAN, pCAN->acode, pCAN->amask);
			break;

		case IOCTL_CAN_START:
			if (pCAN->started)
			{
				rt = -2;
				break;
			}
			if (ls2k_can_hw_start(pCAN))
			{
				rt = -2;
				break;
			}
			break;

		case IOCTL_CAN_STOP:
			if (!pCAN->started)
			{
				rt = -2;
				break;
			}
			ls2k_can_hw_stop(pCAN);
			break;

		case IOCTL_CAN_SET_CORE:
			CAN_STARTED_BREAK;
			val = (unsigned long)arg;
			pCAN->coremode = val;
			break;

		case IOCTL_CAN_SET_WORKMODE:
			CAN_STARTED_BREAK;
			val = (unsigned long)arg;
			pCAN->workmode = val;
			break;

		case IOCTL_CAN_SET_TIMEOUT:
			CAN_STARTED_BREAK;
			val = (unsigned long)arg;
			pCAN->timeout = val;
			break;

		default:
			rt = 0;
			break;
	}

	return rt;
}

/******************************************************************************
 * Interrupt Implement
 ******************************************************************************/

/*
 * XXX ls2k CAN interrupt only support extended - CAN2.0B
 */
static void ls2k_can_interrupt_handler(int vector, void *arg)
{
	unsigned char iflags;
	unsigned char ch, errcode, arbcode;
	int tx_error_cnt, rx_error_cnt, rx_flag=0, tx_flag=0;
	CANMsg_t *msg;
    CAN_t *pCAN = (CAN_t *)arg;

    if (NULL == pCAN)
    {
        return;
    }

	pCAN->stats.ints++;

	iflags = pCAN->hwCAN->intflags;
	if (pCAN->coremode == CAN_CORE_20A)
	{
    	iflags &= 0x1F;
    }

	while (iflags != 0)
	{
		/*
         * still interrupts to handle
		 */
		if (iflags & CAN_IFLAG_RX)
		{
			/* the rx fifo is not empty put 1 message into rxfifo for later use.
			 * get empty (or make room) message
			 */
			msg = can_fifo_put_claim(pCAN->rxfifo, 1);

			if (pCAN->coremode & CAN_CORE_20A)
			{
				/* Standard frame */
				ch = pCAN->hwCAN->R10.std.rx.id[1];
				msg->extended = 0;
				msg->rtr = (ch >> 4) & 0x01;
				msg->len = ch = ch & 0x0F;

				msg->id = (pCAN->hwCAN->R10.std.rx.id[0] << 3) |
						  (pCAN->hwCAN->R10.std.rx.id[1] >> 5);
				while (ch--)
				{
					msg->data[ch] = pCAN->hwCAN->R10.std.rx.data[ch];
				}
			}
			else
			{
				ch = pCAN->hwCAN->R10.ext.msg.rx.frameinfo;
				msg->extended = ch >> 7;

				if (msg->extended)
				{
					/* Extended Frame */
					msg->rtr = (ch >> 6) & 0x01;
					msg->len = ch = ch & 0x0F;
					msg->id = (pCAN->hwCAN->R10.ext.msg.rx.ext.id[0] << (5+8+8)) |
							  (pCAN->hwCAN->R10.ext.msg.rx.ext.id[1] << (5+8)) |
							  (pCAN->hwCAN->R10.ext.msg.rx.ext.id[2] << 5) |
							  (pCAN->hwCAN->R10.ext.msg.rx.ext.id[3] >> 3);
					while (ch--)
					{
						msg->data[ch] = pCAN->hwCAN->R10.ext.msg.rx.ext.data[ch];
					}
				}
				else
				{
					/* Standard frame */
					msg->rtr = (ch >> 4) & 0x01;
					msg->len = ch = ch & 0x0F;
					msg->id = (pCAN->hwCAN->R10.ext.msg.rx.std.id[0] << 3) |
							  (pCAN->hwCAN->R10.ext.msg.rx.std.id[1] >> 5);
					while (ch--)
					{
						msg->data[ch] = pCAN->hwCAN->R10.ext.msg.rx.std.data[ch];
					}
				}
			}

			/* Re-Enable RX buffer for a new message
			 */
			pCAN->hwCAN->cmd = CAN_CMD_RELEASERXBUF;

			/* make message available to the user
			 */
			can_fifo_put(pCAN->rxfifo);

			pCAN->stats.rx_msgs++;

			/* signal the semaphore only once
			 */
            rx_flag = 1;
		}

		if (iflags & CAN_IFLAG_TX)
		{
			/*
             * there is room in tx fifo of HW
			 */
			if (!can_fifo_empty(pCAN->txfifo))
			{
				/* send 1 more messages
				 */
				msg = can_fifo_claim_get(pCAN->txfifo);

				if (ls2k_can_send_msg(pCAN, msg))
				{
					/* ERROR! We got an TX interrupt telling us tx fifo is empty,
					 * yet it is not. Complain about this max 10 times
					 */
					if (pCAN->stats.tx_buf_err < 10)
					{
						DBG_OUT("CAN: got TX interrupt but TX fifo in not empty\r\n");
					}

					pCAN->status |= CAN_STATUS_QUEUE_ERROR;
					pCAN->stats.tx_buf_err++;
				}

				/* free software-fifo space taken by sent message
				 */
				can_fifo_get(pCAN->txfifo);

				pCAN->stats.tx_msgs++;

				/* wake any sleeping thread waiting for "fifo not full"
				 */
                tx_flag = 1;
			}
		}

		if (iflags & CAN_IFLAG_ERROR)
		{
			tx_error_cnt = pCAN->hwCAN->R10.ext.txerrcnt;
			rx_error_cnt = pCAN->hwCAN->R10.ext.rxerrcnt;

			/* if bus off tx error counter = 127
			 */
			if ((tx_error_cnt > 96) || (rx_error_cnt > 96))
			{
				/* in Error Active Warning area or BUS OFF
				 */
				pCAN->status |= CAN_STATUS_WARN;

				if (pCAN->hwCAN->ctrl & CAN_CTRL_RESET)
				{
					DBG_OUT("CAN stop\r\n");
					pCAN->status |= CAN_STATUS_ERR_BUSOFF | CAN_STATUS_RESET;

					/* stop CAN. turn off interrupts, enter reset mode.
					 */
					pCAN->hwCAN->R4.ext.inten = 0;

					/* User must issue a ioctl(START) to get going again.
					 */
					pCAN->started = false;

					/* signal any waiting read/write threads, so that they
					 * can handle the bus error.
					 */
					rx_flag = 1;
					tx_flag = 1;

					break;
				}
			}
			else
			{
            	/* not in Upper Error Active area any more
				 */
				pCAN->status &= ~(CAN_STATUS_WARN);
			}

			pCAN->stats.err_warn++;
		}

		if (iflags & CAN_IFLAG_DATAOVERFLOW)
		{
			pCAN->status |= CAN_STATUS_OVERRUN;
			pCAN->stats.err_dovr++;
			DBG_OUT("DOVR\r\n");
		}

		if (iflags & CAN_IFLAG_ERRORPASSIVE_EXT)
		{
			/* Let the error counters decide what kind of
			 * interrupt it was. In/Out of EPassive area.
			 */
			tx_error_cnt = pCAN->hwCAN->R10.ext.txerrcnt;
			rx_error_cnt = pCAN->hwCAN->R10.ext.rxerrcnt;

			if ((tx_error_cnt > 127) || (rx_error_cnt > 127))
			{
            	pCAN->status |= CAN_STATUS_ERR_PASSIVE;
            }
			else
			{
            	pCAN->status &= ~(CAN_STATUS_ERR_PASSIVE);
            }

			pCAN->stats.err_errp++;
		}

		if (iflags & CAN_IFLAG_ARBITRATELOST_EXT)
		{
			arbcode = pCAN->hwCAN->R10.ext.arblost;
			pCAN->stats.err_arb_bitnum[arbcode & CAN_ARBLOST_MASK_EXT]++;
			pCAN->stats.err_arb++;
			DBG_OUT("ARB (0x%x)\r\n", arbcode & CAN_ARBLOST_MASK_EXT);
		}

		if (iflags & CAN_IFLAG_BUSERROR_EXT)
		{
			errcode = pCAN->hwCAN->R10.ext.errcode;
			DBG_OUT("buserr %02X\n", errcode);

			/* Some kind of BUS error, only used for statistics.
			 * Error Register is decoded and put into can->stats.
			 */
			switch (errcode & CAN_ERRCODE_MASK_EXT)
			{
				case CAN_ERRCODE_BIT_EXT:
					pCAN->stats.err_bus_bit++;
					break;
				case CAN_ERRCODE_FORM_EXT:
					pCAN->stats.err_bus_form++;
					break;
				case CAN_ERRCODE_STUFF_EXT:
					pCAN->stats.err_bus_stuff++;
					break;
				case CAN_ERRCODE_OTHER_EXT:
					pCAN->stats.err_bus_other++;
					break;
			}

			/* Get Direction (TX/RX)
			 */
			if (errcode & CAN_ERRCODE_DIR_EXT)
			{
            	pCAN->stats.err_bus_rx++;
            }
			else
			{
            	pCAN->stats.err_bus_tx++;
            }

			pCAN->stats.err_bus_segs[errcode & CAN_ERRCODE_SEG_MASK_EXT]++;

			pCAN->stats.err_bus++;
		}

		iflags = pCAN->hwCAN->intflags;

		if (pCAN->coremode == CAN_CORE_20A)
		{
        	iflags &= 0x1F;
        }

	}	/* End of While. */

	/* signal Binary semaphore, messages available!
	 */
	if (rx_flag)
	{
	    osal_event_send(pCAN->p_event, CAN_RX_EVENT);
	}

	if (tx_flag)
	{
	    osal_event_send(pCAN->p_event, CAN_TX_EVENT);
	}
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * CAN driver operators
 */
static const driver_ops_t ls2k_can_drv_ops =
{
    .init_entry  = CAN_init,
    .open_entry  = CAN_open,
    .close_entry = CAN_close,
    .read_entry  = CAN_read,
    .write_entry = CAN_write,
    .ioctl_entry = CAN_ioctl,
};

const driver_ops_t *can_drv_ops = &ls2k_can_drv_ops;
#endif

/******************************************************************************
 * device name
 */
const char *ls2k_can_get_device_name(const void *pCAN)
{
    return ((CAN_t *)pCAN)->dev_name;
}

#endif

/*
 * @@ End
 */



