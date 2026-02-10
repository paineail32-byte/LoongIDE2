/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_can.c
 *
 * created: 2024-08-09
 *  author: Bian
 */

#include "bsp.h"

#if BSP_USE_CAN

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "cpu.h"
#include "ls2k300.h"
#include "ls2k300_irq.h"

#include "osal.h"
 
#include "ls2k_drv_io.h"
#include "drv_os_priority.h"

#include "ls2k_can.h"
#include "ls2k_can_hw.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define CAN_DEBUG       0

#if CAN_DEBUG
#define debug_prt(...)  printk(__VA_ARGS__)
#else
#define debug_prt(...)  do { } while (0)
#endif

/*
 * Default RX/TX timeout ms
 */
#define RXTX_TIMEOUT 	100

//-----------------------------------------------------------------------------
// CAN SPEED - calculate from baudrate
//-----------------------------------------------------------------------------

typedef struct
{
    union
    {
        struct                      /* 对应速率配置寄存器 */
        {
            unsigned int prop :7;   /* RW bit[6:0] 传播段宽度 */
            unsigned int ph1 :6;    /* RW bit[12:7] 相位段1 宽度 */
            unsigned int ph2 :6;    /* RW bit[18:13] 相位段2 宽度 */
            unsigned int brp :8;    /* RW bit[26:19] 位速率预分频系数 */
            unsigned int sjw :5;    /* RW bit[31:27] 同步补偿宽度 */
        };
        unsigned int btr32;
    };

    unsigned int frac;             /* 数据位速率小数分频系数. */
    int Sample_pt;                 /* 采样点百分比 % */
    int tPTS;                      /* CAN报文在CAN总线上的传输时时间延迟 ns */

} CAN_speed_t;

/******************************************************************************
 * fifo interface
 */

#define RX_FIFO_LEN		64
#define TX_FIFO_LEN		64

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
	/*
     * hardware shortcuts
	 */
	HW_CAN_t       *hwCAN;			    /* CAN 硬件 */
	unsigned int 	irqNum;			    /* 中断号 */
#if USE_EXTINT
	unsigned int 	irqNum_buf;		    /* 中断号 */
#endif

    /*
     * ioctl set CAN configures
     */
	unsigned int	workmode;
	unsigned int	baudrate; 			/* Baudrate in HZ */
	CAN_speed_t     speed;              /* Calculate from Baudrate */
    CAN_filter_t    filter;
    CAN_range_t     range;
    int             re_tx_threshold;    /* set.CAN_SET_RTXTH */
    int             timestamp_psc;      /* Inner timestamp prescale */
    int             rx_timeout;
    int             tx_timeout;

    unsigned int    r_mode;
    unsigned int    r_set;
    unsigned int    r_fltctrl;
    int             config_update;
    
	/*
     * rx and tx fifos
	 */
	CAN_fifo_t		*rxfifo;
	CAN_fifo_t		*txfifo;

    /*
     * run-time info
     */
    osal_event_t    p_event;

    unsigned int    status;
	CAN_stats_t		stats;

    int             initialized;
    int             opened;

    /*
     * device name
     */
    char            dev_name[16];
} CAN_t;

//-----------------------------------------------------------------------------
// Here is CAN interface defination
//-----------------------------------------------------------------------------

#if BSP_USE_CAN0
static CAN_t ls2k_CAN0 =
{
	.hwCAN       = (HW_CAN_t *)PHYS_TO_UNCACHED(CAN0_BASE),
#if USE_EXTINT
    .irqNum      = EXTI0_CAN0_CORE_IRQ,
    .irqNum_buf  = EXTI0_CAN0_BUF_IRQ,
#else
    .irqNum      = INTC0_CAN0_IRQ,
#endif
	.initialized = 0,
	.dev_name    = "can0",
};
const void *devCAN0 = (void *)&ls2k_CAN0;
#endif

#if BSP_USE_CAN1
static CAN_t ls2k_CAN1 =
{
	.hwCAN       = (HW_CAN_t *)PHYS_TO_UNCACHED(CAN1_BASE),
#if USE_EXTINT
    .irqNum      = EXTI0_CAN1_CORE_IRQ,
    .irqNum_buf  = EXTI0_CAN1_BUF_IRQ,
#else
    .irqNum      = INTC0_CAN1_IRQ,
#endif
	.initialized = 0,
	.dev_name    = "can1",
};
const void *devCAN1 = (void *)&ls2k_CAN1;
#endif

#if BSP_USE_CAN2
static CAN_t ls2k_CAN2 =
{
	.hwCAN       = (HW_CAN_t *)PHYS_TO_UNCACHED(CAN2_BASE),
#if USE_EXTINT
    .irqNum      = EXTI0_CAN2_CORE_IRQ,
    .irqNum_buf  = EXTI0_CAN2_BUF_IRQ,
#else
    .irqNum      = INTC0_CAN2_IRQ,
#endif
	.initialized = 0,
	.dev_name    = "can2",
};
const void *devCAN2 = (void *)&ls2k_CAN2;
#endif

#if BSP_USE_CAN3
static CAN_t ls2k_CAN3 =
{
	.hwCAN       = (HW_CAN_t *)PHYS_TO_UNCACHED(CAN3_BASE),
#if USE_EXTINT
    .irqNum      = EXTI0_CAN3_CORE_IRQ,
    .irqNum_buf  = EXTI0_CAN3_BUF_IRQ,
#else
    .irqNum      = INTC0_CAN3_IRQ,
#endif
	.initialized = 0,
	.dev_name    = "can3",
};
const void *devCAN3 = (void *)&ls2k_CAN3;
#endif

/******************************************************************************
 * FIFO IMPLEMENTATION
 ******************************************************************************/

