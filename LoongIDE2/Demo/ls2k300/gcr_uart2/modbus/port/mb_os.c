/**************************************************************************************************
 * MODBUS uC/OS-II LAYER INTERFACE
 *
 * Filename: mb_os.c
 * Version:
 **************************************************************************************************/

/**************************************************************************************************
 * INCLUDE FILES
 **************************************************************************************************/

#include "bsp.h"

#include "osal.h"

#include "../src/mb.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

/**************************************************************************************************
 * DEFINES
 **************************************************************************************************/

#define MB_OS_CFG_RX_TASK_STK_SIZE     4*1024
#define MB_OS_CFG_RX_TASK_PRIO         5        // 优先级多少合适 ?
#define MB_OS_CFG_RX_TASK_ID           0x55

/**************************************************************************************************
 * LOCAL / GLOBAL VARIABLES
 **************************************************************************************************/

#if (MODBUS_CFG_SLAVE_EN  == 1)
static osal_task_t  mb_rx_task;         /* modbus 接收任务 */
#endif

/**************************************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 **************************************************************************************************/

#if (MODBUS_CFG_MASTER_EN == 1)
static void modbus_os_master_init(void);
static void modbus_os_master_exit(void);
#endif

#if (MODBUS_CFG_SLAVE_EN == 1)
static void modbus_os_slave_init(void);
static void modbus_os_slave_exit(void);
static void modbus_os_rx_task(void *p_arg);
#endif

/**************************************************************************************************
 * LOCAL CONFIGURATION ERRORS
 **************************************************************************************************/

/**************************************************************************************************
 * function:    modbus_os_init()
 * Description: This function initializes the RTOS interface.  This function creates the following:
 *              (1) A message queue to signal the reception of a packet.
 *              (2) A task that waits for packets to be received.
 * Argument(s): none
 * Return(s):   none.
 *
 * Caller(s):   modbus_init().
 * Note(s):     none.
 **************************************************************************************************/

void modbus_os_init(void)
{
#if (MODBUS_CFG_MASTER_EN == 1)
    modbus_os_master_init();
#endif

#if (MODBUS_CFG_SLAVE_EN == 1)
    modbus_os_slave_init();
#endif
}

/**************************************************************************************************
 * function:    modbus_os_master_init()
 * Description: This function initializes and creates the kernel objectes needed for Modbus Master
 * Argument(s): none.
 * Return(s):   none.
 *
 * Caller(s):   modbus_os_init().
 * Note(s):     none.
 **************************************************************************************************/

#if (MODBUS_CFG_MASTER_EN == 1)
static void modbus_os_master_init(void)
{
    return;
}
#endif

/**************************************************************************************************
 * function:    modbus_os_slave_init()
 * Description: This function initializes and creates the kernel objectes needed for Modbus Salve
 * Argument(s): none.
 * Return(s):   none.
 *
 * Caller(s):   modbus_os_init().
 * Note(s):     none.
 **************************************************************************************************/

#if (MODBUS_CFG_SLAVE_EN == 1)
static void modbus_os_slave_init(void)
{
    mb_rx_task = osal_task_create("Modbus Rx",
                                   MB_OS_CFG_RX_TASK_STK_SIZE,
                                   MB_OS_CFG_RX_TASK_PRIO,
                                   10,
                                   modbus_os_rx_task,
                                   NULL );

    if (NULL == mb_rx_task)
    {
        printk("create modbus slave rx task fail.\r\n");
    }
}
#endif

/**************************************************************************************************
 * function:    modbus_os_exit()
 * Description: This function is called to terminate the RTOS interface for Modbus channels.
 *              We delete the following uC/OS-II objects:
 *              (1) An event flag group to signal the reception of a packet.
 *              (2) A task that waits for packets to be received.
 * Argument(s): none.
 * Return(s):   none.
 *
 * Caller(s):   modbus_exit().
 * Note(s): none.
 **************************************************************************************************/

void modbus_os_exit(void)
{
#if (MODBUS_CFG_MASTER_EN == 1)
    modbus_os_master_exit();
#endif

#if (MODBUS_CFG_SLAVE_EN == 1)
    modbus_os_slave_exit();
#endif
}

/**************************************************************************************************
 * function:    modbus_os_master_exit()
 * Description: This function is called to terminate the RTOS interface for Modbus Master channels.
 *              The following objects are deleted.
 *              (1) An event flag group to signal the reception of a packet.
 * Argument(s): none.
 * Return(s):   none.
 *
 * Caller(s):   modbus_os_exit().
 * Note(s):     none.
 **************************************************************************************************/

#if (MODBUS_CFG_MASTER_EN == 1)
static void modbus_os_master_exit(void)
{
    return;
}
#endif

/**************************************************************************************************
 * function:    modbus_os_slave_exit()
 * Description: This function is called to terminate the RTOS interface for Modbus Salve channels.
 *              The following objects are deleted.
 *              (1) A task that waits for packets to be received.
 *              (2) A message queue to signal the reception of a packet.
 * Argument(s): none
 * Return(s):   none.
 *
 * Caller(s):   modbus_os_exit().
 * Note(s):     none.
 **************************************************************************************************/

#if (MODBUS_CFG_SLAVE_EN == 1)
void modbus_os_slave_exit(void)
{
    osal_task_delete(mb_rx_task);
}
#endif

