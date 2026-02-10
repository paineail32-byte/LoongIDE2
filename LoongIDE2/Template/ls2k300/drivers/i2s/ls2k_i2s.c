/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_i2s.c
 *
 * created: 2024-10-22
 *  author:
 */

#include "bsp.h"

#if BSP_USE_I2S

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <larchintrin.h>

#include "osal.h"

#include "cpu.h"

#include "ls2k300.h"
#include "ls2k300_irq.h"
#include "ls2k_drv_io.h"
#include "ls2k_dma.h"

#include "ls2k_i2s_hw.h"
#include "ls2k_i2s.h"

//-----------------------------------------------------------------------------

#if 0 // I2S_DEBUG
#define debug(...)      printk(__VA_ARGS__)
#else
#define debug(...)      do { } while (0)
#endif

extern void *aligned_malloc(size_t size, unsigned int align);
extern void aligned_free(void *addr);

//-----------------------------------------------------------------------------

struct i2s_data;

//-----------------------------------------------------------------------------

#define I2S_USE_EVENT       1

#if I2S_USE_EVENT
/*
 * playback
 */
#define I2S_TX_NEXT_EVENT   0x01        /* 下一个数据包 */
#define I2S_TX_ERR_EVENT    0x02        /* DMA 传输出错 */
#define I2S_TX_DONE_EVENT   0x04        /* DMA 传输结束 */
/*
 * capture
 */
#define I2S_RX_NEXT_EVENT   0x10
#define I2S_RX_ERR_EVENT    0x20
#define I2S_RX_DONE_EVENT   0x40

#define I2S_EVENTS          0x77        /* 全部事件 */

#endif // #if I2S_USE_EVENT

typedef struct I2S
{
	HW_I2S_t  *hwI2S;
	int		   irqVector;				/* Irq vector number */

	I2S_Mode_t cur_mode;
	int		   working;					/* 当前正在工作 */

	struct dma_chnl_cfg tx_dma_cfg;		/* DMA 参数 */
	struct dma_chnl_cfg rx_dma_cfg;

	struct i2s_data *tx_list;			/* DMA send buffer list */
	struct i2s_data *rx_list;			/* DMA receive buffer list */

	int sent_bytes;						/* 已发送字节数 */
	int received_bytes;					/* 已接收字节数 */

	int total_tx_bytes;					/* 总的发送字节数 */
	int total_rx_bytes;					/* 总的接收字节数 */

	data_callback_t f_data_cb;			/* 数据回调函数 */
	void *tx_data_device;				/* 发送数据来源设备 */
	void *rx_data_device;				/* 接收数据目的设备 */
	done_callback_t f_done_cb;			/* 结束回调函数 */

#if I2S_USE_EVENT
    osal_event_t    p_event;
#endif

	int	 	 initialized;
	int	 	 opened;
	char	 dev_name[16];
} I2S_t;

/**
 * I2S private struct
 */
static I2S_t m_i2s_priv =
{
	.hwI2S	   = (HW_I2S_t *)PHYS_TO_UNCACHED(I2S_BASE),
#if USE_EXTINT
	.irqVector   = EXTI0_I2S_IRQ,
#else
	.irqVector   = INTC0_I2S_IRQ,
#endif

	.tx_list = 0,
	.rx_list = 0,
	.f_data_cb = NULL,
	.tx_data_device = NULL,
	.rx_data_device = NULL,
	.f_done_cb = NULL,

	.sent_bytes = 0,
	.received_bytes = 0,
	.total_tx_bytes = 0,
	.total_rx_bytes = 0,

	.initialized = 0,
	.opened	  = 0,
	.dev_name	= "i2s",
};

const void *devI2S = (void *)&m_i2s_priv;

//-----------------------------------------------------------------------------
// 数据缓冲区处理
//-----------------------------------------------------------------------------

struct i2s_data
{
	char  *buffer;
	int	   buf_len;
	struct i2s_data *last;
	struct i2s_data *next;
};

/*
 * 数据缓冲区加到发送列表
 */
