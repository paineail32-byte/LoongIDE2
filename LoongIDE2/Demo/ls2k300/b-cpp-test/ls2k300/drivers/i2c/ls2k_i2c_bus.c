/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_i2c_bus.c
 *
 * created: 2024-07-4
 *  author: Bian
 */

/*
 * 注: 本驱动程序仅测试了 master
 *
 *     slave 的实现 TODO
 */

#include "bsp.h"

#if (BSP_USE_I2C)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include "ls2k300.h"
#include "ls2k300_irq.h"

#include "osal.h"

#include "ls2k_drv_io.h"
#include "drv_os_priority.h"

#include "ls2k_i2c_bus_hw.h"
#include "ls2k_i2c_bus.h"

//-----------------------------------------------------------------------------

/*
 * 使用中断
 */
#define I2C_USE_INT     1

/*
 * 使用 DMA
 */
#define I2C_USE_DMA     0

#if I2C_USE_DMA
#include "ls2k_dma.h"
#endif

//-----------------------------------------------------------------------------

#if 0
#define DEBUG(fmt, ...)      printk(fmt, ##__VA_ARGS__ )
#else
#define DEBUG(fmt, ...)
#endif

//-----------------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------------

#define LOCK(p)     osal_mutex_obtain(p->p_mutex, OSAL_WAIT_FOREVER)
#define UNLOCK(p)   osal_mutex_release(p->p_mutex)

//-----------------------------------------------------------------------------
// 中断处理中发出事件, 有 wait_for_event 来响应.
//-----------------------------------------------------------------------------

#if I2C_USE_INT

#define I2C_USE_EVENT           0

#define IRESULT_SUCCESS         0x0100
#define IRESULT_ERROR           0x0200
#define IRESULT_MASK            0x0300      /* IRESULT_SUCCESS | IRESULT_ERROR */

/*
 * 数据
 */
typedef struct
{
    unsigned char *buf;         /* 数据缓冲区 */
    int count;                  /* 数据字节数 */
    int remain;                 /* 剩余字节数 */
    int flag;                   // 备用
} I2C_data_t;

#endif // #if I2C_USE_INT

//-----------------------------------------------------------------------------
// device soft control
//-----------------------------------------------------------------------------

typedef struct
{
    HW_I2C_t *hwI2C;                    /* pointer to HW registers */
    unsigned int irqNum;                /* interrupt num */
    unsigned int speed;                 /* transfer speed */
    unsigned int WorkMode;              /* I2C_RX_DMA | I2C_TX_POLL etc. */

    osal_mutex_t p_mutex;               /* Mutex */

    volatile unsigned char address;     /* 8bits address, bit[0] means read or write */
    volatile bool is_started;           /* has run START command */

#if I2C_USE_INT
    volatile I2C_data_t *data;
  #if I2C_USE_EVENT
    osal_event_t p_event;               /* Event */
  #else
    volatile int xfer_result;           /* 传输结果 */
  #endif
#endif

#if I2C_USE_DMA

#endif

    unsigned char fast_duty;            /* 400KHZ duty */

    int  initialized;
    char dev_name[16];

} I2C_bus_t;

//-----------------------------------------------------------------------------

static int ls2k_i2c_handle_error(I2C_bus_t *pIIC, unsigned int sr1);

STATIC_DRV int I2C_initialize(const void *bus);
STATIC_DRV int I2C_send_stop(const void *bus, unsigned int Addr);

/******************************************************************************
 * I2C hardware
 ******************************************************************************/

/*
 * 等待 SR1 寄存器的状态. 请求的可能是几个 "|" 的状态组
 *
 * 如果 tmo<=0, 无限等待
 */
static int ls2k_i2c_wait_status1(I2C_bus_t *pIIC, unsigned int *wait_sr, int wait_us)
{
    unsigned int sr1, mask = *wait_sr;

    sr1 = pIIC->hwI2C->sr1;
    while ((sr1 & mask) == 0)
    {
        if (sr1 & I2C_SR1_ITERREN_MASK)
        {
            ls2k_i2c_handle_error(pIIC, sr1);
            return -EIO;
        }

        if (wait_us-- <= 0)
        {
			printk("timeout when wait %s sr1 0x%08x done!\r\n", pIIC->dev_name, (int)mask);
			return -ETIMEDOUT;
        }

        delay_us(1);

        sr1 = pIIC->hwI2C->sr1;
    }

    *wait_sr = sr1;

    return 0;
}

/**
 * 等待 STOP == 0
 */
static int ls2k_i2c_wait_stop_done(I2C_bus_t *pIIC)
{
    int wait_us = 1000;     /* 实测使用 > 350us */

    while (pIIC->hwI2C->cr1 & I2C_CR1_STOP)
    {
        if (wait_us-- <= 0)
        {
            I2C_initialize((void *)pIIC);
            printk("timeout when wait %s cr1 STOP done\r\n", pIIC->dev_name);
            return -ETIMEDOUT;
        }

        delay_us(1);
    }

    return 0;
}

