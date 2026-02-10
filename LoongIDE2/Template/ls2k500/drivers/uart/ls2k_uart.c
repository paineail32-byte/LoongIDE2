/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_uart.c
 *
 * created: 2022-02-24
 *  author: Bian
 */

#include <stddef.h>
#include <larchintrin.h>

#include "bsp.h"

#include "osal.h"

#include "termios.h"
#include "cpu.h"

#include "ls2k500.h"
#include "ls2k500_irq.h"

#include "ls2k_drv_io.h"

#include "ls2k_uart_hw.h"
#include "ls2k_uart.h"

//-----------------------------------------------------------------------------

//#define NS16550_DTR
//#define NS16550_RTS

//-----------------------------------------------------------------------------

#define NS16550_SUPPORT_INT   1         /* use interrupt or not */

#if (NS16550_SUPPORT_INT)

#define UART_BUF_SIZE       512

typedef struct
{
    char  Buf[UART_BUF_SIZE];
    int   Count;
    char *pHead;
    char *pTail;
} NS16550_buf_t;

#endif

//-----------------------------------------------------------------------------

typedef struct NS16550
{
    HW_NS16550_t  *hwUART;
    unsigned int   BaudRate;
    int            bFlowCtrl;           /* DTR/CTS support */
    int            bInterrupt;          /* Interrupt Support */
    int            irqVector;           /* Irq vector number */
#if (NS16550_SUPPORT_INT)
    NS16550_buf_t  RxData;              /* RX Buffer */
    NS16550_buf_t  TxData;              /* TX Buffer */
#endif
    int            initialized;
    int            opened;
    char           dev_name[16];
} NS16550_t;

//-----------------------------------------------------------------------------

#if (NS16550_SUPPORT_INT)

static int NS16550_write_string_int(NS16550_t *pUART, char *buf, int len);

//-----------------------------------------------------------------------------
// buffer: cycle mode, drop the most oldest data when add
//-----------------------------------------------------------------------------

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

static void initialize_buffer(NS16550_buf_t *data)
{
    data->Count = 0;
    data->pHead = data->pTail = data->Buf;
}

/*
 * If oveflow, overwrite always
 */
static int enqueue_to_buffer(NS16550_t *pUART, NS16550_buf_t *data, char *buf, int len)
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

static int dequeue_from_buffer(NS16550_t *pUART, NS16550_buf_t *data, char *buf, int len)
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

#endif

//-----------------------------------------------------------------------------
// interrupt
//-----------------------------------------------------------------------------

/*
 * This routine initializes the port to have the specified interrupts masked.
 */
static inline void NS16550_set_interrupt(NS16550_t *pUART, int mask)
{
#if (NS16550_SUPPORT_INT)
    pUART->hwUART->R1.ier = mask;
#else
    pUART->hwUART->R1.ier = 0;
#endif
}

#if (NS16550_SUPPORT_INT)

/******************************************************************************
 * Process interrupt.
 */
static void NS16550_interrupt_process(NS16550_t *pUART)
{
    int i, count = 0;
    char buf[NS16550_FIFO_SIZE+1];

    /*
     * Iterate until no more interrupts are pending
     */
    do
    {
        /*
         * 收到数据 0xc1
         */
        if ((pUART->hwUART->R2.isr & NS16550_ISR_II_MASK) == NS16550_ISR_II_RxTRIG)
        {
            /* Fetch received characters */
            for (i=0; i<NS16550_FIFO_SIZE; ++i)
            {
                if (pUART->hwUART->lsr & NS16550_LSR_DR)
                    buf[i] = (char)pUART->hwUART->R0.dat;
                else
                    break;
            }

            /*
             * Enqueue fetched characters to buffer
             */
            enqueue_to_buffer(pUART, &pUART->RxData, buf, i);
        }

        /* check if we need transmit characters go on
         */
        if ((pUART->TxData.Count > 0) && (pUART->hwUART->lsr & NS16550_LSR_TFE))
        {
            /* Dequeue transmitted characters from buffer
             */
            count = dequeue_from_buffer(pUART, &pUART->TxData, buf, NS16550_FIFO_SIZE);

            for (i=0; i<count; ++i)
            {
                pUART->hwUART->R0.dat = buf[i];
            }
        }

    #ifdef NS16550_DTR
        // TODO
    #endif
    #ifdef NS16550_RTS
        // TODO
    #endif

        if (count > 0)
            pUART->hwUART->R1.ier = NS16550_IER_ITxE | NS16550_IER_IRxE;
        else
            pUART->hwUART->R1.ier = NS16550_IER_IRxE;

    } while ((pUART->hwUART->R2.isr & NS16550_ISR_INTp) == 0);
}