static int add_tx_data_to_list(I2S_t *pI2S, char *buf, int len)
{
	struct i2s_data *item, *tmp;

	item = (struct i2s_data *)malloc(sizeof(struct i2s_data));
	if (item)
	{
		item->buffer  = buf;
		item->buf_len = len;
		item->last = NULL;
		item->next = NULL;

		if (pI2S->tx_list)
		{
			/*
			 * 找到最后一个
			 */
			tmp = pI2S->tx_list;
			while (tmp->next)
				tmp = tmp->next;

			loongarch_critical_enter();
			pI2S->total_tx_bytes += len;
			tmp->next = item;
			loongarch_critical_exit();
		}
		else
		{
			pI2S->tx_list = item;
		}

		return 0;
	}

	return -1;
}

/*
 * 从发送列表移去数据项: 总是头部一个
 */
static int remove_tx_data_from_list(I2S_t *pI2S)
{
	struct i2s_data *item;

	item = pI2S->tx_list;
	if (item)
	{
		loongarch_critical_enter();
		pI2S->tx_list = item->next;
		loongarch_critical_exit();

        aligned_free(item->buffer);
		free(item);
	}

	return 0;
}

/*
 * 从发送列表移去全部数据项
 */
static int remove_all_tx_data_from_list(I2S_t *pI2S)
{
	struct i2s_data *item;

	item = pI2S->tx_list;
	while (item)
	{
		loongarch_critical_enter();
		pI2S->tx_list = item->next;
		loongarch_critical_exit();

        aligned_free(item->buffer);
		free(item);

		item = pI2S->tx_list;
	}

	return 0;
}

/*
 * 数据缓冲区加到接收列表
 */
static int add_rx_data_to_list(I2S_t *pI2S, char *buf, int len)
{
	struct i2s_data *item, *tmp;

	item = (struct i2s_data *)malloc(sizeof(struct i2s_data));
	if (item)
	{
		item->buffer  = buf;
		item->buf_len = len;
		item->last = NULL;
		item->next = NULL;

		if (pI2S->rx_list)
		{
			/*
			 * 找到最后一个
			 */
			tmp = pI2S->rx_list;
			while (tmp->next)
				tmp = tmp->next;

			loongarch_critical_enter();
			pI2S->total_rx_bytes += len;
			tmp->next = item;
			loongarch_critical_exit();
		}
		else
		{
			pI2S->rx_list = item;
		}

		return 0;
	}

	return -1;
}

/*
 * 从发送列表移去数据项: 总是头部一个
 */
static int remove_rx_data_from_list(I2S_t *pI2S)
{
	struct i2s_data *item = pI2S->rx_list;

	if (item)
	{
		loongarch_critical_enter();
		pI2S->rx_list = item->next;
		loongarch_critical_exit();

        aligned_free(item->buffer);
		free(item);
	}

	return 0;
}

/*
 * 从发送列表移去全部数据项
 */
