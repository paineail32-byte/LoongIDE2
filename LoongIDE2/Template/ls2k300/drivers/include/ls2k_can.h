/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_can.h
 *
 * created: 2024-08-09
 *  author: Bian
 */

#ifndef _LS2K_CAN_H
#define _LS2K_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * How to use:
 *
 * 1st Init:
 *
 *      ls2k_can_init(devCANx);
 *
 *      Then set the devCANx before open():
 *
 *      ls2k_can_ioctl(devCANx, IOCTL_CAN_SET_WORKMODE, mode);
 *      ls2k_can_ioctl(devCANx, IOCTL_CAN_SET_BAUDRATE, CAN_SPEED_500K);
 *      ls2k_can_ioctl(devCANx, IOCTL_CAN_SET_FILTER, CAN_filter_t*);
 *      ls2k_can_ioctl(devCANx, IOCTL_CAN_SET_RANGE, CAN_range_t*);
 *
 *
 * 2nd Open:
 *
 *      ls2k_can_open(devCANx, NULL);
 *
 *
 * 3rd Read:
 *
 *      int readed;
 *      CANMsg_t msg;
 *
 *      msg.id=x;
 *      ...
 *      readed = ls2k_can_read(devCANx, &msg, sizeof(msg), NULL);
 *      if ((readed == sizeof(msg)) && msg.len>0)
 *      {
 *          ...
 *      }
 *
 *
 * 4th Write:
 *
 *      int writted;
 *      CANMsg_t msg;
 *
 *      msg.id=x;
 *      ...
 *      writted = ls2k_can_write(devCANx, &msg, sizeof(msg), NULL);
 *      if (writted == sizeof(msg))
 *      {
 *          ...
 *      }
 *
 *
 * 5th Close:
 *
 *      ls2k_can_close(devCANx);
 *
 */

//*****************************************************************************

#define CAN_USE_FD      1

#define CAN_USE_DMA     1

//-----------------------------------------------------------------------------
// CAN Message
//
//  if can.mode.FDE = 1 then data[] length max is 64
//  else data[] length max is 8. (CAN2.0 format)
//
//-----------------------------------------------------------------------------

typedef struct
{
    unsigned int  id;               /* CAN message id */
    char          rtr;              /* RTR - CAN2.0 Remote Transmission Request */
    char          extended;         /* whether extended message package */
    unsigned char len;              /* length of data */
#if CAN_USE_FD
    unsigned char data[64];         /* data for transfer */
#else
    unsigned char data[8];
#endif
} CANMsg_t;

//*****************************************************************************
//-----------------------------------------------------------------------------
// CAN Mask Filter
//-----------------------------------------------------------------------------

#define CAN_FILTER_STD      0x01    /* Filter Stand RX message */
#define CAN_FILTER_EXT      0x02    /* Filter Extend RX message */

typedef struct
{
    unsigned int fltmask[3];
    unsigned int fltvalue[3];
    int filter[3];                  /* CAN_FILTER_STD | CAN_FILTER_EXT */
} CAN_filter_t;

//-----------------------------------------------------------------------------
// CAN Range Filter
//-----------------------------------------------------------------------------

#define CAN_RANGE_STD       0x01    /* Range Filter Stand RX message */
#define CAN_RANGE_EXT       0x02    /* Range Filter Extend RX message */

typedef struct
{
    unsigned int rangelo;
    unsigned int rangehi;
    int enable;                     /* CAN_RANGE_STD | CAN_RANGE_EXT */
} CAN_range_t;

//-----------------------------------------------------------------------------
// CAN STATUS
//-----------------------------------------------------------------------------

typedef struct
{
    /* tx/rx stats */
    int rx_msgs;
    int tx_msgs;

    /* Error Interrupt counters */
    int err_of;                     /* Overload frame */
    int err_bus;                    /* Bus error interrupt */
    int err_alost;                  /* Arbitration lost */
    int err_dover;                  /* Rx Data overflow */
    int err_ewl;                    /* Error Warning level */
    int err_fcs;                    /* error status change  */

    /* ALC error */
    int alc_base_id;                /* base identifier segment */
    int alc_srr_rtr;                /* base identifier 1st bit */
    int alc_ide;                    /* at IDE bit */
    int alc_ext;                    /* Identifier extension segment */
    int alc_rtr;                    /* RTR at extension mode */
    int alc_bit_pos[32];            /* lost position. only for BASE_ID or EXTENSION */

    /* Error capture */
    int err_bit;                    /* Bit error */
    int err_crc;                    /* CRC Error */
    int err_form;                   /* Form Error */
    int err_ack;                    /* Ack Error */
    int err_stuff;                  /* Stuff Error */
    int err_other;                  /* Other Error */
    int err_pos[10];                /* "detail" see below */

    /*
     * SUMMARY
     */
    int ints;                       /* total number of interrupts */
    int rxbuf_errors;
    int txbuf_errors;

    int rx_errors;                  /* from register ERC */
    int tx_errors;
    int std_rate_errors;            /* from register BRE */
    int fd_rate_errors;

} CAN_stats_t;

