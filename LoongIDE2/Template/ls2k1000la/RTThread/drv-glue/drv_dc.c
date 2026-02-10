/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * drv_dc.c
 *
 * created: 2022/6/7
 *  author: Bian
 */

#include "bsp.h"

#if BSP_USE_DC

#include <rtthread.h>
#include <rthw.h>

#include "ls2k_dc.h"

#include "drv_dc.h"

//-------------------------------------------------------------------------------------------------
// RTThread device
//-------------------------------------------------------------------------------------------------

/*
 * These functions glue FrameBuffer device to RTThread.
 */
static rt_err_t rt_dc_init(struct rt_device *dev)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    if (ls2k_dc_init(dev->user_data, RT_NULL) != 0)
        return RT_ERROR;

    return RT_EOK;
}

static rt_err_t rt_dc_open(struct rt_device *dev, rt_uint16_t oflag)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    if (ls2k_dc_open(dev->user_data, RT_NULL) != 0)
        return RT_ERROR;

    dev->open_flag = (oflag | RT_DEVICE_FLAG_STREAM) & 0xff;    /* set open flags */

    return RT_EOK;
}

static rt_err_t rt_dc_close(struct rt_device *dev)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    ls2k_dc_close(dev->user_data, RT_NULL);

    return RT_EOK;
}

static rt_size_t rt_dc_read(struct rt_device *dev,
                            rt_off_t          pos,
                            void             *buffer,
                            rt_size_t         size)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    if (size == 0)
        return 0;

    /*
     * buffer is unsigned char *, pos is frame-buffer inner linear address
     */
    return ls2k_dc_read(dev->user_data, buffer, (int)size, (void *)pos);
}

static rt_size_t rt_dc_write(struct rt_device *dev,
                             rt_off_t          pos,
                             const void       *buffer,
                             rt_size_t         size)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    if (size == 0)
        return 0;
        
    /*
     * buffer is unsigned char *, pos is frame-buffer inner linear address
     */
    return ls2k_dc_write(dev->user_data, (void *)buffer, (int)size, (void *)pos);
}

static rt_err_t rt_dc_control(struct rt_device *dev,
                              int               cmd,
                              void             *args)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(dev->user_data != RT_NULL);

    ls2k_dc_ioctl(dev->user_data, (unsigned)cmd, args);

    return RT_EOK;
}

/******************************************************************************
 * DC devices
 */
static struct rt_device rt_ls2k_dc;

/******************************************************************************
 * DC register
 */
static rt_err_t rt_ls2k_dc_register(struct rt_device *dev, const void *pDC, rt_uint32_t flag)
{
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(pDC != RT_NULL);

    dev->type        = RT_Device_Class_Graphic;
    dev->rx_indicate = RT_NULL;
    dev->tx_complete = RT_NULL;

    dev->init        = rt_dc_init;
    dev->open        = rt_dc_open;
    dev->close       = rt_dc_close;
    dev->read        = rt_dc_read;
    dev->write       = rt_dc_write;
    dev->control     = rt_dc_control;

    dev->user_data   = (void *)pDC;

    /* register a graphic device
     */
    return rt_device_register(dev, FB_DEVICE_NAME, flag);
}

//-----------------------------------------------------------------------------
// Register DC device
//-----------------------------------------------------------------------------

void rt_ls2k_dc_install(void)
{
    rt_ls2k_dc_register(&rt_ls2k_dc, devDC0, RT_DEVICE_FLAG_RDWR);
}

#endif


