/**************************************************************************************************
 * Modbus Board Support Package
 *
 * Filename: mb_bsp.C
 * Version:
 **************************************************************************************************/

/**************************************************************************************************
 * INCLUDE FILES
 **************************************************************************************************/

#include <stdint.h>
#include <string.h>

//-----------------------------------------------------------------------------
// PORTING FOR LS2K BARE/RTOS PROGRAMMING
//-----------------------------------------------------------------------------

#include "bsp.h"

#include "termios.h"

#include "ls2k_uart.h"

//-----------------------------------------------------------------------------

#include "../app/mb_cfg.h"
#include "../src/mb.h"

#include "mb_bsp.h"

/**************************************************************************************************
 * LOCAL DEFINES
 **************************************************************************************************/

/**************************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************************/

uint8_t  mb_devices_count = 0;                  /* Modbus channel counter (0..MODBUS_MAX_CH) */
MODBUS_t mb_devices_tbl[MODBUS_CFG_CHNL_MAX];   /* Modbus channels */

/**************************************************************************************************
 * LOCAL VARIABLES
 **************************************************************************************************/

/**************************************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 **************************************************************************************************/

/**************************************************************************************************
 * function:    modbus_hw_port_exit()
 * Description: This function is called to terminate Modbus communications.
 *              All Modbus channels are close.
 * Argument(s): none
 * Return(s):   none.
 *
 * Caller(s):   modbus_exit()
 * Note(s):     none.
 **************************************************************************************************/

void modbus_hw_port_exit(void)
{
    uint8_t   i;
    MODBUS_t *p_mb = &mb_devices_tbl[0];

    for (i = 0; i < MODBUS_CFG_CHNL_MAX; i++)
    {
        if (NULL != p_mb->PtrUART)
        {
            ls2k_uart_close(p_mb->PtrUART, NULL);
            p_mb->PtrUART = NULL;
        }

        p_mb++;
    }
}

/**************************************************************************************************
 * function:    modbus_hw_port_config()
 * Description: This function initializes the serial port to the desired baud rate and the UART
 *              will be configured for N, 8, 1 (No parity, 8 bits, 1 stop).
 * Argument(s): p_mb       is a pointer to the modbus channel
 *              baud       is the desired baud rate for the serial port.
 *              parity     is the desired parity and can be either:
 *                         MODBUS_PARITY_NONE
 *                         MODBUS_PARITY_ODD
 *                         MODBUS_PARITY_EVEN
 *              bits       specifies the number of bit and can be either 7 or 8.
 *              stops      specifies the number of stop bits and can either be 1 or 2
 * Return(s):   none.
 *
 * Caller(s):   modbus_config_node()
 * Note(s):     none.
 **************************************************************************************************/

void modbus_hw_port_config(MODBUS_t *p_mb,
                           uint32_t  baud,
                           uint8_t   bits,
                           uint8_t   parity,
                           uint8_t   stops)
{
    struct termios t;

    if ((NULL == p_mb) || (NULL == p_mb->PtrUART))
        return;

    p_mb->BaudRate = baud;
    p_mb->Parity   = parity;
    p_mb->Bits     = bits;
    p_mb->Stops    = stops;

    ls2k_uart_init(p_mb->PtrUART, NULL);            /* 1st Initialize the UART */

    memset(&t, 0, sizeof(struct termios));

    /* Baudrate */
    switch (baud)
    {
    	case 1200:	 t.c_cflag |= B1200;   break;
    	case 2400:	 t.c_cflag |= B2400;   break;
    	case 4800:	 t.c_cflag |= B4800;   break;
    	case 9600:	 t.c_cflag |= B9600;   break;
    	case 19200:	 t.c_cflag |= B19200;  break;
    	case 38400:	 t.c_cflag |= B38400;  break;
    	case 57600:	 t.c_cflag |= B57600;  break;
    	case 115200: t.c_cflag |= B115200; break;
    	default:     t.c_cflag |= B9600;   break;
    }

    /* Parity */
    if (parity == 'N')              /* None */
    {
        t.c_cflag &=~ PARENB;
    }
    else if (parity == 'E')         /* Even */
    {
        t.c_cflag |= PARENB;
        t.c_cflag &=~ PARODD;
    }
    else                            /* Odd */
    {
        t.c_cflag |= PARENB;
        t.c_cflag |= PARODD;
    }

    /* Character Size */
    switch (bits)
    {
    	case 5:  t.c_cflag |= CS5; break;
    	case 6:  t.c_cflag |= CS6; break;
    	case 7:  t.c_cflag |= CS7; break;
    	case 8:
    	default: t.c_cflag |= CS8; break;
    }

    /* Stop Bits */
    if (stops == 1)                 /* Stop bit = 1 */
        t.c_cflag &= ~CSTOPB;
    else                            /* Stop bit = 2 */
        t.c_cflag |= CSTOPB;

    /******************************************************
     * 用中断模式来接收和发送
     ******************************************************/

#ifdef LS2K300
    ls2k_uart_ioctl(p_mb->PtrUART,
                    IOCTL_UART_SET_RXTX_MODE,
                    (void *)(UART_RX_INT | UART_TX_POLL));
#else
    ls2k_uart_ioctl(p_mb->PtrUART, IOCTL_NS16550_USE_IRQ, NULL);
#endif

    /**
     * 打开设备
     */
    ls2k_uart_open(p_mb->PtrUART, (void *)&t);      /* 3nd Open the UART */

    /* Now modbus begin receiving. */
}

