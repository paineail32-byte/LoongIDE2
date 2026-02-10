/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_uart.c
 *
 * created: 2022-02-24
 *  author: Bian
 */

#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <larchintrin.h>

#include "bsp.h"

#include "termios.h"
#include "cpu.h"

#include "ls2k300.h"
#include "ls2k300_irq.h"

#include "ls2k_drv_io.h"

#include "ls2k_uart_hw.h"
#include "ls2k_uart.h"

#include "osal.h"

//-----------------------------------------------------------------------------

/*
 * 使用中断
 */
#define UART_USE_INT    1

/*
 * 使用 DMA
 */
#define UART_USE_DMA    0

#if UART_USE_DMA
#include "ls2k_dma.h"
#endif

//-----------------------------------------------------------------------------

#if UART_USE_DMA || UART_USE_INT

#define DMA_BUF_SIZE    64

#define UART_BUF_SIZE   1024

typedef struct
{
    char  Buf[UART_BUF_SIZE];
    int   Count;
    char *pHead;
    char *pTail;
} UART_buf_t;

#endif

//-----------------------------------------------------------------------------

typedef struct
{
    HW_UART_t    *hwUART;
    unsigned int  BaudRate;
    unsigned int  RxTxMode;             /* UART_RX_DMA | UART_TX_INT etc */
    int           irqVector;            /* Irq vector number */

#if UART_USE_DMA || UART_USE_INT
    UART_buf_t RxData;                  /* RX Buffer */
    UART_buf_t TxData;                  /* TX Buffer */
#endif

#if UART_USE_DMA

#endif

    int  initialized;
    int  opened;
    char dev_name[16];
} UART_t;

//-------------------------------------------------------------------------------------------------
// Buffer: cycle mode, drop the most oldest data when add
//-------------------------------------------------------------------------------------------------

#if UART_USE_DMA || UART_USE_INT

/*
 * 保存 UART_BUF_SIZE 个字符.
 *
 * 字节: 0  1  2  3  4  5        ...
 *       __ __ xx xx xx xx __ __ ...
 *             ^           ^
 *             pHead       |
 *                         pTail
 *
 * if full or empty: pHead==pTail;
 */

static void initialize_buffer(UART_buf_t *data)
{
    data->Count = 0;
    data->pHead = data->pTail = data->Buf;
}

/*
 * If oveflow, overwrite always
 */
static int enqueue_to_buffer(UART_buf_t *data, char *buf, int len)
{
    int i;

    for (i=0; i<len; i++)
    {
        *data->pTail = buf[i];
        data->Count++;
        data->pTail++;
        if (data->pTail >= data->Buf + UART_BUF_SIZE)
            data->pTail = data->Buf;
    }

    /*
     * if overflow, override the lastest data
     */
    if (data->Count > UART_BUF_SIZE)    // overflow
    {
        data->Count = UART_BUF_SIZE;
        data->pHead = data->pTail;
    }

    return len;
}

static int dequeue_from_buffer(UART_buf_t *data, char *buf, int len)
{
    int i, count;

    count = len < data->Count ? len : data->Count;

    for (i=0; i<count; i++)
    {
        buf[i] = *data->pHead;
        data->Count--;
        data->pHead++;
        if (data->pHead >= data->Buf + UART_BUF_SIZE)
            data->pHead = data->Buf;
    }

    return count;
}

#endif // #if UART_USE_DMA || UART_USE_INT

//-------------------------------------------------------------------------------------------------
// UART Comomon Functions
//-------------------------------------------------------------------------------------------------

#define UART_USE_DTR    0
#define UART_USE_RTS    0

/******************************************************************************
 * These routines provide control of the RTS and DTR lines
 */

#if UART_USE_RTS
/*
 * UART_assert_RTS
 */
static int ls2k_uart_assert_RTS(UART_t *pUART)
{
	loongarch_critical_enter();
    pUART->hwUART->mcr |= UART_MCR_RTSC;
    loongarch_critical_exit();
    return 0;
}

/*
 * UART_negate_RTS
 */
static int ls2k_uart_negate_RTS(UART_t *pUART)
{
	loongarch_critical_enter();
    pUART->hwUART->mcr &= ~UART_MCR_RTSC;
    loongarch_critical_exit();
    return 0;
}
#endif

/******************************************************************************
 * These flow control routines utilise a connection from the local DTR
 * line to the remote CTS line
 */

#if UART_USE_DTR
/*
 * UART_assert_DTR
 */
static int ls2k_uart_assert_DTR(UART_t *pUART)
{
	loongarch_critical_enter();
    pUART->hwUART->mcr |= UART_MCR_DTRC;
    loongarch_critical_exit();
    return 0;
}

/*
 * UART_negate_DTR
 */
static int ls2k_uart_negate_DTR(UART_t *pUART)
{
	loongarch_critical_enter();
    pUART->hwUART->mcr &= ~UART_MCR_DTRC;
    loongarch_critical_exit();
    return 0;
}
#endif

/******************************************************************************
 * UART reset fifo
 */
#if 1
static inline void ls2k_uart_reset(UART_t *pUART)
{
    pUART->hwUART->R2.fcr = UART_FCR_FIFO_EN |
                            UART_FCR_RxRESET |
                            UART_FCR_TxRESET |
                            UART_FCR_TL_1B;
    delay_us(1);
}

