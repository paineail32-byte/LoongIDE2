/*
 * uart_rx_test.c
 *
 * created: 2024-12-01
 * author:
 */

/*
    使用一个简单的通信协议来实现 UART 的发送和接收

    描述:   head        length      content     crc
    长度:   2 bytes     2 bytes     n bytes     2 bytes

    1. head:    总是 0xbeaf
    2. length:  content 的长度, 实际接收长度需要 length 加上 crc 的 2 字节
    3. content: 报文内容
    4. crc:     length+content 的 uint8_t 类型做加法运算结果, 保存到 unt16_t

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "bsp.h"
#include "ls2k_uart.h"

#include "osal.h"

extern int ioctl(int fd, unsigned cmd, ...);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/*
 * 接收间隔:
 *
 * 115200 8N1 时, 1ms 最多传输 12 字节
 *
 * 缓冲区最大长度 206 字节的传输耗时 18 ms
 */

#define RX_INTERVAL_MS      100 // 250 //

//-----------------------------------------------------------------------------

/*
 * 接收串口
 */
static int fd_RxU = -1;

//-----------------------------------------------------------------------------

unsigned short caclulate_crc(unsigned char *buf, int len)
{
    unsigned short crc = 0;

    for (int i=0; i<len; i++)
        crc += buf[i];

    return crc;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/*
 * 接收状态
 */
#define RX_STATE_HEAD1      0x01    /* 1st byte */
#define RX_STATE_HEAD2      0x02    /* 2nd byte */

#define RX_STATE_LEN1       0x04    /* 1st byte */
#define RX_STATE_LEN2       0x08    /* 2nd byte */

#define RX_STATE_LCONTENT   0x10    /* any byte until length */

#define RX_STATE_CRC1       0x20    /* 1st byte */
#define RX_STATE_CRC2       0x40    /* 2nd byte */

#define PKG_HEAD_LEN        2
#define PKG_LEN_LEN         2
#define PKG_DATA_MAX        64 // 200
#define PKG_CRC_LEN         2

#define RX_BUF_LEN          (PKG_DATA_MAX+6) // 206

//-----------------------------------------------------------------------------

/*
 * 接收超时:
 *
 * 115200 8N1 时, 1ms 传输 12 字节
 *
 * 缓冲区最大长度 206 字节的传输耗时 18 ms
 *
 */
static int rx_timeout = 10; // 25; //

/*
 * 一次接收数据的函数
 */

#define INC_RX(st)  do { p++; totalbytes++; state = st; } while (0)
#define RESET_RX    do { p = rx_buf; totalbytes = 0; state = RX_STATE_HEAD1; } while (0)

static void uart_do_receive_1_pkg(void *arg)
{
    unsigned char rx_buf[RX_BUF_LEN];
    int totalbytes = 0, thisbytes = 0, pkglen;
    int tmo, state = RX_STATE_HEAD1;
    unsigned short calc_crc, recv_crc = 0;
    unsigned char *p = rx_buf;

    tmo = rx_timeout;

    /*
     * 如果不超时, 循环
     */
    for (;;)
    {
        /*
         * 每次读一个字节
         */
        thisbytes = read(fd_RxU, p, 1);
        if ((thisbytes == 0) || (*p == 0xFF))
        {
            if (rx_timeout > 0)
            {
                tmo--;
                if (tmo <= 0)
                {
                    printf("uart rx timeout!\r\n");
                    return;
                }
            }

            osal_task_sleep(10);
            continue;
        }

        switch (state)
        {
            case RX_STATE_HEAD1:
                if (*p == 0xbe)
                    INC_RX(RX_STATE_HEAD2);
                break;

            case RX_STATE_HEAD2:
                if (*p == 0xaf)
                    INC_RX(RX_STATE_LEN1);
                else
                    RESET_RX;
                break;

            case RX_STATE_LEN1:
                INC_RX(RX_STATE_LEN2);
                break;

            case RX_STATE_LEN2:
                INC_RX(RX_STATE_LCONTENT);
                pkglen = rx_buf[2] | (rx_buf[3] << 8);
                if (pkglen > PKG_DATA_MAX)
                    RESET_RX;       /* 太长的不接收 */
                break;

            case RX_STATE_LCONTENT:
                INC_RX(RX_STATE_LCONTENT);
                if (totalbytes - 4 == pkglen)
                    state = RX_STATE_CRC1;
                break;

            case RX_STATE_CRC1:
                INC_RX(RX_STATE_CRC2);
                break;

            case RX_STATE_CRC2:
                INC_RX(RX_STATE_CRC2);
                recv_crc = rx_buf[totalbytes - 2] | (rx_buf[totalbytes - 1] << 8);
                break;

            default:
                break;
        }

        /*
         * 验证 crc
         */
        if ((recv_crc != 0) && (state == RX_STATE_CRC2))
        {
            calc_crc = caclulate_crc(rx_buf + 2, pkglen + 2);

            if (recv_crc == calc_crc)
            {
                rx_buf[pkglen + 4] = '\0';
                printf("RX[%i]: %s\r\n", pkglen, rx_buf + 4);
            }
            else
            {
                printf("rx crc error!\r\n");
            }

            break;
        }
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


/*
 * UART 接收任务
 */
static void uart_receive_task(void *arg)
{
    printk("uart receive task started...\r\n");

    for ( ; ; )
    {
        /*
         * Add UART task code here.
         */
        uart_do_receive_1_pkg(NULL);

        /* abandon cpu time to run other task */

        osal_task_sleep(RX_INTERVAL_MS);
    }

    /*
     * if task delete...
     */
    close(fd_RxU);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define UART_STK_SIZE       1024

#define UART_TASK_PRIO      16
#define UART_TASK_SLICE     10

static osal_task_t uart_rx_task = NULL;

/*
 * 启动 UART 接收
 */
void start_uart_rx(char *dev_uart_name)
{
    if (!dev_uart_name)
    {
        return;
    }

    /*
     * 默认 115200,8N1
     */
    fd_RxU = open(dev_uart_name, O_RDWR, NULL);
    if (fd_RxU < 0)
    {
        printf("open device %s fail!\r\n", dev_uart_name);
    }

    /* * 【关键修改】：将 UART_RX_INT 改为 UART_RX_POLL
     * 解决中断未配置导致的接收超时问题
     */
    ioctl(fd_RxU, IOCTL_UART_SET_RXTX_MODE, (void *)UART_RX_POLL);

    /*
     * 创建 UART 接收线程
     */
    uart_rx_task = osal_task_create("UARTrx",
                                     UART_STK_SIZE * sizeof(size_t),
                                     UART_TASK_PRIO,
                                     UART_TASK_SLICE,
                                     uart_receive_task,
                                     NULL);

    if (uart_rx_task == NULL)
    {
        close(fd_RxU);
        printf("create uart rx test task fail!\r\n");
    }
}

//-----------------------------------------------------------------------------

/*
 * @END
 */
