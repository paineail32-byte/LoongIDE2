/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_adc.c
 *
 * created: 2024-07-28
 *  author: Bian
 */

/*
 * TODO 1. DMA 通道号的申请和使用有次序上的问题
 *      2. 注入通道
 */

#include "bsp.h"

#if BSP_USE_ADC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "termios.h"
#include "cpu.h"

#include "ls2k300.h"
#include "ls2k300_irq.h"
#include "ls2k_drv_io.h"
#include "ls2k_dma.h"

#include "ls2k_adc_hw.h"
#include "ls2k_adc.h"

#include "osal.h"

//-----------------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------------

#define ADC_USE_MUTEX   1

#if ADC_USE_MUTEX
#define LOCK()      osal_mutex_obtain(m_adc_priv.p_mutex, OSAL_WAIT_FOREVER)
#define UNLOCK()    osal_mutex_release(m_adc_priv.p_mutex)
#else
#define LOCK()
#define UNLOCK()
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

typedef struct ADC
{
    HW_ADC_t    *hwADC;
    int          irqVector;             /* Irq vector number */

    ADC_Mode_t   mode;
    ADC_Inject_t inject;

    struct dma_chnl_cfg dma_cfg;        /* DMA 参数 */
    volatile int dma_cb_result;         /* DMA 中断回调结果 */

#if ADC_USE_MUTEX
    osal_mutex_t p_mutex;
#endif

    int          initialized;
    int          opened;
    char         dev_name[16];
} ADC_t;

/**
 * ADC private struct
 */
static ADC_t m_adc_priv =
{
    .hwADC       = (HW_ADC_t *)PHYS_TO_UNCACHED(ADC_BASE),
#if USE_EXTINT
    .irqVector   = EXTI1_ADC_IRQ,
#else
    .irqVector   = INTC0_ADC_IRQ,
#endif
    .initialized = 0,
    .opened      = 0,
    .dev_name    = "adc",
};

const void *devADC = (void *)&m_adc_priv;

/**
 * ADC DMA buffer
 */
static unsigned int m_dma_buf[16]; 

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static int channel_virt_to_real(int vChannel)
{
    switch (vChannel)
    {
        case ADC_CH_1:  return ADC_Channel_0;
        case ADC_CH_2:  return ADC_Channel_1;
        case ADC_CH_3:  return ADC_Channel_2;
        case ADC_CH_4:  return ADC_Channel_3;
        case ADC_CH_5:  return ADC_Channel_4;
        case ADC_CH_6:  return ADC_Channel_5;
        case ADC_CH_7:  return ADC_Channel_6;
        case ADC_CH_8:  return ADC_Channel_7;
    }

    return ADC_Channel_0;
}

//-----------------------------------------------------------------------------

static void ls2k_adc_dma_callback(struct dma_chnl_cfg *cfg, int bytes, unsigned int status)
{
    if (status & DMA_SR_DONE)   // 传输完成
    {
        m_adc_priv.dma_cb_result = 0;
    }
    else if (status)            // DMA_SR_ERROR
    {
        m_adc_priv.dma_cb_result = -EIO;
    }

    return;
}

/*
 * 设置使用 DMA 接收数据
 */