/*
 * indexes into CAN_stats.err_pos[index]
 */
#define CAN_ERR_POS_SOF			0			/* Error in Start of Frame; */
#define CAN_ERR_POS_ARB			1			/* Error in Arbitration Filed; */
#define CAN_ERR_POS_CTRL		2			/* Error in Control field; */
#define CAN_ERR_POS_DATA		3			/* Error in Data Field; */
#define CAN_ERR_POS_CRC			4			/* Error in CRC Field; */
#define CAN_ERR_POS_ACK			5			/* Error in CRC delimiter, ACK field or ACK delimiter */
#define CAN_ERR_POS_EOF			6			/* Error in End of frame field; */
#define CAN_ERR_POS_FRM			7			/* Error during Error frame; */
#define CAN_ERR_POS_OVRL		8			/* Error in Overload frame; */
#define CAN_ERR_POS_OTHER		9			/* Other position of error. */

//-----------------------------------------------------------------------------
// CAN work mode
//-----------------------------------------------------------------------------

#if CAN_USE_FD
#define CAN_MODE_FD             0x0010      /* CAN work as CAN-FD, else CAN2.0 */
#endif
#if CAN_USE_DMA
#define CAN_MODE_RX_DMA         0x0020      /* RX message with DMA */
#endif
#define CAN_MODE_RX_ADD_TS1     0x0040      /* RX message add timestamp @ RX beginning */
#define CAN_MODE_RX_CAN_TS      0x0080      /* RX message timestamp use can timer */
#define CAN_MODE_RX_NO_ACK      0x0100      /* RX message do not return ACK */
#define CAN_MODE_BUS_MONITOR    0x0200      /* Bus monitoring mode */
#define CAN_MODE_TX_TIMED       0x0400      /* Timed transmission mode */
#define CAN_MODE_SELF_TEST      0x0800      /* Self test mode */

#define CAN_MODE_IGNORE_RTR     0x1000      /* Ignore remote frame */
#define CAN_MODE_PROTOCOL_E     0x2000      /* Protocol exception handling: When the invisible bit is
                                             * read back in the r0 bit, it enters the join bus state */
#define CAN_MODE_NON_ISO        0x4000      /* NON-ISO CANFD enable */
#define CAN_MODE_LOOPBACK       0x8000      /* Inner loopback enable */
#define CAN_RE_TX_THRESH_MASK   0x000F      /* Retransmission threshold */

/*
 * General work mode
 */

//-----------------------------------------------------------------------------
// CAN Baudrate
//-----------------------------------------------------------------------------

#if CAN_USE_FD
/* CANFD */
#define CAN_SPEED_10M           10000000
#define CAN_SPEED_5M            5000000
#define CAN_SPEED_2M5           2500000
#define CAN_SPEED_4M            4000000
#define CAN_SPEED_2M            2000000
#endif

/* CAN 2.0 */
#define CAN_SPEED_1M            1000000
#define CAN_SPEED_500K          500000
#define CAN_SPEED_250K          250000
#define CAN_SPEED_125K          125000
#define CAN_SPEED_100K          100000
#define CAN_SPEED_75K           75000
#define CAN_SPEED_50K           50000
#define CAN_SPEED_25K           25000
#define CAN_SPEED_10K           10000

//-----------------------------------------------------------------------------
// CAN Status
//-----------------------------------------------------------------------------

#define CAN_STATUS_BUS_OFF          0x0001
#define CAN_STATUS_ERROR_PASSIVE    0x0002
#define CAN_STATUS_ERROR_ACTIVE     0x0004
#define CAN_STATUS_IDLE             0x0008
#define CAN_STATUS_RX_OVERFLOW      0x0010
#define CAN_STATUS_RXBUF_NOT_EMPTY  0x0020
#define CAN_STATUS_BUF_ERROR        0x0040

//-----------------------------------------------------------------------------
// Ioctl command
//-----------------------------------------------------------------------------

#define IOCTL_CAN_SET_WORKMODE  0x0001      /* unsigned int  - see "CAN work mode" above */
#define IOCTL_CAN_SET_BAUDRATE  0x0002      /* unsigned int  - see "CAN Baudrate" above  */
#define IOCTL_CAN_SET_FILTER    0x0004      /* CAN_filter_t* - see struct above */
#define IOCTL_CAN_SET_RANGE     0x0008      /* CAN_range_t*  - see struct above */

#define IOCTL_CAN_SET_SAMPLEPT	0x0010      /* int           - Set CAN sample-point */
#define IOCTL_CAN_SET_PROP_NS	0x0020      /* int           - Set CAN propagate delay ns */
#define IOCTL_CAN_SET_TS_PSC	0x0040      /* unsigned int  - Set CAN inner Timestamp prescale */

#define IOCTL_CAN_SET_RX_TMO    0x0100      /* unsigned int - ms, 0: NO_TIMEOUT=BLOCK MODE */
#define IOCTL_CAN_SET_TX_TMO    0x0200      /* unsigned int - ms, 0: NO_TIMEOUT=BLOCK MODE */

