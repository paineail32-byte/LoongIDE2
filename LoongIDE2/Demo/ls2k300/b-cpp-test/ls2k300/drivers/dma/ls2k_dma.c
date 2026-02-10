/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_dma.c
 *
 * created: 2024-06-19
 *  author: 
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include "bsp.h"

#include "ls2k300.h"
#include "ls2k300_irq.h"

#include "ls2k_drv_io.h"

#include "ls2k_dma_hw.h"
#include "ls2k_dma.h"

//-----------------------------------------------------------------------------

/*
 * XXX 如果 ConsolePort 使用 DMA, 将无法 print
 */
#if 0
#define DEBUG(fmt, ...)      printk(fmt, ##__VA_ARGS__ )
#else
#define DEBUG(fmt, ...)
#endif

//-----------------------------------------------------------------------------

#define DMA_STATEMACHINE    0

typedef struct
{
    struct dma_chnl_cfg cfg;
    int  irqVector;                 /* Irq vector number */

#if DMA_STATEMACHINE
    int  state;

#define DMA_STATE_IDLE      0x00    /* 未使用: 空闲 */
#define DMA_STATE_READY     0x01    /* 已使用: 就绪 */
#define DMA_STATE_PAUSE     0x02    /*         暂停 */
#define DMA_STATE_TXING     0x10    /*         正在发送 */
#define DMA_STATE_RXING     0x20    /*         正在接收 */

#else
    int  idle;                      /* 1==idle */
#endif

    char dev_name[16];              /* 设备名称 */
} DMA_CHNL_t;

//-----------------------------------------------------------------------------
// DMA devices
//-----------------------------------------------------------------------------

/**
 * DMA 物理设备
 */
static HW_DMA_t *hwDMA = (HW_DMA_t *)PHYS_TO_UNCACHED(DMA_BASE);

static int m_dma_initialized = 0;   /* 初始化标志 */

/**
 * DMA 通道配置
 */
static DMA_CHNL_t dma_channels[CHNL_COUNT];

//-----------------------------------------------------------------------------

static void ls2k_dma_channel_interrupt_enable(DMA_CHNL_t *chnl);
static void ls2k_dma_channel_interrupt_disable(DMA_CHNL_t *chnl);

//-----------------------------------------------------------------------------
// DMA funcs
//-----------------------------------------------------------------------------

/**
 * DMA 外设编号转换为外设地址
 */
static char *peripheral_device_name(struct dma_chnl_cfg *cfg)
{
    char *s = "";
    
    switch (cfg->devNum)
    {
        case DMA_UART0: if (cfg->ccr.dir) s = "UART0-TX"; else s = "UART0-RX"; break;
        case DMA_UART1: if (cfg->ccr.dir) s = "UART1-TX"; else s = "UART1-RX"; break;
        case DMA_UART2: if (cfg->ccr.dir) s = "UART2-TX"; else s = "UART2-RX"; break;
        case DMA_UART3: if (cfg->ccr.dir) s = "UART3-TX"; else s = "UART3-RX"; break;
        case DMA_UART4: if (cfg->ccr.dir) s = "UART4-TX"; else s = "UART4-RX"; break;
        case DMA_UART5: if (cfg->ccr.dir) s = "UART5-TX"; else s = "UART5-RX"; break;
        case DMA_UART6: if (cfg->ccr.dir) s = "UART6-TX"; else s = "UART6-RX"; break;
        case DMA_UART7: if (cfg->ccr.dir) s = "UART7-TX"; else s = "UART7-RX"; break;
        case DMA_UART8: if (cfg->ccr.dir) s = "UART8-TX"; else s = "UART8-RX"; break;
        case DMA_UART9: if (cfg->ccr.dir) s = "UART9-TX"; else s = "UART9-RX"; break;

        case DMA_I2C0:  if (cfg->ccr.dir) s = "I2C0-TX";  else s = "I2C0-RX";  break;
        case DMA_I2C1:  if (cfg->ccr.dir) s = "I2C1-TX";  else s = "I2C1-RX";  break;
        case DMA_I2C2:  if (cfg->ccr.dir) s = "I2C2-TX";  else s = "I2C2-RX";  break;
        case DMA_I2C3:  if (cfg->ccr.dir) s = "I2C3-TX";  else s = "I2C3-RX";  break;

        case DMA_SPI2:  if (cfg->ccr.dir) s = "SPI2-TX";  else s = "SPI2-RX";  break;
        case DMA_SPI3:  if (cfg->ccr.dir) s = "SPI3-TX";  else s = "SPI3-RX";  break;

        case DMA_I2S:   if (cfg->ccr.dir) s = "I2S-TX";   else s = "I2S-RX";   break;

        case DMA_ADC:   if (cfg->ccr.dir) s = "ADC-dmaE";  else s = "ADC-RX";  break;

        case DMA_CAN0:  if (cfg->ccr.dir) s = "CAN0-dmaE"; else s = "CAN0-RX"; break;
        case DMA_CAN1:  if (cfg->ccr.dir) s = "CAN1-dmaE"; else s = "CAN1-RX"; break;
        case DMA_CAN2:  if (cfg->ccr.dir) s = "CAN2-dmaE"; else s = "CAN2-RX"; break;
        case DMA_CAN3:  if (cfg->ccr.dir) s = "CAN3-dmaE"; else s = "CAN3-RX"; break;

        case DMA_ATIM:  s = "ATIM"; break;
        case DMA_GTIM:  s = "GTIM"; break;
    }

    return s;
}