static int ls2k_adc_set_dma_transfer(ADC_t *pADC, int channelcount)
{
    int rx_chnl;
    struct dma_chnl_cfg *cfg = &pADC->dma_cfg;

    if (dma_get_idle_channel(DMA_ADC, &rx_chnl, NULL) == 0)
    {
        memset((void *)cfg, 0, sizeof(struct dma_chnl_cfg));

        cfg->chNum   = rx_chnl;
        cfg->devNum  = DMA_ADC;
        cfg->device  = (void *)devADC;
        cfg->memAddr = (unsigned long)&m_dma_buf[0];

        cfg->transbytes = channelcount * sizeof(int);
        cfg->cb = ls2k_adc_dma_callback;

    //  cfg->ccr.en    = 0;              // disable the channel first.
        cfg->ccr.tcie  = 1;              // trans done int-disable
    //  cfg->ccr.htie  = 0;              // trans half int-disable
        cfg->ccr.teie  = 1;              // trans error int-disable
    //  cfg->ccr.dir   = 0;              // 0: peripheral to mem; 1: mem to peripheral.
        cfg->ccr.circ  = 1;              // not circle mode
    //  cfg->ccr.mem2mem = 0;            // memory to memory mode

    //  cfg->ccr.pinc  = 0;              // 1=auto inc peripheral address
        cfg->ccr.minc  = 1;              // 1=auto inc mem address
        cfg->ccr.psize = 2;              // peripheral data width: 0=8bits, 2=32bits
        cfg->ccr.msize = 2;              // memory data width:     0=8bits, 2=32bits
        cfg->ccr.priority = 2;           // channel priority: high

        return 0;
    }

    cfg->chNum = -1;

    return -1;
}
        
//-----------------------------------------------------------------------------
     
/*
 * 设置 ADC 默认配置
 */
static int ls2k_adc_set_mode(ADC_t *pADC, ADC_Mode_t *mode)
{
    int i;
    unsigned int cr1, cr2;

    if (mode == NULL)
        return -1;

    cr1 = pADC->hwADC->cr1;
    cr2 = pADC->hwADC->cr2;
    
    // LOCK();

    /*
     * 读出 CR1, CR2, Clear
     */
    cr1 &= CR1_CLEAR_MASK_R;
    cr2 &= CR2_CLEAR_MASK_R;

    /* 控制信号相位调节 */
    if (ADC_OPHASE_BEFORE_1 == mode->OutPhaseSel)
        cr1 |= ADC_CR1_OPS_CLKBEFORE1;
    else if (ADC_OPHASE_AFTER_1 == mode->OutPhaseSel)
        cr1 |= ADC_CR1_OPS_CLKAFTER1;

    /* 分频系数 */
    cr1 |= (mode->ClkDivider << ADC_CR1_CLK_SHIFT) & ADC_CR1_CLK_MASK;
    cr2 |= (mode->ClkDivider << (ADC_CR2_CLK_SHIFT-6)) & ADC_CR2_CLK_MASK;

    if (mode->DiffMode)                 /* 差分输入 */
        cr1 |= ADC_CR1_DIFFMOD;

    if (mode->ScanMode)                 /* 扫描模式 */
        cr1 |= ADC_CR1_SCAN;

#if 0
    if (mode->ContinuousMode)           /* 连续转换 */
        cr2 |= ADC_CR2_CONT;
#endif

    if (mode->TrigEdgeDown)             /* 时钟触发沿: 1=下降沿 */
        cr2 |= ADC_CR2_EDGE_DOWN;

    /* 规则通道外部触发源 */
    switch (mode->ExternalTrigSrc)
    {
        case ADC_TRIG_ATIM_CC1: cr2 |= ADC_CR2_EXTSEL_ATIM_CC1; break;
        case ADC_TRIG_ATIM_CC2: cr2 |= ADC_CR2_EXTSEL_ATIM_CC2; break;
        case ADC_TRIG_ATIM_CC3: cr2 |= ADC_CR2_EXTSEL_ATIM_CC3; break;
        case ADC_TRIG_GTIM_CC2: cr2 |= ADC_CR2_EXTSEL_GTIM_CC2; break;
        case ADC_TRIG_EXTI_11:  cr2 |= ADC_CR2_EXTSEL_EXTI_11;  break;
        case ADC_TRIG_SWATART:
        default:                cr2 |= ADC_CR2_EXTSEL_SWATART;  break;
    }

    if (mode->DataAlignLeft)            /* 转换结果左对齐 */
        cr2 |= ADC_CR2_ALIGN_LEFT;

    /*
     * Set CR1 & CR2
     */
    pADC->hwADC->cr1 = cr1;
    pADC->hwADC->cr2 = cr2;
    
    /*
     * 规则通道数
     */
    adc_set_sq_len(pADC->hwADC, mode->RegularChannelCount);    

    for (i=0; i<mode->RegularChannelCount; i++)
    {
        /* 规则通道 */
        adc_set_sq(pADC->hwADC, i+1, channel_virt_to_real(mode->RegularChannels[i]));

        /* 采样周期 */
        adc_set_samp_time(pADC->hwADC, i+1, mode->SampleClocks[i]);
    }

    //-----------------------------------------------------
    // 启用 DMA
    //-----------------------------------------------------
    
    if (mode->RegularChannelCount > 1)
    {
        if (ls2k_adc_set_dma_transfer(pADC, mode->RegularChannelCount) == 0)
        {
            /*
             * Use DMA now
             */
            pADC->hwADC->cr2 |= ADC_CR2_DMA;
            
            adc_set_discnum(pADC->hwADC, mode->RegularChannelCount, 0);
        }
        else
        {
        	// UNLOCK();
            return -1;
        }
    }
    else if (pADC->dma_cfg.chNum >= 0)
    {
        ls2k_dma_close(NULL, (void *)(long)pADC->dma_cfg.chNum);
        pADC->dma_cfg.chNum = -1;
    }

    pADC->mode = *mode;
    
    // UNLOCK();
    return 0;
}