static void NS16550_interrupt_handler(int vector, void *arg)
{
    if (NULL != arg)
    {
        NS16550_t *pUART = (NS16550_t *)arg;

#if USE_EXTINT
        ls2k_interrupt_disable(vector);          /* 关中断 */
#endif

        NS16550_interrupt_process(pUART);      	/* 中断处理 */

#if USE_EXTINT
        ls2k_interrupt_enable(vector);           /* 开中断 */
#endif
    }
}

/*
 *  This routine install the interrupt handler.
 */
static void NS16550_install_irq(NS16550_t *pUART)
{
    if ((pUART != NULL) && (pUART->irqVector > 0) && (pUART->bInterrupt))
    {
        ls2k_install_irq_handler(pUART->irqVector, NS16550_interrupt_handler, pUART);
#if !USE_EXTINT
        ls2k_set_irq_routeip(pUART->irqVector, INT_ROUTE_IP4);
        ls2k_set_irq_triggermode(pUART->irqVector, INT_TRIGGER_LEVEL);
#endif
        ls2k_interrupt_enable(pUART->irqVector);

        pUART->hwUART->R1.ier = NS16550_IER_IRxE;
    }
}

static void NS16550_remove_irq(NS16550_t *pUART)
{
    if ((pUART != NULL) && (pUART->irqVector > 0) && (pUART->bInterrupt))
    {
        pUART->hwUART->R1.ier = 0;

        ls2k_interrupt_disable(pUART->irqVector);
 
        ls2k_remove_irq_handler(pUART->irqVector);
    }
}

#endif

/******************************************************************************
 * These routines provide control of the RTS and DTR lines
 ******************************************************************************/

#ifdef NS16550_RTS
/*
 * NS16550_assert_RTS
 */
static int NS16550_assert_RTS(NS16550_t *pUART)
{
	loongarch_critical_enter();
    pUART->hwUART->mcr |= NS16550_MCR_RTSC;
    loongarch_critical_exit();
    return 0;
}

/*
 * NS16550_negate_RTS
 */
static int NS16550_negate_RTS(NS16550_t *pUART)
{
	loongarch_critical_enter();
    pUART->hwUART->mcr &= ~NS16550_MCR_RTSC;
    loongarch_critical_exit();
    return 0;
}
#endif

/******************************************************************************
 * These flow control routines utilise a connection from the local DTR
 * line to the remote CTS line
 ******************************************************************************/

#ifdef NS16550_DTR
/*
 * NS16550_assert_DTR
 */
static int NS16550_assert_DTR(NS16550_t *pUART)
{
	loongarch_critical_enter();
    pUART->hwUART->mcr |= NS16550_MCR_DTRC;
    loongarch_critical_exit();
    return 0;
}

/*
 * NS16550_negate_DTR
 */
static int NS16550_negate_DTR(NS16550_t *pUART)
{
	loongarch_critical_enter();
    pUART->hwUART->mcr &= ~NS16550_MCR_DTRC;
    loongarch_critical_exit();
    return 0;
}
#endif

/******************************************************************************
 * NS16550_set_attributes
 */
