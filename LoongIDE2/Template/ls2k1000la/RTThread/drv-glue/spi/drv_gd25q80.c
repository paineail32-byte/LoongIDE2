/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * drv_gd25q80.c
 *
 * created: 2022/10/12
 *  author: Bian
 */

#include "bsp.h"

#if GD25Q80_DRV

#include <rtthread.h>
#include <rthw.h>

#include "ls2k_spi_bus.h"

#include "spi/gd25q80.h"

#include "drv_gd25q80.h"

//-------------------------------------------------------------------------------------------------
// RTThread device
//-------------------------------------------------------------------------------------------------

/*
 * These functions glue SPI0-GD25Q80 device to RTThread.
 */
static rt_err_t rt_gd25q80_init(struct rt_device *dev)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    if (ls2k_gd25q80_init(dev->user_data, RT_NULL) != 0)
        return RT_ERROR;

    return RT_EOK;
}

static rt_err_t rt_gd25q80_open(struct rt_device *dev, rt_uint16_t oflag)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    if (ls2k_gd25q80_open(dev->user_data, RT_NULL) != 0)
        return RT_ERROR;

    dev->open_flag = (oflag | RT_DEVICE_FLAG_STREAM) & 0xff;    /* set open flags */

    return RT_EOK;
}

static rt_err_t rt_gd25q80_close(struct rt_device *dev)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    ls2k_gd25q80_close(dev->user_data, RT_NULL);

    return RT_EOK;
}

static rt_size_t rt_gd25q80_read(struct rt_device *dev,
                                rt_off_t          pos,
                                void             *buffer,
                                rt_size_t         size)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    if (size == 0)
        return 0;

    /*
     * buffer is unsigned char *, pos is nor-flash inner linear address
     */
    return ls2k_gd25q80_read(dev->user_data, buffer, (int)size, (void *)pos);
}

static rt_size_t rt_gd25q80_write(struct rt_device *dev,
                                 rt_off_t          pos,
                                 const void       *buffer,
                                 rt_size_t         size)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    if (size == 0)
        return 0;
        
    /*
     * buffer is unsigned char *, pos is nor-flash inner linear address
     */
    return ls2k_gd25q80_write(dev->user_data, (void *)buffer, (int)size, (void *)pos);
}

static rt_err_t rt_gd25q80_control(struct rt_device *dev,
                                  int               cmd,
                                  void             *args)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    ls2k_gd25q80_ioctl(dev->user_data, (unsigned)cmd, args);

    return RT_EOK;
}

/******************************************************************************
 * SPI0-GD25Q80 devices
 */
static struct rt_device rt_ls2k_spi_gd25q80;

/******************************************************************************
 * SPI0-GD25Q80 register
 */
static rt_err_t rt_ls2k_gd25q80_register(struct rt_device *dev, const void *bus, rt_uint32_t flag)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(bus != RT_NULL);

    dev->type        = RT_Device_Class_SPIDevice;
    dev->rx_indicate = RT_NULL;
    dev->tx_complete = RT_NULL;

    dev->init        = rt_gd25q80_init;
    dev->open        = rt_gd25q80_open;
    dev->close       = rt_gd25q80_close;
    dev->read        = rt_gd25q80_read;
    dev->write       = rt_gd25q80_write;
    dev->control     = rt_gd25q80_control;

    dev->user_data   = (void *)bus;

    /* register a spi-device device
     */
    return rt_device_register(dev, GD25Q80_DEVICE_NAME, flag);
}

//-----------------------------------------------------------------------------
// Register SPI0-GD25Q80 devices
//-----------------------------------------------------------------------------

void rt_ls2k_gd25q80_install(void)
{
    rt_ls2k_gd25q80_register(&rt_ls2k_spi_gd25q80, busSPI0, RT_DEVICE_FLAG_RDWR);
}

#endif


