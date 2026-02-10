/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * drv_is24c16.c
 *
 * created: 2022-10-12
 *  author: Bian
 */

#include "bsp.h"

#if IS24C16_DRV

#include <rtthread.h>
#include <rthw.h>

#include "ls2k_i2c_bus.h"

#include "i2c/is24c16.h"

#include "drv_is24c16.h"

//-------------------------------------------------------------------------------------------------
// RTThread device
//-------------------------------------------------------------------------------------------------

/*
 * These functions glue I2C0-IS24C16 device to RTThread.
 */

static rt_size_t rt_is24c16_read(struct rt_device *dev,
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
    return ls2k_is24c16_read(dev->user_data, buffer, (int)size, (void *)pos);
}

static rt_size_t rt_is24c16_write(struct rt_device *dev,
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
    return ls2k_is24c16_write(dev->user_data, (void *)buffer, (int)size, (void *)pos);
}

/******************************************************************************
 * I2C0-IS24C16 devices
 */
static struct rt_device rt_ls2k_i2c0_is24c16;

/******************************************************************************
 * I2C0-IS24C16 register
 */
static rt_err_t rt_ls2k_is24c16_register(struct rt_device *dev, const void *bus, rt_uint32_t flag)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(bus != RT_NULL);

    dev->type        = RT_Device_Class_I2CBUS;
    dev->rx_indicate = RT_NULL;
    dev->tx_complete = RT_NULL;

    dev->init        = RT_NULL;
    dev->open        = RT_NULL;
    dev->close       = RT_NULL;
    dev->read        = rt_is24c16_read;
    dev->write       = rt_is24c16_write;
    dev->control     = RT_NULL;

    dev->user_data   = (void *)bus;

    /* register a i2c-device device
     */
    return rt_device_register(dev, IS24C16_DEVICE_NAME, flag);
}

//-----------------------------------------------------------------------------
// Register I2C0-IS24C16 devices
//-----------------------------------------------------------------------------

void rt_ls2k_is24c16_install(void)
{
    rt_ls2k_is24c16_register(&rt_ls2k_i2c0_is24c16, busI2C0, RT_DEVICE_FLAG_RDWR);
}

#endif