/**
 * DMA 外设编号转换为外设地址
 */
static unsigned int peripheral_number_to_address(struct dma_chnl_cfg *cfg)
{
    unsigned int addr = 0;
    
    switch (cfg->devNum)
    {
        case DMA_UART0: addr = 0x16100000; break;   // RX & TX
        case DMA_UART1: addr = 0x16100400; break;   // RX & TX
        case DMA_UART2: addr = 0x16100800; break;   // RX & TX
        case DMA_UART3: addr = 0x16100c00; break;   // RX & TX
        case DMA_UART4: addr = 0x16101000; break;   // RX & TX
        case DMA_UART5: addr = 0x16101400; break;   // RX & TX
        case DMA_UART6: addr = 0x16101800; break;   // RX & TX
        case DMA_UART7: addr = 0x16101c00; break;   // RX & TX
        case DMA_UART8: addr = 0x16102000; break;   // RX & TX
        case DMA_UART9: addr = 0x16102400; break;   // RX & TX

        case DMA_I2C0:  addr = 0x16108010; break;   // RX & TX
        case DMA_I2C1:  addr = 0x16109010; break;   // RX & TX
        case DMA_I2C2:  addr = 0x1610a010; break;   // RX & TX
        case DMA_I2C3:  addr = 0x1610b010; break;   // RX & TX

        case DMA_SPI2:  addr = 0x1610c040; break;   // RX & TX
        case DMA_SPI3:  addr = 0x1610e040; break;   // RX & TX

        case DMA_I2S:   if (0 == cfg->ccr.dir)
                            addr = 0x1611400c;      // RX
                        else
                            addr = 0x16114010;      // TX
                        break;

        case DMA_ADC:   addr = 0x1611c04c; break;   // RX

        case DMA_CAN0:  addr = 0x16110098; break;   // RX
        case DMA_CAN1:  addr = 0x16111098; break;   // RX
        case DMA_CAN2:  addr = 0x16112098; break;   // RX
        case DMA_CAN3:  addr = 0x16113098; break;   // RX

        case DMA_ATIM:  addr = 0x16118000; break;   // fixed: CH1 CH2 CH3 CH4 COM UP TRG
        case DMA_GTIM:  addr = 0x16119000; break;   // fixed: CH1 CH2 CH3 CH4  -  UP TRG
    }
    
    return addr;
}

/**
 * DMA 通道设置
 */