#else
#define ls2k_uart_reset(pUART) \
    do { \
        pUART->hwUART->R2.fcr = UART_FCR_FIFO_EN | \
                                UART_FCR_RxRESET | \
                                UART_FCR_TxRESET | \
                                UART_FCR_TL_1B; \
        delay_us(1); \
    } while (0)

#endif

/******************************************************************************
 * UART_set_attributes
 */
static int ls2k_uart_set_attributes(UART_t *pUART, const struct termios *t)
{
    unsigned int div, dec, baud_requested;
    unsigned char lineCtrl, trash, irq_mask;

    if (t == NULL)
    {
    	return -1;
    }

    /* Calculate the baud rate divisor */
    baud_requested = t->c_cflag & CBAUD;
    if (baud_requested)
    	baud_requested = CFLAG_TO_BAUDRATE(baud_requested);
    else
        baud_requested = pUART->BaudRate;	/* default to 115200 baud */

    baud_requested = CFLAG_TO_BAUDRATE(baud_requested);
    div = apb_frequency / baud_requested / 16;
    dec = apb_frequency - (div * baud_requested * 16);
    dec = (dec * 255) / baud_requested / 16;

    lineCtrl = 0;

    /* Parity */
    if (t->c_cflag & PARENB)
    {
        lineCtrl |= UART_LCR_PE;
        if (!(t->c_cflag & PARODD))
            lineCtrl |= UART_LCR_EPS;
    }

    /* Character Size */
    if (t->c_cflag & CSIZE)
    {
        switch (t->c_cflag & CSIZE)
        {
            case CS5:  lineCtrl |= UART_LCR_BEC_5b; break;
            case CS6:  lineCtrl |= UART_LCR_BEC_6b; break;
            case CS7:  lineCtrl |= UART_LCR_BEC_7b; break;
            case CS8:  lineCtrl |= UART_LCR_BEC_8b; break;
        }
    }
    else
    {
        lineCtrl |= UART_LCR_BEC_8b;         /* default to 9600,8,N,1 */
    }

    /* Stop Bits */
    if (t->c_cflag & CSTOPB)
        lineCtrl |= UART_LCR_SB;             /* 2 stop bits */

    /*
     * Now actually set the chip
     */

    /* Save port interrupt mask */
    irq_mask = pUART->hwUART->R1.ien;

    /* Set the baud rate */
    pUART->hwUART->lcr = UART_LCR_DLAB;

    /* XXX are these registers right? */
    pUART->hwUART->R0.dll = (unsigned char)(div & 0xFFU);
    pUART->hwUART->R1.dlh = (unsigned char)((div >> 8) & 0xFFU);
    pUART->hwUART->R2.dld = (unsigned char)dec;

    /* Now write the line control */
    pUART->hwUART->lcr = lineCtrl;

    /* Restore port interrupt mask */
    pUART->hwUART->R1.ien = irq_mask;

    trash = pUART->hwUART->lsr;
    (void)trash;
    trash = pUART->hwUART->R0.dat;
    (void)trash;
    trash = pUART->hwUART->msr;
    (void)trash;

    return 0;
}

/**
 * 等待 UART 发送缓冲区为空
 */
