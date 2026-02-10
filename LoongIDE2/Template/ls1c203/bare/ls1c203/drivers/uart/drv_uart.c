/*
 * Copyright (C) 2021-2025 Suzhou Tiancheng Software Ltd. All Rights Reserved.
 */

#include "bsp.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <larchintrin.h>

#include "cpu.h"
#include "termios.h"

#include "ls1c203.h"
#include "ls1c203_uart_hw.h"

#include "drv_uart.h"
#include "drv_gpio.h"

//-----------------------------------------------------------------------------

#define UART_FIFO_SIZE   16

//-----------------------------------------------------------------------------

#define UART_BUF_SIZE       64

typedef struct
{
    char  Buf[UART_BUF_SIZE];
    int   Count;
    char *pHead;
    char *pTail;
} UART_buf_t;

typedef struct
{
    HW_UART_t     *hwUART;
    bool           bInterrupt;
    unsigned int   irqNum;
    int            baudRate;
    UART_buf_t     rx_buf;
    UART_buf_t     tx_buf;
} UART_t;

//-----------------------------------------------------------------------------

/*
 * predefind functions
 */
static int UART_write_string_int(UART_t *pUART, char *buf, int len);

//-----------------------------------------------------------------------------
// buffer: ring mode, drop the most oldest data when add
//-----------------------------------------------------------------------------

static void clear_data_buffer(UART_buf_t *data)
{
    data->Count = 0;
    data->pHead = data->pTail = data->Buf;
}

static int enqueue_to_buffer(UART_buf_t *data, char *buf, int len, bool overWrite)
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

/******************************************************************************
 * UART Interrupt.
 */
static void ls1c203_enable_uart_irq(unsigned int uart_base)
{
    if (uart_base == UART0_BASE)
    {
        ls1c203_interrupt_enable(LS1C203_IRQ_UART0);
    }
    else if (uart_base == UART1_BASE)
    {
        ls1c203_interrupt_enable(LS1C203_IRQ_UART1);
    }
}

static void ls1c203_disable_uart_irq(unsigned int uart_base)
{
    if (uart_base == UART0_BASE)
    {
        ls1c203_interrupt_disable(LS1C203_IRQ_UART0);
    }
    else if (uart_base == UART1_BASE)
    {
        ls1c203_interrupt_disable(LS1C203_IRQ_UART1);
    }
}

static void ls1c203_clear_uart_irq(unsigned int uart_base)
{
    if (uart_base == UART0_BASE)
    {
        //OR_REG32(INTC_SR_BASE, INTC_UART0_BIT);
    }
    else if (uart_base == UART1_BASE)
    {
        //OR_REG32(INTC_SR_BASE, INTC_UART1_BIT);
    }
}

static void UART_interrupt_process(UART_t *pUART)
{
    int i = 0;
    char buf[UART_FIFO_SIZE];

    /*
     * Iterate until no more interrupts are pending
     */
    do
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
         * Enqueue fetched characters
         */
        enqueue_to_buffer(&pUART->rx_buf, buf, i, true);
        i = 0;

        /* Check if we can dequeue transmitted characters */
        if ((pUART->tx_buf.Count > 0) && (pUART->hwUART->lsr & UART_LSR_TFE))
        {
            /*
             * Dequeue transmitted characters
             */
            i = dequeue_from_buffer(&pUART->tx_buf, buf, UART_FIFO_SIZE);

            /* transmit data with interrupt */
            UART_write_string_int(pUART, buf, i);
        }

        if (i > 0) // (pUART->tx_buf.Count > 0)
            pUART->hwUART->R1.ien = UART_IEN_TX | UART_IEN_RX;
        else
            pUART->hwUART->R1.ien = UART_IEN_RX;

    } while (!(pUART->hwUART->R2.isr & UART_ISR_PENDING));
}

static void UART_interrupt_handler(int vector, void *arg)
{
    UART_t *pUART = (UART_t *)arg;

    if (NULL != pUART)
    {
        ls1c203_disable_uart_irq((unsigned int)pUART->hwUART);  /* 关中断 */
        ls1c203_clear_uart_irq((unsigned int)pUART->hwUART);    /* 清中断 */

        UART_interrupt_process(pUART);                       /* 中断处理 */

        ls1c203_enable_uart_irq((unsigned int)pUART->hwUART);   /* 开中断 */
    }
}

/******************************************************************************
 * These routines provide control of the RTS and DTR lines
 ******************************************************************************/

#ifdef UART_RTS
/*
 * UART_assert_RTS
 */
static int UART_assert_RTS(UART_t *pUART)
{
    loongarch_critical_enter();
    pUART->hwUART->
    pUART->ModemCtrl |= SP_MODEM_RTS;
    UART_set_r(pUART->CtrlPort, UART_MODEM_CONTROL, pUART->ModemCtrl);
    loongarch_critical_exit();
    return 0;
}