/**************************************************************************************************
 * function:    modbus_os_rx_signal()
 * Description: This function signals the reception of a packet either from the Rx ISR(s) or
 *              the RTU timeout timer(s) to indicate that a received packet needs to be processed.
 * Argument(s): p_mb    specifies the Modbus channel data structure in which a packet was received.
 * Return(s):   none.
 *
 * Caller(s):   modbus_ascii_rx_byte(),
 *              modbus_rtu_timer_update().
 * Note(s):     none.
 **************************************************************************************************/

/*
 * RTU  模式: 通过接收超时定时器中断调用modbus_rtu_timer_update()==>...
 * ASCII模式: 通过modbus_os_rx_wait()==>modbus_rx_byte()==>...
 */
void modbus_os_rx_signal(MODBUS_t *p_mb)
{
    p_mb->RxDoneFlag = 1;

#if (MODBUS_CFG_ASCII_EN == 1)
    if (p_mb->Mode == MODBUS_MODE_ASCII)
    {
        modbus_rx_task(p_mb);
    }
#endif
}

/**************************************************************************************************
 * function:    modbus_os_rx_wait()
 * Description: This function waits for a response from a slave.
 * Argument(s): p_mb    specifies the Modbus channel data structure to wait on.
 *              perr    is a pointer to a variable that will receive an error code.
 *                      Possible errors are:
 *                      MODBUS_ERR_NONE        the call was successful and a packet was received
 *                      MODBUS_ERR_TIMED_OUT   a packet was not received within the specified timeout
 *                      MODBUS_ERR_NOT_MASTER  the channel is not a Master
 *                      MODBUS_ERR_INVALID     an invalid error was detected
 * Return(s):   none.
 *
 * Caller(s):   modbus_master_fcxx()  Modbus Master Functions
 * Return(s):   none.
 **************************************************************************************************/

void modbus_os_rx_wait(MODBUS_t *p_mb, uint16_t *perr)
{
#if (MODBUS_CFG_MASTER_EN == 1)

    if (p_mb != (MODBUS_t *)0)
    {
        if (p_mb->MasterSlave == MODBUS_MASTER)     // @@ MASTER
        {
            uint8_t rx_byte;
            int start_time = get_clock_ticks();

            p_mb->RxDoneFlag = 0;
            while (p_mb->RxDoneFlag == 0)           // Receive Done Flag
            {
                if (modbus_rx_1byte(p_mb, &rx_byte, 0) == 1)
                {
                    p_mb->RxCtr++;
                    modbus_rx_byte(p_mb, rx_byte);
                }
                else if (get_clock_ticks() - start_time < p_mb->RxTimeout) 
                {
                    osal_msleep(1);
                }
                else // 超时
                {
    #if (MODBUS_CFG_RTU_EN == 1)
                    if (p_mb->Mode == MODBUS_MODE_RTU)
                    {
                        /*
                         * 需要重新启动 Timer 吗?
                         */
                        void modbus_rtu_timer_restart(MODBUS_t *p_mb);
                        modbus_rtu_timer_restart(p_mb);

                        *perr = MODBUS_ERR_TIMED_OUT;
                        return;
                    }
                    else
    #endif
                    if (p_mb->Mode == MODBUS_MODE_ASCII)
                    {
                        break;
                    }
                }
            }

            *perr = MODBUS_ERR_NONE;
        }
        else
        {
            *perr = MODBUS_ERR_NOT_MASTER;
        }
    }
    else
    {
        *perr = MODBUS_ERR_NULLPTR;
    }
    
#else
    *perr = MODBUS_ERR_INVALID;
#endif
}

/**************************************************************************************************
 * function:    modbus_os_rx_task()
 * Description: This task is created by MB_OS_Init() and waits for signals from either the Rx ISR(s)
 *              or the RTU timeout timer(s) to indicate that a packet needs to be processed.
 * Argument(s): p_arg   is a pointer to an optional argument that is passed by uC/OS-II to the task.
 *                       This argument is not used.
 * Return(s):   none.
 *
 * Caller(s):   This is a Task.
 * Return(s):   none.
 **************************************************************************************************/

#if (MODBUS_CFG_SLAVE_EN == 1)
static void modbus_os_rx_task(void *p_arg)
{
    MODBUS_t *p_mb;

    (void)p_arg;

    while (1)
    {
        int i, rxcount = 0;

        p_mb = &mb_devices_tbl[0];
        for (i=0; i < MODBUS_CFG_CHNL_MAX; i++)
        {
            if ((p_mb->MasterSlave == MODBUS_SLAVE) && (NULL != p_mb->PtrUART))
            {
                uint8_t rx_byte;

                if (modbus_rx_1byte(p_mb, &rx_byte, 0) == 1)
                {
                    rxcount++;
                    p_mb->RxCtr++;
                    modbus_rx_byte(p_mb, rx_byte);
                }

    #if (MODBUS_CFG_RTU_EN == 1)
                if ((p_mb->Mode == MODBUS_MODE_RTU) && (p_mb->RxDoneFlag != 0))
                {
                    modbus_rx_task(p_mb);
                    p_mb->RxDoneFlag = 0;
                }
    #endif
            }

            p_mb++;
        }

        if (rxcount == 0)
        {
            osal_msleep(1);
        }
    }
}

#endif

//-----------------------------------------------------------------------------
/*
 * @@ END
 */