/*
 * 校正
 */
static int ls2k_adc_do_calibrate(HW_ADC_t *hwADC)
{
    int tmo = 1000;
    unsigned int cr2;

    hwADC->cr2 |= ADC_CR2_ADON;
    cr2 = hwADC->cr2;

    cr2 &= ~(ADC_CR2_RSTCAL | ADC_CR2_CAL);
    cr2 |= ADC_CR2_RSTCAL | ADC_CR2_CAL;

    hwADC->cr2 = cr2;

    while (hwADC->cr2 & (ADC_CR2_RSTCAL | ADC_CR2_CAL))
    {
        if (tmo-- <= 0)
        {
            printk("adc calibrate timeout = %i\r\n", tmo);
            return -1;
        }
    }

    return 0;
}

/*
 * 设置一个规则通道
 */
static int ls2k_adc_set_1_regular_channel(ADC_t *pADC, int vchnl, int rank, int samptime)
{
    int channel = channel_virt_to_real(vchnl);

    if ((samptime < ADC_SAMP_1P) || (samptime > ADC_SAMP_128P))
        samptime = ADC_SAMP_64P;

    adc_set_samp_time(pADC->hwADC, rank, samptime);

    if (adc_get_sq(pADC->hwADC, rank) != channel)
    {
        adc_set_sq(pADC->hwADC, rank, channel);
    }
    
    return 0;
}

/*
 * 软件触发转换
 */
static int ls2k_adc_start_trigger_convert(ADC_t *pADC, int Jmode)
{
    unsigned int sr = pADC->hwADC->sr;
    unsigned int cr2 = pADC->hwADC->cr2;

    cr2 |= Jmode ? ADC_CR2_JEXTTRIG : ADC_CR2_EXTTRIG;
    pADC->hwADC->cr2 = cr2;

    /*
     * clear EOC
     */
    sr &= ~(Jmode ? ADC_SR_JEOC : ADC_SR_EOC);
    pADC->hwADC->sr = sr;

    /*
     * start AD convert
     */
    pADC->hwADC->cr2 |= Jmode ? ADC_CR2_JSWSTART : ADC_CR2_SWSTART;

    return 0;
}

/*
 * 查询状态寄存器, 等待转换结束
 */
static int ls2k_adc_wait_convert_done(ADC_t *pADC, unsigned int wait_sr)
{
    int tmo = 1000;

    while (!(pADC->hwADC->sr & wait_sr))
    {
        if (tmo-- <= 0)
        {
            printk("poll adc convert done timeout!\r\n");
            return -1;
        }
    }
    
    return 0;
}