/*
 * UART_negate_RTS
 */
static int UART_negate_RTS(UART_t *pUART)
{
    loongarch_critical_enter();
    pUART->ModemCtrl &= ~SP_MODEM_RTS;
    UART_set_r(pUART->CtrlPort, UART_MODEM_CONTROL, pUART->ModemCtrl);
    loongarch_critical_exit();
    return 0;
}
#endif

/******************************************************************************
 * These flow control routines utilise a connection from the local DTR
 * line to the remote CTS line
 ******************************************************************************/

#ifdef UART_DTR
/*
 * UART_assert_DTR
 */
static int UART_assert_DTR(UART_t *pUART)
{
    loongarch_critical_enter();
    pUART->ModemCtrl |= SP_MODEM_DTR;
    UART_set_r(pUART->CtrlPort, UART_MODEM_CONTROL, pUART->ModemCtrl);
    loongarch_critical_exit();
    return 0;
}

/*
 * UART_negate_DTR
 */
static int UART_negate_DTR(UART_t *pUART)
{
    loongarch_critical_enter();
    pUART->ModemCtrl &= ~SP_MODEM_DTR;
    UART_set_r(pUART->CtrlPort, UART_MODEM_CONTROL, pUART->ModemCtrl);
    loongarch_critical_exit();
    return 0;
}

#endif

/******************************************************************************
 * UART_set_attributes
 */
static int UART_set_attributes(UART_t *pUART, const struct termios *t)
{
    unsigned int divisor, decimal, baud_requested, irq_mask;
    unsigned char lcr = 0;

    if (NULL == t)
    {
    	return -1;
    }

    /* Calculate the baud rate divisor */
    baud_requested = t->c_cflag & CBAUD;
    if (baud_requested)
    	baud_requested = CFLAG_TO_BAUDRATE(baud_requested);
    else
        baud_requested = pUART->baudRate;	/* default to 115200 baud */

    divisor = (bus_frequency >> 4) / baud_requested;
    decimal = (bus_frequency >> 4) - (divisor * baud_requested);
    decimal = (decimal * 255) / baud_requested;

    /* Parity */
    if (t->c_cflag & PARENB)
    {
        lcr |= UART_LCR_PE;
        if (!(t->c_cflag & PARODD))
            lcr |= UART_LCR_EPS;
    }

    /* Character Size */
    if (t->c_cflag & CSIZE)
    {
        switch (t->c_cflag & CSIZE)
        {
            case CS5:  lcr |= UART_LCR_BITS_5; break;
            case CS6:  lcr |= UART_LCR_BITS_6; break;
            case CS7:  lcr |= UART_LCR_BITS_7; break;
            case CS8:  lcr |= UART_LCR_BITS_8; break;
        }
    }
    else
    {
        lcr |= UART_LCR_BITS_8;         /* default to 9600,8,N,1 */
    }

    /* Stop Bits */
    if (t->c_cflag & CSTOPB)
        lcr |= UART_LCR_SB;             /* 2 stop bits */

    /*
     * Now actually set the chip
     */
    loongarch_critical_enter();

    /* Save port interrupt mask */
    irq_mask = pUART->hwUART->R1.ien;

    /* Set the baud rate */
    pUART->hwUART->lcr = UART_LCR_DLAB;

    /* XXX are these registers right? */
    pUART->hwUART->R0.dll = divisor & 0xFF;
    pUART->hwUART->R1.dlh = (divisor >> 8) & 0xFF;
    pUART->hwUART->R2.dld = decimal & 0xFF;

    /* Now write the line control */
    pUART->hwUART->lcr = lcr;

    /* Restore port interrupt mask */
    pUART->hwUART->R1.ien = irq_mask;

    READ_REG8(&pUART->hwUART->lsr);
    READ_REG8(&pUART->hwUART->R0.dat);
#ifdef UART_DTR
    READ_REG8(&pUART->hwUART->msr);
#endif

    loongarch_critical_exit();

    return 0;
}

/******************************************************************************
 * UART_init
 */