static int ls2k_dma_channel_config(int channel, unsigned int devNum)
{
    unsigned int cc13, cc14, cc15;

    if ((channel < 0) || (channel >= CHNL_COUNT))
    {
        return -1;
    }

    /*
     * 成对使用通道. 00: 通道0、1;  01: 通道2、3;  10: 通道4、5;  11: 通道6、7.
     */
    switch (devNum)
    {
        case DMA_UART0:
        case DMA_UART1:
        case DMA_UART2:
        case DMA_UART3:
        case DMA_UART4:
        case DMA_UART5:
        case DMA_UART6:
        case DMA_UART7:
        case DMA_UART8:
        case DMA_UART9:
        case DMA_I2C0:
        case DMA_I2C1:
        case DMA_I2C2:
        case DMA_I2C3:
        case DMA_SPI2:
        case DMA_SPI3:
        case DMA_I2S:
#if 0
            if (channel & 0x1)      /* 通道号 1/3/5/7 不需要配置 */
            {
                return 0;
            }
#endif

            channel >>= 1;          /* 通道号 0/2/4/6 转换为: 0/1/2/3 */
            break;
    }

    switch (devNum)
    {
        /*
         * 成对使用通道
         */
        case DMA_UART0:
            cc13 = READ_REG32(CHIP_CTRL13_BASE);
            cc13 &= ~CTRL13_UART0_DMA_MAP_MASK;
            cc13 |= channel << CTRL13_UART0_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL13_BASE, cc13);
            break;

        case DMA_UART1:
            cc13 = READ_REG32(CHIP_CTRL13_BASE);
            cc13 &= ~CTRL13_UART1_DMA_MAP_MASK;
            cc13 |= channel << CTRL13_UART1_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL13_BASE, cc13);
            break;
            
        case DMA_UART2:
            cc13 = READ_REG32(CHIP_CTRL13_BASE);
            cc13 &= ~CTRL13_UART2_DMA_MAP_MASK;
            cc13 |= channel << CTRL13_UART2_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL13_BASE, cc13);
            break;
            
        case DMA_UART3:
            cc13 = READ_REG32(CHIP_CTRL13_BASE);
            cc13 &= ~CTRL13_UART3_DMA_MAP_MASK;
            cc13 |= channel << CTRL13_UART3_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL13_BASE, cc13);
            break;
            
        case DMA_UART4:
            cc13 = READ_REG32(CHIP_CTRL13_BASE);
            cc13 &= ~CTRL13_UART4_DMA_MAP_MASK;
            cc13 |= channel << CTRL13_UART4_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL13_BASE, cc13);
            break;
            
        case DMA_UART5:
            cc13 = READ_REG32(CHIP_CTRL13_BASE);
            cc13 &= ~CTRL13_UART5_DMA_MAP_MASK;
            cc13 |= channel << CTRL13_UART5_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL13_BASE, cc13);
            break;
            
        case DMA_UART6:
            cc13 = READ_REG32(CHIP_CTRL13_BASE);
            cc13 &= ~CTRL13_UART6_DMA_MAP_MASK;
            cc13 |= channel << CTRL13_UART6_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL13_BASE, cc13);
            break;
            
        case DMA_UART7:
            cc13 = READ_REG32(CHIP_CTRL13_BASE);
            cc13 &= ~CTRL13_UART7_DMA_MAP_MASK;
            cc13 |= channel << CTRL13_UART7_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL13_BASE, cc13);
            break;
            
        case DMA_UART8:
            cc14 = READ_REG32(CHIP_CTRL14_BASE);
            cc14 &= ~CTRL14_UART8_DMA_MAP_MASK;
            cc14 |= channel << CTRL14_UART8_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL14_BASE, cc14);
            break;
            
        case DMA_UART9:
            cc14 = READ_REG32(CHIP_CTRL14_BASE);
            cc14 &= ~CTRL14_UART9_DMA_MAP_MASK;
            cc14 |= channel << CTRL14_UART9_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL14_BASE, cc14);
            break;

        case DMA_I2C0:
            cc14 = READ_REG32(CHIP_CTRL14_BASE);
            cc14 &= ~CTRL14_I2C0_DMA_MAP_MASK;
            cc14 |= channel << CTRL14_I2C0_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL14_BASE, cc14);
            break;

        case DMA_I2C1:
            cc14 = READ_REG32(CHIP_CTRL14_BASE);
            cc14 &= ~CTRL14_I2C1_DMA_MAP_MASK;
            cc14 |= channel << CTRL14_I2C1_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL14_BASE, cc14);
            break;

        case DMA_I2C2:
            cc14 = READ_REG32(CHIP_CTRL14_BASE);
            cc14 &= ~CTRL14_I2C2_DMA_MAP_MASK;
            cc14 |= channel << CTRL14_I2C2_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL14_BASE, cc14);
            break;

        case DMA_I2C3:
            cc14 = READ_REG32(CHIP_CTRL14_BASE);
            cc14 &= ~CTRL14_I2C3_DMA_MAP_MASK;
            cc14 |= channel << CTRL14_I2C3_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL14_BASE, cc14);
            break;

        case DMA_SPI2:
            cc14 = READ_REG32(CHIP_CTRL14_BASE);
            cc14 &= ~CTRL14_SPI2_DMA_MAP_MASK;
            cc14 |= channel << CTRL14_SPI2_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL14_BASE, cc14);
            break;
            
        case DMA_SPI3:
            cc14 = READ_REG32(CHIP_CTRL14_BASE);
            cc14 &= ~CTRL14_SPI3_DMA_MAP_MASK;
            cc14 |= channel << CTRL14_SPI3_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL14_BASE, cc14);
            break;

        case DMA_I2S:
            cc14 = READ_REG32(CHIP_CTRL14_BASE);
            cc14 &= ~CTRL14_I2S_DMA_MAP_MASK;
            cc14 |= channel << CTRL14_I2S_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL14_BASE, cc14);
            break;

        /*
         * 任意通道. 对应的DMA通道为000-111: 通用DMA通道0-7.
         */
        case DMA_ADC:
            cc14 = READ_REG32(CHIP_CTRL14_BASE);
            cc14 &= ~CTRL14_ADC_DMA_MAP_MASK;
            cc14 |= channel << CTRL14_ADC_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL14_BASE, cc14);
            break;
            
        case DMA_CAN0:
            cc14 = READ_REG32(CHIP_CTRL14_BASE);
            cc14 &= ~CTRL14_CAN0_DMA_MAP_MASK;
            cc14 |= channel << CTRL14_CAN0_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL14_BASE, cc14);
            break;

        case DMA_CAN1:
            cc14 = READ_REG32(CHIP_CTRL14_BASE);
            cc14 &= ~CTRL14_CAN1_DMA_MAP_MASK;
            cc14 |= channel << CTRL14_CAN1_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL14_BASE, cc14);
            break;

        case DMA_CAN2:
            cc14 = READ_REG32(CHIP_CTRL14_BASE);
            cc14 &= ~CTRL14_CAN2_DMA_MAP_MASK;
            cc14 |= channel << CTRL14_CAN2_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL14_BASE, cc14);
            break;

        case DMA_CAN3:
            cc14 = READ_REG32(CHIP_CTRL14_BASE);
            cc14 &= ~CTRL14_CAN3_DMA_MAP_MASK;
            cc14 |= channel << CTRL14_CAN3_DMA_MAP_SHIFT;
            WRITE_REG32(CHIP_CTRL14_BASE, cc14);
            
            cc15 = READ_REG32(CHIP_CTRL15_BASE);
            cc15 &= ~CTRL15_CAN3_DMA_MAP_HI;
            cc15 |= channel >> 2;                   /* 通道号的高1位 */
            WRITE_REG32(CHIP_CTRL15_BASE, cc15);
            break;

        /*
         * 固定通道 TODO something?
         */
        case DMA_ATIM:
        case DMA_GTIM:
            break;
    
        default:
            return -1;
    }

    return 0;
}
 