static inline int ls2k_uart_wait_txfifo_empty(UART_t *pUART)
{
    while (!(pUART->hwUART->lsr & UART_LSR_TFE))
    {
        if (pUART->hwUART->lsr & UART_LSR_ERROR)
        {
            ls2k_uart_reset(pUART);
        }

        delay_us(1);
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
// DMA Support
//-------------------------------------------------------------------------------------------------
   
#if UART_USE_DMA

#endif // #if UART_USE_DMA

//-------------------------------------------------------------------------------------------------
// Interrupt Support
//-------------------------------------------------------------------------------------------------

#if UART_USE_INT

/*
 * 等待中断接收完成
 */
/*
static void ls2k_uart_int_wait_rx_done(UART_t *pUART)
{
    // do nothing
}
 */
 
/*
 * 等待中断传输完成
 */
static void ls2k_uart_int_wait_tx_done(UART_t *pUART)
{
    if (pUART->RxTxMode & UART_TX_INT)
    {
        while (pUART->TxData.Count > 0)
            delay_us(10);
    }

    ls2k_uart_wait_txfifo_empty(pUART);
}

/******************************************************************************
 * Process interrupt.
 */
static void ls2k_uart_interrupt_process(UART_t *pUART)
{
    char buf[UART_FIFO_SIZE+1];

    /*
     * Iterate until no more interrupts are pending
     */
    do
    {
        int i, count = 0;
        volatile unsigned char ien = pUART->hwUART->R1.ien;

        if (pUART->hwUART->lsr & UART_LSR_ERROR)
        {
            ls2k_uart_reset(pUART);
            break;
        }

        ien &= ~(UART_IEN_ITx | UART_IEN_IRx);
        if (pUART->RxTxMode & UART_RX_INT)
            ien |= UART_IEN_IRx;

        /*
         * 收到数据 0xc1
         */
        if ((pUART->hwUART->R2.isr & UART_ISR_MASK) == UART_ISR_RxTRIG)
        {
            /* Fetch received characters */
            for (i=0; i<UART_FIFO_SIZE; ++i)
            {
                if (pUART->hwUART->lsr & UART_LSR_DR)
                    buf[i] = (char)pUART->hwUART->R0.dat;
                else
                    break;
            }

            /*
             * Enqueue fetched characters to buffer
             */
            enqueue_to_buffer(&pUART->RxData, buf, i);
        }

        /* check if we need transmit characters go on
         */
        if ((pUART->hwUART->R2.isr & UART_ISR_MASK) == UART_ISR_TxEMPTY)
        {
            if (pUART->TxData.Count > 0)
            {
                /* Dequeue transmitted characters from buffer
                 */
                count = dequeue_from_buffer(&pUART->TxData, buf, UART_FIFO_SIZE);

                for (i=0; i<count; ++i)
                {
                    pUART->hwUART->R0.dat = buf[i];
                }
            }
        }

    #if UART_USE_DTR
        // TODO
    #endif
    #if UART_USE_RTS
        // TODO
    #endif

        if (count > 0)
            ien |= UART_IEN_ITx;

        pUART->hwUART->R1.ien = ien;

    } while ((pUART->hwUART->R2.isr & UART_ISR_INTp) == 0);
}

static void ls2k_uart_interrupt_handler(int vector, void *arg)
{
    if (NULL != arg)
    {
        UART_t *pUART = (UART_t *)arg;

#if USE_EXTINT

        ls2k_uart_interrupt_process(pUART);     /* 中断处理 */

#else

        unsigned char ien, isr;

        if (INTC0_UART_2_5_IRQ == vector)
        {
    #if (BSP_USE_UART2)
            pUART = (UART_t *)devUART2;
            ien = pUART->hwUART->R1.ien;
            isr = pUART->hwUART->R2.isr;
            if (((ien & UART_IEN_ITx) && (isr & UART_ISR_TxEMPTY)) ||
                ((ien & UART_IEN_IRx) && (isr & (UART_ISR_RxTRIG | UART_ISR_RxTMO))))
                ls2k_uart_interrupt_process(pUART);
    #endif
    
    #if (BSP_USE_UART3)
            pUART = (UART_t *)devUART3;
            ien = pUART->hwUART->R1.ien;
            isr = pUART->hwUART->R2.isr;
            if (((ien & UART_IEN_ITx) && (isr & UART_ISR_TxEMPTY)) ||
                ((ien & UART_IEN_IRx) && (isr & (UART_ISR_RxTRIG | UART_ISR_RxTMO))))
                ls2k_uart_interrupt_process(pUART);
    #endif
    
    #if (BSP_USE_UART4)
            pUART = (UART_t *)devUART4;
            ien = pUART->hwUART->R1.ien;
            isr = pUART->hwUART->R2.isr;
            if (((ien & UART_IEN_ITx) && (isr & UART_ISR_TxEMPTY)) ||
                ((ien & UART_IEN_IRx) && (isr & (UART_ISR_RxTRIG | UART_ISR_RxTMO))))
                ls2k_uart_interrupt_process(pUART);
    #endif

    #if (BSP_USE_UART5)
            pUART = (UART_t *)devUART5;
            ien = pUART->hwUART->R1.ien;
            isr = pUART->hwUART->R2.isr;
            if (((ien & UART_IEN_ITx) && (isr & UART_ISR_TxEMPTY)) ||
                ((ien & UART_IEN_IRx) && (isr & (UART_ISR_RxTRIG | UART_ISR_RxTMO))))
                ls2k_uart_interrupt_process(pUART);
    #endif
        }

        else if (INTC0_UART_6_9_IRQ == vector)
        {
    #if (BSP_USE_UART6)
            pUART = (UART_t *)devUART6;
            ien = pUART->hwUART->R1.ien;
            isr = pUART->hwUART->R2.isr;
            if (((ien & UART_IEN_ITx) && (isr & UART_ISR_TxEMPTY)) ||
                ((ien & UART_IEN_IRx) && (isr & (UART_ISR_RxTRIG | UART_ISR_RxTMO))))
                ls2k_uart_interrupt_process(pUART);
    #endif
    
    #if (BSP_USE_UART7)
            pUART = (UART_t *)devUART7;
            ien = pUART->hwUART->R1.ien;
            isr = pUART->hwUART->R2.isr;
            if (((ien & UART_IEN_ITx) && (isr & UART_ISR_TxEMPTY)) ||
                ((ien & UART_IEN_IRx) && (isr & (UART_ISR_RxTRIG | UART_ISR_RxTMO))))
                ls2k_uart_interrupt_process(pUART);
    #endif
    
    #if (BSP_USE_UART8)
            pUART = (UART_t *)devUART8;
            ien = pUART->hwUART->R1.ien;
            isr = pUART->hwUART->R2.isr;
            if (((ien & UART_IEN_ITx) && (isr & UART_ISR_TxEMPTY)) ||
                ((ien & UART_IEN_IRx) && (isr & (UART_ISR_RxTRIG | UART_ISR_RxTMO))))
                ls2k_uart_interrupt_process(pUART);
    #endif
    
    #if (BSP_USE_UART9)
            pUART = (UART_t *)devUART9;
            ien = pUART->hwUART->R1.ien;
            isr = pUART->hwUART->R2.isr;
            if (((ien & UART_IEN_ITx) && (isr & UART_ISR_TxEMPTY)) ||
                ((ien & UART_IEN_IRx) && (isr & (UART_ISR_RxTRIG | UART_ISR_RxTMO))))
                ls2k_uart_interrupt_process(pUART);
    #endif
        }

        else // INTC0_UART0_IRQ || INTC0_UART1_IRQ
        {
            ls2k_uart_interrupt_process(pUART);
        }

#endif
    }
}

/*
 *  This routine install the interrupt handler.
 */
static void ls2k_uart_int_install_isr(UART_t *pUART)
{
    if (pUART && (pUART->RxTxMode & UART_WORK_INT))
    {
        ls2k_install_irq_handler(pUART->irqVector,
                                 ls2k_uart_interrupt_handler,
                                 pUART);
#if !USE_EXTINT
        ls2k_set_irq_routeip(pUART->irqVector, INT_ROUTE_IP1);
#endif
        ls2k_interrupt_enable(pUART->irqVector);

        if (pUART->RxTxMode & UART_RX_INT)
            pUART->hwUART->R1.ien |= UART_IEN_IRx;
    }
}

static void ls2k_uart_int_remove_isr(UART_t *pUART)
{
    if (pUART && (pUART->RxTxMode & UART_WORK_INT))
    {
        pUART->hwUART->R1.ien &= ~(UART_IEN_IRx | UART_IEN_ITx);

        ls2k_interrupt_disable(pUART->irqVector);
 
        ls2k_remove_irq_handler(pUART->irqVector);
    }
}

static int ls2k_uart_int_set_using(UART_t *pUART, int rx_int, int tx_int)
{
    int rx_old = (pUART->RxTxMode & UART_RX_INT) ? 1 : 0;
    int tx_old = (pUART->RxTxMode & UART_TX_INT) ? 1 : 0;

    //-----------------------------------------------------
    // Use Interrupt
    //-----------------------------------------------------

    if (rx_int || tx_int)
    {
        /*
         * if not use originally, use It
         */
        if (!(rx_old || tx_old))
        {
    #if UART_USE_DMA

    #endif

            ls2k_uart_wait_txfifo_empty(pUART);

            if (rx_int) pUART->RxTxMode |= UART_RX_INT;     // for install isr
            if (tx_int) pUART->RxTxMode |= UART_TX_INT;     // for install isr

            ls2k_uart_int_install_isr(pUART);
        }
    }
    
    //-----------------------------------------------------
    // Not Use Interrupt
    //-----------------------------------------------------

    else // if (!(rx_int || tx_int))
    {
        /*
         * if use originally, stop It
         */
        if (rx_old || tx_old)
        {
            ls2k_uart_int_wait_tx_done(pUART);

            ls2k_uart_int_remove_isr(pUART);
        }
    }

    return 0;
}

/*
 * UART Interrupt write strings
 */
static int ls2k_uart_int_write_string(UART_t *pUART, char *buf, int len)
{
    int i, sent = 0;
    volatile unsigned char ien;
    
    /* if idle, send immediately
     */
    if (pUART->hwUART->lsr & UART_LSR_ERROR)
    {
        ls2k_uart_reset(pUART);
    }

    if (pUART->hwUART->lsr & UART_LSR_TFE)
    {
        ien = pUART->hwUART->R1.ien;
        
        ien &= ~(UART_IEN_IRx | UART_IEN_ITx);
        if (pUART->RxTxMode | UART_RX_INT)
            ien |= UART_IEN_IRx;

        sent = len <= UART_FIFO_SIZE ? len : UART_FIFO_SIZE;

        /* write data to transmit buffer */
        for (i=0; i<sent; ++i)
        {
            pUART->hwUART->R0.dat = buf[i];
        }

        if (sent > 0)
            ien |= UART_IEN_ITx;

        pUART->hwUART->R1.ien = ien;
    }

    /* add remain data to transmit cached buffer
     */
    if (sent < len)
    {
    	loongarch_critical_enter();
        sent += enqueue_to_buffer(&pUART->TxData, buf + sent, len - sent);
        loongarch_critical_exit();
    }

    return sent;
}

/*
 * UART Interrupt write strings
 */
static int ls2k_uart_int_read_string(UART_t *pUART, char *buf, int len, int timeout)
{
    int count = 0;

    buf[0] = 0xFF;      /* avoid read nothing */

    /*
     * 1st read out received data from rx buffer
     */
    if (pUART->RxData.Count > 0)
    {
        loongarch_critical_enter();
        count = dequeue_from_buffer(&pUART->RxData, buf, len);
        loongarch_critical_exit();
    }

    /*
     * 2nd wait to read with time out
     */
    while ((count < len) && (pUART->RxTxMode | UART_RX_INT))
    {
        /*
         * wait or not
         */
        if (timeout-- < 0)
        {
            errno = ETIMEDOUT;
            break;
        }

        osal_msleep(1);

        /*
         * if received, accept it
         */
        if (pUART->RxData.Count > 0)
        {
            loongarch_critical_enter();
            count += dequeue_from_buffer(&pUART->RxData, buf + count, len - count);
            loongarch_critical_exit();
        }
    }

    return count;
}

#endif // #if UART_USE_INT

//-------------------------------------------------------------------------------------------------
// UART poll access functions
//-------------------------------------------------------------------------------------------------

/*
 * UART Polled write char
 */
static void ls2k_uart_poll_output_char(UART_t *pUART, char ch)
{
    unsigned char saved_ien = pUART->hwUART->R1.ien;    /* Save port interrupt mask */
    pUART->hwUART->R1.ien = 0;                          /* Disable port interrupts */

    while (1)
    {
        /* Try to transmit the character in a critical section */
    	loongarch_critical_enter();

        /* Read the transmitter holding register and check it */
        if (pUART->hwUART->lsr & UART_LSR_TFE)
        {
            pUART->hwUART->R0.dat = ch;         /* Transmit character */
            loongarch_critical_exit();          /* Finished */
            break;
        }
        else
        {
        	loongarch_critical_exit();
        }

        /* Wait for transmitter holding register to be empty
         */
        ls2k_uart_wait_txfifo_empty(pUART);
    }

    pUART->hwUART->R1.ien = saved_ien;          /* Restore port interrupt mask */
}

/*
 * UART Polled write string
 */
static int ls2k_uart_poll_write_string(UART_t *pUART, char *buf, int len)
{
    int nwrite = 0;

    /*
     * poll each byte in the string out of the port.
     */
    while (nwrite < len)
    {
        /* transmit character */
        ls2k_uart_poll_output_char(pUART, *buf++);
        nwrite++;
    }

    /* return the number of bytes written. */
    return nwrite;
}

/*
 * Polled get char nonblocking
 */
static int ls2k_uart_poll_inbyte_nonblocking(UART_t *pUART)
{
    int ret = -1;
    volatile unsigned char ch;

    if (pUART->hwUART->lsr & UART_LSR_ERROR)
    {
        ls2k_uart_reset(pUART);
    }

    if (pUART->hwUART->lsr & UART_LSR_DR)
    {
        ch = pUART->hwUART->R0.dat;
        ret = (int)ch;
    }

    return ret;
}

/*
 * Polled get char blocking
 */
static int ls2k_uart_poll_inbyte_blocking(UART_t *pUART)
{
	volatile unsigned char ch = 0xFF;

    for ( ; ; )
    {
        if (pUART->hwUART->lsr & UART_LSR_ERROR)
        {
            ls2k_uart_reset(pUART);
        }

        if (pUART->hwUART->lsr & UART_LSR_DR)
        {
            ch = pUART->hwUART->R0.dat;
            break;
        }

        /*
         * if use RTOS, delay with RTOS func to throw-out time?
         */
        osal_msleep(1);
    }

    return (int)ch;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static int ls2k_uart_set_rxtx_mode(UART_t *pUART, int rxtx)
{
    unsigned int new_mode = 0;

#if UART_USE_DMA

#endif

#if UART_USE_INT

    int rx_int = (rxtx & UART_RX_INT) ? 1 : 0;
    int tx_int = (rxtx & UART_TX_INT) ? 1 : 0;

    ls2k_uart_int_set_using(pUART, rx_int, tx_int);

    if (rx_int)
        new_mode |= UART_RX_INT;
    if (tx_int)
        new_mode |= UART_TX_INT;

#endif

    pUART->RxTxMode = new_mode;

    return 0;
}

static int ls2k_uart_get_rxtx_mode(UART_t *pUART)
{
    return (int)pUART->RxTxMode;
}

static void ls2k_uart_print_rxtx_mode(UART_t *pUART)
{
    if (pUART->initialized)
    {
        if (pUART->RxTxMode & UART_RX_DMA)
            printk("%s is using RX DMA\r\n", pUART->dev_name);
        if (pUART->RxTxMode & UART_TX_DMA)
            printk("%s is using TX DMA\r\n", pUART->dev_name);
            
        if (pUART->RxTxMode & UART_RX_INT)
            printk("%s is using RX INT\r\n", pUART->dev_name);
        if (pUART->RxTxMode & UART_TX_INT)
            printk("%s is using TX INT\r\n", pUART->dev_name);
            
        if (!(pUART->RxTxMode & (UART_RX_INT | UART_RX_DMA)))
            printk("%s is using RX Poll\r\n", pUART->dev_name);
        if (!(pUART->RxTxMode & (UART_TX_INT | UART_TX_DMA)))
            printk("%s is using TX Poll\r\n", pUART->dev_name);
    }
}

//-------------------------------------------------------------------------------------------------
// Driver Implements
//-------------------------------------------------------------------------------------------------

static int bConsolePortInitialized = 0;

/******************************************************************************
 * UART_initialize
 */

extern int ls2k_uart_init_hook(const void *dev);

STATIC_DRV int UART_initialize(const void *dev, void *arg)
{
    unsigned int div, dec, baud_requested;
    unsigned char trash;
    UART_t *pUART = (UART_t *)dev;

    if (dev == NULL)
    {
    	return -1;
    }

    if (pUART->initialized)
    {
    	return 0;
    }

    ls2k_uart_init_hook(dev);
    
    if (arg != NULL)
        baud_requested = (unsigned int)((unsigned long)arg);
    else
        baud_requested = pUART->BaudRate;	/* default is 115200,8N1 */

    div = apb_frequency / baud_requested / 16;
    dec = apb_frequency - (div * baud_requested * 16);
    dec = (dec * 255) / baud_requested / 16;

    /* Clear the divisor latch, clear all interrupt enables,
     * and reset and disable the FIFO's. */
    pUART->hwUART->lcr = 0;
    pUART->hwUART->R1.ien = 0;

    /* Set the divisor latch and set the baud rate. */
    pUART->hwUART->lcr = UART_LCR_DLAB;

    pUART->hwUART->R0.dll = (unsigned char)(div & 0xFFU);
    pUART->hwUART->R1.dlh = (unsigned char)((div >> 8) & 0xFFU);
    pUART->hwUART->R2.dld = (unsigned char)dec;

    /* Clear the divisor latch and set the character size to eight bits
     * with one stop bit and no parity checking. 8.N.1 */
    pUART->hwUART->lcr = UART_LCR_BEC_8b;

    /* Enable and reset transmit and receive FIFOs. */
    pUART->hwUART->R2.fcr = // UART_FCR_FIFO_EN |
                            UART_FCR_RxRESET |
                            UART_FCR_TxRESET |
                            UART_FCR_TL_1B;
    pUART->hwUART->R1.ien = 0;

    /* Set data terminal ready. */
    /* And open interrupt tristate line */
    pUART->hwUART->mcr = 0; // UART_MCR_OUT2;    // TODO modem

    trash = pUART->hwUART->lsr;
    (void)trash;
    trash = pUART->hwUART->R0.dat;
    (void)trash;
    trash = pUART->hwUART->msr;
    (void)trash;

#if UART_USE_INT
    /*
     * XXX 控制台串口使用查询模式
     */
    if (pUART == (UART_t *)ConsolePort)
        pUART->RxTxMode = 0;

#endif

#if UART_USE_DMA

#endif

    if (pUART == ConsolePort)
    {
        bConsolePortInitialized = 1;
    }

    if (bConsolePortInitialized && (pUART != (UART_t *)ConsolePort))
    {
        printk("UART%i controller initialized.\r\n", \
               (VA_TO_PHYS(pUART->hwUART) == UART0_BASE) ? 0 : \
               (VA_TO_PHYS(pUART->hwUART) == UART1_BASE) ? 1 : \
               (VA_TO_PHYS(pUART->hwUART) == UART2_BASE) ? 2 : \
               (VA_TO_PHYS(pUART->hwUART) == UART3_BASE) ? 3 : \
               (VA_TO_PHYS(pUART->hwUART) == UART4_BASE) ? 4 : \
               (VA_TO_PHYS(pUART->hwUART) == UART5_BASE) ? 5 : \
               (VA_TO_PHYS(pUART->hwUART) == UART6_BASE) ? 6 : \
               (VA_TO_PHYS(pUART->hwUART) == UART7_BASE) ? 7 : \
               (VA_TO_PHYS(pUART->hwUART) == UART8_BASE) ? 8 : \
               (VA_TO_PHYS(pUART->hwUART) == UART9_BASE) ? 9 : -1);
    }

    pUART->initialized = 1;
    return 0;
}

/******************************************************************************
 * UART_open
 */
STATIC_DRV int UART_open(const void *dev, void *arg)
{
    UART_t *pUART = (UART_t *)dev;
    struct termios *t = (struct termios *)arg;

    if (dev == NULL)
    {
    	return -1;
    }

    if (pUART->opened)
    {
    	return 0;
    }

#if UART_USE_DTR
    ls2k_uart_assert_DTR(pUART);    /* Assert DTR */
#endif
#if UART_USE_RTS
    ls2k_uart_assert_RTS(pUART);    /* Assert RTS */
#endif

    /* Set initial baud */
    if (t != NULL)
        ls2k_uart_set_attributes(pUART, t);

#if UART_USE_DMA || UART_USE_INT
    initialize_buffer(&pUART->RxData);
    initialize_buffer(&pUART->TxData);
#endif

#if UART_USE_INT
    if (pUART->RxTxMode & UART_WORK_INT)
        ls2k_uart_int_install_isr(pUART);
#endif

    pUART->opened = 1;
    return 0;
}

/******************************************************************************
 * UART_close
 */
STATIC_DRV int UART_close(const void *dev, void *arg)
{
    UART_t *pUART = (UART_t *)dev;

    if (dev == NULL)
    {
    	return -1;
    }

    pUART->hwUART->R1.ien = 0;

#if UART_USE_DTR
    ls2k_uart_negate_DTR(pUART);    /* Negate DTR */
#endif
#if UART_USE_RTS
    ls2k_uart_negate_RTS(pUART);    /* Negate RTS */
#endif

#if UART_USE_DMA

#endif

#if UART_USE_INT
    if (pUART->RxTxMode & UART_WORK_INT)
        ls2k_uart_int_remove_isr(pUART);
#endif

    pUART->opened = 0;
    return 0;
}

/******************************************************************************
 * UART read
 */
STATIC_DRV int UART_read(const void *dev, void *buf, int size, void *arg)
{
    UART_t *pUART = (UART_t *)dev;
    char *pch = (char *)buf;

    if ((dev == NULL) || (buf == NULL))
    {
    	return -1;
    }

    if (!pUART->opened)
    {
    	return 0;
    }
        
#if UART_USE_DMA

#endif

#if UART_USE_INT

    if ((pUART->RxTxMode & UART_RX_INT) && (pUART->hwUART->R1.ien & UART_IEN_IRx))
    {
        return ls2k_uart_int_read_string(pUART, (char *)buf, size, (long)arg);
    }
    else

#endif

    /**
     * poll read mode
     */
    {
        int i, val;
        int block = (long)arg;

        if (block == 0)                 /* return immediately */
        {
            for (i=0; i<size; i++)
            {
                val = ls2k_uart_poll_inbyte_nonblocking(pUART);
                if (val == -1)
                    return i;           /* can return 0 bytes */

                pch[i] = (char)val;
            }
        }
        else // if (block != 0)         /* blocking read */
        {
            for (i=0; i<size; i++)
            {
                val = ls2k_uart_poll_inbyte_blocking(pUART);
                pch[i] = (char)val;
            }
        }
    }

    return size;
}

/******************************************************************************
 * UART write
 */
STATIC_DRV int UART_write(const void *dev, void *buf, int size, void *arg)
{
    UART_t *pUART = (UART_t *)dev;
    int count;

    if ((dev == NULL) || (buf == NULL))
    {
    	return -1;
    }

    if (!pUART->opened)
    {
    	return 0;
    }

#if UART_USE_DMA

#endif

#if UART_USE_INT

    if (pUART->RxTxMode & UART_TX_INT) 
    {
        count = ls2k_uart_int_write_string(pUART, (char *)buf, size);
    }
    else
    
#endif

    /**
     * poll write mode
     */
    {
        count = ls2k_uart_poll_write_string(pUART, (char *)buf, size);
    }

    return count;
}

/******************************************************************************
 * UART control
 */
STATIC_DRV int UART_ioctl(const void *dev, int cmd, void *arg)
{
    int ret = 0;
    UART_t *pUART = (UART_t *)dev;

    if (dev == NULL)
    {
    	return -1;
    }

    if (!pUART->initialized)
    {
    	return 0;
    }
        
    switch (cmd)
    {
        case IOCTL_UART_SET_TERMIOS:
            ret = ls2k_uart_set_attributes(pUART, (struct termios *)arg);
            break;

        case IOCTL_UART_SET_RXTX_MODE:
            ret = ls2k_uart_set_rxtx_mode(pUART, (long)arg);
            break;
            
        case IOCTL_UART_GET_RXTX_MODE:
            ret = ls2k_uart_get_rxtx_mode(pUART);
            break;

        case IOCTL_UART_PRINT_RXTX_MODE:
            ls2k_uart_print_rxtx_mode(pUART);
            break;
            
        default:
            break;
    }

    return ret;
}

//-------------------------------------------------------------------------------------------------
// UART devices
//-------------------------------------------------------------------------------------------------

/* UART 0 */
#if (BSP_USE_UART0)
static UART_t ls2k_UART0 =
{
    .hwUART    = (HW_UART_t *)PHYS_TO_UNCACHED(UART0_BASE),
    .BaudRate  = 115200,
    .RxTxMode  = 0,
#if USE_EXTINT
    .irqVector = EXTI0_UART0_IRQ,
#else
    .irqVector = INTC0_UART0_IRQ,
#endif
    .initialized = 0,
    .opened    = 0,
    .dev_name  = "uart0",
};
const void *devUART0 = &ls2k_UART0;
#endif

/* UART 1 */
#if (BSP_USE_UART1)
static UART_t ls2k_UART1 =
{
    .hwUART    = (HW_UART_t *)PHYS_TO_UNCACHED(UART1_BASE),
    .BaudRate  = 115200,
    .RxTxMode  = 0,
#if USE_EXTINT
    .irqVector = EXTI0_UART1_IRQ,
#else
    .irqVector = INTC0_UART1_IRQ,
#endif
    .initialized = 0,
    .opened    = 0,
    .dev_name  = "uart1",
};
const void *devUART1 = &ls2k_UART1;
#endif

/* UART 2 */
#if (BSP_USE_UART2)
static UART_t ls2k_UART2 =
{
    .hwUART    = (HW_UART_t *)PHYS_TO_UNCACHED(UART2_BASE),
    .BaudRate  = 115200,
    .RxTxMode  = 0,
#if USE_EXTINT
    .irqVector = EXTI0_UART2_IRQ,
#else
    .irqVector = INTC0_UART_2_5_IRQ,
#endif
    .initialized = 0,
    .opened    = 0,
    .dev_name  = "uart2",
};
const void *devUART2 = &ls2k_UART2;
#endif

/* UART 3 */
#if (BSP_USE_UART3)
static UART_t ls2k_UART3 =
{
    .hwUART    = (HW_UART_t *)PHYS_TO_UNCACHED(UART3_BASE),
    .BaudRate  = 115200,
    .RxTxMode  = 0,
#if USE_EXTINT
    .irqVector = EXTI0_UART3_IRQ,
#else
    .irqVector = INTC0_UART_2_5_IRQ,
#endif
    .initialized = 0,
    .opened    = 0,
    .dev_name  = "uart2",
};
const void *devUART3 = &ls2k_UART3;
#endif

/* UART 4 */
#if (BSP_USE_UART4)
static UART_t ls2k_UART4 =
{
    .hwUART    = (HW_UART_t *)PHYS_TO_UNCACHED(UART4_BASE),
    .BaudRate  = 115200,
    .RxTxMode  = 0,
#if USE_EXTINT
    .irqVector = EXTI0_UART4_IRQ,
#else
    .irqVector = INTC0_UART_2_5_IRQ,
#endif
    .initialized = 0,
    .opened    = 0,
    .dev_name  = "uart4",
};
const void *devUART4 = &ls2k_UART4;
#endif

/* UART 5 */
#if (BSP_USE_UART5)
static UART_t ls2k_UART5 =
{
    .hwUART    = (HW_UART_t *)PHYS_TO_UNCACHED(UART5_BASE),
    .BaudRate  = 115200,
    .RxTxMode  = 0,
#if USE_EXTINT
    .irqVector = EXTI0_UART5_IRQ,
#else
    .irqVector = INTC0_UART_2_5_IRQ,
#endif
    .initialized = 0,
    .opened    = 0,
    .dev_name  = "uart5",
};
const void *devUART5 = &ls2k_UART5;
#endif

/* UART 6 */
#if (BSP_USE_UART6)
static UART_t ls2k_UART6 =
{
    .hwUART    = (HW_UART_t *)PHYS_TO_UNCACHED(UART6_BASE),
    .BaudRate  = 115200,
    .RxTxMode  = 0,
#if USE_EXTINT
    .irqVector = EXTI0_UART6_IRQ,
#else
    .irqVector = INTC0_UART_6_9_IRQ,
#endif
    .initialized = 0,
    .opened    = 0,
    .dev_name  = "uart6",
};
const void *devUART6 = &ls2k_UART6;
#endif

/* UART 7 */
#if (BSP_USE_UART7)
static UART_t ls2k_UART7 =
{
    .hwUART    = (HW_UART_t *)PHYS_TO_UNCACHED(UART7_BASE),
    .BaudRate  = 115200,
    .RxTxMode  = 0,
#if USE_EXTINT
    .irqVector = EXTI0_UART7_IRQ,
#else
    .irqVector = INTC0_UART_6_9_IRQ,
#endif
    .initialized = 0,
    .opened    = 0,
    .dev_name  = "uart7",
};
const void *devUART7 = &ls2k_UART7;
#endif

/* UART 8 */
#if (BSP_USE_UART8)
static UART_t ls2k_UART8 =
{
    .hwUART    = (HW_UART_t *)PHYS_TO_UNCACHED(UART8_BASE),
    .BaudRate  = 115200,
    .RxTxMode  = 0,
#if USE_EXTINT
    .irqVector = EXTI0_UART8_IRQ,
#else
    .irqVector = INTC0_UART_6_9_IRQ,
#endif
    .initialized = 0,
    .opened    = 0,
    .dev_name  = "uart8",
};
const void *devUART8 = &ls2k_UART8;
#endif

/* UART 9 */
#if (BSP_USE_UART9)
static UART_t ls2k_UART9 =
{
    .hwUART    = (HW_UART_t *)PHYS_TO_UNCACHED(UART9_BASE),
    .BaudRate  = 115200,
    .RxTxMode  = 0,
#if USE_EXTINT
    .irqVector = EXTI0_UART9_IRQ,
#else
    .irqVector = INTC0_UART_6_9_IRQ,
#endif
    .initialized = 0,
    .opened    = 0,
    .dev_name  = "uart9",
};
const void *devUART9 = &ls2k_UART9;
#endif

//-------------------------------------------------------------------------------------------------
// UART drivers
//-------------------------------------------------------------------------------------------------

#if (PACK_DRV_OPS)
/******************************************************************************
 * UART driver operators
 */
static const driver_ops_t ls2k_uart_drv_ops =
{
    .init_entry  = UART_initialize,
    .open_entry  = UART_open,
    .close_entry = UART_close,
    .read_entry  = UART_read,
    .write_entry = UART_write,
    .ioctl_entry = UART_ioctl,
};

const driver_ops_t *uart_drv_ops = &ls2k_uart_drv_ops;
#endif

/******************************************************************************
 * UART device name
 */
const char *ls2k_uart_get_device_name(const void *pUART)
{
    if (NULL == pUART)
        return NULL;

    return ((UART_t *)pUART)->dev_name;
}

//-------------------------------------------------------------------------------------------------
// Console Support
//-------------------------------------------------------------------------------------------------

char Console_get_char(void *pUART)
{
    return (char)ls2k_uart_poll_inbyte_nonblocking((UART_t *)pUART);
}

void Console_output_char(void *pUART, char ch)
{
    ls2k_uart_poll_output_char((UART_t *)pUART, ch);
}

/*
 * 默认控制台
 */
// void *ConsolePort = &ls2k_UART6;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/*
 * @@ END
 */

