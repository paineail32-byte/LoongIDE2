/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * drv_uart.c
 *
 * created: 2022/6/7
 *  author: Bian
 */

#include <rtthread.h>
#include <rthw.h>

#include "bsp.h"
#include "ls2k_uart.h"

#include "drv_uart.h"

//-------------------------------------------------------------------------------------------------
// RTThread device for LS2K
//-------------------------------------------------------------------------------------------------

/*
 * These functions glue UART device to RTThread.
 */
static rt_err_t rt_uart_init(struct rt_device *dev)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    if (ls2k_uart_init(dev->user_data, RT_NULL) != 0)
        return RT_ERROR;

    return RT_EOK;
}

static rt_err_t rt_uart_open(struct rt_device *dev, rt_uint16_t oflag)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    if (ls2k_uart_open(dev->user_data, RT_NULL) != 0)           // arg: struct termios *
        return RT_ERROR;

    dev->open_flag = (oflag | RT_DEVICE_FLAG_STREAM) & 0xff;    /* set open flags */

    return RT_EOK;
}

static rt_err_t rt_uart_close(struct rt_device *dev)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);
    
    ls2k_uart_close(dev->user_data, RT_NULL);
    
    return RT_EOK;
}

static rt_size_t rt_uart_read(struct rt_device *dev,
                              rt_off_t          pos,
                              void             *buffer,
                              rt_size_t         size)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);
    
    if (size == 0)
        return 0;

    /*
     * buffer is unsigned char *
     */
    return ls2k_uart_read(dev->user_data, buffer, (int)size, 0);
}

static rt_size_t rt_uart_write(struct rt_device *dev,
                               rt_off_t          pos,
                               const void       *buffer,
                               rt_size_t         size)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    if (size == 0)
        return 0;
        
    /*
     * buffer is unsigned char *
     */
    return ls2k_uart_write(dev->user_data, (void *)buffer, (int)size, 0);
}

static rt_err_t rt_uart_control(struct rt_device *dev,
                                int               cmd,
                                void             *args)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    ls2k_uart_ioctl(dev->user_data, (unsigned)cmd, args);
 
    return RT_EOK;
}

/******************************************************************************
 * uart devices
 */

#if BSP_USE_UART0
static struct rt_device rt_ls2k_uart0;
#endif
#if BSP_USE_UART1
static struct rt_device rt_ls2k_uart1;
#endif
#if BSP_USE_UART2
static struct rt_device rt_ls2k_uart2;
#endif
#if BSP_USE_UART3
static struct rt_device rt_ls2k_uart3;
#endif
#if BSP_USE_UART4
static struct rt_device rt_ls2k_uart4;
#endif
#if BSP_USE_UART5
static struct rt_device rt_ls2k_uart5;
#endif
#if BSP_USE_UART6
static struct rt_device rt_ls2k_uart6;
#endif
#if BSP_USE_UART7
static struct rt_device rt_ls2k_uart7;
#endif
#if BSP_USE_UART8
static struct rt_device rt_ls2k_uart8;
#endif
#if BSP_USE_UART9
static struct rt_device rt_ls2k_uart9;
#endif

/******************************************************************************
 * uart register
 */
static rt_err_t rt_ls2k_uart_register(struct rt_device *dev, const void *pUART, rt_uint32_t flag)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(pUART != RT_NULL);

    if (pUART == ConsolePort)
        return 0;
    
    dev->type        = RT_Device_Class_Char;
    dev->rx_indicate = RT_NULL;
    dev->tx_complete = RT_NULL;

    dev->init        = rt_uart_init;
    dev->open        = rt_uart_open;
    dev->close       = rt_uart_close;
    dev->read        = rt_uart_read;
    dev->write       = rt_uart_write;
    dev->control     = rt_uart_control;
    
    dev->user_data   = (void *)pUART;

    /* register a character device
     */
    return rt_device_register(dev, ls2k_uart_get_device_name(pUART), flag);
}

//-----------------------------------------------------------------------------
// Register Loongson uart devices
//-----------------------------------------------------------------------------

void rt_ls2k_uart_install(void)
{
#if BSP_USE_UART0
    rt_ls2k_uart_register(&rt_ls2k_uart0, devUART0, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_UART1
    rt_ls2k_uart_register(&rt_ls2k_uart1, devUART1, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_UART2
    rt_ls2k_uart_register(&rt_ls2k_uart2, devUART2, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_UART3
    rt_ls2k_uart_register(&rt_ls2k_uart3, devUART3, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_UART4
    rt_ls2k_uart_register(&rt_ls2k_uart4, devUART4, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_UART5
    rt_ls2k_uart_register(&rt_ls2k_uart5, devUART5, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_UART6
    rt_ls2k_uart_register(&rt_ls2k_uart6, devUART6, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_UART7
    rt_ls2k_uart_register(&rt_ls2k_uart7, devUART7, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_UART8
    rt_ls2k_uart_register(&rt_ls2k_uart8, devUART8, RT_DEVICE_FLAG_RDWR);
#endif
#if BSP_USE_UART9
    rt_ls2k_uart_register(&rt_ls2k_uart9, devUART9, RT_DEVICE_FLAG_RDWR);
#endif
}

/**
 * Console
 */
#if 0

void rt_ls2k_console_install(void)
{
#if BSP_USE_UART2
    rt_ls2k_uart_register(&rt_ls2k_uart2, devUART2, RT_DEVICE_FLAG_RDWR);
#endif
}

#endif


