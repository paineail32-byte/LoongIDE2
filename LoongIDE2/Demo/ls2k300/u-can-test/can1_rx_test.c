/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bsp.h"

#if BSP_USE_CAN1

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "osal.h"

#include "ls2k_can.h"

extern int ioctl(int fd, unsigned cmd, ...);

//-----------------------------------------------------------------------------

#define CAN_STK_SIZE    4096

#define CAN_TASK_PRIO   18
#define CAN_TASK_SLICE  10

/*
 * 任务延迟时间
 */
#define RX_DELAY_MS     250

//-----------------------------------------------------------------------------

static int fd_can1 = -1;

static void can1_do_receive(void *arg, int *next_interval)
{
    int rd_cnt;
	CANMsg_t msg;
    char info_buf[64];
    
    if (fd_can1 < 0)
    {
        return;
    }

	/*
	 * 接收数据.
	 */
    rd_cnt = read(fd_can1, (void *)&msg, sizeof(CANMsg_t));

    if (0 == msg.rtr)
    {
	    if (rd_cnt > 0)
	    {
            snprintf(info_buf, 64, "%02x %02x %02x %02x %02x %02x %02x %02x",
		             msg.data[0], msg.data[1], msg.data[2], msg.data[3],
                     msg.data[4], msg.data[5], msg.data[6], msg.data[7]);

            printk("CAN1 RX: %s\r\n", info_buf);
        }
	}
	else
	{
	    /*
         * CANFD 忽略远程帧
         */
	    printk("CAN1 RX: rtr frame\r\n");
	}
}

/*
 * CAN 任务
 */
 
static osal_task_t can1_rx_task = NULL;

static void can1_receive_task(void *arg)
{
    /*
     * Add CAN initialize code here.
     */
    printk("can1 receive task started...\r\n");

    osal_task_sleep(10); // DELAY_MS(50);
    
    for ( ; ; )
    {
        /*
         * Add CAN task code here.
         */
        can1_do_receive(NULL, NULL);

        /* abandon cpu time to run other task */
        osal_task_sleep(RX_DELAY_MS);
    }

    /*
     * if task delete...
     */
    close(fd_can1);
}

int can1_start_receive(void)
{
    fd_can1 = open("/dev/can1", O_RDWR);

    if (fd_can1 < 0)
    {
        printf("open /dev/can1 fail.\r\n");
        return -1;
    }

    ioctl(fd_can1, IOCTL_CAN_SET_PROP_NS, (void *)100);     /* 传播延迟 */

#if CAN_USE_FD
    // ioctl(fd_can1, IOCTL_CAN_SET_BAUDRATE, (void *)CAN_SPEED_10M);
    ioctl(fd_can1, IOCTL_CAN_SET_BAUDRATE, (void *)CAN_SPEED_1M);
#else
    ioctl(fd_can1, IOCTL_CAN_SET_BAUDRATE, (void *)CAN_SPEED_500K);
#endif

#if CAN_USE_DMA
  #if CAN_USE_FD
    ioctl(fd_can1, IOCTL_CAN_SET_WORKMODE, (void *)(CAN_MODE_FD | CAN_MODE_RX_DMA));
  #else
    ioctl(fd_can1, IOCTL_CAN_SET_WORKMODE, (void *)CAN_MODE_RX_DMA);
  #endif
#endif

    can1_rx_task = osal_task_create("can1rx",
                                    CAN_STK_SIZE,
                                    CAN_TASK_PRIO,
                                    CAN_TASK_SLICE,
                                    can1_receive_task,
                                    NULL );

    if (can1_rx_task == NULL)
    {
        printk("create can1 rx task fail!\r\n");
		return -1;
	}

    return 0;
}

#endif // #if BSP_USE_CAN1

//-----------------------------------------------------------------------------

/*
 * @END
 */