/*
 * 复位 I2C 控制器
 */
static int ls2k_i2c_do_reset(I2C_bus_t *pIIC)
{
    /*
     * set control register 1 as 0, renable controller
     */
    pIIC->hwI2C->cr1 = 0;
    delay_us(10);
    pIIC->hwI2C->cr1 = I2C_CR1_PE;

    /*
     * if in master mode then send stop single
     * else reset the bus
     */
    if (pIIC->hwI2C->sr2 & I2C_SR2_MSL)
        pIIC->hwI2C->cr1 |= I2C_CR1_STOP;       // master mode
    else
        pIIC->hwI2C->cr1 |= I2C_CR1_RECOVER;    // slave mode

    delay_us(10);

    return 0;
}

/*
 * 设置 I2C 通信速度
 */
static int ls2k_i2c_set_speed(I2C_bus_t *pIIC, unsigned int speed)
{
    unsigned int ccr;

    if (pIIC->initialized && (pIIC->speed == speed))
    {
        return 0;
    }

    /* real speed: 100K or 400K */
    speed = (speed >= 400000) ? 400000 : 100000;
    pIIC->speed = speed;

    ccr = apb_frequency / speed;

    if (speed == 400000)                    /* fast mode 400KHZ */
    {
        if (pIIC->fast_duty)                /* 占空比: 1==9:16 */
        {
            ccr /= 9 + 16;
            ccr &= I2C_CCR_MASK;
            ccr |= I2C_CCR_DUTY;
        }
        else                                /* 占空比: 0==1:2 */
        {
            ccr /= 1 + 2;
            ccr &= I2C_CCR_MASK;
        }

        ccr |= I2C_CCR_F_S;                 // 1==快速模式
    }
    else                                    /* stardand mode 100KHZ */
    {
        ccr /= 1 + 1;
        ccr &= I2C_CCR_MASK;
    }

    /* set ccr register */
    pIIC->hwI2C->ccr = ccr;

	return 0;
}

#if 0
/**
 * 设置 7 位 slave address
 */
static int ls2k_i2c_set_slave_address(I2C_bus_t *pIIC, unsigned char saddr_7bits)
{
    pIIC->saddr_7bits = saddr_7bits;

    /* set oar register */
    if (saddr_7bits)
        pIIC->hwI2C->oar = (unsigned int)(saddr_7bits << 1);
    else
        pIIC->hwI2C->oar = 0;

    return 0;
}
#endif

//-----------------------------------------------------------------------------
// 数据传输
//-----------------------------------------------------------------------------

/*
 * process before read after I2C_SR1_ADDR
 */
static void ls2k_i2c_set_ack_pos(I2C_bus_t *pIIC, int count)
{
    if (count <= 1)
    {
        pIIC->hwI2C->cr1 &= ~(I2C_CR1_ACK | I2C_CR1_POS);
    }
    else if (count == 2)
    {
        pIIC->hwI2C->cr1 &= ~I2C_CR1_ACK;
        pIIC->hwI2C->cr1 |= I2C_CR1_POS;
    }
    else // >= 3
    {
        pIIC->hwI2C->cr1 |= I2C_CR1_ACK;
        pIIC->hwI2C->cr1 &= ~I2C_CR1_POS;
    }
}

static int ls2k_i2c_handle_error(I2C_bus_t *pIIC, unsigned int sr1)
{
    /* Arbitration lost */
    if (sr1 & I2C_SR1_ARLO)
    {
        pIIC->hwI2C->sr1 &= ~I2C_SR1_ARLO;
    }

    /*
     * Acknowledge failure:
     * In master transmitter mode a Stop must be generated by software
     */
    if (sr1 & I2C_SR1_AF)
    {
        pIIC->hwI2C->cr1 |= I2C_CR1_STOP;
        pIIC->hwI2C->sr1 &= ~I2C_SR1_AF;
    }

    /* Bus error */
    if (sr1 & I2C_SR1_BERR)
    {
        pIIC->hwI2C->sr1 &= ~I2C_SR1_BERR;
    }

    // ls2k_interrupt_disable(pIIC->irqNum);
    return 0;
}

//-----------------------------------------------------------------------------
// 中断方式
//-----------------------------------------------------------------------------

#if I2C_USE_INT

static int ls2k_i2c_int_set_using(I2C_bus_t *pIIC, int mode_int)
{
    if (pIIC->WorkMode != mode_int)
    {
        pIIC->WorkMode = mode_int;
    }
        
    return 0;
}

/*
 * 中断处理
 */