//-----------------------------------------------------------------------------

/*
 * 设置注入通道
 */
static int ls2k_adc_set_inject(ADC_t *pADC, ADC_Inject_t *inject)
{
    int i;
    unsigned int cr1 = pADC->hwADC->cr1;
    unsigned int cr2 = pADC->hwADC->cr2;

    if (inject == NULL)
        return -1;

    LOCK();
    
    /*
     * 读出 CR1, CR2, Clear
     */
    cr1 &= CR1_CLEAR_MASK_J;
    cr2 &= CR2_CLEAR_MASK_J;

    /* 开启自动的注入通道组转换 */
    if (inject->JTrigAuto)
        cr1 |= ADC_CR1_JAUTO;
        
    /* 注入触发模式 */
    if (ADC_JTRIG_END == inject->JTrigMode)
        cr1 |= 2 << ADC_CR2_JTRIGMOD_SHIFT;
    else if (ADC_JTRIG_END_RESET == inject->JTrigMode)
        cr1 |= 1 << ADC_CR2_JTRIGMOD_SHIFT;
        
    /* 注入通道外部触发源 */
    switch (inject->ExternalJTrigSrc)
    {
        case ADC_JTRIG_ATIM_TRGO: cr2 |= ADC_CR2_JEXTSEL_ATIM_TRGO; break;
        case ADC_JTRIG_ATIM_CC4:  cr2 |= ADC_CR2_JEXTSEL_ATIM_CC4;  break;
        case ADC_JTRIG_GTIM_TRGO: cr2 |= ADC_CR2_JEXTSEL_GTIM_TRGO; break;
        case ADC_JTRIG_GTIM_CC1:  cr2 |= ADC_CR2_JEXTSEL_GTIM_CC1;  break;
        case ADC_JTRIG_EXTI_15:   cr2 |= ADC_CR2_JEXTSEL_EXTI_15;   break;
        case ADC_JTRIG_JSWSTART:
        default:                  cr2 |= ADC_CR2_JEXTSEL_JSWSTART;  break;
    }

    /*
     * Set CR1 & CR2
     */
    pADC->hwADC->cr1 = cr1;
    pADC->hwADC->cr2 = cr2;
    
    /*
     * 注入通道数
     */
    adc_set_jsq_len(pADC->hwADC, inject->InjectChannelCount);   
    
    for (i=0; i<inject->InjectChannelCount; i++)
    {
        /* 规则通道 */
        adc_set_jsq(pADC->hwADC, i+1, channel_virt_to_real(inject->InjectChannels[i]));

        /* 注入通道偏移 */
        adc_set_joff(pADC->hwADC, i+1, inject->InjectOffsets[i]);
        
        /* 采样周期 */
        adc_set_samp_time(pADC->hwADC, i+1, inject->SampleClocks[i]);
    }

    UNLOCK();
    return 0;
}

/*
 * 获取注入一个通道转换结果
 *
 * 参数: *param     入口注入通道顺序号, 出口转换结果
 *
 */
static int ls2k_adc_get_inject_result(ADC_t *pADC, int *param)
{
    if (param)
    {
        int rank = *param;
        *param = adc_get_jsq_result(pADC->hwADC, rank);
        return 0;
    }
    
    return -1;
}

/******************************************************************************
 * initialize the device
 */