static int ls2k_dma_get_idle_channel_number(int devNum, int *rx_chnl, int *tx_chnl)
{
    int i;

    if (rx_chnl) *rx_chnl = -1;
    if (tx_chnl) *tx_chnl = -1;
    
    switch (devNum)
    {
        case DMA_UART0:         // RX & TX
        case DMA_UART1:
        case DMA_UART2:
        case DMA_UART3:
        case DMA_UART4:
        case DMA_UART5:
        case DMA_UART6:
        case DMA_UART7:
        case DMA_UART8:
        case DMA_UART9:
        case DMA_I2C0:
        case DMA_I2C1:
        case DMA_I2C2:
        case DMA_I2C3:
        case DMA_SPI2:
        case DMA_SPI3:
        case DMA_I2S:
            for (i=0; i<CHNL_COUNT/2; i++)
            {
    #if DMA_STATEMACHINE
                if ((dma_channels[i*2  ].state == DMA_STATE_IDLE) &&
                    (dma_channels[i*2+1].state == DMA_STATE_IDLE))
    #else
                if (dma_channels[i*2].idle && dma_channels[i*2+1].idle)
    #endif
                {
                    if (rx_chnl) *rx_chnl = i*2;
                    if (tx_chnl) *tx_chnl = i*2 + 1;
                    return 0;
                }
            }
            break;
            
        case DMA_ADC:           // RX
        case DMA_CAN0:
        case DMA_CAN1:
        case DMA_CAN2:
        case DMA_CAN3:
        case DMA_MEM:           // mem2mem
            for (i=0; i<CHNL_COUNT; i++)
            {
    #if DMA_STATEMACHINE
                if (dma_channels[i].state == DMA_STATE_IDLE)
    #else
                if (dma_channels[i].idle)
    #endif
                {
                    if (rx_chnl) *rx_chnl = i;
                    return 0;
                }
            }
            break;

        case DMA_ATIM:          // fixed: CH1 CH2 CH3 CH4 COM UP TRG
        case DMA_GTIM:          // fixed: CH1 CH2 CH3 CH4  -  UP TRG
    #if DMA_STATEMACHINE
            if ((dma_channels[0].state == DMA_STATE_IDLE) &&
                (dma_channels[1].state == DMA_STATE_IDLE) &&
                (dma_channels[2].state == DMA_STATE_IDLE) &&
                (dma_channels[3].state == DMA_STATE_IDLE))
    #else
            if (dma_channels[0].idle && dma_channels[1].idle &&
                dma_channels[2].idle && dma_channels[3].idle)
    #endif
            {
                return 0;
            }
            break;
    }

    return -1;
}
 
/**
 * DMA 通道启动
 */
static int ls2k_dma_channel_start(int channel, int priority)
{
    if ((channel >= 0) && (channel < CHNL_COUNT))
    {
        switch (priority)
        {
            case DMA_PRIORITY_LOW:
                hwDMA->Channels[channel].ccr &= ~DMA_CCR_PL_MASK;
                break;
            case DMA_PRIORITY_MID:
                hwDMA->Channels[channel].ccr &= ~DMA_CCR_PL_MASK;
                hwDMA->Channels[channel].ccr |= 1 << DMA_CCR_PL_SHIFT;
                break;
            case DMA_PRIORITY_HIGH:
                hwDMA->Channels[channel].ccr &= ~DMA_CCR_PL_MASK;
                hwDMA->Channels[channel].ccr |= 2 << DMA_CCR_PL_SHIFT;
                break;
            case DMA_PRIORITY_HIGHEST:
                hwDMA->Channels[channel].ccr &= ~DMA_CCR_PL_MASK;
                hwDMA->Channels[channel].ccr |= 3 << DMA_CCR_PL_SHIFT;
                break;
        }

        /*
         * 开中断
         */
        ls2k_dma_channel_interrupt_enable( &dma_channels[channel] );

        hwDMA->Channels[channel].ccr |= DMA_CCR_EN;
        
    #if 0 // DMA_STATEMACHINE
        dma_channels[channel].state = dma_channels[channel].cfg.ccr.dir ?
                                      DMA_STATE_TXING : DMA_STATE_RXING;
    #endif

        return 0;
    }
    
    return -1;
}

/**
 * DMA 通道关闭
 */
static int ls2k_dma_channel_stop(int channel)
{
    if ((channel >= 0) && (channel < CHNL_COUNT))
    {
        /*
         * 关中断
         */
        ls2k_dma_channel_interrupt_disable( &dma_channels[channel] );
        
        hwDMA->Channels[channel].ccr &= ~DMA_CCR_EN;

#if DMA_STATEMACHINE
        dma_channels[channel].state = DMA_STATE_IDLE;
#endif
        return 0;
    }

    return -1;
}