static void ls2k_i2c_irq_process(I2C_bus_t *pIIC)
{
    unsigned int sr1;
    I2C_data_t *data;

    sr1 = pIIC->hwI2C->sr1;
    if (sr1 == 0)
    {
        DEBUG("%s isr but no event?\r\n", pIIC->dev_name);
        return;
    }

    /*
     * 发生错误中断. I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR
     */
    if (sr1 & I2C_SR1_ITERREN_MASK)
    {
        DEBUG("%s error occured!\r\n", pIIC->dev_name);

        pIIC->hwI2C->sr1 &= ~I2C_SR1_ITERREN_MASK;

        if (sr1 & I2C_SR1_AF)   // 应答失败
        {
            pIIC->hwI2C->cr1 |= I2C_CR1_STOP;
        }

#if I2C_USE_EVENT
        osal_event_send(pIIC->p_event, IRESULT_ERROR);
#else
        pIIC->xfer_result = IRESULT_ERROR;
#endif
        return;
    }

    /*
     * 发生启动中断.
     */
    if (sr1 & I2C_SR1_SB)
    {
        DEBUG(":SB");
        pIIC->is_started = true;                /* this is a flag */
        pIIC->hwI2C->dr = pIIC->address;        /* write clear I2C_SR1_SB */
        return;
    }

    /*
     * 发生地址中断.
     */
    if (sr1 & I2C_SR1_ADDR)
    {
        DEBUG(":ADDR");

        (void)READ_REG32(&pIIC->hwI2C->sr2);    /* read clear I2C_SR1_ADDR */

        /*
         * 开启缓冲类中断
         */
        pIIC->hwI2C->cr2 |= I2C_IEN_ITBUF;
    }

    //-----------------------------------------------------
    // validate parameters
    //-----------------------------------------------------

    data = (I2C_data_t *)pIIC->data;

    if (!data || !data->buf)
    {
        /*
         * 如果出现这个情况, 如何处理 ?
         */
    //  pIIC->hwI2C->cr2 &= ~I2C_IEN_ITBUFEN;
    //  pIIC->hwI2C->sr1 = 0;
        DEBUG("%s isr but no data, sr1=0x%04x.", pIIC->dev_name, sr1);
        return;
    }

    /*
     * 初次赋值 data
     */
    if ((data->count > 0) && (data->count == data->remain))
    {
        if (pIIC->address & 0x1)
        {
            ls2k_i2c_set_ack_pos(pIIC, data->count);
            if (!(sr1 & I2C_SR1_RXNE))
            {
                return;
            }
        }
        else
        {
            pIIC->hwI2C->cr1 &= ~I2C_CR1_ACK;
        }
    }

    //-------------------------------------------
    // 有需要收发的数据
    //-------------------------------------------

    if (data->remain > 0)
    {
        unsigned char *pch, ch;

        pch = data->buf + (data->count - data->remain);

        /*
         * 接收中断, 读数据
         */
        if (pIIC->address & 0x1)
        {
            int last_remain = data->remain;

            /* read 1st byte: I2C_SR1_RXNE
             */
            if (sr1 & I2C_SR1_RXNE)
            {
                ch = (unsigned char)pIIC->hwI2C->dr;
                *pch++ = ch;
                data->remain--;
            }

            if (sr1 & I2C_SR1_BTF) // ((sr1 & 0x44) == 0x44) //
            {
                ch = (unsigned char)pIIC->hwI2C->dr;
                if (data->remain > 0)
                {
                    *pch++ = ch;
                    data->remain--;
                }
                else
                {
                    DEBUG("%s rx buf is full\r\n", pIIC->dev_name);
                }
            }

            /* set ACK pos while remain bytes equals 1 or 2
             */
            if ((last_remain > 2) && ((data->remain > 0) || (data->remain < 3)))
            {
                ls2k_i2c_set_ack_pos(pIIC, data->remain);
            }

            if (0 == data->remain)
            {
                pIIC->hwI2C->cr1 &= ~(I2C_CR1_POS | I2C_CR1_ACK);
            }
        }

        /*
         * 发送中断, 写数据
         */
        else
        {
            /* write 1st byte
             */
            if (sr1 & I2C_SR1_TXE)
            {
                ch = *pch++;
                pIIC->hwI2C->dr = (unsigned int)ch;
                data->remain--;
            }

            /* write 2nd byte: I2C_SR1_BTF
             */
            if ((data->remain > 0) && (sr1 & I2C_SR1_BTF)) // ((sr1 & 0x84) == 0x84)) //
            {
                ch = *pch++;
                pIIC->hwI2C->dr = (unsigned int)ch;
                data->remain--;
            }

            return;     // return for wait next TXE
        }
    }

    //---------------------------------
    // 传输完成
    //---------------------------------

    if ((0 == data->remain) && (0 == data->flag))
    {
        data->flag = 1;

        pIIC->hwI2C->cr2 &= ~I2C_IEN_ITBUF;
        pIIC->hwI2C->sr1 = 0;

#if I2C_USE_EVENT
        osal_event_send(pIIC->p_event, IRESULT_SUCCESS);
#else
        pIIC->xfer_result = IRESULT_SUCCESS;
#endif
    }
    
    return;
}