static CAN_fifo_t *can_fifo_create(int count)
{
	CAN_fifo_t *fifo;

	fifo = osal_malloc(sizeof(CAN_fifo_t) + count * sizeof(CANMsg_t));
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
		osal_free(fifo);
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
 * CAN functions
 ******************************************************************************/

/* This function calculates BTR0 and BTR1 values for a given bitrate.
 *
 * Set communication parameters.
 * param: rate Requested baud rate in bits/second.
 * param: result Pointer to where resulting BTRs will be stored.
 * return: zero if successful to calculate a baud rate.
 *
 *
 * LS2K300 baudrate:
 *
 * 位速率分频=BRP*(PROP+PH1+PH2+1+(FRC/0x100)).
 *
 *   其中: 1 == SYNC_SEG
 *
 *   例: BRP=4; PROP=2; PH1=3; PH2=4; FRC=8'b01100000;
 *       DIV=4*(3+4+2+1+0.375) = 41.5.
 *
 *       Baudrate =
 */

#define SYNC_SEG    1           /* 同步段 */

#define PROP_MIN    1           /* 传播段 */
#define PH1_MIN     2           /* 相位段1 */
#define PH2_MIN     1           /* 相位段2 */
#define FRC_MIN     0           /* 小数段 */

#define PROP_MAX    0x7F
#define PH1_MAX     0x3F
#define PH2_MAX     0x3F
#define FRC_MAX     0xFF

#define BRP_MIN     0x2
#define BRP_MAX     0xFF

#define SJW_MIN     0x1
#define SJW_MAX     0x1F

static int ls2k_can_calc_bittiming(CAN_t *pCAN, int baudrate, CAN_speed_t *result)
{
	int error=0, best_error = 2000000000;
	int nbt=0, best_nbt=0, brp=0, best_brp=0;
	int prop=0, ph1=0, ph2=0, frc=0, sjw=0;
    int sampl_pt, tPTS=0, nbt_min, nbt_max, tmp;

    /*
     * Baudrate limits
     */
    {
        if (baudrate > CAN_SPEED_1M)
            baudrate = CAN_SPEED_1M;
    }

    if (baudrate < CAN_SPEED_10K)
        baudrate = CAN_SPEED_10K;

    /*
     * Calculate best_tbit / best_brp / brp
     */
    nbt_min = SYNC_SEG + PROP_MIN + PH1_MIN + PH2_MIN;
    nbt_max = SYNC_SEG + (PH1_MAX * 130) / 100;     // PH2_MAX = PH1_MAX * 30%
    nbt_max = nbt_max * 120 / 100;                  // PROP_MAX = NBT * 20%

    for (nbt = nbt_min; nbt < nbt_max; nbt++)
    {
        brp = apb_frequency / baudrate / nbt;
        if ((brp < BRP_MIN) || (brp > BRP_MAX))
        {
            continue;
        }

		error = baudrate - apb_frequency / brp / nbt;

		if (error < 0)
			error = -error;

		if (error <= best_error)
		{
			best_error = error;
			best_nbt = nbt;
			best_brp = brp;
		}

        // if ((error == 0) && (nbt >= 20))
        //     break;
    }

    /*
     * Calculate FRC
     */
    if (best_error != 0)
    {
	    frc = (int)(((double)apb_frequency / baudrate / best_brp - best_nbt) * 0x100);
    }

    /*
     * 传播段的延迟时间是 tPTS = 500ns
     */
    if (result)
        tPTS = (int)result->tPTS;
    if (tPTS <= 0)
        tPTS = 500;

    tmp = (tPTS * 100) / (1000000000 / baudrate);
    if (tmp > 30)
        tmp = 30;                   /* Total percent max 30% */
    prop = best_nbt * tmp / 100;

    if (best_nbt - prop < SYNC_SEG + PH1_MIN + PH2_MIN)
        prop = best_nbt - (SYNC_SEG + PH1_MIN + PH2_MIN);
    if (prop <= 0)
        prop = 1;

    nbt = best_nbt - prop - SYNC_SEG;       /* remain TQ */

    /*
     * 采样点
     */
    sampl_pt = pCAN->speed.Sample_pt;
    if (0 == sampl_pt)
    {
        sampl_pt = 75;
    }

	if (nbt <= 10)
    	sampl_pt = 80;
	else if (nbt <= 15)
    	sampl_pt = 70;

    /*
     * 相位段2
     */
	ph2 = nbt - (sampl_pt * (nbt + SYNC_SEG)) / 100;
	if (ph2 < 1)
    	ph2 = 1;
	if (ph2 > PH2_MAX)
    	ph2 = PH2_MAX;

    /*
     * 相位段1
     */
	ph1 = nbt - ph2;
	if (ph1 > PH1_MAX)
	{
		ph1 = PH1_MAX;
		ph2 = nbt - ph1;
	}

    /*
     * SJW
     */
    sjw = (ph1 < ph2) ? ph1 : ph2;
    sjw = (sjw < 4) ? sjw : 4;
    
    #if 0
    {
        // 位速率分频=BRP*(PROP+PH1+PH2+1+(FRC/0x100)).
        
        int baud = apb_frequency / best_brp / (prop + ph1 + ph2 + 1);

        if (baudrate != baud)
        {
            printk("need %i : resule %i\r\n", baudrate, baud);
        }
        
    }
    #endif

    if (result)
    {
        result->prop = prop;
        result->ph1  = ph1;
        result->ph2  = ph2;
        result->brp  = best_brp;
        result->sjw  = sjw;
        result->frac = frc;
    }

    return 0;
}

static int ls2k_can_set_baudrate(CAN_t *pCAN, int baudrate)
{
    if (pCAN->baudrate == baudrate)
        return 0;

    int rt = ls2k_can_calc_bittiming(pCAN, baudrate, &pCAN->speed);

    if (0 == rt)
    {
        pCAN->baudrate = baudrate;
        pCAN->config_update = 1;
    }

    return rt;
}

static int ls2k_can_set_workmode(CAN_t *pCAN, unsigned int mode)
{
    if (pCAN->workmode == mode)
        return 0;

    pCAN->workmode = mode;

    /*
     * can.mode register
     */
    pCAN->r_mode = CAN_MODE_BUFM;           /* Always use Inner TX Buffer */
    if (mode & CAN_MODE_RX_ADD_TS1)
        pCAN->r_mode |= CAN_MODE_RTSOP;
    if (mode & CAN_MODE_RX_CAN_TS)
        pCAN->r_mode |= CAN_MODE_ITSM;
    if (mode & CAN_MODE_RX_NO_ACK)
        pCAN->r_mode |= CAN_MODE_ACF;
    if (mode & CAN_MODE_BUS_MONITOR)
        pCAN->r_mode |= CAN_MODE_BMM;
    else if (mode & CAN_MODE_SELF_TEST)
        pCAN->r_mode |= CAN_MODE_STM;
    if (mode & CAN_MODE_TX_TIMED)
        pCAN->r_mode |= CAN_MODE_TTTM;

    /*
     * can.set register
     */
    pCAN->r_set = 0;
    if (mode & CAN_MODE_IGNORE_RTR)
        pCAN->r_set |= CAN_SET_FDRF;
    if (mode & CAN_MODE_PROTOCOL_E)
        pCAN->r_set |= CAN_SET_PEX;
    if (mode & CAN_MODE_LOOPBACK)
        pCAN->r_set |= CAN_SET_ILBP;
    pCAN->re_tx_threshold = mode & CAN_RE_TX_THRESH_MASK;
    if (pCAN->re_tx_threshold)
    {
        pCAN->r_set |= CAN_SET_RTXLE;
        pCAN->r_set |= pCAN->re_tx_threshold << CAN_SET_RTXTH_SHIFT;
    }

    /*
     * Auto Increase RX Buffer Pointer: Always Add
     */
    pCAN->r_mode |= CAN_MODE_RXBAM;     

    pCAN->config_update = 1;
    return 0;
}

static int ls2k_can_set_filter(CAN_t *pCAN, CAN_filter_t *filter)
{
    CAN_filter_t default_flt;
    
    if (NULL == filter)
    {
        filter = &default_flt;

        filter->fltmask[0]  = 0xFFFFFFFF;
        filter->fltmask[1]  = 0xFFFFFFFF;
        filter->fltmask[2]  = 0xFFFFFFFF;
        filter->fltvalue[0] = 0;
        filter->fltvalue[1] = 0;
        filter->fltvalue[2] = 0;
        filter->filter[0] = 0;
        filter->filter[1] = 0;
        filter->filter[2] = 0;
    }

    if ((pCAN->filter.fltmask[0]  != filter->fltmask[0])  ||
        (pCAN->filter.fltmask[1]  != filter->fltmask[1])  ||
        (pCAN->filter.fltmask[2]  != filter->fltmask[2])  ||
        (pCAN->filter.fltvalue[0] != filter->fltvalue[0]) ||
        (pCAN->filter.fltvalue[1] != filter->fltvalue[1]) ||
        (pCAN->filter.fltvalue[2] != filter->fltvalue[2]) ||
        (pCAN->filter.filter[0]   != filter->filter[0])   ||
        (pCAN->filter.filter[1]   != filter->filter[1])   ||
        (pCAN->filter.filter[2]   != filter->filter[2]) )
    {
        pCAN->hwCAN->fltmaskA = pCAN->filter.fltmask[0]  = filter->fltmask[0];
        pCAN->hwCAN->fltmaskB = pCAN->filter.fltmask[1]  = filter->fltmask[1];
        pCAN->hwCAN->fltmaskC = pCAN->filter.fltmask[2]  = filter->fltmask[2];
        pCAN->hwCAN->fltvalA  = pCAN->filter.fltvalue[0] = filter->fltvalue[0];
        pCAN->hwCAN->fltvalB  = pCAN->filter.fltvalue[1] = filter->fltvalue[1];
        pCAN->hwCAN->fltvalC  = pCAN->filter.fltvalue[2] = filter->fltvalue[2];
        pCAN->filter.filter[0] = filter->filter[0];
        pCAN->filter.filter[1] = filter->filter[1];
        pCAN->filter.filter[2] = filter->filter[2];
        
        pCAN->r_fltctrl &= ~0x0FFF;
        
        if (pCAN->r_mode & CAN_MODE_FDE)
        {
            if (filter->filter[0] & CAN_FILTER_STD)
                pCAN->r_fltctrl |= CAN_FCTRL_FA_FE;
            else if (filter->filter[0] & CAN_FILTER_EXT)
                pCAN->r_fltctrl |= CAN_FCTRL_FA_FB;

            if (filter->filter[1] & CAN_FILTER_STD)
                pCAN->r_fltctrl |= CAN_FCTRL_FB_FE;
            else if (filter->filter[1] & CAN_FILTER_EXT)
                pCAN->r_fltctrl |= CAN_FCTRL_FB_FB;

            if (filter->filter[2] & CAN_FILTER_STD)
                pCAN->r_fltctrl |= CAN_FCTRL_FC_FE;
            else if (filter->filter[2] & CAN_FILTER_EXT)
                pCAN->r_fltctrl |= CAN_FCTRL_FC_FB;
        }
        else    /* CAN 2.0 */
        {
            if (filter->filter[0] & CAN_FILTER_STD)
                pCAN->r_fltctrl |= CAN_FCTRL_FA_NE;
            else if (filter->filter[0] & CAN_FILTER_EXT)
                pCAN->r_fltctrl |= CAN_FCTRL_FA_NB;

            if (filter->filter[1] & CAN_FILTER_STD)
                pCAN->r_fltctrl |= CAN_FCTRL_FB_NE;
            else if (filter->filter[1] & CAN_FILTER_EXT)
                pCAN->r_fltctrl |= CAN_FCTRL_FB_NB;

            if (filter->filter[2] & CAN_FILTER_STD)
                pCAN->r_fltctrl |= CAN_FCTRL_FC_NE;
            else if (filter->filter[2] & CAN_FILTER_EXT)
                pCAN->r_fltctrl |= CAN_FCTRL_FC_NB;
        }

        if (pCAN->filter.filter[0] || pCAN->filter.filter[1] || pCAN->filter.filter[2])
            pCAN->r_mode |= CAN_MODE_AFM;
        
        pCAN->config_update = 1;
    }

    return 0;
}

static int ls2k_can_set_range(CAN_t *pCAN, CAN_range_t *range)
{
    CAN_range_t default_rng;
    
    if (NULL == range)
    {
        range = &default_rng;
        range->rangelo = 0;
        range->rangehi = 0;
        range->enable  = 0;
    }

    if ((pCAN->range.rangelo != range->rangelo) ||
        (pCAN->range.rangehi != range->rangehi) ||
        (pCAN->range.enable  != range->enable) )
    {
        pCAN->hwCAN->fltrlo = pCAN->range.rangelo = range->rangelo;
        pCAN->hwCAN->fltrhi = pCAN->range.rangehi = range->rangehi;
        pCAN->range.enable  = range->enable;

        pCAN->r_fltctrl &= ~0xF000;

        if (pCAN->r_mode & CAN_MODE_FDE)
        {
            if (range->enable & CAN_RANGE_STD)
                pCAN->r_fltctrl |= CAN_FCTRL_FR_FE;
            else if (range->enable & CAN_RANGE_EXT)
                pCAN->r_fltctrl |= CAN_FCTRL_FR_FB;
        }
        else    /* CAN 2.0 */
        {
            if (range->enable & CAN_RANGE_STD)
                pCAN->r_fltctrl |= CAN_FCTRL_FR_NE;
            else if (range->enable & CAN_RANGE_EXT)
                pCAN->r_fltctrl |= CAN_FCTRL_FR_NB;
        }

        if (pCAN->range.enable)
            pCAN->r_mode |= CAN_MODE_AFM;

        pCAN->config_update = 1;
    }

    return 0;
}

static int ls2k_can_set_ts_psc(CAN_t *pCAN, unsigned int psc)
{
    if (pCAN->timestamp_psc != psc)
    {
        pCAN->timestamp_psc = psc;
        pCAN->hwCAN->ts &= ~CAN_TS_PSC_MASK;
        pCAN->hwCAN->ts |= psc << CAN_TS_PSC_SHIFT;
    }

    return 0;
}

static int ls2k_can_get_statics(CAN_t *pCAN, CAN_stats_t **st)
{
    pCAN->stats.rx_errors = (pCAN->hwCAN->errcnt >> 16) & 0x1FF;
    pCAN->stats.tx_errors = pCAN->hwCAN->errcnt & 0x1FF;
    
    pCAN->stats.std_rate_errors = pCAN->hwCAN->brerr >> 16;
    pCAN->stats.fd_rate_errors = pCAN->hwCAN->brerr & 0xFFFF;

    *st = &pCAN->stats;

    return 0;
}

static int ls2k_can_get_status(CAN_t *pCAN, unsigned int *status)
{
    unsigned int rv;
    
    *status = pCAN->status;     /* Only record CAN_STATUS_BUF_ERROR */
    
    rv = pCAN->hwCAN->status;
    
    if (rv & CAN_SR_IDLE)
        *status |= CAN_STATUS_IDLE;
    if (rv & CAN_SR_DOR)
        *status |= CAN_STATUS_RX_OVERFLOW;
    if (rv & CAN_SR_RXNE)
        *status |= CAN_STATUS_RXBUF_NOT_EMPTY;

    rv = pCAN->hwCAN->errsr;
    
    if (rv & CAN_ERRSR_BUSOFF)
        *status |= CAN_STATUS_BUS_OFF;
    if (rv & CAN_ERRSR_ERP)
        *status |= CAN_STATUS_ERROR_PASSIVE;
    if (rv & CAN_ERRSR_ERA)
        *status |= CAN_STATUS_ERROR_ACTIVE;

    return 0;
}

/******************************************************************************
 * CAN DMA support
 ******************************************************************************/

/******************************************************************************
 * CAN Hardware Operating
 ******************************************************************************/

static inline void ls2k_can_hw_reset(CAN_t *pCAN)
{
    pCAN->hwCAN->mode |= CAN_MODE_RST;
    asm volatile( "nop; nop; nop; nop; nop; nop;" );
    pCAN->hwCAN->mode &= ~CAN_MODE_RST;
}

/*
 * put the CAN-Control to ENABLE mode
 */
static int ls2k_can_hw_start(CAN_t *pCAN)
{
    unsigned int rv;

	if (pCAN->opened)
        return 0;
        
	if (!pCAN->rxfifo || !pCAN->txfifo)
    	return -1;

	can_fifo_clear(pCAN->txfifo);           /* empty the TX fifo */
	can_fifo_clear(pCAN->rxfifo);           /* empty the RX fifo */
	
    memset(&pCAN->stats, 0, sizeof(CAN_stats_t));

    /*
     * Set CAN.mode register
     */
    rv = pCAN->hwCAN->mode;
    if (rv != pCAN->r_mode)
        pCAN->hwCAN->mode = pCAN->r_mode;

    /*
     * Set CAN.btr register
     */
    if (pCAN->speed.btr32)
    {
        {
            pCAN->hwCAN->btrnormal = pCAN->speed.btr32;
            pCAN->hwCAN->frcdiv &= ~CAN_FRC_NBT_MASK;
            pCAN->hwCAN->frcdiv |= pCAN->speed.frac;
        }
    }

    /*
     * Set CAN.mode register
     */
    rv = pCAN->hwCAN->fltctrl;
    if (rv != pCAN->r_fltctrl)
        pCAN->hwCAN->fltctrl = pCAN->r_fltctrl;

    /*
     * Set CAN.set register
     */
    rv = pCAN->hwCAN->set;
    if (rv != pCAN->r_set)
        pCAN->hwCAN->set = pCAN->r_set;

    pCAN->hwCAN->set |= CAN_SET_ENABLE;     /* Enable the CAN Controller */

    /*
     * Set register for Clear something
     */
    pCAN->hwCAN->cmd = 0xFC;
    pCAN->hwCAN->isr = 0x1FFF;
    pCAN->hwCAN->ien = 0x1FFF << 16;
    pCAN->hwCAN->imask = 0x1FFF << 16;
    
    /*
     * Enable CAN interrupt
     */
    ls2k_interrupt_enable(pCAN->irqNum);

    pCAN->hwCAN->ien |= 0x0EEF;

    return 0;
}

/*
 * put the CAN-Control to DISABLE mode
 */
static void ls2k_can_hw_stop(CAN_t *pCAN)
{
	/*
     * Disable Can all interrupts
	 */
    pCAN->hwCAN->ien = 0x1FFF << 16;
    pCAN->hwCAN->isr = 0x1FFF;
    pCAN->hwCAN->imask = 0x1FFF << 16;

	/*
     * Disable CAN interrupt
	 */
	ls2k_interrupt_disable(pCAN->irqNum);

    pCAN->hwCAN->set &= ~CAN_SET_ENABLE;

    return;
}

/**
 * Try to send message "msg", if hardware txfifo is full, then -1 is returned.
 * Be sure to have disabled CAN interrupts when entering this function.
 */
static int ls2k_can_send_msg(CAN_t *pCAN, CANMsg_t *msg)
{
    int i;
    MSGT0_t t0;
    MSGT1_t t1;
    unsigned int vals[2];       /* Total 8 bytes */
    unsigned int txsel;

    /*
     * prepare tx data
     */
    t0.value = 0;
    t0.rtr = msg->rtr;                  /* Only for CAN 2.0 */
    if (msg->extended)					/* Extended Frame */
	{
	    t0.id = msg->id;
	    t0.xtd = 1;
	}
	else								/* Standard Frame */
	{
	    t0.id = msg->id << 18;
	}

	t1.value = 0;
    t1.dlc = (msg->len <= 8) ? msg->len : 8;
    t1.dlc = ((t1.dlc + 3) / 4) * 4;    /* Multiples of 4 */

    vals[0] = 0;
    vals[1] = 0;

    if (0 == msg->rtr)
    {
        for (i=0; i<t1.dlc; i++)
        {
            int j = i / 4;
            int k = i % 4;

            vals[j] |= (unsigned int)msg->data[i] << (k * 8);
        }
    }
    else
    {
        /* CAN2.0 Remote Transmission Request */
    }

    /*
     * Use tx buffer index
     */
    txsel = 0;

    pCAN->hwCAN->txsel = txsel;

    pCAN->hwCAN->head0 = t0.value;
    pCAN->hwCAN->head1 = t1.value;

    if (0 == msg->rtr)
    {
        for (i=0; i<(t1.dlc+3)/4; i++)
            pCAN->hwCAN->txdata[i] = vals[i];
    }

    /*
     * set transmit command
     */
    pCAN->hwCAN->txcmd = 1 << txsel;

    /*
     * TODO more ?
     */

	return 0;
}

/**
 * Receive a message from DMA or Inner RX Buffer
 */
static int ls2k_can_receive_message(CAN_t *pCAN, int fromDMA)
{
    int i;
	MSGT0_t t0;
	MSGT1_t t1;
	CANMsg_t *msg;
    unsigned int vals[2];       /* Total 8 bytes */

    /*
     * the rx fifo is not empty put 1 message into rxfifo for later use.
     * get empty (or make room) message
     */
    msg = can_fifo_put_claim(pCAN->rxfifo, 1);

    {
        /*
         * Protocol leader
         */
        t0.value = pCAN->hwCAN->rxdata;
        t1.value = pCAN->hwCAN->rxdata;
    }

    msg->rtr = t0.rtr;
    msg->extended = t0.xtd;
    msg->len = (t1.dlc <= 8) ? t1.dlc : 8;

    if (msg->extended)                  /* Extended Frame */
        msg->id = t0.id;
    else                                /* Standard frame */
        msg->id = t0.id >> 18;

    for (i=0; i<8; i++)
        msg->data[i] = 0;

    if (0 == msg->rtr)
    {
        for (i=0; i<(msg->len+3)/4; i++)
        {
            {
                /*
                 * Inner buffer Rx data flag
                 */
                if (pCAN->hwCAN->rxsr & CAN_RXSR_MOF)   
                {
                    vals[i] = pCAN->hwCAN->rxdata;
                }
            }
        }

        for (i=0; i<msg->len; i++)
        {
            int j = i / 4;
            int k = i % 4;

            msg->data[i] = (unsigned char)(vals[j] >> (k * 8));
        }
    }
    else                /* CAN2.0 Remote Transmission Request */
    {
        // RTR frame has no data.
    }

    /*
     * make message available to the user
     */
    can_fifo_put(pCAN->rxfifo);

    return 0;
}

/**
 * Interrupt Handler
 */
STATIC_DRV int CAN_close(const void *dev, void *arg);

static void ls2k_can_interrupt_handler(int vector, void *arg)
{
	unsigned int isr0, isr, ien;
    int rx_flag=0, tx_flag=0;
	CANMsg_t *msg;
    CAN_t *pCAN = (CAN_t *)arg;

    if (NULL == pCAN)
    {
        return;
    }

	pCAN->stats.ints++;
	ien = pCAN->hwCAN->ien;
	isr0 = isr = pCAN->hwCAN->isr;
	isr &= ien;

	while (isr != 0)
	{
		/**
         * Receive Packet
		 */
		if (isr & (CAN_ISR_RX | CAN_ISR_RXF | CAN_ISR_RBNE))
		{
		    unsigned int rxsr, prop_cnt;

            rxsr = pCAN->hwCAN->rxsr;
		    prop_cnt = (rxsr & CAN_RXSR_FRC_MASK) >> CAN_RXSR_FRC_SHIFT;

            while (prop_cnt-- > 0) // && !(rxsr & CAN_RXSR_RXE))
            {
                ls2k_can_receive_message(pCAN, 0);

			    pCAN->stats.rx_msgs++;

			    /*
                 * signal the semaphore only once
			     */
                rx_flag = 1;

                /*
                 * continue;
                 */
            }

			/*
             * Re-Enable RX buffer for a new message
			 */
		    // pCAN->hwCAN->cmd = CAN_CMD_RELEASERXBUF;
		    debug_prt("RXI\r\n");
		}

		/**
         * Send Packet Done
		 */
		if (isr & CAN_ISR_TX)
		{
			/*
             * there is room in tx fifo of HW
			 */
			if (!can_fifo_empty(pCAN->txfifo))
			{
				/*
                 * send 1 more messages
				 */
				msg = can_fifo_claim_get(pCAN->txfifo);

				if (ls2k_can_send_msg(pCAN, msg))
				{
					/* ERROR! We got an TX interrupt telling us tx fifo is empty,
					 * yet it is not. Complain about this max 10 times
					 */
					if (pCAN->stats.txbuf_errors < 10)
					{
						debug_prt("CAN: got TX interrupt but TX fifo in not empty\r\n");
					}

					pCAN->status |= CAN_STATUS_BUF_ERROR;
					pCAN->stats.txbuf_errors++;
				}

				/*
                 * free software-fifo space taken by sent message
				 */
				can_fifo_get(pCAN->txfifo);

				pCAN->stats.tx_msgs++;

				/*
                 * wake any sleeping thread waiting for "fifo not full"
				 */
                tx_flag = 1;

                /*
                 * continue;
                 */
			}

			debug_prt("TXI\r\n");
		}

		/**
         * Error & Warn Interrupt
		 */
		if (isr & CAN_ISR_EWL)
		{
            pCAN->hwCAN->cmd |= CAN_CMD_ERCRST;

			pCAN->stats.err_ewl++;
			debug_prt("EWL\r\n");
		}

		if (isr & CAN_ISR_DO)
		{
		    pCAN->hwCAN->cmd |= CAN_CMD_RRB;
			pCAN->stats.err_dover++;
			debug_prt("RxOV\r\n");
		}

		if (isr & CAN_ISR_FCS)
		{
			pCAN->stats.err_fcs++;
			debug_prt("FCS\r\n");
		}

		if (isr & CAN_ISR_AL)
		{
            unsigned int alc = pCAN->hwCAN->alc;

            switch (alc & CAN_ALC_ID_MASK)
            {
                case CAN_ALC_BASE_ID:
                    pCAN->stats.alc_base_id++;
                    pCAN->stats.alc_bit_pos[alc & 0x1F]++;
                    break;

                case CAN_ALC_SRR_RTR:
                    pCAN->stats.alc_srr_rtr++;
                    break;

                case CAN_ALC_IDE:
                    pCAN->stats.alc_ide++;
                    break;

                case CAN_ALC_EXTENSION:
                    pCAN->stats.alc_ext++;
                    pCAN->stats.alc_bit_pos[alc & 0x1F]++;
                    break;

                case CAN_ALC_RTR:
                    pCAN->stats.alc_rtr++;
                    break;
            }

            pCAN->stats.err_alost++;
            debug_prt("AL\r\n");
		}

		if (isr & CAN_ISR_BE)
		{
			unsigned int errcode = pCAN->hwCAN->errcapt;
			errcode &= CAN_ECAPT_TYPE_MASK;
			errcode >>= CAN_ECAPT_TYPE_SHIFT;

			debug_prt("BUSE %02X\r\n", errcode);

			/* Some kind of BUS error, only used for statistics.
			 * Error Register is decoded and put into can->stats.
			 */
			switch (errcode)
			{
				case CAN_ETYPE_BIT:
					pCAN->stats.err_bit++;
					break;
				case CAN_ETYPE_FRM:
					pCAN->stats.err_form++;
					break;
				case CAN_ETYPE_STUP:
					pCAN->stats.err_stuff++;
					break;
				case CAN_ETYPE_CRC:
				    pCAN->stats.err_crc++;
				    break;
				case CAN_ETYPE_ACK:
				    pCAN->stats.err_ack++;
				    break;
				default:
					pCAN->stats.err_other++;
					break;
			}

            errcode = pCAN->hwCAN->errcapt;
            errcode &= CAN_ECAPT_POS_MASK;
            if (errcode > 9) errcode = 9;
			pCAN->stats.err_pos[errcode]++;
			pCAN->stats.err_bus++;

            /**
             * BUS OFF, shut controller
             */
            if (pCAN->hwCAN->errsr & CAN_ERRSR_BUSOFF)
            {
                pCAN->status |= CAN_STATUS_BUS_OFF;
                CAN_close(pCAN, NULL);
            }
            else
            {
                pCAN->hwCAN->cmd |= CAN_CMD_ERCRST;
            }
		}

        if (isr & CAN_ISR_TXBHC)
        {
            pCAN->hwCAN->txcmd |= 1 << 16;
            debug_prt("TXBHC\r\n");
        }

        /*
         * write then clear
         */
        pCAN->hwCAN->isr = isr0;

        /*
         * re-read check for loop
         */
		isr0 = isr = pCAN->hwCAN->isr;
		isr &= ien;

	}	/* End of While. */

	/**
     * signal Binary semaphore, messages available!
	 */
    if (osal_is_osrunning())
    {
	    if (rx_flag)
	    {
	        osal_event_send(pCAN->p_event, CAN_RX_EVENT);
	    }

        if (tx_flag)
	    {
	        osal_event_send(pCAN->p_event, CAN_TX_EVENT);
	    }
	}
}

/******************************************************************************
 * CAN Driver Implement
 ******************************************************************************/

extern int ls2k_can_init_hook(const void *dev);

/**
 * CAN_initialize
 */
STATIC_DRV int CAN_initialize(const void *dev, void *arg)
{
    CAN_t *pCAN = (CAN_t *)dev;

	if (dev == NULL)
    {
        errno = EINVAL;
    	return -1;
    }

    if (pCAN->initialized)
        return 0;

    /*
     * 引脚复用
     */
    ls2k_can_init_hook(dev);
    
    /*
     * Clear CAN_t* first
     */
    pCAN->workmode = -1;
    pCAN->baudrate = -1;
    pCAN->rx_timeout = RXTX_TIMEOUT;
    pCAN->tx_timeout = RXTX_TIMEOUT;
    pCAN->timestamp_psc = 0;
    pCAN->config_update = 0;

    memset(&pCAN->speed, 0,  sizeof(CAN_speed_t));
    memset(&pCAN->filter, 0, sizeof(CAN_filter_t));
    memset(&pCAN->range, 0,  sizeof(CAN_range_t));

    /*
     * Reset the CAN Controller, Must reset first
     */
	ls2k_can_hw_reset(pCAN);

    /*
     * Set CAN default configures
     */
    pCAN->speed.Sample_pt = 80;
    pCAN->speed.tPTS = 400;

    ls2k_can_set_workmode(pCAN, 0);
    ls2k_can_set_baudrate(pCAN, CAN_SPEED_500K);
    ls2k_can_set_filter(pCAN, NULL);
    ls2k_can_set_range(pCAN, NULL);
    ls2k_can_set_ts_psc(pCAN, 0);

    /*
     * Event
     */
    pCAN->p_event = osal_event_create(pCAN->dev_name, 0);
    if (NULL == pCAN->p_event)
    {
        printk("create CAN event fail.\r\n");
        return -1;
    }

	/*
     * CAN interrupt handler
	 */
    ls2k_install_irq_handler(pCAN->irqNum, ls2k_can_interrupt_handler, (void *)pCAN);
#if !USE_EXTINT
    ls2k_set_irq_routeip(pCAN->irqNum, INT_ROUTE_IP3);
#endif

    pCAN->initialized = 1;

    printk("CAN%i controller initialized.\r\n",
            (VA_TO_PHYS(pCAN->hwCAN) == CAN0_BASE) ? 0 :
            (VA_TO_PHYS(pCAN->hwCAN) == CAN1_BASE) ? 1 :
            (VA_TO_PHYS(pCAN->hwCAN) == CAN2_BASE) ? 2 :
            (VA_TO_PHYS(pCAN->hwCAN) == CAN3_BASE) ? 3 : -1);

	return 0;
}

/**
 * CAN_open
 */
STATIC_DRV int CAN_open(const void *dev, void *arg)
{
    CAN_t *pCAN = (CAN_t *)dev;

	if (dev == NULL)
    {
        errno = EINVAL;
    	return -1;
    }

    if (!pCAN->initialized)
    {
        errno = EIO;
    	return -1;
    }
    
    if (pCAN->opened)
        return 0;

	/*
     * allocate fifos
	 */
    if (!pCAN->rxfifo)
    {
		pCAN->rxfifo = can_fifo_create(RX_FIFO_LEN);
		if (!pCAN->rxfifo)
		{
			errno = ENOMEM;
			return -1;
		}
    }

    if (!pCAN->txfifo)
    {
		pCAN->txfifo = can_fifo_create(TX_FIFO_LEN);
		if (!pCAN->txfifo)
		{
			can_fifo_free(pCAN->rxfifo);
			errno = ENOMEM;
			return -1;
		}
    }

	ls2k_can_hw_start(pCAN);

    pCAN->opened = 1;
	return 0;
}

/**
 * CAN_close
 */
STATIC_DRV int CAN_close(const void *dev, void *arg)
{
    CAN_t *pCAN = (CAN_t *)dev;

	if (dev == NULL)
    {
        errno = EINVAL;
    	return -1;
    }

    if (!pCAN->opened)
        return 0;

	/*
     * Stop CAN contoller, and set to reset mode
	 */
	ls2k_can_hw_stop(pCAN);

#if 0
    /*
     * Release the Buffer. 因为有中断内调用, 不能释放. rtthread
     */
	can_fifo_free(pCAN->rxfifo);
	can_fifo_free(pCAN->txfifo);
#else
	can_fifo_clear(pCAN->rxfifo);
	can_fifo_clear(pCAN->txfifo);
#endif

    pCAN->opened = 0;
	return 0;
}

/**
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
        errno = EINVAL;
    	return -1;
    }

    if (!pCAN->opened)
    {
        errno = EIO;
        return -1;
    }

	while (left >= sizeof(CANMsg_t))
	{
		/*
         * A bus off error may have occured after read
		 */
		if (pCAN->hwCAN->errsr & CAN_ERRSR_BUSOFF)
		{
		    errno = EIO;
        	break;
        }

		srcmsg = can_fifo_claim_get(pCAN->rxfifo);

		if (!srcmsg)
		{
		    int tmo = pCAN->rx_timeout;
		    unsigned int recv_event;

			/* No more messages in reception fifo. Wait for incoming packets
			 * return if no wait OR readed some messages.
			 */
			if ((tmo == 0) || (left != size))
            	break;

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

			/*
             * no errors detected, it must be a message
			 */
			continue;
		}

		/*
         * got message, copy it to userspace buffer
		 */
		*dstmsg = *srcmsg;

		/*
         * Return borrowed message, RX interrupt can use it again
		 */
		can_fifo_get(pCAN->rxfifo);

		left -= sizeof(CANMsg_t);
		dstmsg++;
	}

    return size - left;
}