#define IOCTL_CAN_GET_CUR_TS    0x1000      /* unsigned int* - Get CAN current inner Timestamp */
#define IOCTL_CAN_GET_STATS     0x2000      /* CAN_stats_t** - see struct above */
#define IOCTL_CAN_GET_STATUS    0x4000      /* unsigned int* - see "CAN Status" above */
#define IOCTL_CAN_GET_BUFS      0x8000      /* int*          - Get CAN message cache buffer count */

//*****************************************************************************
//-----------------------------------------------------------------------------
// CAN devices
//-----------------------------------------------------------------------------

#if BSP_USE_CAN0
extern const void *devCAN0;
#endif
#if BSP_USE_CAN1
extern const void *devCAN1;
#endif
#if BSP_USE_CAN2
extern const void *devCAN2;
#endif
#if BSP_USE_CAN3
extern const void *devCAN3;
#endif

//-----------------------------------------------------------------------------
// CAN driver operators
//-----------------------------------------------------------------------------

#include "ls2k_drv_io.h"

#if (PACK_DRV_OPS)

extern const driver_ops_t *can_drv_ops;

#define ls2k_can_init(can, arg)             can_drv_ops->init_entry(can, arg)
#define ls2k_can_open(can, arg)             can_drv_ops->open_entry(can, arg)
#define ls2k_can_close(can, arg)            can_drv_ops->close_entry(can, arg)
#define ls2k_can_read(can, buf, size, arg)  can_drv_ops->read_entry(can, buf, size, arg)
#define ls2k_can_write(can, buf, size, arg) can_drv_ops->write_entry(can, buf, size, arg)
#define ls2k_can_ioctl(can, cmd, arg)       can_drv_ops->ioctl_entry(can, cmd, arg)

#else

/*
 * 初始化CAN设备
 * 参数:    dev     devCAN0~devCAN3
 *          arg     NULL
 *
 * 返回:    0=成功
 *
 * 默认值:  内核模式: CAN 2.0
 *          通信速率: CAN_SPEED_500K
 *          RX 不使用 DMA
 *
 */
int CAN_initialize(const void *dev, void *arg);

/*
 * 打开CAN设备
 * 参数:    dev     devCAN0~devCAN3
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int CAN_open(const void *dev, void *arg);

/*
 * 关闭CAN设备
 * 参数:    dev     devCAN0~devCAN3
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int CAN_close(const void *dev, void *arg);

/*
 * 从CAN设备读数据(接收)
 * 参数:    dev     devCAN0~devCAN3
 *          buf     类型: CANMsg_t *, 数组, 用于存放读取数据的缓冲区
 *          size    类型: int, 待读取的字节数, 长度不能超过 buf 的容量, sizeof(CANMsg_t)倍数
 *          arg     NULL
 *
 * 返回:    读取的字节数
 *
 * 说明:    CAN使用中断接收, 接收到的数据存放在驱动内部缓冲区, 读操作总是从缓冲区读取.
 *          必须注意接收数据缓冲区溢出.
 */
int CAN_read(const void *dev, void *buf, int size, void *arg);

/*
 * 向CAN设备写数据(发送)
 * 参数:    dev     devCAN0~devCAN3
 *          buf     类型: CANMsg_t *, 数组, 用于存放待写数据的缓冲区
 *          size    类型: int, 待写入的字节数, 长度不能超过 buf 的容量, sizeof(CANMsg_t)倍数
 *          arg     NULL
 *
 * 返回:    写入的字节数
 *
 * 说明:    CAN使用中断发送, 待发送的数据直接发送, 或者存放在驱动内部缓冲区待中断发生时继续发送.
 *          必须注意发送数据缓冲区溢出.
 */
int CAN_write(const void *dev, void *buf, int size, void *arg);

/*
 * 向CAN设备发送控制命令
 * 参数:    dev     devCAN0~devCAN3
 *
 *      ---------------------------------------------------------------------------------
 *          cmd                         |   arg
 *      ---------------------------------------------------------------------------------
 *
 * 返回:    0=成功
 */
int CAN_ioctl(const void *dev, int cmd, void *arg);

#define ls2k_can_init(can, arg)             CAN_initialize(can, arg)
#define ls2k_can_open(can, arg)             CAN_open(can, arg)
#define ls2k_can_close(can, arg)            CAN_close(can, arg)
#define ls2k_can_read(can, buf, size, arg)  CAN_read(can, buf, size, arg)
#define ls2k_can_write(can, buf, size, arg) CAN_write(can, buf, size, arg)
#define ls2k_can_ioctl(can, cmd, arg)       CAN_ioctl(can, cmd, arg)

#endif

//-----------------------------------------------------------------------------
// CAN device name
//-----------------------------------------------------------------------------

const char *ls2k_can_get_device_name(const void *pCAN);

#ifdef __cplusplus
}
#endif

#endif // _LS2K_CAN_H