/**
 * I2C 中断处理
 */
static void ls2k_i2c_interrupt_handler(int vector, void *arg)
{
    I2C_bus_t *pIIC;
    
#if USE_EXTINT

    pIIC = (I2C_bus_t *)arg;
    if (NULL != pIIC)
    {
        ls2k_i2c_irq_process(pIIC);
    }

#else

    unsigned int cr1, cr2;

    if (vector == INTC0_I2C_0_1_IRQ)
    {
  #if BSP_USE_I2C0
        pIIC = (I2C_bus_t *)busI2C0;
        cr1 = pIIC->hwI2C->cr1;
        cr2 = pIIC->hwI2C->cr2;
        if ((cr1 & I2C_CR1_PE) && (cr2 & I2C_IEN_IRQ_MASK))
        {
            ls2k_i2c_irq_process(pIIC);
        }
  #endif
  
  #if BSP_USE_I2C1
        pIIC = (I2C_bus_t *)busI2C1;
        cr1 = pIIC->hwI2C->cr1;
        cr2 = pIIC->hwI2C->cr2;
        if ((cr1 & I2C_CR1_PE) && (cr2 & I2C_IEN_IRQ_MASK))
        {
            ls2k_i2c_irq_process(pIIC);
        }
  #endif
    }

    else if (vector == INTC0_I2C_2_3_IRQ)
    {
  #if BSP_USE_I2C2
        pIIC = (I2C_bus_t *)busI2C2;
        cr1 = pIIC->hwI2C->cr1;
        cr2 = pIIC->hwI2C->cr2;
        if ((cr1 & I2C_CR1_PE) && (cr2 & I2C_IEN_IRQ_MASK))
        {
            ls2k_i2c_irq_process(pIIC);
        }
  #endif
  
  #if BSP_USE_I2C3
        pIIC = (I2C_bus_t *)busI2C3;
        cr1 = pIIC->hwI2C->cr1;
        cr2 = pIIC->hwI2C->cr2;
        if ((cr1 & I2C_CR1_PE) && (cr2 & I2C_IEN_IRQ_MASK))
        {
            ls2k_i2c_irq_process(pIIC);
        }
  #endif
    }

#endif // #if USE_EXTINT

}
#endif // #if I2C_USE_INT

//-----------------------------------------------------------------------------
// DMA Support
//-----------------------------------------------------------------------------

#if I2C_USE_DMA

#endif // #if I2C_USE_DMA

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static int ls2k_i2c_set_workmode(I2C_bus_t *pIIC, int new_mode)
{
    if (pIIC->WorkMode == new_mode)
        return 0;

    /*
     * Stop DMA first
     */
#if I2C_USE_DMA

#endif

    switch (new_mode)
    {
        case I2C_WORK_DMA:
#if I2C_USE_DMA

#endif
            break;

        case I2C_WORK_INT:
#if I2C_USE_INT
            ls2k_i2c_int_set_using(pIIC, new_mode);
#endif
            break;

        case I2C_WORK_POLL:
        default:
            new_mode = I2C_WORK_POLL;
            break;
    }

    pIIC->WorkMode = new_mode;

    return 0;
}

static int ls2k_i2c_get_workmode(I2C_bus_t *pIIC)
{
    return (int)pIIC->WorkMode;
}

static void ls2k_i2c_print_workmode(I2C_bus_t *pIIC)
{
    if (pIIC->initialized)
    {
        switch (pIIC->WorkMode)
        {
            case I2C_WORK_DMA:
                printk("%s is using DMA\r\n", pIIC->dev_name);
                break;

            case I2C_WORK_INT:
                printk("%s is using INT\r\n", pIIC->dev_name);
                break;

            case I2C_WORK_POLL:
            default:
                printk("%s is using Poll\r\n", pIIC->dev_name);
                break;
        }
    }
}

//-----------------------------------------------------------------------------
// INT 读写数据
//-----------------------------------------------------------------------------

#if I2C_USE_INT