/**
 * CAN write
 */
STATIC_DRV int CAN_write(const void *dev, void *buf, int size, void *arg)
{
    int left = size;
    CAN_t *pCAN = (CAN_t *)dev;
    CANMsg_t *msg = (CANMsg_t *)buf, *fifo_msg;
    unsigned int txsr;

	if ((dev == NULL) || (buf == NULL) || (left < sizeof(CANMsg_t)))
    {
        errno = EINVAL;
    	return -1;
    }

    if (!pCAN->opened)
    {
        errno = EIO;
        return -1;
    }
    
	/*
     * A bus off may have occured before being send
	 */
	if (pCAN->hwCAN->errsr & CAN_ERRSR_BUSOFF)
    {
        errno = ENETDOWN;
    	return -2;
    }

	msg->len = (msg->len > 8) ? 8 : msg->len;

    /*
     * is device idle?
	 */
    txsr = pCAN->hwCAN->txsr & CAN_TXSR_MASK;

	/**
     * If no messages in software tx fifo, we will try to send first message
	 * by putting it directly into the HW TX fifo.
	 */
	if ((txsr == CAN_TXSR_IDLE) && can_fifo_empty(pCAN->txfifo))
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

	/**
     * Put messages into software fifo
	 */
	while (left >= sizeof(CANMsg_t))
	{
		msg->len = (msg->len > 8) ? 8 : msg->len;

		fifo_msg = can_fifo_put_claim(pCAN->txfifo, 0);

		if (!fifo_msg)
		{
		    int tmo = pCAN->tx_timeout;
		    unsigned int recv_event;

            /* Waiting only if no messages previously sent.
             * return if no wait OR written some messages.
			 */
			if ((tmo == 0) || (left != size))
            	break;

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

			/*
             * did we get woken up by a BUS OFF error?
			 */
			if (pCAN->hwCAN->errsr & CAN_ERRSR_BUSOFF)
			{
                errno = ENETDOWN;
				break;
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

/**
 * CAN control
 */
#define CAN_OPENED_BREAK    if (pCAN->opened)  { errno = EBUSY;  rt = -1; break; }
#define PTR_NULL_BREAK(ptr) if (ptr == NULL)   { errno = EINVAL; rt = -1; break; }

STATIC_DRV int CAN_ioctl(const void *dev, int cmd, void *arg)
{
    int rt = 0;
    unsigned int val;
    CAN_t *pCAN = (CAN_t *)dev;

	if (dev == NULL)
    {
        errno = EINVAL;
    	return -1;
    }

	switch (cmd)
	{
		case IOCTL_CAN_SET_WORKMODE:
		    CAN_OPENED_BREAK;
		    rt = ls2k_can_set_workmode(pCAN, (unsigned int)(uintptr_t)arg);
		    break;

		case IOCTL_CAN_SET_BAUDRATE:
		    CAN_OPENED_BREAK;
		    rt = ls2k_can_set_baudrate(pCAN, (int)(uintptr_t)arg);
		    break;

		case IOCTL_CAN_SET_FILTER:
		    CAN_OPENED_BREAK;
		    PTR_NULL_BREAK(arg);
		    rt = ls2k_can_set_filter(pCAN, (CAN_filter_t *)arg);
		    break;

		case IOCTL_CAN_SET_RANGE:
		    CAN_OPENED_BREAK;
		    PTR_NULL_BREAK(arg);
		    rt = ls2k_can_set_range(pCAN, (CAN_range_t *)arg);
		    break;

		case IOCTL_CAN_SET_SAMPLEPT:
            CAN_OPENED_BREAK;
            pCAN->speed.Sample_pt = (int)(uintptr_t)arg;
            break;

		case IOCTL_CAN_SET_PROP_NS:
		    CAN_OPENED_BREAK;
		    pCAN->speed.tPTS = (int)(uintptr_t)arg;
		    break;

		case IOCTL_CAN_SET_TS_PSC:
		    CAN_OPENED_BREAK;
		    rt = ls2k_can_set_ts_psc(pCAN, (unsigned int)(uintptr_t)arg);
		    break;

		case IOCTL_CAN_GET_CUR_TS:
		    PTR_NULL_BREAK(arg);
		    *((unsigned int *)arg) = pCAN->hwCAN->ts & CAN_TS_CURRENT_MASK;
		    break;

		case IOCTL_CAN_GET_STATS:
		    PTR_NULL_BREAK(arg);
		    rt = ls2k_can_get_statics(pCAN, (CAN_stats_t **)arg);
		    break;

		case IOCTL_CAN_GET_STATUS:
		    PTR_NULL_BREAK(arg);
		    rt = ls2k_can_get_status(pCAN, (unsigned int *)arg);
		    break;

		case IOCTL_CAN_SET_RX_TMO:
		    val = (unsigned int)(uintptr_t)arg;
		    pCAN->rx_timeout = val < 1000 ? val : 1000;
		    break;

		case IOCTL_CAN_SET_TX_TMO:
		    val = (unsigned int)(uintptr_t)arg;
		    pCAN->tx_timeout = val < 1000 ? val : 1000;
		    break;

		case IOCTL_CAN_GET_BUFS:
		    PTR_NULL_BREAK(arg);
		    *((unsigned int *)arg) = RX_FIFO_LEN;
		    break;

		default:
		    errno = ENOTSUP;
			rt = -1;
			break;
	}

	return rt;
}

/******************************************************************************
 * CAN driver operators
 ******************************************************************************/
 
#if (PACK_DRV_OPS)
static const driver_ops_t ls2k_can_drv_ops =
{
    .init_entry  = CAN_initialize,
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
    if (NULL == pCAN)
        return NULL;

    return ((CAN_t *)pCAN)->dev_name;
}

#endif // #if BSP_USE_CAN

//-----------------------------------------------------------------------------
/*
 * @@ End
 */