static int ls2k_dma_channel_get_sr(int channel)
{
    if ((channel >= 0) && (channel < CHNL_COUNT))
    {
        unsigned int sr = hwDMA->isr;
        sr >>= (channel * 4);

        return (int) sr;
    }

    return -1;
}

static int ls2k_dma_wait_transfer_done(int channel, int timeout)
{
    int tmo, ret = -1;

    if (timeout < 0) timeout = 0;
    tmo = timeout;
    
    if ((channel >= 0) && (channel < CHNL_COUNT))
    {
        unsigned int sr = 0;

        while (1)
        {
            sr = hwDMA->isr;
            sr >>= channel * 4;
            sr &= 0xF;
            
            if (sr)
                break;
                
            if ((timeout) && (tmo-- < 0))
                break;
        }

        hwDMA->iclr |= 0xF << (channel * 4);   /* clear isr */
        
        if (tmo < 0)
            ret = -ETIMEDOUT;

        else if (sr & DMA_ISR_TE)
            ret = -EIO;

        else if (sr & DMA_ISR_TC)
            ret = 0;
    }

    return ret;
}

//-----------------------------------------------------------------------------
// DMA 中断
//-----------------------------------------------------------------------------

static void ls2k_dma_channel_interrupt_enable(DMA_CHNL_t *chnl)
{
    if (chnl->cfg.ccr.tcie || chnl->cfg.ccr.htie || chnl->cfg.ccr.teie)
    {
        ls2k_interrupt_enable(chnl->irqVector);
    }
}

static void ls2k_dma_channel_interrupt_disable(DMA_CHNL_t *chnl)
{
    ls2k_interrupt_disable(chnl->irqVector);
}

static void ls2k_dma_channel_interrupt_handler(int vector, void *arg)
{
    DMA_CHNL_t *chnl = (DMA_CHNL_t *)arg;
    unsigned int sr;

    if (!chnl)
    {
        hwDMA->iclr |= 0xFFFFFFFF;
        return;
    }

#if DMA_STATEMACHINE
    chnl->state = DMA_STATE_READY;
#endif

    sr = hwDMA->isr;                                /* get int status */

    hwDMA->iclr |= 0xF << (chnl->cfg.chNum * 4);    /* clear isr */

    if (chnl && chnl->cfg.cb)
    {
        int thisbytes = chnl->cfg.transbytes;

        sr >>= chnl->cfg.chNum * 4;
        chnl->cfg.cb(&chnl->cfg, thisbytes, sr);
    }

}

//-----------------------------------------------------------------------------
// DMA 驱动
//-----------------------------------------------------------------------------

/**
 * 初始化, 安装中断
 */
STATIC_DRV int DMA_initialize(const void *dma, void *arg)
{
    int i;

    if (m_dma_initialized)
        return 0;

    hwDMA->iclr = 0xFFFFFFFF;

    for (i=0; i<CHNL_COUNT; i++)
    {
        DMA_CHNL_t *dmachnl = &dma_channels[i];
        
        snprintf(dmachnl->dev_name, 15, "dma-ch%i", i);
    #if DMA_STATEMACHINE
        dmachnl->state = DMA_STATE_IDLE;
    #else
        dmachnl->idle = 1;
    #endif

        switch (i)
        {
    #if (!USE_EXTINT)
            case 0: dmachnl->irqVector = INTC0_DMA0_IRQ; break;
            case 1: dmachnl->irqVector = INTC0_DMA1_IRQ; break;
            case 2: dmachnl->irqVector = INTC0_DMA2_IRQ; break;
            case 3: dmachnl->irqVector = INTC0_DMA3_IRQ; break;
            case 4: dmachnl->irqVector = INTC0_DMA4_IRQ; break;
            case 5: dmachnl->irqVector = INTC0_DMA5_IRQ; break;
            case 6: dmachnl->irqVector = INTC0_DMA6_IRQ; break;
            case 7: dmachnl->irqVector = INTC0_DMA7_IRQ; break;
    #else
            case 0: dmachnl->irqVector = EXTI1_DMA0_IRQ; break;
            case 1: dmachnl->irqVector = EXTI1_DMA1_IRQ; break;
            case 2: dmachnl->irqVector = EXTI1_DMA2_IRQ; break;
            case 3: dmachnl->irqVector = EXTI1_DMA3_IRQ; break;
            case 4: dmachnl->irqVector = EXTI1_DMA4_IRQ; break;
            case 5: dmachnl->irqVector = EXTI1_DMA5_IRQ; break;
            case 6: dmachnl->irqVector = EXTI1_DMA6_IRQ; break;
            case 7: dmachnl->irqVector = EXTI1_DMA7_IRQ; break;
    #endif
        }

        /**
         * 安装 DMA 中断
         */
        ls2k_install_irq_handler(dmachnl->irqVector,
                                 ls2k_dma_channel_interrupt_handler,
                                 dmachnl);

    #if (!USE_EXTINT)
        /**
         * 配置 Route
         */
        ls2k_set_irq_routeip(dmachnl->irqVector, INT_ROUTE_IP3);

    #endif

    }

    m_dma_initialized = 1;
    return 0;
}