/**************************************************************************************************
 * function:    modbus_tx_bytes()
 * Description: This function Tx data buffer.
 * Argument(s): p_mb    is a pointer to the modbus channel
 * Return(s):   none.
 *
 * Caller(s):   modbus_tx()
 *
 * Note(s):     none.
 **************************************************************************************************/

void modbus_tx_bytes(MODBUS_t *p_mb)
{
    if (p_mb->TxBufByteCtr > 0)
    {
        int txcount;
        int count = (int)p_mb->TxBufByteCtr;

        while (count > 0)
        {
            txcount = ls2k_uart_write(p_mb->PtrUART,
                                      (void *)p_mb->TxBufPtr,
                                      (int)p_mb->TxBufByteCtr,
                                      NULL);

            if (txcount > 0)
            {
                count -= txcount;
                p_mb->TxBufByteCtr -= txcount;

                p_mb->TxCtr += txcount;
                p_mb->TxBufPtr += txcount;
            }
        }

#if (MODBUS_CFG_MASTER_EN == 1)
        if (p_mb->MasterSlave == MODBUS_MASTER)
        {
    #if (MODBUS_CFG_RTU_EN == 1)
            p_mb->RTU_TimeoutEn = false;            /* Disable RTU timeout timer until we start receiving */
    #endif
            p_mb->RxBufByteCtr = 0;                 /* Flush Rx buffer */
        }
#endif
    }
    else                                            /* If there is nothing to do end transmission */
    {
    	p_mb->TxBufPtr = &p_mb->TxBuf[0];           /* Reset at beginning of buffer */
    }
}

/**************************************************************************************************
 * function:    modbus_rx_1byte()
 * Description: This function Rx a byte.
 * Argument(s): p_mb    is a pointer to the modbus channel
 * Return(s):   none.
 *
 * Caller(s):   modbus_os_rx_wait()
 *              modbus_os_rx_task()
 *
 * Note(s):     none.
 **************************************************************************************************/

int modbus_rx_1byte(MODBUS_t *p_mb, uint8_t *rx_byte, int timeout)
{
    return ls2k_uart_read(p_mb->PtrUART,
                          (void *)rx_byte,
                          1,
                          (void *)(long)timeout);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

#if (MODBUS_CFG_RTU_EN == 1)

//-------------------------------------------------------------------------------------------------

uint16_t mb_rtu_frequency = 0;      /* Frequency at which RTU timer is running */
uint32_t mb_rtu_timer_count = 0;    /* Incremented every Modbus RTU timer interrupt */

#if BSP_USE_HPET0

#include "ls2k_hpet.h"

static void modbus_rtu_timer_callback(const void *hpet, int timer, int *stop)
{
    *stop = 0;
    mb_rtu_timer_count++;
    modbus_rtu_timer_update();
}

#elif BSP_USE_RTC

#include "ls2k_rtc.h"

static void modbus_rtu_timer_callback(int device, unsigned match, int *stop)
{
    *stop = 0;
    mb_rtu_timer_count++;           /* Indicate that we had activities on this interrupt. */
    modbus_rtu_timer_update();      /* Check for RTU timers that have expired */
}

#else
#error "no rtu timer defined."
#endif

/**************************************************************************************************
 * function:    modbus_rtu_timer_init()
 * Description: This function is called to initialize the RTU timeout timer.
 * Argument(s): freq          Is the frequency of the modbus RTU timer interrupt.
 * Return(s):   none.
 *
 * Caller(s):   modbus_init().
 * Note(s):     none.
 **************************************************************************************************/

void modbus_rtu_timer_init(void)
{
    if (mb_rtu_frequency == 0)
        mb_rtu_frequency = 100;                         /* 100HZ = 10ms */

#if BSP_USE_HPET0
    hpet_cfg_t cfg = { 0 };

    cfg.work_mode   = HPET_MODE_CYCLE;
    cfg.interval_ns = 1000*1000*1000 / mb_rtu_frequency;
    cfg.cb          = modbus_rtu_timer_callback;

    ls2k_hpet_timer_start(devHPET0, HPET_TIMER0, &cfg);
    
#elif BSP_USE_RTC
    rtc_cfg_t cfg = { 0 };

    cfg.interval_ms = 1000 / mb_rtu_frequency;          /* TODO 这个参数 */
    if (cfg.interval_ms == 0)
        cfg.interval_ms = 1;

    cfg.cb = modbus_rtu_timer_callback;                 /* called by match-isr */

    ls2k_rtc_timer_start(DEVICE_RTCMATCH0, &cfg);       // DEVICE_RTCMATCH0 用作定时器

#endif

    /*
     * Reset all the RTU timers.
     */
    modbus_rtu_timer_reset_all();
}

void modbus_rtu_timer_restart(MODBUS_t *p_mb)
{
    
}

/**************************************************************************************************
 * function:    modbus_rtu_timer_exit()
 * Description: This function is called to disable the RTU timeout timer.
 * Argument(s): none.
 * Return(s):   none.
 *
 * Caller(s):   modbus_exit()
 * Note(s):     none.
 **************************************************************************************************/

void modbus_rtu_timer_exit(void)
{
#if BSP_USE_HPET0
    ls2k_hpet_timer_stop(devHPET0, HPET_TIMER0);
#elif BSP_USE_RTC
    ls2k_rtc_timer_stop(DEVICE_RTCMATCH0);
#endif
}

#endif // #if (MODBUS_CFG_RTU_EN == 1)

//-----------------------------------------------------------------------------
/*
 * @@ END
 */

