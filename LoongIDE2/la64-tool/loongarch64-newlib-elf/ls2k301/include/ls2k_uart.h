/*
 * Copyright (C) 2021-2025 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

#ifndef _LS2K_UART_H
#define _LS2K_UART_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------

#define CFLAG_TO_BAUDRATE(flag)      \
        ((flag == B1200)   ? 1200 :  \
         (flag == B2400)   ? 2400 :  \
         (flag == B4800)   ? 4800 :  \
         (flag == B9600)   ? 9600 :  \
         (flag == B19200)  ? 19200 : \
         (flag == B38400)  ? 38400 : \
         (flag == B57600)  ? 57600 : \
         (flag == B115200) ? 115200 : 9600)

#define BAUDRATE_TO_CFLAG(baud)      \
        ((baud == 1200)   ? B1200 :  \
         (baud == 2400)   ? B2400 :  \
         (baud == 4800)   ? B4800 :  \
         (baud == 9600)   ? B9600 :  \
         (baud == 19200)  ? B19200 : \
         (baud == 38400)  ? B38400 : \
         (baud == 57600)  ? B57600 : \
         (baud == 115200) ? B115200 : B9600)

//-----------------------------------------------------------------------------
// UART devices
//-----------------------------------------------------------------------------

#if (BSP_USE_UART0)
extern const void *devUART0;
#endif
#if (BSP_USE_UART1)
extern const void *devUART1;
#endif
#if (BSP_USE_UART2)
extern const void *devUART2;
#endif
#if (BSP_USE_UART3)
extern const void *devUART3;
#endif
#if (BSP_USE_UART4)
extern const void *devUART4;
#endif
#if (BSP_USE_UART5)
extern const void *devUART5;
#endif
#if (BSP_USE_UART6)
extern const void *devUART6;
#endif
#if (BSP_USE_UART7)
extern const void *devUART7;
#endif
#if (BSP_USE_UART8)
extern const void *devUART8;
#endif
#if (BSP_USE_UART9)
extern const void *devUART9;
#endif

//-----------------------------------------------------------------------------
// UART io control command                      param type
//-----------------------------------------------------------------------------

#define IOCTL_NS16550_SET_MODE      0x1000      // struct termios *

#define IOCTL_UART_SET_TERMIOS      0x1000      // xxx Same as IOCTL_NS16550_SET_MODE

/*
 * LS2K300/LS2K301 新特性
 */
#define IOCTL_UART_SET_RXTX_MODE    0x1001      // int: UART work mode, see below

#define IOCTL_UART_GET_RXTX_MODE    0x1002      // return: UART work mode, see below

#define IOCTL_UART_PRINT_RXTX_MODE  0x1003

/*
 * 数据收发方式: DMA or INT else POLL
 *
 * 例如: ConsolePort 设置为: UART_RX_POLL | UART_TX_DMA
 *       devUART1    设置为: UART_RX_INT | UART_TX_DMA
 *
 */
#define UART_RX_DMA         0x01        /* receive use DMA */
#define UART_TX_DMA         0x02        /* transfer use DMA */

#define UART_RX_INT         0x04        /* receive use INT */
#define UART_TX_INT         0x08        /* transfer use INT */

#define UART_RX_POLL        0x10        /* receive use POLL */
#define UART_TX_POLL        0x20        /* transfer use POLL */

#define UART_WORK_DMA       (UART_RX_DMA  | UART_TX_DMA)
#define UART_WORK_INT       (UART_RX_INT  | UART_TX_INT)
#define UART_WORK_POLL      (UART_RX_POLL | UART_TX_POLL)

//-----------------------------------------------------------------------------
// UART function
//-----------------------------------------------------------------------------

#include "ls2k_drv_io.h"

#if (PACK_DRV_OPS)

extern const driver_ops_t *uart_drv_ops;