static int NS16550_set_attributes(NS16550_t *pUART, const struct termios *t)
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

    div = apb_frequency / baud_requested / 16;
    dec = apb_frequency - (div * baud_requested * 16);
    dec = (dec * 255) / baud_requested / 16;

    lineCtrl = 0;

    /* Parity */
    if (t->c_cflag & PARENB)
    {
        lineCtrl |= NS16550_LCR_PE;
        if (!(t->c_cflag & PARODD))
            lineCtrl |= NS16550_LCR_EPS;
    }

    /* Character Size */
    if (t->c_cflag & CSIZE)
    {
        switch (t->c_cflag & CSIZE)
        {
            case CS5:  lineCtrl |= NS16550_LCR_BEC_5b; break;
            case CS6:  lineCtrl |= NS16550_LCR_BEC_6b; break;
            case CS7:  lineCtrl |= NS16550_LCR_BEC_7b; break;
            case CS8:  lineCtrl |= NS16550_LCR_BEC_8b; break;
        }
    }
    else
    {
        lineCtrl |= NS16550_LCR_BEC_8b;         /* default to 9600,8,N,1 */
    }

    /* Stop Bits */
    if (t->c_cflag & CSTOPB)
        lineCtrl |= NS16550_LCR_SB;             /* 2 stop bits */

    /*
     * Now actually set the chip
     */

    /* Save port interrupt mask */
    irq_mask = pUART->hwUART->R1.ier;
    
    /* Set the baud rate */
    pUART->hwUART->lcr = NS16550_LCR_DLAB;

    /* XXX are these registers right? */
    pUART->hwUART->R0.dll = (unsigned char)(div & 0xFFU);
    pUART->hwUART->R1.dlh = (unsigned char)((div >> 8) & 0xFFU);
    pUART->hwUART->R2.dld = (unsigned char)dec;
    
    /* Now write the line control */
    pUART->hwUART->lcr = lineCtrl;

    /* Restore port interrupt mask */
    pUART->hwUART->R1.ier = irq_mask;

    *(&trash) = pUART->hwUART->lsr;    
    *(&trash) = pUART->hwUART->R0.dat; 
    *(&trash) = pUART->hwUART->msr;   

    return 0;
}

/******************************************************************************
 * NS16550_init
 */

extern int ls2k_uart_init_hook(const void *dev);

