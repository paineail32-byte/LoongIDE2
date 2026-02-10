/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bsp.h"

#if BSP_USE_CAN0

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "osal.h"

#include "ls2k_can.h"

extern int ioctl(int fd, unsigned cmd, ...);

//-----------------------------------------------------------------------------

#define CAN_STK_SIZE    4096

#define CAN_TASK_PRIO   17
#define CAN_TASK_SLICE  10

/*
 * 任务延迟时间
 */
#define TX_DELAY_MS     250

//-----------------------------------------------------------------------------

static int fd_can0 = -1;

static int tx_count = 0;

static void can0_do_transmit(void *arg, int *next_interval)
{
    int wr_cnt;
    char ch, info_buf[64];
    CANMsg_t msg;

    if (fd_can0 < 0)
    {
        return;
    }

    tx_count++;
    ch = tx_count % 256;

	/*
	 * 每隔 0.1秒发送一次数据.
	 */
	msg.id = 0xbeaf; // MSG_ID;
	msg.extended = 1;
	msg.rtr = (tx_count % 10) ? 0 : 1;     /* 10 整次数时, 发送一个远程帧 */

	msg.data[0] = 0x01 + ch;
	msg.data[1] = 0x02 + ch;
	msg.data[2] = 0x03 + ch;
	msg.data[3] = 0x04 + ch;

	msg.data[4] = 0x0A + ch;
	msg.data[5] = 0x0B + ch;
	msg.data[6] = 0x0C + ch;
	msg.data[7] = 0x0D + ch;

	msg.len = 8;

	/*
	 * Send Message
	 */
    wr_cnt = write(fd_can0, (const void *)&msg, sizeof(CANMsg_t));

    if (wr_cnt > 0)
    {
        snprintf(info_buf, 64, "%02x %02x %02x %02x %02x %02x %02x %02x",
		         msg.data[0], msg.data[1], msg.data[2], msg.data[3],
                 msg.data[4], msg.data[5], msg.data[6], msg.data[7]);

        printk("CAN0 TX: %s\r\n", info_buf);
    }
    else
    {
        printk("CAN0 TX: fail\r\n");
    }

}

/*
 * CAN 任务
 */

static osal_task_t can0_tx_task = NULL;

static void can0_transmit_task(void *arg)
{
    /*
     * Add CAN initialize code here.
     */

    printk("can0 transmit task started...\r\n");

    for ( ; ; )
    {
        /*
         * Add CAN task code here.
         */

        can0_do_transmit(NULL, NULL);

        /* abandon cpu time to run other task */
        osal_task_sleep(TX_DELAY_MS);
    }

    /*
     * if task delete...
     */
    close(fd_can0);
}

int can0_start_transmit(void)
{
    fd_can0 = open("/dev/can0", O_RDWR);
    
    if (fd_can0 < 0)
    {
        printf("open /dev/can0 fail.\r\n");
        return -1;
    }

    ioctl(fd_can0, IOCTL_CAN_SET_PROP_NS, (void *)100);     /* 传播延迟 */

#if CAN_USE_FD
    // ioctl(fd_can0, IOCTL_CAN_SET_BAUDRATE, (void *)CAN_SPEED_10M);
    ioctl(fd_can0, IOCTL_CAN_SET_BAUDRATE, (void *)CAN_SPEED_1M);
#else
    ioctl(fd_can0, IOCTL_CAN_SET_BAUDRATE, (void *)CAN_SPEED_500K);
#endif

    /**
     * 创建 CAN 任务
     */
    can0_tx_task = osal_task_create("can0tx",
                                    CAN_STK_SIZE,
                                    CAN_TASK_PRIO,
                                    CAN_TASK_SLICE,
                                    can0_transmit_task,
                                    NULL );

    if (can0_tx_task == NULL)
    {
        printf("create can0 transmit task fail!\r\n");
		return -1;
	}

    return 0;
}

#endif // #if BSP_USE_CAN0

//-----------------------------------------------------------------------------

/*
 * @END
 */
 