STATIC_DRV int ADC_initialize(const void *dev, void *arg)
{
	ADC_t *pADC = &m_adc_priv;
    ADC_Mode_t *mode = (ADC_Mode_t *)arg;
    
    if (pADC->initialized)
        return 0;

#if ADC_USE_MUTEX
    pADC->p_mutex = osal_mutex_create(pADC->dev_name, OSAL_OPT_FIFO);
    if (pADC->p_mutex == NULL)
    {
    	return -1;
    }
#endif

    if (!mode)
    {
        ADC_Mode_t mode1;
        
	    mode1.OutPhaseSel = ADC_OPHASE_SAME;
	    mode1.ClkDivider = 1;
	    mode1.DiffMode = 0;
	    mode1.ScanMode = 1;
	    mode1.ContinuousMode = 0;
	    mode1.TrigEdgeDown = 0;
	    mode1.ExternalTrigSrc = ADC_TRIG_SWATART;
	    mode1.DataAlignLeft = 0;

	    /*
         * 仅设置一个. 采样周期与分频系数相关决定采样速率
         */
	    mode1.RegularChannelCount = 1;
	    mode1.RegularChannels[0] = ADC_CH_5;
	    mode1.SampleClocks[0] = ADC_SAMP_64P;
        
        mode = &mode1;
    }

    pADC->dma_cfg.chNum = -1;
    
    if (ls2k_adc_set_mode(pADC, mode) != 0)
        return -1;

    //-------------------------------------------------------------------------
    // 中断
    //-------------------------------------------------------------------------

    // TODO
    
    pADC->initialized = 1;

    return 0;
}

/******************************************************************************
 * open the device
 */
STATIC_DRV int ADC_open(const void *dev, void *arg)
{
	ADC_t *pADC = &m_adc_priv;
	ADC_Mode_t *mode = (ADC_Mode_t *)arg;

    if (pADC->opened)
        return 0;
        
    if (!pADC->initialized && !mode)
        return -1;

    if ((mode) && (ls2k_adc_set_mode(pADC, mode) != 0))
        return -1;

    LOCK();

    /*
     * 使用DMA进行数据传输
     */
    if (pADC->mode.RegularChannelCount > 1)
    {
        if (dma_start(&pADC->dma_cfg, 0) != 0)
        {
            UNLOCK();
            return -1;
        }
    }

    /*
     * 进行校正
     */
    pADC->hwADC->cr2 |= ADC_CR2_ADON;

    if (ls2k_adc_do_calibrate(pADC->hwADC) != 0)
    {
        UNLOCK();
        return -1;
    }

    /*
     * 开中断
     */
    // TODO

    pADC->opened = 1;

    UNLOCK();
    return 0;
}

/******************************************************************************
 * close the device
 */
STATIC_DRV int ADC_close(const void *dev, void *arg)
{
	ADC_t *pADC = &m_adc_priv;
    
    LOCK();
    pADC->hwADC->cr2 &= ~ADC_CR2_ADON;
    
    /*
     * 关中断
     */
    // TODO

    /*
     * 停止 DMA
     */
    if (pADC->mode.RegularChannelCount > 1)
    {
        ls2k_dma_close(NULL, (void *)(long)pADC->dma_cfg.chNum);
        pADC->dma_cfg.chNum = -1;
    }
    
    pADC->opened = 0;
    UNLOCK();
    
    return 0;
}

/******************************************************************************
 * read from the device
 */