STATIC_DRV int NS16550_init(const void *dev, void *arg)
{
    unsigned int div, dec, baud_requested;
    unsigned char trash;
    NS16550_t *pUART = (NS16550_t *)dev;

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
    pUART->hwUART->R1.ier = 0;
    
    /* Set the divisor latch and set the baud rate. */
    pUART->hwUART->lcr = NS16550_LCR_DLAB;

    pUART->hwUART->R0.dll = (unsigned char)(div & 0xFFU);
    pUART->hwUART->R1.dlh = (unsigned char)((div >> 8) & 0xFFU);
    pUART->hwUART->R2.dld = (unsigned char)dec;

    /* Clear the divisor latch and set the character size to eight bits
     * with one stop bit and no parity checking. 8.N.1 */
    pUART->hwUART->lcr = NS16550_LCR_BEC_8b;

    /* Enable and reset transmit and receive FIFOs. */
    pUART->hwUART->R2.fcr = NS16550_FCR_FIFO_EN |
                            NS16550_FCR_RxRESET |
                            NS16550_FCR_TxRESET |
                            NS16550_FCR_TL_1B;
    pUART->hwUART->R1.ier = 0;

    /* Set data terminal ready. */
    /* And open interrupt tristate line */
    pUART->hwUART->mcr = 0; // NS16550_MCR_OUT2;    // TODO modem
    
    *(&trash) = pUART->hwUART->lsr;
    *(&trash) = pUART->hwUART->R0.dat;
    *(&trash) = pUART->hwUART->msr;

#if (NS16550_SUPPORT_INT)
    /*
     * XXX 控制台串口使用查询模式
     */
    if (pUART == (NS16550_t *)ConsolePort)
        pUART->bInterrupt = 0;
#endif

    if (pUART != (NS16550_t *)ConsolePort)
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
 * NS16550_open
 */
STATIC_DRV int NS16550_open(const void *dev, void *arg)
{
    NS16550_t *pUART = (NS16550_t *)dev;
    struct termios *t = (struct termios *)arg;

    if (dev == NULL)
    {
    	return -1;
    }

    if (pUART->opened)
    {
    	return 0;
    }

    if (pUART->bFlowCtrl)
    {
#ifdef NS16550_DTR
        NS16550_assert_DTR(pUART);      /* Assert DTR */
#endif
#ifdef NS16550_RTS
        NS16550_assert_RTS(pUART);      /* Assert RTS */
#endif
    }

    /* Set initial baud */
    if (t != NULL)
        NS16550_set_attributes(pUART, t);

#if (NS16550_SUPPORT_INT)
    initialize_buffer(&pUART->RxData);
    initialize_buffer(&pUART->TxData);

    if (pUART->bInterrupt)
    {
        NS16550_install_irq(pUART);
    }
#endif

    pUART->opened = 1;
    return 0;
}

/******************************************************************************
 * NS16550_close
 */
STATIC_DRV int NS16550_close(const void *dev, void *arg)
{
    NS16550_t *pUART = (NS16550_t *)dev;

    if (dev == NULL)
    {
    	return -1;
    }

    if (pUART->bFlowCtrl)
    {
#ifdef NS16550_DTR
        NS16550_negate_DTR(pUART);      /* Negate DTR */
#endif
#ifdef NS16550_RTS
        NS16550_negate_RTS(pUART);      /* Negate RTS */
#endif
    }

    pUART->hwUART->R1.ier = 0;

#if (NS16550_SUPPORT_INT)
    if (pUART->bInterrupt)
    {
        NS16550_remove_irq(pUART);
    }
#endif

    pUART->opened = 0;
    return 0;
}

/*
 * Polled write for NS16550.
 */
static void NS16550_output_char_polled(NS16550_t *pUART, char ch)
{
    unsigned char saved_irq;

    saved_irq = pUART->hwUART->R1.ier;          /* Save port interrupt mask */
    pUART->hwUART->R1.ier = 0;                  /* Disable port interrupts */

    while (1)
    {
        /* Try to transmit the character in a critical section */
    	loongarch_critical_enter();

        /* Read the transmitter holding register and check it */
        if (pUART->hwUART->lsr & NS16550_LSR_TFE)
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
         * FIXME add timeout about 2ms, one byte transfer at 4800bps
         */
        while (!(pUART->hwUART->lsr & NS16550_LSR_TFE));
    }

    /* Restore port interrupt mask */
    pUART->hwUART->R1.ier = saved_irq;
}

#if (NS16550_SUPPORT_INT)
/*
 * Interrupt write for NS16550.
 */
static int NS16550_write_string_int(NS16550_t *pUART, char *buf, int len)
{
    int i, sent = 0;

    /* if idle, send immediately
     */
    if (pUART->hwUART->lsr & NS16550_LSR_TFE)
    {
        sent = len <= NS16550_FIFO_SIZE ? len : NS16550_FIFO_SIZE;

        /* write data to transmit buffer */
        for (i=0; i<sent; ++i)
        {
            pUART->hwUART->R0.dat = buf[i];
        }

        if (sent > 0)
            pUART->hwUART->R1.ier = NS16550_IER_ITxE | NS16550_IER_IRxE;
        else
            pUART->hwUART->R1.ier = NS16550_IER_IRxE;
    }

    /* add remain data to transmit cached buffer
     */
    if (sent < len)
    {
    	loongarch_critical_enter();
        sent += enqueue_to_buffer(pUART, &pUART->TxData, buf + sent, len - sent);
        loongarch_critical_exit();
    }

    return sent;
}
#endif

/*
 * Polled write string
 */
static int NS16550_write_string_polled(NS16550_t *pUART, char *buf, int len)
{
    int nwrite = 0;

    /*
     * poll each byte in the string out of the port.
     */
    while (nwrite < len)
    {
        /* transmit character */
        NS16550_output_char_polled(pUART, *buf++);
        nwrite++;
    }

    /* return the number of bytes written. */
    return nwrite;
}

/*
 * Polled get char nonblocking
 */
static int NS16550_inbyte_nonblocking_polled(NS16550_t *pUART)
{
    volatile unsigned char ch;

    if (pUART->hwUART->lsr & NS16550_LSR_DR)
    {
        ch = pUART->hwUART->R0.dat;
        return (int)ch;
    }

    return -1;
}

/*
 * Polled get char blocking
 */
static int NS16550_inbyte_blocking_polled(NS16550_t *pUART)
{
	volatile unsigned char ch = 0xFF;

    for ( ; ; )
    {
        if (pUART->hwUART->lsr & NS16550_LSR_DR)
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

/******************************************************************************
 * NS16550 read
 */
STATIC_DRV int NS16550_read(const void *dev, void *buf, int size, void *arg)
{
    NS16550_t *pUART = (NS16550_t *)dev;
    char *pchbuf = (char *)buf;
    int timeout = (long int)arg;

    if ((dev == NULL) || (buf == NULL))
    {
    	return -1;
    }

#if (NS16550_SUPPORT_INT)
    if (pUART->bInterrupt)                      /* read from buffer */
    {
        int count;
        pchbuf[0] = 0xFF;                       /* avoid read nothing */

        loongarch_critical_enter();
        count = dequeue_from_buffer(pUART, &pUART->RxData, (char *)pchbuf, size);
        loongarch_critical_exit();

        /*
         * time out read
         */
        while ((count < size) && (timeout-- > 0))
        {
            osal_msleep(1);                     /* RTOS will give-up time */

            pchbuf = (char *)buf + count;

            loongarch_critical_enter();
            count += dequeue_from_buffer(pUART, &pUART->RxData, (char *)pchbuf, size - count);
            loongarch_critical_exit();
        }

        return count;
    }
    else
#endif
    if (timeout == 0)                           /* return immediately */
    {
        int i=0, val;
        for (i=0; i<size; i++)
        {
            val = NS16550_inbyte_nonblocking_polled(pUART);
            pchbuf[i] = (char)val;
            if (val == -1)
            {
                return i;                       /* can return 0 bytes */
            }
        }
    }
    else // if (timeout != 0)                   /* blocking read */
    {
        int i=0, val;
        for (i=0; i<size; i++)
        {
            val = NS16550_inbyte_blocking_polled(pUART);
            pchbuf[i] = (char)val;
        }
    }

    return size;
}

/******************************************************************************
 * NS16550 write
 */
STATIC_DRV int NS16550_write(const void *dev, void *buf, int size, void *arg)
{
    NS16550_t *pUART = (NS16550_t *)dev;
    int count;

    if ((dev == NULL) || (buf == NULL))
    {
    	return -1;
    }

#if (NS16550_SUPPORT_INT)
    if (pUART->bInterrupt)
        count = NS16550_write_string_int(pUART, (char *)buf, size);
    else
#endif
        count = NS16550_write_string_polled(pUART, (char *)buf, size);

    return count;
}

/******************************************************************************
 * NS16550 control
 */
STATIC_DRV int NS16550_ioctl(const void *dev, int cmd, void *arg)
{
    NS16550_t *pUART = (NS16550_t *)dev;

    if (dev == NULL)
    {
    	return -1;
    }

    switch (cmd)
    {
        case IOCTL_NS16550_SET_MODE:
            /* Set initial baud */
            if (arg != NULL)
                NS16550_set_attributes(pUART, (struct termios *)arg);
            break;

        case IOCTL_NS16550_USE_IRQ:
            if (arg != NULL)
                *(int *)arg = (int)pUART->bInterrupt;
            break;

        default:
            break;
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Console Support
//-----------------------------------------------------------------------------

char Console_get_char(const void *pUART)
{
    return (char)NS16550_inbyte_nonblocking_polled((NS16550_t *)pUART);
}

void Console_output_char(const void *pUART, char ch)
{
    NS16550_output_char_polled((NS16550_t *)pUART, ch);
}

//-----------------------------------------------------------------------------
// UART devices
//-----------------------------------------------------------------------------

/* UART 0 */
#if (BSP_USE_UART0)
static NS16550_t ls2k_UART0 =
{
    .hwUART      = (HW_NS16550_t *)PHYS_TO_UNCACHED(UART0_BASE),
    .BaudRate    = 115200,
    .bFlowCtrl   = 0,
    .bInterrupt  = 1,
#if USE_EXTINT
    .irqVector   = EXTINTC0_UART0_IRQ,
#else
    .irqVector   = INTC0_UART0_IRQ,
#endif
    .initialized = 0,
    .opened      = 0,
    .dev_name    = "uart0",
};

const void *devUART0 = &ls2k_UART0;
#endif

/* UART 1 */
#if (BSP_USE_UART1)
static NS16550_t ls2k_UART1 =
{
    .hwUART      = (HW_NS16550_t *)PHYS_TO_UNCACHED(UART1_BASE),
    .BaudRate    = 115200,
    .bFlowCtrl   = 0,
    .bInterrupt  = 1,
#if USE_EXTINT
    .irqVector   = EXTINTC0_UART1_IRQ,
#else
    .irqVector   = INTC0_UART1_IRQ,
#endif
    .initialized = 0,
    .opened      = 0,
    .dev_name    = "uart1",
};

const void *devUART1 = &ls2k_UART1;
#endif

/* UART 2 */
#if (BSP_USE_UART2)
static NS16550_t ls2k_UART2 =
{
    .hwUART      = (HW_NS16550_t *)PHYS_TO_UNCACHED(UART2_BASE),
    .BaudRate    = 115200,
    .bFlowCtrl   = 0,
    .bInterrupt  = 1,
#if USE_EXTINT
    .irqVector   = EXTINTC0_UART2_IRQ,
#else
    .irqVector   = INTC0_UART2_IRQ,
#endif
    .initialized = 0,
    .opened      = 0,
    .dev_name    = "uart2",
};

const void *devUART2 = &ls2k_UART2;
#endif

/* UART 3 */
#if (BSP_USE_UART3)
static NS16550_t ls2k_UART3 =
{
    .hwUART      = (HW_NS16550_t *)PHYS_TO_UNCACHED(UART3_BASE),
    .BaudRate    = 115200,
    .bFlowCtrl   = 0,
    .bInterrupt  = 1,
#if USE_EXTINT
    .irqVector   = EXTINTC0_UART3_IRQ,
#else
    .irqVector   = INTC0_UART3_IRQ,
#endif
    .initialized = 0,
    .opened      = 0,
    .dev_name    = "uart2",
};

const void *devUART3 = &ls2k_UART3;
#endif

/* UART 4 */
#if (BSP_USE_UART4)
static NS16550_t ls2k_UART4 =
{
    .hwUART      = (HW_NS16550_t *)PHYS_TO_UNCACHED(UART4_BASE),
    .BaudRate    = 115200,
    .bFlowCtrl   = 0,
    .bInterrupt  = 1,
#if USE_EXTINT
    .irqVector   = EXTINTC0_UART4_IRQ,
#else
    .irqVector   = INTC1_UART4_6_IRQ,
#endif
    .initialized = 0,
    .opened      = 0,
    .dev_name    = "uart4",
};

const void *devUART4 = &ls2k_UART4;
#endif

/* UART 5 */
#if (BSP_USE_UART5)
static NS16550_t ls2k_UART5 =
{
    .hwUART      = (HW_NS16550_t *)PHYS_TO_UNCACHED(UART5_BASE),
    .BaudRate    = 115200,
    .bFlowCtrl   = 0,
    .bInterrupt  = 1,
#if USE_EXTINT
    .irqVector   = EXTINTC0_UART5_IRQ,
#else
    .irqVector   = INTC1_UART4_6_IRQ,
#endif
    .initialized = 0,
    .opened      = 0,
    .dev_name    = "uart5",
};

const void *devUART5 = &ls2k_UART5;
#endif

/* UART 6 */
#if (BSP_USE_UART6)
static NS16550_t ls2k_UART6 =
{
    .hwUART      = (HW_NS16550_t *)PHYS_TO_UNCACHED(UART6_BASE),
    .BaudRate    = 115200,
    .bFlowCtrl   = 0,
    .bInterrupt  = 1,
#if USE_EXTINT
    .irqVector   = EXTINTC0_UART6_IRQ,
#else
    .irqVector   = INTC1_UART4_6_IRQ,
#endif
    .initialized = 0,
    .opened      = 0,
    .dev_name    = "uart6",
};

const void *devUART6 = &ls2k_UART6;
#endif

/* UART 7 */
#if (BSP_USE_UART7)
static NS16550_t ls2k_UART7 =
{
    .hwUART      = (HW_NS16550_t *)PHYS_TO_UNCACHED(UART7_BASE),
    .BaudRate    = 115200,
    .bFlowCtrl   = 0,
    .bInterrupt  = 1,
#if USE_EXTINT
    .irqVector   = EXTINTC0_UART7_IRQ,
#else
    .irqVector   = INTC1_UART7_9_IRQ,
#endif
    .initialized = 0,
    .opened      = 0,
    .dev_name    = "uart7",
};

const void *devUART7 = &ls2k_UART7;
#endif

/* UART 8 */
#if (BSP_USE_UART8)
static NS16550_t ls2k_UART8 =
{
    .hwUART      = (HW_NS16550_t *)PHYS_TO_UNCACHED(UART8_BASE),
    .BaudRate    = 115200,
    .bFlowCtrl   = 0,
    .bInterrupt  = 1,
#if USE_EXTINT
    .irqVector   = EXTINTC0_UART8_IRQ,
#else
    .irqVector   = INTC1_UART7_9_IRQ,
#endif
    .initialized = 0,
    .opened      = 0,
    .dev_name    = "uart8",
};

const void *devUART8 = &ls2k_UART8;
#endif

/* UART 9 */
#if (BSP_USE_UART9)
static NS16550_t ls2k_UART9 =
{
    .hwUART      = (HW_NS16550_t *)PHYS_TO_UNCACHED(UART9_BASE),
    .BaudRate    = 115200,
    .bFlowCtrl   = 0,
    .bInterrupt  = 1,
#if USE_EXTINT
    .irqVector   = EXTINTC0_UART9_IRQ,
#else
    .irqVector   = INTC1_UART7_9_IRQ,
#endif
    .initialized = 0,
    .opened      = 0,
    .dev_name    = "uart9",
};

const void *devUART9 = &ls2k_UART9;
#endif

//-----------------------------------------------------------------------------
// UART drivers
//-----------------------------------------------------------------------------

#if (PACK_DRV_OPS)
/******************************************************************************
 * UART driver operators
 */
static const driver_ops_t ls2k_uart_drv_ops =
{
    .init_entry  = NS16550_init,
    .open_entry  = NS16550_open,
    .close_entry = NS16550_close,
    .read_entry  = NS16550_read,
    .write_entry = NS16550_write,
    .ioctl_entry = NS16550_ioctl,
};

const driver_ops_t *uart_drv_ops = &ls2k_uart_drv_ops;
#endif

/******************************************************************************
 * device name
 */
const char *ls2k_uart_get_device_name(const void *pUART)
{
    return ((NS16550_t *)pUART)->dev_name;
}

//-----------------------------------------------------------------------------
// UART as Console
//-----------------------------------------------------------------------------

// void *ConsolePort = &ls2k_UART2;

//-----------------------------------------------------------------------------

/*
 * @@ END
 */

