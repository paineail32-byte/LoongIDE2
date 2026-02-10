/*
 * uart_tx_test.c
 *
 * created: 2024-12-01
 *  author: 
 */

/*
    使用一个简单的通信协议来实现 UART 的发送和接收

    描述:   head        length      content     crc
    长度:   2 bytes     2 bytes     n bytes     2 bytes

    1. head:    总是 0xbeaf
    2. length:  content 的长度, 实际接收长度需要 length 加上 crc 的 2 字节
    3. content: 报文内容
    4. crc:     length+content 的 uint8_t 类型做加法运算结果, 保存到 uint16_t

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
 * 发送间隔:
 *
 *   115200 8N1 时, 1ms 最多传输 12 字节
 *
 *   缓冲区最大长度 206 字节的传输耗时 18 ms
 */

#define TX_INTERVAL_MS      250 // 100 //

//-----------------------------------------------------------------------------

/*
 * 接收串口
 */
static int fd_TxU = -1;

//-----------------------------------------------------------------------------

extern unsigned short caclulate_crc(unsigned char *buf, int len);

//-----------------------------------------------------------------------------

#define STR_DOG     "The quick brown fox jumps over a lazy dog on ground."
#define STR_DOGLEN  52

#define PKG_HEAD_LEN        2
#define PKG_LEN_LEN         2
#define PKG_DATA_MAX        STR_DOGLEN // 200
#define PKG_CRC_LEN         2

#define TX_BUF_LEN          (PKG_DATA_MAX+6)

/*
 * 发送数据的函数
 */
static void uart_do_send_1_pkg(void *arg)
{
    unsigned char tx_buf[TX_BUF_LEN];
    int totalbytes = 0, pkglen;
    unsigned short calc_crc;
    unsigned char *p = tx_buf;

    /*
     * 数据一次性发送
     */
    pkglen = rand();
    pkglen %= PKG_DATA_MAX;
    if (pkglen <= 0)
        pkglen = STR_DOGLEN;

    /*
     * head
     */
    *p++ = 0xbe;
    *p++ = 0xaf;
    
    /*
     * length
     */
    *p++ = pkglen & 0xFF;
    *p++ = (pkglen >> 8) & 0xFF;
    
    /*
     * content
     */
    strncpy((char *)p, STR_DOG, (size_t)pkglen);
    p += pkglen;
    
    /*
     * crc
     */
    calc_crc = caclulate_crc(tx_buf + 2, pkglen + 2);
    
    *p++ = calc_crc & 0xFF;
    *p++ = (calc_crc >> 8) & 0xFF;
    
    /*
     * send package
     */
    totalbytes = pkglen + 6;

    if (write(fd_TxU, tx_buf, totalbytes) == totalbytes)
    {
        printf("uart send %i bytes success.\r\n", totalbytes);
    }
    else
    {
        printf("uart send fail!\r\n");
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/*
 * UART 发送任务
 */
static void uart_send_task(void *arg)
{
    printk("uart send task started...\r\n");

    for ( ; ; )
    {
        /*
         * Add UART task code here.
         */
        uart_do_send_1_pkg(NULL);

        /* abandon cpu time to run other task */

        osal_task_sleep(TX_INTERVAL_MS);
    }

    /*
     * if task delete...
     */
    close(fd_TxU);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define UART_STK_SIZE      1024

#define UART_TASK_PRIO     16
#define UART_TASK_SLICE    10

static osal_task_t uart_tx_task = NULL;

/*
 * 启动 UART 发送
 */
void start_uart_tx(char *dev_uart_name)
{
    if (!dev_uart_name)
    {
        return;
    }

    /*
     * 默认 115200,8N1
     */
    fd_TxU = open(dev_uart_name, O_RDWR, NULL);
    if (fd_TxU < 0)
    {
        printf("open device %s fail!\r\n", dev_uart_name);
    }

    ioctl(fd_TxU, IOCTL_UART_SET_RXTX_MODE, (void *)UART_TX_DMA);
    
    /*
     * 创建 UART 发送线程
     */
    uart_tx_task = osal_task_create("UARTtx",
                                     UART_STK_SIZE * sizeof(size_t),
                                     UART_TASK_PRIO,
                                     UART_TASK_SLICE,
                                     uart_send_task,
                                     NULL);

    if (uart_tx_task == NULL)
    {
        close(fd_TxU);
        printf("create uart tx test task fail!\r\n");
	}
}

//-----------------------------------------------------------------------------

/*
 * @END
 */