STATIC_DRV int ADC_read(const void *dev, void *buf, int size, void *arg)
{
    int ret = 0;
	ADC_t *pADC = &m_adc_priv;
    int vchnl = (long)arg;
    int channelcount;

    if (!pADC->opened)
        return -1;

    if (!buf || ((long)buf & 0x3))
        return -1;

    LOCK();

    channelcount = pADC->mode.RegularChannelCount;

    /**
     * 一个规则通道
     */
    if ((channelcount == 1) && (vchnl >= ADC_CH_1) && (vchnl <= ADC_CH_8))
    {
        unsigned int result;

        ls2k_adc_set_1_regular_channel(pADC, vchnl, 1, ADC_SAMP_64P);

        ls2k_adc_start_trigger_convert(pADC, 0);

        ls2k_adc_wait_convert_done(pADC, ADC_SR_EOC);

        result = pADC->hwADC->dr;
            
        *(unsigned int *)buf = result;
        ret = 4;
    }

    /**
     * 多个规则通道, 转换结果是通过 DMA存放的
     */
    else if (channelcount > 1)
    {
        int dma_rt = 1;
        
        pADC->dma_cb_result = 1;

        ls2k_adc_start_trigger_convert(pADC, 0);

        if (pADC->dma_cfg.ccr.tcie || pADC->dma_cfg.ccr.teie)   /* DMA 中断 */
        {
            /*
             * 等待 DMA 中断回调, timeout?
             */
            while (pADC->dma_cb_result == 1)
                ;

            dma_rt = pADC->dma_cb_result;
        }
        else                                                  
        {
            dma_rt = dma_wait_done(pADC->dma_cfg.chNum, 1000);  /* 查询 */
        }

        if (dma_rt == 0)
        {
            /*
             * 读指定通道转换结果
             */
            if ((vchnl >= ADC_CH_1) && (vchnl <= ADC_CH_8))
            {
                *(unsigned int *)buf = m_dma_buf[vchnl - 1];
                ret = 4;
            }
            else
            {
                unsigned int *p = (unsigned int *)buf;
                int i, count;

                count = size / sizeof(int);
                count = (channelcount < count) ? channelcount : count;

                for (i=0; i<count; i++)
                {
                    *p++ = m_dma_buf[i];
                }
            
                ret = count * sizeof(int);
            }
        }
        else
        {
            // DMA Result ERROR
        }
    }

    UNLOCK();
    return ret;
}

/******************************************************************************
 * Driver ioctl handler
 */
STATIC_DRV int ADC_ioctl(const void *dev, int cmd, void *arg)
{
    int ret = 0;
	ADC_t *pADC = &m_adc_priv;

    switch (cmd)
    {
        case IOCTL_ADC_SET_MODE:
        {
            int reopenIt = pADC->opened;
            if (reopenIt)
            {
                ADC_close(NULL, NULL);
            }

            ret = ls2k_adc_set_mode(pADC, (ADC_Mode_t *)arg);

            if (reopenIt)
            {
                ADC_open(NULL, NULL);
            }

            break;
        }

        case IOCTL_ADC_CALIBRATE:
        	LOCK();
            ls2k_adc_do_calibrate(pADC->hwADC);
            UNLOCK();
            break;

        case IOCTL_ADC_SET_INJECT:
        	LOCK();
            ret = ls2k_adc_set_inject(pADC, (ADC_Inject_t *)arg);
            UNLOCK();
            break;

        case IOCTL_ADC_GET_JRESULT:
            ret = ls2k_adc_get_inject_result(pADC, (int *)arg);
            break;
            
        default:
            ret = -1;
            break;
    }

    return ret;
}

//---------------------------------------------------------------------------------------

#if (PACK_DRV_OPS)
/******************************************************************************
 * ADC driver operators
 */
static const driver_ops_t ls2k_adc_drv_ops =
{
    .init_entry  = ADC_initialize,
    .open_entry  = ADC_open,
    .close_entry = ADC_close,
    .read_entry  = ADC_read,
    .write_entry = NULL,
    .ioctl_entry = ADC_ioctl,
};

const driver_ops_t *adc_drv_ops = &ls2k_adc_drv_ops;
#endif

/******************************************************************************
 * Device name
 */
const char *ls2k_adc_get_device_name(void)
{
    return m_adc_priv.dev_name;
}

//-----------------------------------------------------------------------------
// User API
//-----------------------------------------------------------------------------

int adc_read_1(int channel)
{
    unsigned int adc_val;

    if (ADC_read(NULL, &adc_val, 4, (void *)(long)channel) == 4)
        return (int)adc_val;

    return -1;
}

#endif // #if BSP_USE_ADC

//-----------------------------------------------------------------------------
/*
 * @@ END
 */

 