/*
 * 传输数量, 应该根据源的大小来处理
 */
static void ls2k_dma_set_cndtr_register(struct dma_chnl_cfg *p_cfg)
{
    int bytesperxfer = 1;
    int channel = p_cfg->chNum;
    
    /*
     * 1: 从存储器读
     */
    if (p_cfg->ccr.dir)
    {
        switch (p_cfg->ccr.msize)
        {
            case DMA_CCR_MSIZE_32b: bytesperxfer = 4; break;
            case DMA_CCR_MSIZE_16b: bytesperxfer = 2; break;
            case DMA_CCR_MSIZE_8b:
            default:                bytesperxfer = 1; break;
        }
    }
    
    /*
     * 0: 从外设读
     */
    else
    {
        switch (p_cfg->ccr.psize)
        {
            case DMA_CCR_PSIZE_32b: bytesperxfer = 4; break;
            case DMA_CCR_PSIZE_16b: bytesperxfer = 2; break;
            case DMA_CCR_PSIZE_8b:
            default:                bytesperxfer = 1; break;
        }
    }

    hwDMA->Channels[channel].cndtr = p_cfg->transbytes / bytesperxfer;
}

/**
 * 打开时执行DMA通道配置
 */
STATIC_DRV int DMA_open(const void *dma, void *arg)
{
    struct dma_chnl_cfg *p_cfg = (struct dma_chnl_cfg *)arg;
    int channel;

    if (!m_dma_initialized)
        return -1;

    if (!p_cfg || !p_cfg->devNum || !p_cfg->memAddr)
    {
        return -1;
    }

    channel = p_cfg->chNum;
    if ((channel < 0) || (channel >= CHNL_COUNT))
    {
        return -1;
    }

    /******************************************************
     * 配置 DMA 设备
     */

    hwDMA->Channels[channel].cmar = p_cfg->memAddr;

    /*
     * 内存到内存数据传输
     */
    if (p_cfg->ccr.mem2mem)
    {
        hwDMA->Channels[channel].cpar = p_cfg->devNum;  /* mem2mem=1: 是内存地址 */
        
        /*
         * 实测: 1. 内存传输宽度至少是 16bits
         *
         *       2. cfg->ccr.dir == 0 时, 传输方向 cmar->cpar
         *          cfg->ccr.dir == 1 时, 传输方向 cpar->cmar
         *
         */
        if (p_cfg->ccr.psize <= DMA_CCR_PSIZE_8b)
            p_cfg->ccr.psize = DMA_CCR_PSIZE_16b;
        else if (p_cfg->ccr.psize > DMA_CCR_PSIZE_32b)
            p_cfg->ccr.psize = DMA_CCR_PSIZE_32b;

        if (p_cfg->ccr.msize <= DMA_CCR_MSIZE_8b)
            p_cfg->ccr.msize = DMA_CCR_MSIZE_16b;
        else if (p_cfg->ccr.msize > DMA_CCR_MSIZE_32b)
            p_cfg->ccr.msize = DMA_CCR_MSIZE_32b;
    }
    
    /*
     * 设备和内存数据传输
     */
    else                        
    {
        unsigned int devAddr = peripheral_number_to_address(p_cfg);
        
        if ((devAddr & 0xFFF00000) != 0x16100000)
        {
            return -1;
        }

        /**
         * chip control 寄存器
         */
        ls2k_dma_channel_config(channel, p_cfg->devNum);

        hwDMA->Channels[channel].cpar = devAddr;

        if (p_cfg->ccr.psize > DMA_CCR_PSIZE_32b)
            p_cfg->ccr.psize = DMA_CCR_PSIZE_32b;

        if (p_cfg->ccr.msize > DMA_CCR_MSIZE_32b)
            p_cfg->ccr.msize = DMA_CCR_MSIZE_32b;
    }

    /*
     * 设置传输数量
     */
    ls2k_dma_set_cndtr_register(p_cfg);

    p_cfg->ccr.en = 0;                      /* HW not set Enable */
    hwDMA->Channels[channel].ccr = p_cfg->ccr32;

    p_cfg->ccr.en = 1;                      /* XXX 用作 started 标志 */

#if DMA_STATEMACHINE
    dma_channels[channel].state = DMA_STATE_READY;
#else
    dma_channels[channel].idle = 0;
#endif

    dma_channels[channel].cfg = *p_cfg;     /* 保存配置 */

    return 0;
}

/**
 * 关闭DMA通道
 */
STATIC_DRV int DMA_close(const void *dma, void *arg)
{
    int channel = (long)arg;
    
    if ((channel >= 0) && (channel < CHNL_COUNT) &&
#if DMA_STATEMACHINE
        (dma_channels[channel].state == DMA_STATE_READY ||
         dma_channels[channel].state == DMA_STATE_PAUSE))
#else
        (dma_channels[channel].idle == 0))