static int ls2k_i2c_read_bytes_int(I2C_bus_t *pIIC, unsigned char *buf, int len)
{
    int rt = 0;
	I2C_data_t data;

	data.buf    = buf;
	data.count  = len;
	data.remain = len;
	data.flag   = 0;

	pIIC->data = &data;

	/*
	 * Waiting for Event...
	 */
#if I2C_USE_EVENT
    unsigned int recv_event;
    
    recv_event = osal_event_receive(pIIC->p_event,
					                IRESULT_MASK,
                                    OSAL_EVENT_FLAG_OR | OSAL_EVENT_FLAG_CLEAR,
                                    1000);

	if (recv_event != IRESULT_SUCCESS)
	{
		DEBUG("%s read %i bytes fail\r\n", pIIC->dev_name, len);
		pIIC->data = NULL;
		return 0;
	}

#else

    pIIC->xfer_result = 0;

	//-------------------------------------------
	// 超时等待中断处理完成
	//-------------------------------------------

	int wait_us = 1000000;     // 1s

	while (0 == pIIC->xfer_result)
	{
		if (wait_us-- <= 0)
		{
			// I2C_initialize((void *)pIIC);
			printk("timeout when wait %s read %i bytes\r\n", pIIC->dev_name, len);
			errno = ETIMEDOUT;
			pIIC->data = NULL;
			return 0;
		}

		delay_us(1);
	}
#endif

	rt = len - data.remain;
	pIIC->data = NULL;

    return rt;
}

static int ls2k_i2c_write_bytes_int(I2C_bus_t *pIIC, unsigned char *buf, int len)
{
    int rt = 0;
	I2C_data_t data;

	data.buf    = buf;
	data.count  = len;
	data.remain = len;
	data.flag   = 0;

	pIIC->data = &data;

	/*
	 * Waiting for Event...
	 */
#if I2C_USE_EVENT
    unsigned int recv_event;

    recv_event = osal_event_receive(pIIC->p_event,
					                IRESULT_MASK,
                                    OSAL_EVENT_FLAG_OR | OSAL_EVENT_FLAG_CLEAR,
                                    1000);

	if (recv_event != IRESULT_SUCCESS)
	{
		DEBUG("%s write %i bytes fail\r\n", pIIC->dev_name, len);
		pIIC->data = NULL;
		return 0;
	}

#else

    pIIC->xfer_result = 0;

	//-------------------------------------------
	// 超时等待中断处理完成
	//-------------------------------------------

	int wait_us = 1000000;    // 1s

	while (0 == pIIC->xfer_result)
	{
		if (wait_us-- <= 0)
		{
			// I2C_initialize((void *)pIIC);
			printk("timeout when wait %s write %i bytes\r\n", pIIC->dev_name, len);
			pIIC->data = NULL;
			return 0;
		}

		delay_us(1);
	}

#endif

	rt = len - data.remain;
	pIIC->data = NULL;
	
	return rt;
}

#endif

//-----------------------------------------------------------------------------
// DMA 读写数据
//-----------------------------------------------------------------------------

#if I2C_USE_DMA

#endif

//-----------------------------------------------------------------------------
// POLL 读写数据
//-----------------------------------------------------------------------------

static int ls2k_i2c_read_bytes_poll(I2C_bus_t *pIIC, unsigned char *buf, int len)
{
	unsigned char *pch = buf;
	int rt = 0, remain = len;

	/*
	 * 读之前先设置 ACK
	 */
	ls2k_i2c_set_ack_pos(pIIC, remain);

	//-----------------------------------------
	// 读数据
	//-----------------------------------------

	while (remain > 0)
	{
		unsigned char ch;
		unsigned int wait_sr = I2C_SR1_RXNE | I2C_SR1_BTF;

		rt = ls2k_i2c_wait_status1(pIIC, &wait_sr, 1000);  // 1ms
		if (rt < 0)
		{
			break;
		}

		/* read 1st byte: I2C_SR1_RXNE
		 */
		if (wait_sr & I2C_SR1_RXNE)
		{
			ch = (unsigned char)pIIC->hwI2C->dr;
			*pch++ = ch;
			remain--;
		}

		if (wait_sr & I2C_SR1_BTF)      // ((wait_sr & 0x44) == 0x44)
		{
			ch = (unsigned char)pIIC->hwI2C->dr;
			if (remain > 0)
			{
				*pch++ = ch;
				remain--;
			}
			else
			{
				// printk("%s rx buf is full\r\n", pIIC->dev_name);
			}
		}

		/* set ACK pos while remain bytes equals 1 or 2
		 */
		if ((remain == 1) || (remain == 2))
		{
			ls2k_i2c_set_ack_pos(pIIC, remain);
		}
	}

	rt = pch - buf;
    return rt;
}

static int ls2k_i2c_write_bytes_poll(I2C_bus_t *pIIC, unsigned char *buf, int len)
{
	unsigned char *pch = buf;
	int rt = 0, remain = len;

	//-----------------------------------------
	// 写数据
	//-----------------------------------------

	while (remain > 0)
	{
		unsigned char ch;
		unsigned int wait_sr = I2C_SR1_TXE | I2C_SR1_BTF;

		rt = ls2k_i2c_wait_status1(pIIC, &wait_sr, 1000);   // 1ms
		if (rt < 0)
		{
			break;
		}

		/* write 1st byte
		 */
		if (wait_sr & I2C_SR1_TXE)
		{
			ch = *pch++;
			pIIC->hwI2C->dr = (unsigned int)ch;
			remain--;
		}

		/* write 2nd byte: I2C_SR1_BTF
		 */
		if ((remain > 0) && (wait_sr & I2C_SR1_BTF))
		{
			ch = *pch++;
			pIIC->hwI2C->dr = (unsigned int)ch;
			remain--;
		}
	}

	/**
	 * 写入最后一个字节, 必须等待 I2C_SR1_TXE 才能 STOP ?
	 */
	if (remain == 0)
	{
		unsigned int wait_sr = I2C_SR1_TXE;

		ls2k_i2c_wait_status1(pIIC, &wait_sr, 1000);
	}

	rt = pch - buf;
	return rt;
}

