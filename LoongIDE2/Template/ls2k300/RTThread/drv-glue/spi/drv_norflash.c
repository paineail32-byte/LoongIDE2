/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * drv_norflash.c
 *
 * created: 2022/6/7
 *  author: Bian
 */

#include "bsp.h"

#if NORFLASH_DRV

#include <rtthread.h>
#include <rthw.h>

#include "ls2k_spi_bus.h"

#include "spi/norflash.h"

#include "drv_norflash.h"

//-------------------------------------------------------------------------------------------------
// RTThread device for Loongson 2K300
//-------------------------------------------------------------------------------------------------

/*
 * These functions glue SPI0-NORFlash device to RTThread.
 */
static rt_err_t rt_norflash_init(struct rt_device *dev)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    if (ls2k_norflash_init(dev->user_data, RT_NULL) != 0)
        return RT_ERROR;

    return RT_EOK;
}

static rt_err_t rt_norflash_open(struct rt_device *dev, rt_uint16_t oflag)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    if (ls2k_norflash_open(dev->user_data, RT_NULL) != 0)
        return RT_ERROR;

    dev->open_flag = (oflag | RT_DEVICE_FLAG_STREAM) & 0xff;    /* set open flags */

    return RT_EOK;
}

static rt_err_t rt_norflash_close(struct rt_device *dev)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    ls2k_norflash_close(dev->user_data, RT_NULL);

    return RT_EOK;
}

static rt_size_t rt_norflash_read(struct rt_device *dev,
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
    return ls2k_norflash_read(dev->user_data, buffer, (int)size, (void *)pos);
}

static rt_size_t rt_norflash_write(struct rt_device *dev,
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
    return ls2k_norflash_write(dev->user_data, (void *)buffer, (int)size, (void *)pos);
}

static rt_err_t rt_norflash_control(struct rt_device *dev,
                                    int               cmd,
                                    void             *args)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    ls2k_norflash_ioctl(dev->user_data, (unsigned)cmd, args);

    return RT_EOK;
}

/******************************************************************************
 * SPI0-NorFlash devices
 */
static struct rt_device rt_ls2k_spi0_norflash;

/******************************************************************************
 * SPI0-NorFlash register
 */
static rt_err_t rt_ls2k_norflash_register(struct rt_device *dev, const void *pSPI, rt_uint32_t flag)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(pSPI != RT_NULL);

    dev->type        = RT_Device_Class_SPIDevice;
    dev->rx_indicate = RT_NULL;
    dev->tx_complete = RT_NULL;

    dev->init        = rt_norflash_init;
    dev->open        = rt_norflash_open;
    dev->close       = rt_norflash_close;
    dev->read        = rt_norflash_read;
    dev->write       = rt_norflash_write;
    dev->control     = rt_norflash_control;

    dev->user_data   = (void *)pSPI;

    /* register a spi-device device
     */
    return rt_device_register(dev, NORFLASH_DEVICE_NAME, flag);
}

//-----------------------------------------------------------------------------
// Register Loongson SPI0-NorFlash devices
//-----------------------------------------------------------------------------

void rt_ls2k_norflash_install(void)
{
    rt_ls2k_norflash_register(&rt_ls2k_spi0_norflash, busSPI0, RT_DEVICE_FLAG_RDWR);
}

#endif