int UART_init(void *dev, void *arg)
{
	struct termios t;
    UART_t *pUART = (UART_t *)dev;

    if (NULL == dev)
    {
        return -1;
    }

#if (BSP_USE_UART0)
    if (dev == devUART0)
    {
        /*
         * 必须要配置端口一下才能 RX/TX
         */
        ls1c203_io_cfg(14, IO_IN_FLOAT);      // UART0_RX 外接上拉4.7K电阻
        ls1c203_io_cfg(15, IO_OUT_PULL);      // UART0_TX 外接下拉100K电阻

        ls1c203_io_sel(14, IOSEL_MAIN);
        ls1c203_io_sel(15, IOSEL_MAIN);
    }
#endif
#if (BSP_USE_UART1)
    else if (dev == devUART1)
    {
        // TODO
    }
#endif
    else
    {
        return -1;
    }

    if (NULL != arg)
        t.c_cflag = BAUDRATE_TO_CFLAG((unsigned int)arg) | CS8;
    else
    	t.c_cflag = BAUDRATE_TO_CFLAG(pUART->baudRate) | CS8;
    
    UART_set_attributes(pUART, &t);

    /* Enable and reset transmit and receive FIFOs. */
    pUART->hwUART->R2.fcr = UART_FCR_FIFO_EN |
                            UART_FCR_TXFIFO_RST |
                            UART_FCR_RXFIFO_RST |
                            UART_FCR_TRIGGER(1);

#ifdef UART_DTR
    /* Set data terminal ready. */
    /* And open interrupt tristate line */
    pUART->hwUART->mcr = SP_MODEM_IRQ;
    READ_REG8(&pUART->hwUART->msr);
#endif

    READ_REG8(&pUART->hwUART->lsr);
    READ_REG8(&pUART->hwUART->R0.dat);

    /* Disable Console Port Interrupt Mode */
    if (pUART == ConsolePort)
        pUART->bInterrupt = false;

    return 0;
}

/******************************************************************************
 * UART_open
 */
int UART_open(void *dev, void *arg)
{
    UART_t *pUART = (UART_t *)dev;
    struct termios *t = (struct termios *)arg;

    if (NULL == dev)
    {
        return -1;
    }

    clear_data_buffer(&pUART->rx_buf);
    clear_data_buffer(&pUART->tx_buf);

#ifdef UART_DTR
    /* Assert DTR */
    if (pUART->bFlowCtrl)
        UART_assert_DTR(pUART);
#endif

    pUART->hwUART->R1.ien = 0;
    
    /* Set initial baud */
    if (NULL != t)
    {
        UART_set_attributes(pUART, t);
    }

    if (pUART->bInterrupt)
    {
        /**
         * 安装中断向量
         */
        ls1c203_install_isr(pUART->irqNum,
                            UART_interrupt_handler,
                            dev);
        
        ls1c203_enable_uart_irq((unsigned int)pUART->hwUART);   /* 开中断 */
        pUART->hwUART->R1.ien = UART_IEN_RX;
    }

    return 0;
}

/******************************************************************************
 * UART_close
 */
int UART_close(void *dev, void *arg)
{
    UART_t *pUART = (UART_t *)dev;

    if (NULL == dev)
    {
        return -1;
    }
        
#ifdef UART_DTR
    /* Negate DTR */
    if (pUART->bFlowCtrl)
        UART_negate_DTR(pUART);
#endif

    pUART->hwUART->R1.ien = 0;
    if (pUART->bInterrupt)
    {
        ls1c203_disable_uart_irq((unsigned int)pUART->hwUART);
        ls1c203_remove_isr(pUART->irqNum);          /* 移除中断向量 */
    }

    return 0;
}

/*
 * Polled write for UART.
 */
static void UART_output_char_polled(UART_t *pUART, char ch)
{
    /* Save port interrupt mask */
    unsigned char irq_mask = pUART->hwUART->R1.ien;

    /* Disable port interrupts */
    pUART->hwUART->R1.ien = 0;

    while (true)
    {
        /* Try to transmit the character in a critical section */
        loongarch_critical_enter();

        /* Read the transmitter holding register and check it */
        if (pUART->hwUART->lsr & UART_LSR_TFE)
        {
            /* Transmit character */
            pUART->hwUART->R0.dat = ch;
            /* Finished */
            loongarch_critical_exit();
            break;
        }
        else
        {
            loongarch_critical_exit();
        }

        /* Wait for transmitter holding register to be empty
         * FIXME add timeout about 2ms, one byte transfer at 4800bps
         */
        while (!(pUART->hwUART->lsr & UART_LSR_TFE))
            ;
    }

    /* Restore port interrupt mask */
    pUART->hwUART->R1.ien = irq_mask;
}

/*
 * Interrupt write for UART.
 */
static int UART_write_string_int(UART_t *pUART, char *buf, int len)
{
    int i, out = 0;

    if (pUART->hwUART->lsr & UART_LSR_TFE)
    {
        out = len > UART_FIFO_SIZE ? UART_FIFO_SIZE : len;
        for (i=0; i<out; ++i)
        {
            pUART->hwUART->R0.dat = buf[i];
        }

        if (out > 0)
            pUART->hwUART->R1.ien = UART_IEN_TX | UART_IEN_RX;
        else
            pUART->hwUART->R1.ien = UART_IEN_RX;
    }

    /*
     * remain bytes to buffer
     */
    if (len > out)
    {
        loongarch_critical_enter();
        out += enqueue_to_buffer(&pUART->tx_buf, buf + out, len - out, false);
        loongarch_critical_exit();
    }

    return out;
}