/******************************************************************************
 * I2C driver Implement
 ******************************************************************************/

extern int ls2k_i2c_init_hook(const void *bus);

/**
 * 初始化 I2C bus: 配置硬件寄存器, 安装中断等
 */
STATIC_DRV int I2C_initialize(const void *bus)
{
	I2C_bus_t *pIIC = (I2C_bus_t *)bus;

    if (bus == NULL)
    {
        return -1;
    }

    if (pIIC->initialized)
    {
        return 0;
    }

    ls2k_i2c_init_hook(bus);
    
    /* Disable I2C controller */
    pIIC->hwI2C->cr1 = 0;

	/* initialize the baudrate */
	ls2k_i2c_set_speed(pIIC, pIIC->speed);

    /**
     * 只能取最大值 0x3F
     */
    pIIC->hwI2C->cr2 = 0xE03F;          // XXX 0xE000 bit[15:13] 没有说明
    pIIC->hwI2C->trise = 0x3F;

    /* Enable I2C controller */
    pIIC->hwI2C->cr1 = I2C_CR1_PE;

    pIIC->p_mutex = osal_mutex_create(pIIC->dev_name, OSAL_OPT_FIFO);
    if (pIIC->p_mutex == NULL)
    {
        printk("create mutex for %s fail\r\n", pIIC->dev_name);
    	return -1;
    }

#if I2C_USE_INT

  #if I2C_USE_EVENT
  
    pIIC->p_event = osal_event_create(pIIC->dev_name, OSAL_OPT_FIFO);
    if (NULL == pIIC->p_event)
    {
        osal_mutex_delete(pIIC->p_mutex);
        pIIC->p_mutex = NULL;
        printk("create event for %s fail.\r\n", pIIC->dev_name);
        return -1;
    }

  #endif

    /*
     * 安装中断
     */
    ls2k_interrupt_disable(pIIC->irqNum);

    ls2k_install_irq_handler(pIIC->irqNum,
                             ls2k_i2c_interrupt_handler,
                             (void *)pIIC);
  #if !USE_EXTINT
    ls2k_set_irq_routeip(pIIC->irqNum, INT_ROUTE_IP0);
  #endif

    pIIC->data = NULL;

#endif

    pIIC->WorkMode = 0;
    
    /*
     * Default workmode is DMA ?
     */
    // ls2k_i2c_dma_set_using(pIIC, I2C_WORK_DMA);

    pIIC->is_started = false;
    pIIC->address = 0;

    pIIC->initialized = 1;

    printk("I2C%i controller initialized.\r\n", \
           (VA_TO_PHYS(pIIC->hwI2C) == I2C0_BASE) ? 0 : \
           (VA_TO_PHYS(pIIC->hwI2C) == I2C1_BASE) ? 1 : \
           (VA_TO_PHYS(pIIC->hwI2C) == I2C2_BASE) ? 2 : \
           (VA_TO_PHYS(pIIC->hwI2C) == I2C3_BASE) ? 3 : -1);

	return 0;
}

/**
 * 启动 I2C bus: 抢占设备、检查是否 busy、使能中断
 */
STATIC_DRV int I2C_send_start(const void *bus, unsigned int Addr)
{
    int tmo = 0;
	I2C_bus_t *pIIC = (I2C_bus_t *)bus;

    if (bus == NULL)
    {
        return -1;
    }

    LOCK(pIIC);

	/**
     * wait for bus idle
     */
	while (pIIC->hwI2C->sr2 & I2C_SR2_BUSY)
	{
		if (tmo++ > 10)
		{
		    printk("%s is always BUSY!\r\n", pIIC->dev_name);

		    UNLOCK(pIIC);
        	return -EBUSY;
        }

        ls2k_i2c_do_reset(pIIC);
	}

    if (tmo)
    {
        printk("wait BUSY %i\r\n", tmo);
    }

#if I2C_USE_INT
    if (pIIC->WorkMode == I2C_WORK_INT)
    {
        /*
         * 使能中断
         */
        pIIC->hwI2C->cr2 &= ~I2C_IEN_IRQ_MASK;

        ls2k_interrupt_enable(pIIC->irqNum);

        /*
         * XXX 不要使能 I2C_IEN_ITBUFEN 中断, 否则立即产生中断
         */
        pIIC->hwI2C->cr2 |= I2C_IEN_ITEVT | I2C_IEN_ITERR; // I2C_IEN_IRQ_MASK
    }
#endif

	return 0;
}