#define ls2k_uart_init(uart, arg)             uart_drv_ops->init_entry(uart, arg)
#define ls2k_uart_open(uart, arg)             uart_drv_ops->open_entry(uart, arg)
#define ls2k_uart_close(uart, arg)            uart_drv_ops->close_entry(uart, arg)
#define ls2k_uart_read(uart, buf, size, arg)  uart_drv_ops->read_entry(uart, buf, size, arg)
#define ls2k_uart_write(uart, buf, size, arg) uart_drv_ops->write_entry(uart, buf, size, arg)
#define ls2k_uart_ioctl(uart, cmd, arg)       uart_drv_ops->ioctl_entry(uart, cmd, arg)

#else

/*
 * 初始化串口
 * 参数:    dev     见上面定义的 UART 设备
 *          arg     类型 unsigned int, 串口波特率. 当该参数为 0 或 NULL时, 串口设置为默认模式 115200,8N1
 *
 * 返回:    0=成功
 */
int UART_initialize(const void *dev, void *arg);

/*
 * 打开串口. 如果串口配置为中断模式, 安装中断向量
 * 参数:    dev     见上面定义的 UART 设备
 *          arg     类型 struct termios *, 把串口配置为指定参数模式. 该参数可为 0 或 NULL.
 *
 * 返回:    0=成功
 */
int UART_open(const void *dev, void *arg);

/*
 * 关闭串口. 如果串口配置为中断模式, 移除中断向量
 * 参数:    dev     见上面定义的 UART 设备
 *          arg     总是 0 或 NULL.
 *
 * 返回:    0=成功
 */
int UART_close(const void *dev, void *arg);

/*
 * 从串口读数据(接收)
 * 参数:    dev     见上面定义的 UART 设备
 *          buf     类型 char *, 用于存放读取数据的缓冲区
 *          size    类型 int, 待读取的字节数, 长度不能超过 buf 的容量
 *          arg     类型 int.
 *                  如果串口工作在中断模式:
 *                    >0:   该值用作读操作的超时等待毫秒数
 *                    =0:   读操作立即返回
 *                  如果串口工作在查询模式:
 *                    !=0:  读操作工作在阻塞模式, 直到读取size个字节才返回
 *                    =0:   读操作立即返回
 *
 * 返回:    读取的字节数
 *
 * 说明:    串口工作在中断模式: 读操作总是读的内部数据接收缓冲区
 *          串口工作在查询模式: 读操作直接对串口设备进行读
 */
int UART_read(const void *dev, void *buf, int size, void *arg);

/*
 * 向串口写数据(发送)
 * 参数:    dev     见上面定义的 UART 设备
 *          buf     类型 char *, 用于存放待发送数据的缓冲区
 *          size    类型 int, 待发送的字节数, 长度不超过 buf 的容量
 *          arg     总是 0 或 NULL
 *
 * 返回:    发送的字节数
 *
 * 说明:    串口工作在中断模式: 写操作总是写的内部数据发送缓冲区
 *          串口工作在查询模式: 写操作直接对串口设备进行写
 */
int UART_write(const void *dev, void *buf, int size, void *arg);

/*
 * 向串口发送控制命令
 * 参数:    dev         见上面定义的 UART 设备
 *          cmd         IOCTL_UART_SET_MODE
 *          arg         类型 struct termios *, 把串口配置为指定参数模式.
 *
 * 返回:    0=成功
 */
int UART_ioctl(const void *dev, int cmd, void *arg);

#define ls2k_uart_init(uart, arg)             UART_initialize(uart, arg)
#define ls2k_uart_open(uart, arg)             UART_open(uart, arg)
#define ls2k_uart_close(uart, arg)            UART_close(uart, arg)
#define ls2k_uart_read(uart, buf, size, arg)  UART_read(uart, buf, size, arg)
#define ls2k_uart_write(uart, buf, size, arg) UART_write(uart, buf, size, arg)
#define ls2k_uart_ioctl(uart, cmd, arg)       UART_ioctl(uart, cmd, arg)

#endif

//-----------------------------------------------------------------------------
// UART device name
//-----------------------------------------------------------------------------

const char *ls2k_uart_get_device_name(const void *pUART);

//-----------------------------------------------------------------------------
// UART as Console
//-----------------------------------------------------------------------------

extern void *ConsolePort;

char Console_get_char(void *pUART);
void Console_output_char(void *pUART, char ch);

#ifdef __cplusplus
}
#endif

#endif // _LS2K_UART_H