#endif
    {
        ls2k_dma_channel_stop(channel);

        hwDMA->Channels[channel].ccr = 0;

#if DMA_STATEMACHINE
        dma_channels[channel].state = DMA_STATE_IDLE;
#else
        dma_channels[channel].idle = 1;
#endif
    }

    return 0;
}

//-----------------------------------------------------------------------------
// DMA drivers
//-----------------------------------------------------------------------------

#if (PACK_DRV_OPS)
/******************************************************************************
 * DMA driver operators
 */
static const driver_ops_t ls2k_dma_drv_ops =
{
    .init_entry  = DMA_initialize,
    .open_entry  = DMA_open,
    .close_entry = DMA_close,
    .read_entry  = NULL,
    .write_entry = NULL,
    .ioctl_entry = NULL,
};

const driver_ops_t *dma_drv_ops = &ls2k_dma_drv_ops;
#endif

//-----------------------------------------------------------------------------
// user api
//-----------------------------------------------------------------------------

/*
 * 获取空闲的DMA通道
 */
int dma_get_idle_channel(int devNum, int *rx_chnl, int *tx_chnl)
{
    if (!m_dma_initialized)
    {
        DMA_initialize(NULL, NULL);
    }
    
    return ls2k_dma_get_idle_channel_number(devNum, rx_chnl, tx_chnl);
}

/*
 * 通道 channel 是否空闲
 */
int dma_channel_is_idle(int channel)
{
    if ((channel >= 0) && (channel < CHNL_COUNT))
    {
#if DMA_STATEMACHINE
        return (dma_channels[channel].state == DMA_STATE_IDLE) ? 1 : 0;
#else
        return dma_channels[channel].idle;
#endif
    }

    return 0;
}

/*
 * 通道 channel 是否就绪
 */
int dma_channel_is_ready(int channel)
{
    if ((channel >= 0) && (channel < CHNL_COUNT))
    {
#if DMA_STATEMACHINE
        if ((dma_channels[channel].state == DMA_STATE_READY) ||
            (dma_channels[channel].state == DMA_STATE_PAUSE))
            return 1;
#else
        return dma_channels[channel].idle ? 0 : 1;
#endif
    }
    
    return 0;
}

#if 0
int dma_channel_is_tx_ready(int channel)
{
    if ((channel >= 0) && (channel < CHNL_COUNT))
    {
#if DMA_STATEMACHINE
        if ((dma_channels[channel].state == DMA_STATE_READY) ||
            (dma_channels[channel].state == DMA_STATE_PAUSE) ||
            (dma_channels[channel].state == DMA_STATE_TXING))
            return 1;
#else
        return dma_channels[channel].idle ? 0 : 1;
#endif
    }

    return 0;
}

int dma_channel_is_rx_ready(int channel)
{
    if ((channel >= 0) && (channel < CHNL_COUNT))
    {
#if DMA_STATEMACHINE
        if ((dma_channels[channel].state == DMA_STATE_READY) ||
            (dma_channels[channel].state == DMA_STATE_PAUSE) ||
            (dma_channels[channel].state == DMA_STATE_RXING))
            return 1;
#else
        return dma_channels[channel].idle ? 0 : 1;
#endif
    }

    return 0;
}
#endif

/*
 * 启动DMA传输
 */
int dma_start(struct dma_chnl_cfg *cfg, int priority)
{
    char *s;
    
    /*
     * parameter is not correct
     */
    if (NULL == cfg)
    {
        DEBUG("dma start parameter is NULL.\r\n");
        return -1;
    }

    s = peripheral_device_name(cfg);

    if (NULL == cfg->device)
    {
        DEBUG("dma start %s cfg.device is not set.\r\n", s);
        return -1;
    }

    if (0 == cfg->memAddr)
    {
        DEBUG("dma start %s cfg.memAddr is not set.\r\n", s);
        return -1;
    }
    
    /*
     * dma channel is started
     */
    if (cfg->ccr.en)
    {
        DEBUG("%s has started dma on channel %i.\r\n", s, cfg->chNum);
        return 0;
    }

    /*
     * 1st 初始化
     */
    if (!m_dma_initialized)
    {
        DMA_initialize(NULL, NULL);
    }

    /*
     * 2nd 确认通道是否可用. 
     */
    if (!dma_channel_is_idle(cfg->chNum))
    {
        int rx_channel, tx_channel;

        /*
         * TODO 这里有 BUG, 双通道时会匹配错误
         */
        if (dma_get_idle_channel(cfg->devNum, &rx_channel, &tx_channel) < 0)
            return -1;

        if (cfg->ccr.dir)
            cfg->chNum = tx_channel;
        else
            cfg->chNum = rx_channel;
    }

    /*
     * 3rd 启动DMA通道
     */
    if (DMA_open(NULL, cfg) < 0)
    {
        cfg->ccr.en = 0;
        DEBUG("%s start dma on channel %i fail.\r\n", s, cfg->chNum);
        return -1;
    }

    if (ls2k_dma_channel_start(cfg->chNum, priority) < 0)
    {
        cfg->ccr.en = 0;
        DMA_close(NULL, (void *)(long)cfg->chNum);
        DEBUG("%s start dma at channel %i fail.\r\n", s, cfg->chNum);
        return -1;
    }

    DEBUG("%s start dma @channel %i successful.\r\n", s, cfg->chNum);
    (void)s;
    return 0;
}