/**
 * I2C bus 发送从设备地址
 */
/*
 * Addr 传入的是 7bits slave 地址
 */
STATIC_DRV int I2C_send_addr(const void *bus, unsigned int Addr, int rw)
{
    I2C_bus_t *pIIC = (I2C_bus_t *)bus;
	unsigned int wait_sr;
	int rt = 0;
	
    if (bus == NULL)
    {
        return -1;
    }

    DEBUG("START\r\n");

    /**
     * Interrupt mode
     */
#if I2C_USE_INT
    if (pIIC->WorkMode == I2C_WORK_INT)
    {
        pIIC->data = NULL;
        pIIC->is_started = false;

        /*
         * set then address, 1: read, 0: write.
         */
        pIIC->address = ((unsigned char)Addr << 1) | ((rw) ? 1 : 0);

        //-------------------------------------------
        // 执行 START 命令, 等待中断响应
        //-------------------------------------------

	    pIIC->hwI2C->cr1 |= I2C_CR1_START;
    }
    else
    
#endif // #if I2C_USE_INT

    /**
     * DMA & POLL mode
     */
    {
        pIIC->is_started = false;

	    /*
         * "start" command to handle the bus
         */
	    pIIC->hwI2C->cr1 |= I2C_CR1_START;

        /*
         * wait for I2C_SR1_SB status
         */
        wait_sr = I2C_SR1_SB;
        rt = ls2k_i2c_wait_status1(pIIC, &wait_sr, 1000);   // 1ms
        if (rt < 0)
        {
            UNLOCK(pIIC);
            return rt;
        }

        pIIC->is_started = true;

        /*
         * write address clear I2C_SR1_SB
         */
        /* sets slave address, 1: read, 0: write. */
        pIIC->address = ((unsigned char)Addr << 1) | ((rw) ? 1 : 0);

        pIIC->hwI2C->dr = pIIC->address;

        /*
         * 等待地址发送成功
         */
        wait_sr = I2C_SR1_ADDR;
        rt = ls2k_i2c_wait_status1(pIIC, &wait_sr, 1000);  // 1ms
        if (rt < 0)
        {
            I2C_send_stop(pIIC, 0);
        }

        /* read for clear I2C_SR1_ADDR */
        (void)READ_REG32(&pIIC->hwI2C->sr2);
    }

    return rt;
}

/**
 * 读 I2C bus: 返回读取的字节数
 */
STATIC_DRV int I2C_read_bytes(const void *bus, unsigned char *buf, int len)
{
	int rt = 0;
	I2C_bus_t *pIIC = (I2C_bus_t *)bus;

    if ((bus == NULL) || (buf == NULL))
    {
        return -1;
    }

    if (len == 0)
    {
        return 0;
    }
    
    /**
     * Interrupt mode
     */
#if I2C_USE_INT
    if (pIIC->WorkMode == I2C_WORK_INT)
    {
        rt = ls2k_i2c_read_bytes_int(pIIC, buf, len);
    }
    else
    
#endif // #if I2C_USE_INT

    /**
     * DMA mode
     */
#if I2C_USE_DMA

#endif // #if I2C_USE_DMA

    /**
     * POLL mode
     */
    {
        rt = ls2k_i2c_read_bytes_poll(pIIC, buf, len);
    }

    return rt;
}

/**
 * 写 I2C bus: 返回写入的字节数
 */
STATIC_DRV int I2C_write_bytes(const void *bus, unsigned char *buf, int len)
{
	int rt = 0;
	I2C_bus_t *pIIC = (I2C_bus_t *)bus;

    if ((bus == NULL) || (buf == NULL))
    {
        return -1;
    }

    if (len == 0)
    {
        return 0;
    }

    /**
     * Interrupt mode
     */
#if I2C_USE_INT
    if (pIIC->WorkMode == I2C_WORK_INT)
    {
        rt = ls2k_i2c_write_bytes_int(pIIC, buf, len);
    }
    else

#endif // #if I2C_USE_INT

    /**
     * DMA mode
     */
#if I2C_USE_DMA

#endif // #if I2C_USE_DMA

    /**
     * POLL mode
     */
    {
        rt = ls2k_i2c_write_bytes_poll(pIIC, buf, len);
    }
    
    return rt;
}

/**
 * 控制 I2C bus: 设置 speed、slave-address 等
 */