/*
 * Polled write string
 */
static int UART_write_string_polled(UART_t *pUART, char *buf, int len)
{
    int nwrite = 0;

    /*
     * poll each byte in the string out of the port.
     */
    while (nwrite < len)
    {
        /* transmit character */
        UART_output_char_polled(pUART, *buf++);
        nwrite++;
    }

    /* return the number of bytes written. */
    return nwrite;
}

/*
 * Polled get char nonblocking
 */
static int UART_inbyte_nonblocking_polled(UART_t *pUART)
{
	volatile unsigned char ch;

    if (pUART->hwUART->lsr & UART_LSR_DR)
    {
        ch = pUART->hwUART->R0.dat;
        return (int)ch;
    }

    return -1;
}

/*
 * Polled get char blocking
 */
static int UART_inbyte_blocking_polled(UART_t *pUART)
{
	volatile unsigned char ch = -1;

    for ( ; ; )
    {
        if (pUART->hwUART->lsr & UART_LSR_DR)
        {
            ch = pUART->hwUART->R0.dat;
            break;
        }

        delay_ms(1);            /* wait 1 ms */
    }

    return (int)ch;
}

/******************************************************************************
 * UART read
 */
int UART_read(void *dev, unsigned char *buf, int size, void *arg)
{
    UART_t *pUART = (UART_t *)dev;
    int blocking, rdBytes;
    
    if (NULL == dev)
    {
        return -1;
    }

    if (NULL == arg)
        blocking = 0;
    else
        blocking = (int)arg;

    if (pUART->bInterrupt)
    {
        /* read from buffer */
        loongarch_critical_enter();
        rdBytes = dequeue_from_buffer(&pUART->rx_buf, (char *)buf, size);
        loongarch_critical_exit();

        return rdBytes;
    }
    else if (!blocking)
    {
        int i=0, val;
        for (i=0; i<size; i++)
        {
            val = UART_inbyte_nonblocking_polled(pUART);
            buf[i] = (unsigned char)val;
            if (val == -1)
                return i;
        }

        return size;
    }
    else
    {
        int i=0, val;
        for (i=0; i<size; i++)
        {
            val = UART_inbyte_blocking_polled(pUART);
            buf[i] = (unsigned char)val;
        }

        return size;
    }
}

/******************************************************************************
 * UART write
 */
int UART_write(void *dev, unsigned char *buf, int size, void *arg)
{
    UART_t *pUART = (UART_t *)dev;

    if (NULL == dev)
    {
        return -1;
    }

    if (pUART->bInterrupt)
    {
        return UART_write_string_int(pUART, (char *)buf, size);
    }
    else
    {
        return UART_write_string_polled(pUART, (char *)buf, size);
    }
}

/******************************************************************************
 * UART control
 */
int UART_ioctl(void *dev, unsigned cmd, void *arg)
{
    UART_t *pUART = (UART_t *)dev;

    if (NULL == dev)
    {
        return -1;
    }

    switch (cmd)
    {
        case IOCTL_UART_SET_MODE:
            /* Set initial baud */
            UART_set_attributes(pUART, (struct termios *)arg);
            break;

        default:
            break;
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Console Support
//-----------------------------------------------------------------------------

char Console_get_char(void *pUART)
{
    return (char)UART_inbyte_nonblocking_polled((UART_t *)pUART);
}

void Console_output_char(void *pUART, char ch)
{
    UART_output_char_polled((UART_t *)pUART, ch);
}

//-----------------------------------------------------------------------------
// UART devices, 必须放在 bss 段
//-----------------------------------------------------------------------------

/* UART 0 */
#if (BSP_USE_UART0)
static UART_t ls1c_UART0 =
{
    .hwUART     = (HW_UART_t *)UART0_BASE,
    .irqNum     = LS1C203_IRQ_UART0,
    .bInterrupt = false,
    .baudRate   = 115200,
};
const void *devUART0 = (const void *)&ls1c_UART0;
#endif

/* UART 1 */
#if (BSP_USE_UART1)
static UART_t ls1c_UART1 =
{
    .hwUART     = (HW_UART_t *)UART1_BASE,
    .irqNum     = LS1C203_IRQ_UART1,
    .bInterrupt = false,
    .baudRate   = 115200,
};
const void *devUART1 = (const void *)&ls1c_UART1;
#endif

//-----------------------------------------------------------------------------
// UART as Console
//-----------------------------------------------------------------------------

void *ConsolePort = (void *)&ls1c_UART0;

//-----------------------------------------------------------------------------

/*
 * @@ END
 */