/**
 * 暂停DMA通道
 */
int dma_pause(int channel)
{
    DMA_CHNL_t *p_chnl;

    if ((channel < 0) || (channel >= CHNL_COUNT))
        return -1;

    p_chnl = &dma_channels[channel];

#if DMA_STATEMACHINE
    if ((p_chnl->state == DMA_STATE_READY) && (p_chnl->cfg.chNum == channel))
#else
    if (!p_chnl->idle && (p_chnl->cfg.chNum == channel))
#endif
    {
        hwDMA->Channels[channel].ccr &= ~DMA_CCR_EN;

#if DMA_STATEMACHINE
        p_chnl->state = DMA_STATE_PAUSE;
#endif
    }

    return 0;
}

/*
 * 重新启动已经配置好的DMA通道
 */
int dma_restart(int channel, char *buf, int size, int xferbits)
{
    DMA_CHNL_t *p_chnl;

    if ((channel < 0) || (channel >= CHNL_COUNT))
        return -1;

    p_chnl = &dma_channels[channel];
    
#if DMA_STATEMACHINE
    if ((p_chnl->state == DMA_STATE_READY || p_chnl->state == DMA_STATE_PAUSE) &&
        (p_chnl->cfg.chNum == channel))
#else
    if (!p_chnl->idle && (p_chnl->cfg.chNum == channel))
#endif
    {
        unsigned int ccr;

        /*
         * Need disable before write?
         */
        hwDMA->Channels[channel].ccr &= ~DMA_CCR_EN;

        /*
         * Set memory size and transfer counter
         */
        p_chnl->cfg.transbytes = size;
        p_chnl->cfg.memAddr = (uintptr_t)buf;
        
        switch (xferbits)
        {
            case DMA_XFER_8b:
                p_chnl->cfg.ccr.msize = DMA_CCR_MSIZE_8b;
                p_chnl->cfg.ccr.psize = DMA_CCR_MSIZE_8b;
                ccr = hwDMA->Channels[channel].ccr;
                ccr &= ~(DMA_CCR_MSIZE_MASK | DMA_CCR_PSIZE_MASK);
                ccr |= DMA_CCR_MSIZE_8b << DMA_CCR_MSIZE_SHIFT;
                ccr |= DMA_CCR_PSIZE_8b << DMA_CCR_PSIZE_SHIFT;
                hwDMA->Channels[channel].ccr = ccr;
                break;

            case DMA_XFER_16b:
                p_chnl->cfg.ccr.msize = DMA_CCR_MSIZE_16b;
                p_chnl->cfg.ccr.psize = DMA_CCR_MSIZE_16b;
                ccr = hwDMA->Channels[channel].ccr;
                ccr &= ~(DMA_CCR_MSIZE_MASK | DMA_CCR_PSIZE_MASK);
                ccr |= DMA_CCR_MSIZE_16b << DMA_CCR_MSIZE_SHIFT;
                ccr |= DMA_CCR_PSIZE_16b << DMA_CCR_PSIZE_SHIFT;
                hwDMA->Channels[channel].ccr = ccr;
                break;

            case DMA_XFER_32b:
                p_chnl->cfg.ccr.msize = DMA_CCR_MSIZE_32b;
                p_chnl->cfg.ccr.psize = DMA_CCR_MSIZE_32b;
                ccr = hwDMA->Channels[channel].ccr;
                ccr &= ~(DMA_CCR_MSIZE_MASK | DMA_CCR_PSIZE_MASK);
                ccr |= DMA_CCR_MSIZE_32b << DMA_CCR_MSIZE_SHIFT;
                ccr |= DMA_CCR_PSIZE_32b << DMA_CCR_PSIZE_SHIFT;
                hwDMA->Channels[channel].ccr = ccr;
                break;
        }

        ls2k_dma_set_cndtr_register(&p_chnl->cfg);

        hwDMA->Channels[channel].cmar = p_chnl->cfg.memAddr;

        /*
         * Restart the channel
         */
        hwDMA->Channels[channel].ccr |= DMA_CCR_EN;

    #if DMA_STATEMACHINE
        dma_channels[channel].state = dma_channels[channel].cfg.ccr.dir ?
                                      DMA_STATE_TXING : DMA_STATE_RXING;
    #endif

        return 0;
    }

    return -1;
}

/**
 * 停止DMA通道
 */
void dma_stop(int channel)
{
    DMA_close(NULL, (void *)(long)channel);
}

/**
 * 获取DMA通道状态寄存器
 */
int dma_get_status(int channel)
{
    return ls2k_dma_channel_get_sr(channel);
}

/**
 * 获取DMA通道状态寄存器
 */
int dma_wait_done(int channel, int timeout)
{
    return ls2k_dma_wait_transfer_done(channel, timeout);
}

//-----------------------------------------------------------------------------

/*
 * @@ END
 */