STATIC_DRV int I2C_ioctl(const void *bus, int cmd, void *arg)
{
	int rt = 0;
	I2C_bus_t *pIIC = (I2C_bus_t *)bus;

    if (bus == NULL)
    {
        return -1;
    }

	switch (cmd)
	{
		case IOCTL_SPI_I2C_SET_TFRMODE:
			rt = -ls2k_i2c_set_speed(pIIC, (unsigned int)(uintptr_t)arg);
			break;

        case IOCTL_I2C_SET_WORKMODE:
            rt = ls2k_i2c_set_workmode(pIIC, (long)arg);
            break;

        case IOCTL_I2C_GET_WORKMODE:
            rt = ls2k_i2c_get_workmode(pIIC);
            break;

        case IOCTL_I2C_PRINT_WORKMODE:
            ls2k_i2c_print_workmode(pIIC);
            break;

		default:
			rt = -1;
			break;
	}

	return rt;
}

/**
 * 停止 I2C bus: 发送stop命令, 禁止中断、释放设备
 */
STATIC_DRV int I2C_send_stop(const void *bus, unsigned int Addr)
{
	I2C_bus_t *pIIC = (I2C_bus_t *)bus;

    if (bus == NULL)
    {
        return -1;
    }

#if I2C_USE_INT

    pIIC->hwI2C->cr2 &= ~I2C_IEN_IRQ_MASK;

    /*
     * 禁止中断
     */
    ls2k_interrupt_disable(pIIC->irqNum);

#endif

    if (pIIC->is_started)
    {
        /* "stop" command to release the bus */
	    pIIC->hwI2C->cr1 |= I2C_CR1_STOP;
        ls2k_i2c_wait_stop_done(pIIC);

        pIIC->is_started = false;

        pIIC->address = 0;

        DEBUG("STOP\r\n");

        UNLOCK(pIIC);
    }

	return 0;
}

//-----------------------------------------------------------------------------
// IIC bus driver ops
//-----------------------------------------------------------------------------

#if (PACK_DRV_OPS)
static const libi2c_ops_t ls2k_i2c_drv_ops =
{
    .init        = I2C_initialize,
    .send_start  = I2C_send_start,
    .send_stop   = I2C_send_stop,
    .send_addr   = I2C_send_addr,
    .read_bytes  = I2C_read_bytes,
    .write_bytes = I2C_write_bytes,
    .ioctl       = I2C_ioctl,
};
const libi2c_ops_t *i2c_drv_ops = &ls2k_i2c_drv_ops;
#endif

//-----------------------------------------------------------------------------
// IIC bus device table
//-----------------------------------------------------------------------------

#if BSP_USE_I2C0
static I2C_bus_t ls2k_I2C0 =
{
	.hwI2C       = (HW_I2C_t *)PHYS_TO_UNCACHED(I2C0_BASE),
#if USE_EXTINT
	.irqNum      = EXTI0_I2C0_IRQ,
#else
	.irqNum      = INTC0_I2C_0_1_IRQ,
#endif
	.speed       = 400000,
#if I2C_USE_DMA
#endif
    .initialized = 0,
    .dev_name    = "i2c0",
};
const void *busI2C0 = &ls2k_I2C0;
#endif

#if BSP_USE_I2C1
static I2C_bus_t ls2k_I2C1 =
{
	.hwI2C       = (HW_I2C_t *)PHYS_TO_UNCACHED(I2C1_BASE),
#if USE_EXTINT
	.irqNum      = EXTI0_I2C1_IRQ,
#else
	.irqNum      = INTC0_I2C_0_1_IRQ,
#endif
	.speed       = 400000,
#if I2C_USE_DMA
#endif
    .initialized = 0,
    .dev_name    = "i2c1",
};
const void *busI2C1 = &ls2k_I2C1;
#endif

#if BSP_USE_I2C2
static I2C_bus_t ls2k_I2C2 =
{
	.hwI2C       = (HW_I2C_t *)PHYS_TO_UNCACHED(I2C2_BASE),
#if USE_EXTINT
	.irqNum      = EXTI0_I2C2_IRQ,
#else
	.irqNum      = INTC0_I2C_2_3_IRQ,
#endif
	.speed       = 400000,
#if I2C_USE_DMA
#endif
    .initialized = 0,
    .dev_name    = "i2c2",
};
const void *busI2C2 = &ls2k_I2C2;
#endif

#if BSP_USE_I2C3
static I2C_bus_t ls2k_I2C3 =
{
	.hwI2C       = (HW_I2C_t *)PHYS_TO_UNCACHED(I2C3_BASE),
#if USE_EXTINT
	.irqNum      = EXTI0_I2C3_IRQ,
#else
	.irqNum      = INTC0_I2C_2_3_IRQ,
#endif
	.speed       = 400000,
#if I2C_USE_DMA
#endif
    .initialized = 0,
    .dev_name    = "i2c3",
};
const void *busI2C3 = &ls2k_I2C3;
#endif

#endif

//-----------------------------------------------------------------------------

/*
 * @@ END
 */