static int remove_all_rx_data_from_list(I2S_t *pI2S)
{
	struct i2s_data *item = pI2S->rx_list;

	while (item)
	{
		loongarch_critical_enter();
		pI2S->rx_list = item->next;
		loongarch_critical_exit();

        aligned_free(item->buffer);
		free(item);

		item = pI2S->rx_list;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// DMA 处理
//-----------------------------------------------------------------------------

/*
 * DMA 发送回调函数
 */
static void ls2k_i2s_dma_tx_callback(struct dma_chnl_cfg *cfg, int bytes, unsigned int status)
{
	int thisbytes = 0;
	I2S_t *pI2S = (I2S_t *)cfg->device;

	pI2S->hwI2S->control &= ~I2S_CTRL_TX_DMA_EN;

	pI2S->sent_bytes += bytes;

	if (status & DMA_SR_DONE)   // 传输完成
	{
	    struct i2s_data *next_buf = pI2S->tx_list->next;

		/*
		 * 继续发送
		 */
		if (next_buf && next_buf->buf_len)
		{
			thisbytes = next_buf->buf_len;
			pI2S->hwI2S->control |= I2S_CTRL_TX_DMA_EN;
			dma_restart(cfg->chNum, (char *)next_buf->buffer, thisbytes, 0);

            osal_event_send(pI2S->p_event, I2S_TX_NEXT_EVENT);
		}
		else
		{
		    pI2S->tx_dma_cfg.ccr.en = 0;
		    dma_stop(cfg->chNum);

		    osal_event_send(pI2S->p_event, I2S_TX_DONE_EVENT);
		}
	}
	else if (status & DMA_SR_ERROR)
	{
		pI2S->tx_dma_cfg.ccr.en = 0;
		dma_stop(cfg->chNum);

		osal_event_send(pI2S->p_event, I2S_TX_ERR_EVENT);
	}

	return;
}

/*
 * DMA 接收回调函数
 */
static void ls2k_i2s_dma_rx_callback(struct dma_chnl_cfg *cfg, int bytes, unsigned int status)
{
	int thisbytes = 0;
	I2S_t *pI2S = (I2S_t *)cfg->device;

	pI2S->hwI2S->control &= ~I2S_CTRL_RX_DMA_EN;

	pI2S->received_bytes += bytes;

	if (status & DMA_SR_DONE)   // 传输完成
	{
	    struct i2s_data *next_buf = pI2S->rx_list->next;
	    
		/*
		 * 继续接收
		 */
		if (next_buf && next_buf->buf_len)
		{
			thisbytes = next_buf->buf_len;
			pI2S->hwI2S->control |= I2S_CTRL_RX_DMA_EN;
			dma_restart(cfg->chNum, (char *)next_buf->buffer, thisbytes, 0);

			osal_event_send(pI2S->p_event, I2S_RX_NEXT_EVENT);
		}
		else
		{
		    pI2S->rx_dma_cfg.ccr.en = 0;
		    dma_stop(cfg->chNum);

		    osal_event_send(pI2S->p_event, I2S_RX_DONE_EVENT);
		}
	}
	else if (status & DMA_SR_ERROR)
	{
		pI2S->rx_dma_cfg.ccr.en = 0;
		dma_stop(cfg->chNum);

		osal_event_send(pI2S->p_event, I2S_RX_ERR_EVENT);
	}

	return;
}

/*
 * Prepare DMA channels
 */
static int ls2k_i2s_prepare_dma(I2S_t *pI2S)
{
	struct dma_chnl_cfg *rx_cfg = &pI2S->rx_dma_cfg;
	struct dma_chnl_cfg *tx_cfg = &pI2S->tx_dma_cfg;
	int rx_chnl = -1;
	int tx_chnl = -1;

	if (!(pI2S->cur_mode.workmode & I2S_WORK_DUAL))
	{
		return -1;
	}

	/**
	 * check whether has IDLE DMA channel
	 */
	if (dma_get_idle_channel(DMA_I2S, &rx_chnl, &tx_chnl) != 0)
		return -1;

	//-----------------------------------------------------
	// RX Channel
	//-----------------------------------------------------

	if (pI2S->cur_mode.workmode & I2S_WORK_CAPTURE)
	{
		rx_cfg->chNum   = rx_chnl;
		rx_cfg->devNum  = DMA_I2S;
		rx_cfg->device  = pI2S;
		rx_cfg->memAddr = (unsigned)(uintptr_t)0x900000000A000000ull;	/* TODO */
		rx_cfg->transbytes = 0;											/* TODO */
		rx_cfg->cb = ls2k_i2s_dma_rx_callback;

		rx_cfg->ccr.en	 = 0;				// disable the channel first.
		rx_cfg->ccr.tcie = 1;				// trans done int-disable
		rx_cfg->ccr.htie = 0;				// trans half int-disable
		rx_cfg->ccr.teie = 1;				// trans error int-disable
		rx_cfg->ccr.dir  = 0;				// 0: peripheral to mem; 1: mem to peripheral.
		rx_cfg->ccr.circ = 0;				// not circle mode
		rx_cfg->ccr.mem2mem = 0;			// memory to memory mode

		rx_cfg->ccr.pinc  = 0;				// 1=auto inc peripheral address
		rx_cfg->ccr.minc  = 1;				// 1=auto inc mem address

		switch (pI2S->cur_mode.bits_per_sample)
        {
			case BITS_PER_SAMP_16:
				rx_cfg->ccr.psize = 1;		// peripheral data width: 0=8bits, 2=32bits
				rx_cfg->ccr.msize = 1;		// memory data width:	  0=8bits, 2=32bits
                break;

			case BITS_PER_SAMP_24:
			case BITS_PER_SAMP_32:
            default:
				rx_cfg->ccr.psize = 2;		// peripheral data width: 0=8bits, 2=32bits
				rx_cfg->ccr.msize = 2;		// memory data width:	  0=8bits, 2=32bits
                break;
		}

		rx_cfg->ccr.priority = 1;			// channel priority: mid

		if (dma_start(rx_cfg, 0) != 0)
		{
			return -1;
		}
	}

	//-----------------------------------------------------
	// TX Channel
	//-----------------------------------------------------

	if (pI2S->cur_mode.workmode & I2S_WORK_PLAYBACK)
	{
		tx_cfg->chNum   = tx_chnl;
		tx_cfg->devNum  = DMA_I2S;
		tx_cfg->device  = pI2S;
		tx_cfg->memAddr = (unsigned)(uintptr_t)0x900000000B000000ull;	/* TODO */
		tx_cfg->transbytes = 0;											/* TODO */
		tx_cfg->cb = ls2k_i2s_dma_tx_callback;

		tx_cfg->ccr.en	 = 0;				// disable the channel first.
		tx_cfg->ccr.tcie = 1;				// trans done int-disable
		tx_cfg->ccr.htie = 0;				// trans half int-disable
		tx_cfg->ccr.teie = 1;				// trans error int-disable
		tx_cfg->ccr.dir  = 1;				// 0: peripheral to mem; 1: mem to peripheral.
		tx_cfg->ccr.circ = 0;				// not circle mode
		tx_cfg->ccr.mem2mem = 0;			// memory to memory mode

		tx_cfg->ccr.pinc  = 0;				// 1=auto inc peripheral address
		tx_cfg->ccr.minc  = 1;				// 1=auto inc mem address

		switch (pI2S->cur_mode.bits_per_sample)
        {
			case BITS_PER_SAMP_16:
				tx_cfg->ccr.psize = 1;		// peripheral data width: 0=8bits, 2=32bits
				tx_cfg->ccr.msize = 1;		// memory data width:	  0=8bits, 2=32bits
                break;

			case BITS_PER_SAMP_24:
			case BITS_PER_SAMP_32:
            default:
				tx_cfg->ccr.psize = 2;		// peripheral data width: 0=8bits, 2=32bits
				tx_cfg->ccr.msize = 2;		// memory data width:	  0=8bits, 2=32bits
                break;
		}

		tx_cfg->ccr.priority = 1;			// channel priority: mid

		if (dma_start(tx_cfg, 0) != 0)
		{
			return -1;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------

static void ls2k_i2s_hw_reset(void)
{
    I2S_t *pI2S = &m_i2s_priv;

    /*
     * I2S_CTRL_RESETn == 0 是 reset 状态
     */
    pI2S->hwI2S->control = 0; // &= ~I2S_CTRL_RESETn;
	delay_us(1);
	pI2S->hwI2S->control |= I2S_CTRL_RESETn;
}

extern unsigned int i2s_frequency;

/**
 * 计算 MCLK/BCLK
 *
 * samples_per_second: 采样频率: 最大 48K
 * bits_per_sample:	量化位数: 最大 32bits
 *
 */
static int ls2k_i2s_set_hw_registers(I2S_Mode_t *mode)
{
    I2S_t *pI2S = &m_i2s_priv;
	unsigned int bratio, mratio, mfrac;
	unsigned int channels, samples_per_second, bits_per_sample, LR_len;
	unsigned int xfs=256;  // 主时钟, 一般是采样频率的 128、256、384或者512倍

	if (!mode)
	{
    	return -1;
    }

	channels = mode->channels;
	samples_per_second = mode->samples_per_second;
	bits_per_sample = mode->bits_per_sample;
	LR_len = mode->bits_per_sample;

	if ((samples_per_second <= 0) || (bits_per_sample <= 0))
		return -1;

	if ((channels != 1) && (channels != 2))
		channels = 2;

	pI2S->hwI2S->control &= ~I2S_CTRL_MCLK_EN;

	/*
     * 位时钟分频系数, 分频数为 MCLK 时钟频率除以2x(RATIO+1)
     */
    bratio = xfs / (bits_per_sample * channels * 2) - 1;

    /*
     * i2s_frequency 需要除于 2
     */
    mratio = (i2s_frequency / 2) / (xfs * samples_per_second);

	mfrac = ((unsigned long)(i2s_frequency / 2) * 0x10000) / (samples_per_second * xfs);
    mfrac -= mratio * 0x10000;

    pI2S->hwI2S->config1 = ((mfrac & 0xFFFF) << 16) | (mratio & 0xFFFF);

	pI2S->hwI2S->config = ((LR_len & 0xFF) << 24) |				/* 左右声道处理的字长 */
						  ((bits_per_sample & 0xFF) << 16) |	/* TX 采样深度 */
						  ((bratio & 0xFF) << 8) |				/* BCLK 分频系数 */
						   (bits_per_sample & 0xFF);			/* RX 采样深度 */

	pI2S->hwI2S->control |= I2S_CTRL_MCLK_EN; // | I2S_CTRL_MASTER | I2S_CTRL_MSB_LSB;

	/*
	 * 等待 MCLK 时钟稳定输出
	 */
	while (!(pI2S->hwI2S->control & I2S_CTRL_MCLK_READY))
	{
		asm volatile( "nop; nop; nop; nop; nop; nop;" );
	}

	/*
	 * 等待位s时钟和声道选择时钟稳定输出
	 */
/*
	while (!(pI2S->hwI2S->control & I2S_CTRL_CLK_READY))
	{
		asm volatile( "nop; nop; nop; nop; nop; nop;" );
	}
 */

	pI2S->cur_mode.channels = channels;
	pI2S->cur_mode.samples_per_second = samples_per_second;
	pI2S->cur_mode.bits_per_sample = bits_per_sample;
	pI2S->cur_mode.workmode = mode->workmode;

	return 0;
}

/*
 * 从 data_device 读取数据,
 */
#define DATA_BUF_LEN    0x100000        /* 1M */

static int ls2k_i2s_read_next_data(I2S_t *pI2S)
{
    char *data;
    int data_len=0, frameBytes, frames;

    switch (pI2S->cur_mode.bits_per_sample)
    {
		case BITS_PER_SAMP_16:
            frameBytes = 2;
            break;

		case BITS_PER_SAMP_24:
		case BITS_PER_SAMP_32:
		default:
            frameBytes = 4;
            break;
    }

    frames = DATA_BUF_LEN / pI2S->cur_mode.channels / frameBytes;

    if (pI2S->cur_mode.workmode & I2S_WORK_PLAYBACK)
    {
		if (!(pI2S->tx_data_device && pI2S->f_data_cb))
		{
			errno = ENXIO;
			return -1;
		}

		data = (char *)aligned_malloc(DATA_BUF_LEN, 32);
		if (!data)
		{
			errno = ENOMEM;
			return -1;
		}

		data_len = pI2S->f_data_cb(pI2S->tx_data_device, data, frames);
		if (data_len <= 0)
		{
			aligned_free(data);
			errno = ENODATA;
			return -1;
		}

        data_len *= frameBytes;
		add_tx_data_to_list(pI2S, data, data_len);
    }

    if (pI2S->cur_mode.workmode & I2S_WORK_CAPTURE)
    {
		if (!(pI2S->rx_data_device && pI2S->f_data_cb))
		{
			errno = ENXIO;
			return -1;
		}

		data = (char *)aligned_malloc(DATA_BUF_LEN, 32);
		if (!data)
		{
			errno = ENOMEM;
			return -1;
		}

		data_len = pI2S->f_data_cb(pI2S->rx_data_device, data, frames);
		if (data_len <= 0)
		{
			aligned_free(data);
			errno = ENODATA;
			return -1;
		}

        data_len *= frameBytes;
		add_rx_data_to_list(pI2S, data, data_len);
    }

    debug("read next %i bytes\r\n", data_len);

    return data_len;
}

static int ls2k_i2s_start_work(I2S_t *pI2S)
{
    int ret = -1;
    unsigned int ctrl;

    if (pI2S->hwI2S->control & (I2S_CTRL_RX_EN | I2S_CTRL_TX_EN))
    {
        errno = EBUSY;
        return -1;
    }

    /*
     * 准备数据
     */
    if (ls2k_i2s_read_next_data(pI2S) <= 0)
    {
        return -1;
    }

    /*
     * 打开 DMA
     */
    ls2k_i2s_prepare_dma(pI2S);

    if (pI2S->cur_mode.workmode & I2S_WORK_PLAYBACK)
    {
        switch (pI2S->cur_mode.bits_per_sample)
        {
            case BITS_PER_SAMP_16:
                ret = dma_restart(pI2S->tx_dma_cfg.chNum,
                                  pI2S->tx_list->buffer,
                                  pI2S->tx_list->buf_len,
                                  16 );
                break;

            case BITS_PER_SAMP_24:
            case BITS_PER_SAMP_32:
            default:
                ret = dma_restart(pI2S->tx_dma_cfg.chNum,
                                  pI2S->tx_list->buffer,
                                  pI2S->tx_list->buf_len,
                                  32 );
                 break;
        }

        if (0 == ret)
        {
            ctrl = I2S_TX_EN |
                   I2S_CTRL_MASTER |
                   I2S_CTRL_MSB_LSB |
                   I2S_CTRL_RESETn;
            pI2S->hwI2S->control |= ctrl;

            /*
             * 预先准备下一个数据包
             */
            ls2k_i2s_read_next_data(pI2S);
        }
    }

    if (pI2S->cur_mode.workmode & I2S_WORK_CAPTURE)
    {
        switch (pI2S->cur_mode.bits_per_sample)
        {
            case BITS_PER_SAMP_16:
                ret = dma_restart(pI2S->rx_dma_cfg.chNum,
                                  pI2S->rx_list->buffer,
                                  pI2S->rx_list->buf_len,
                                  16 );
                break;

            case BITS_PER_SAMP_24:
            case BITS_PER_SAMP_32:
            default:
                ret = dma_restart(pI2S->rx_dma_cfg.chNum,
                                  pI2S->rx_list->buffer,
                                  pI2S->rx_list->buf_len,
                                  32 );
                 break;
        }

        if (0 == ret)
        {
            ctrl = I2S_RX_EN |
                   I2S_CTRL_MASTER |
                   I2S_CTRL_MSB_LSB |
                   I2S_CTRL_RESETn;
            pI2S->hwI2S->control |= ctrl;
            
            /*
             * 预先准备下一个数据包
             */
            // ls2k_i2s_prepare_capture_buf(pI2S);
        }
    }

	return ret;
}

static int ls2k_i2s_pause_work(I2S_t *pI2S)
{
    // TODO

	return 0;
}

static int ls2k_i2s_resume_work(I2S_t *pI2S)
{
    // TODO

	return 0;
}

static int ls2k_i2s_stop_work(I2S_t *pI2S)
{
    if (pI2S->cur_mode.workmode & I2S_WORK_PLAYBACK)
    {
        dma_stop(pI2S->tx_dma_cfg.chNum);

        remove_all_tx_data_from_list(pI2S);

        if (pI2S->f_done_cb)
        {
            pI2S->f_done_cb(pI2S->tx_data_device);
        }
    }

    if (pI2S->cur_mode.workmode & I2S_WORK_CAPTURE)
    {
        dma_stop(pI2S->rx_dma_cfg.chNum);

        remove_all_rx_data_from_list(pI2S);

        if (pI2S->f_done_cb)
        {
            pI2S->f_done_cb(pI2S->rx_data_device);
        }
    }

    pI2S->hwI2S->control &= ~I2S_EN_RXTX;

	return 0;
}

/******************************************************************************
 * initialize the device
 */
STATIC_DRV int I2S_initialize(const void *dev, void *arg)
{
	I2S_t *pI2S = &m_i2s_priv;
	I2S_Mode_t *mode = (I2S_Mode_t *)arg;

	if (pI2S->initialized)
		return 0;

	/**
	 * Hardware Init
	 */
	ls2k_i2s_hw_reset();

	if (mode)
	{
		ls2k_i2s_set_hw_registers(mode);
	}
	else
	{
		pI2S->cur_mode.channels = 2;							/* 立体声 */
		pI2S->cur_mode.samples_per_second = SAMPLES_44K;		/* 44.1K  */
		pI2S->cur_mode.bits_per_sample = BITS_PER_SAMP_16;		/* 16 bits */
		pI2S->cur_mode.workmode = I2S_WORK_PLAYBACK;			/* 播放 */
	}

#if I2S_USE_EVENT
    pI2S->p_event = osal_event_create("I2SEvent", 0);
#endif

	pI2S->initialized = 1;

	return 0;
}

/******************************************************************************
 * open the device
 */

static int ls2k_i2s_start_rw_task(void);

STATIC_DRV int I2S_open(const void *dev, void *arg)
{
	I2S_t *pI2S = &m_i2s_priv;
	I2S_Mode_t *mode = (I2S_Mode_t *)arg;

	if (pI2S->opened)
		return 0;

	if (!pI2S->initialized)
		return -1;

	if (mode)
	{
		ls2k_i2s_set_hw_registers(mode);
	}

	/*
	 * 准备 DMA
	 */

	/*
	 * 开中断
	 */
	 
	/*
	 * 开启 I2S 数据读写线程
	 */
    ls2k_i2s_start_rw_task();

	pI2S->opened = 1;

	return 0;
}

/******************************************************************************
 * close the device
 */
STATIC_DRV int I2S_close(const void *dev, void *arg)
{
	I2S_t *pI2S = &m_i2s_priv;

    if (!pI2S->opened)
    {
        return 0;
    }

	/*
	 * 停止 DMA
	 */

	/*
	 * 关中断
	 */
    // ls2k_interrupt_disable(pI2S->irqVector);

	pI2S->hwI2S->control = 0;

	ls2k_i2s_stop_work(pI2S);

	remove_all_rx_data_from_list(pI2S);
	remove_all_tx_data_from_list(pI2S);

	pI2S->opened = 0;

	return 0;
}

/******************************************************************************
 * read from the device --- capture
 */
STATIC_DRV int I2S_read(const void *dev, void *buf, int size, void *arg)
{
	int ret = 0;
	I2S_t *pI2S = &m_i2s_priv;

	if (!buf || (size <= 0))
		return -1;

	/*
	 * 没有打开或者没有设置录音模式
	 */
	if (!pI2S->opened || !(pI2S->cur_mode.workmode & I2S_WORK_CAPTURE))
		return -1;

	add_rx_data_to_list(pI2S, buf, size);

	ls2k_i2s_start_work(pI2S);

	return ret;
}

/******************************************************************************
 * write to the device --- playback
 */
STATIC_DRV int I2S_write(const void *dev, void *buf, int size, void *arg)
{
	int ret = 0;
	I2S_t *pI2S = &m_i2s_priv;

	if (!buf || (size <= 0))
		return -1;

	/*
	 * 没有打开或者没有设置回放模式
	 */
	if (!pI2S->opened || !(pI2S->cur_mode.workmode & I2S_WORK_PLAYBACK))
		return -1;

	add_tx_data_to_list(pI2S, buf, size);

	ls2k_i2s_start_work(pI2S);

	return ret;
}

/******************************************************************************
 * Driver ioctl handler
 */
STATIC_DRV int I2S_ioctl(const void *dev, int cmd, void *arg)
{
	int ret = 0;
	I2S_t *pI2S = &m_i2s_priv;

	switch (cmd)
	{
		case IOCTL_I2S_SET_MODE:
			ret = ls2k_i2s_set_hw_registers((I2S_Mode_t *)arg);
			break;

		case IOCTL_I2S_DATA_CB:
            pI2S->f_data_cb = (data_callback_t)(arg);
			break;

		case IOCTL_I2S_DATA_DEV:
            pI2S->tx_data_device = arg;     /* 播放数据源, TODO 录音数据源 */
			break;

		case IOCTL_I2S_DONE_CB:
            pI2S->f_done_cb = (done_callback_t)(arg);
			break;

        case IOCTL_I2S_START:
			ret = ls2k_i2s_start_work(pI2S);
			break;

		case IOCTL_I2S_PAUSE:
			ret = ls2k_i2s_pause_work(pI2S);
			break;

		case IOCTL_I2S_RESUME:
			ret = ls2k_i2s_resume_work(pI2S);
			break;

		case IOCTL_I2S_STOP:
			ret = ls2k_i2s_stop_work(pI2S);
			break;

		default:
			ret = -1;
			break;
	}

	return ret;
}

//---------------------------------------------------------------------------------------
// 监控线程
//---------------------------------------------------------------------------------------

#define I2S_TASK_NAME       "I2S_rw"

#define I2S_RW_STK_SIZE     8192

extern int ioctl(int fd, unsigned cmd, ...);

#if defined(OS_RTTHREAD)
/*
 * RT-Thread 优先级说明：
 *
 *     1.RT_THREAD_PRIORITY_MAX == 32
 *     2.允许同优先级、时间片
 *     3.数值越小，优先级越高
 */
#define I2S_RW_TASK_PRIO        10
#define I2S_RW_TASK_SLICE       10

#elif defined(OS_UCOS)
/*
 * UCOSII 优先级说明：
 *
 *     1.OS_LOWEST_PRIO == 127
 *       32~63:  驱动程序使用
 *       64~127: 应用程序使用
 *     2.不允许同优先级、时间片
 *     3.数值越小，优先级越高
 */
/*
 * UCOSII 优先级说明：
 *
 *      OS_CFG_PRIO_MAX 64
 */
#define I2S_RW_TASK_PRIO        20
#define I2S_RW_TASK_SLICE       10

#elif defined(OS_FREERTOS)
/*
 * FreeRTOS 优先级说明：
 *
 *     1.CONFIG_MAX_PRIORITIES == 31
 *     2.允许同优先级、时间片
 *     3.数值越大，优先级越高
 */
#define I2S_RW_TASK_PRIO        22
#define I2S_RW_TASK_SLICE       0

#else // Bare-Metal

#define I2S_RW_TASK_PRIO        0
#define I2S_RW_TASK_SLICE       0

#endif

/*
 * 线程延迟时间 10ms --- 声音数据必须有 10ms 的播放时间
 */
#define I2S_TASK_DELAY_MS       10

static void ls2k_i2s_do_rw_event(void *arg)
{
    I2S_t *pI2S = &m_i2s_priv;
    unsigned int event = (unsigned int)(uintptr_t)arg;

    event &= I2S_EVENTS;

    switch (event)
    {
        case I2S_TX_NEXT_EVENT:
            remove_tx_data_from_list(pI2S);
            ls2k_i2s_read_next_data(pI2S);
            debug("TX_NEXT_EVENT\r\n");
            break;

        case I2S_TX_ERR_EVENT:
            ls2k_i2s_stop_work(pI2S);
            printf("i2s playback error occurred.\r\n");
            break;

        case I2S_TX_DONE_EVENT:
            ls2k_i2s_stop_work(pI2S);
            printf("i2s playback done.\r\n");
            break;

        case I2S_RX_NEXT_EVENT:
            // ls2k_i2s_save_capture_data(pI2S);   // save
            remove_rx_data_from_list(pI2S);
            // ls2k_i2s_prepare_capture_buf(pI2S);
            debug("RX_NEXT_EVENT\r\n");
            break;

        case I2S_RX_ERR_EVENT:
            ls2k_i2s_stop_work(pI2S);
            printf("i2s capture error occurred.\r\n");
            break;

        case I2S_RX_DONE_EVENT:
            ls2k_i2s_stop_work(pI2S);
            printf("i2s capture done.\r\n");
            break;
    }
}

/*
 * I2S DMA 响应工作线程
 */
static void ls2k_i2s_rw_task(void *arg)
{
    I2S_t *pI2S = &m_i2s_priv;
    
    while (1)
    {
		/*
         * wait for incomming event...
	     */
		unsigned int recv = 0;

        recv = osal_event_receive(pI2S->p_event,
                                  I2S_EVENTS,
                                  OSAL_EVENT_FLAG_OR | OSAL_EVENT_FLAG_CLEAR,
                                  OSAL_WAIT_FOREVER);

	    if (recv)
	    {
            ls2k_i2s_do_rw_event((void *)(uintptr_t)recv);
	    }

        osal_task_sleep(I2S_TASK_DELAY_MS);
    }
}

static osal_task_t i2s_rw_task = NULL;

static int ls2k_i2s_start_rw_task(void)
{
    if (i2s_rw_task)
    {
        return 0;
    }
    
    i2s_rw_task = osal_task_create(I2S_TASK_NAME,
                                   I2S_RW_STK_SIZE,
                                   I2S_RW_TASK_PRIO,
                                   I2S_RW_TASK_SLICE,
                                   ls2k_i2s_rw_task,
                                   NULL);

    if (i2s_rw_task == NULL)
    {
        printf("create I2S data access task fail!\r\n");
		return -1;
	}

    return 0;
}

//---------------------------------------------------------------------------------------

#if (PACK_DRV_OPS)
/******************************************************************************
 * I2S driver operators
 */
static const driver_ops_t ls2k_i2s_drv_ops =
{
	.init_entry  = I2S_initialize,
	.open_entry  = I2S_open,
	.close_entry = I2S_close,
	.read_entry  = I2S_read,
	.write_entry = I2S_write,
	.ioctl_entry = I2S_ioctl,
};

const driver_ops_t *i2s_drv_ops = &ls2k_i2s_drv_ops;
#endif

/******************************************************************************
 * Device name
 */
const char *ls2k_i2s_get_device_name(void)
{
	return m_i2s_priv.dev_name;
}

//-----------------------------------------------------------------------------
// User API
//-----------------------------------------------------------------------------

#endif // #if BSP_USE_I2S

//-----------------------------------------------------------------------------
/*
 * @@ END
 */


