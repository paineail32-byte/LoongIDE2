/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_can_hw.h
 *
 * created: 2024-06-11
 *  author: Bian
 */

#ifndef _LS2K_CAN_HW_H
#define _LS2K_CAN_HW_H

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
// CAN 设备
//-------------------------------------------------------------------------------------------------

#define CAN0_BASE		0x16110000
#define CAN1_BASE		0x16110400
#define CAN2_BASE		0x16110800
#define CAN3_BASE		0x16110C00

/*
 * CAN 控制器
 */
typedef struct
{
	volatile unsigned int id;				/* 0x00 CR_ID CANFD 控制器ID == 0x2babe */
	volatile unsigned int mode;				/* 0x04 CR_MODE 模式配置寄存器 */
	volatile unsigned int set;				/* 0x08 CR_SET 设置配置寄存器 */
	volatile unsigned int status;			/* 0x0c CR_STAT 状态寄存器 */
	volatile unsigned int cmd;				/* 0x10 CR_CMD 命令寄存器 */
	volatile unsigned int isr;				/* 0x14 INT_STAT 中断状态寄存器 */
	volatile unsigned int ien;				/* 0x18 INT_ENA 中断使能寄存器 */
	volatile unsigned int imask;			/* 0x1c INT_MASK 中断状态屏蔽寄存器 */
	volatile unsigned int btrnormal;		/* 0x20 BTR_NORM 标准速率配置寄存器 */
	volatile unsigned int btrfd;			/* 0x24 BTR_FD FD 数据速率配置寄存器 */
	volatile unsigned int errl;				/* 0x28 ERL 错误阈值配置寄存器 */
	volatile unsigned int errsr;			/* 0x2c FSTAT 错误状态寄存器 */
	volatile unsigned int errcnt;			/* 0x30 ERC 错误计数寄存器 */
	volatile unsigned int brerr;			/* 0x34 BRE 速率错误计数寄存器 */
	volatile unsigned int ctrpres;			/* 0x38 CTR_PRES 错误计数调试寄存器 */
	volatile unsigned int errcapt;			/* 0x3c ERR_CAPT 错误捕捉状态寄存器 */
	volatile unsigned int retxcnt;			/* 0x40 RETX_CNT 重发计数寄存器 */
	volatile unsigned int alc;				/* 0x44 ALC 失去仲裁捕捉寄存器 */
	volatile unsigned int trvdly;			/* 0x48 TRV_DLY 传输延迟测量寄存器 */
	volatile unsigned int sspcfg;			/* 0x4c SSP_CFG 第二采样点配置寄存器 */
	volatile unsigned int rxfrcnt;			/* 0x50 RX_FR_CNT 接收报文计数寄存器 */
	volatile unsigned int txfrcnt;			/* 0x54 TX_FR_CNT 发送报文计数寄存器 */
	volatile unsigned int debug;			/* 0x58 DEBUG 调试寄存器 */
	volatile unsigned int ts;				/* 0x5c TS 时间戳寄存器 */
	volatile unsigned int txfrmtst;			/* 0x60 TX_FRM_TST 发送报文调试寄存器 */
	volatile unsigned int frcdiv;			/* 0x64 FRC_DIV 小数分频系数寄存器 */
	volatile unsigned int fltmaskA;			/* 0x68 FLT_A_MASK 过滤器A 掩码寄存器 */
	volatile unsigned int fltvalA;			/* 0x6c FLT_A_VAL 过滤器A 数值寄存器 */
	volatile unsigned int fltmaskB;			/* 0x70 FLT_B_MASK 过滤器B 掩码寄存器 */
	volatile unsigned int fltvalB;			/* 0x74 FLT_B_VAL 过滤器B 数值寄存器 */
	volatile unsigned int fltmaskC;			/* 0x78 FLT_C_MASK 过滤器C 掩码寄存器 */
	volatile unsigned int fltvalC;			/* 0x7c FLT_C_VAL 过滤器C 数值寄存器 */
	volatile unsigned int fltrlo;			/* 0x80 FLT_R_LOW 范围过滤器高阈值寄存器 */
	volatile unsigned int fltrhi;			/* 0x84 FLT_R_HI 范围过滤器低阈值寄存器 */
	volatile unsigned int fltctrl;			/* 0x88 FLT_CTRL 过滤器控制寄存器 */
	volatile unsigned int rxmeminfo;		/* 0x8c RX_MEM_INFO 接收缓冲区信息寄存器 */
	volatile unsigned int rxprt;			/* 0x90 RX_PRT 接收缓冲区指针寄存器 */
	volatile unsigned int rxsr;				/* 0x94 RX_STAT 接收缓冲区状态寄存器 */
	volatile unsigned int rxdata;			/* 0x98 RX_DATA 接收数据寄存器 */
	volatile unsigned int txsr;				/* 0x9c TX_STAT 发送缓冲区状态寄存器 */
	volatile unsigned int txcmd;			/* 0xa0 TX_CMD 发送命令控制寄存器 */
	volatile unsigned int txsel;			/* 0xa4 TX_SEL 发送缓冲区选择寄存器 */
	volatile unsigned int rsv[2];
	/*
     * 0xb0~0xf4 发送数据缓冲区
     */
    volatile unsigned int head0;            /* 0xb0 T0 */
    volatile unsigned int head1;            /* 0xb4 T1 */
	volatile unsigned int txdata[16];       /* 0xb8~0xf4 16*4 共64字节数据 */

} HW_CAN_t;

//-----------------------------------------------------------------------------

/*
 * Message T0
 */
typedef struct
{
    union
    {
        struct
        {
	       unsigned int id  :29;	/* bit[0:28] 报文ID, XTD 为1 时[28:0]位为有效位; XTD 为0 时[28:18]位为有效位. */
	       unsigned int rtr :1;		/* bit[29] Remote Transmission Request 位, 仅在CAN2.0 帧有效. */
	       unsigned int xtd :1;		/* bit[30] Extended Identifier Type 位. */
	       unsigned int esi :1;		/* bit[31] Error State Indicator位, 接收报文中表示相应发送节点的错误状态, 仅在CANFD 帧有效.*/
        };

        unsigned int value;
    };
} MSGT0_t;

/*
 * Message T1, 4 bytes
 */

typedef struct
{
    union
    {
        struct
        {
	       unsigned int timestamp :16;  /* bit[0:15] 时间戳. 在接收报文中表示接收时刻的时间戳; 
                                         *           在发送报文中表示定时发送的时间, 仅在TTTM 为1 时有效. */
	       unsigned int dlc :4;		    /* bit[16:19] Data Length Code. 数据长度. */
	       unsigned int brs :1;		    /* bit[20] Bit Rate Shift 位, 仅在CANFD 帧有效.
                                         *         为1 时报文切换速率; 为0 时报文不切换速率. */
	       unsigned int fdf :1;		    /* bit[21] Flexible Data-rate Format 位.
                                         *         为1 时报文为CANFD 报文; 为0 时报文为CAN2.0报文. */
	       unsigned int res :2;
	       unsigned int rxwords :5;		/* bit[24:28] Read Word Counter, 接收报文字数(32 位字). XXX 仅接收报文 */
        };

        unsigned int value;
    };
} MSGT1_t;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/**
 * CANFD 控制器ID(CR_ID) OFFSET: 0x00
 */
#define CAN_VER_MASK			(0xFFFF<<16)	/* RO bit[31:16] 版本号. 第二版. */
#define CAN_ID_MASK				0xFFFF			/* RW bit[15:0] 设备ID. 默认ID 为0xbabe, 仅可在CR_SET[ENA]=0 时进行改写. */

/**
 * 模式配置寄存器(CR_MODE) OFFSET: 0x04
 */
#define CAN_MODE_BUFM			bit(13)			/* RW 发送缓冲区模式. 0: 使用外部缓冲区; 1: 使用控制器内部发送缓冲区. */
#define CAN_MODE_RTSOP			bit(12)			/* RW 接收缓冲区时间戳添加模式. 0: 时间戳添加在报文接收结束时;
                                                 *    1: 时间戳添加在报文接收开始时.
                                                 */
#define CAN_MODE_ITSM			bit(10)			/* RW 内部时间戳模式. 0: 使用系统时间戳; 1: 使用控制器内部生成的时间戳. */
#define CAN_MODE_RXBAM			bit(9)			/* RW 接收缓冲区自动模式. 0: 在读取接收缓冲区的数据后读指针不移动;
                                                 *    1: 在读取接收缓冲区的数据后读指针自动加1.
                                                 */
#define CAN_MODE_TSTM			bit(8)			/* RW 测试模式. 在此模式下, 可对控制器进行调试. */
#define CAN_MODE_ACF			bit(7)			/* RW 无ACK 模式. 在此模式下, 控制器不返回ACK, 即使校验通过. */
#define CAN_MODE_ROM			bit(6)			/* RW 限制操作模式. Restricted operation mode. 0: 关闭; 1: 打开. */
#define CAN_MODE_TTTM			bit(5)			/* RW 定时发送模式. 此模式使能时, 0 发送缓冲区中的报文将在指定的时间发送.
                                                 *    0: 关闭; 1: 打开.
                                                 */
#define CAN_MODE_FDE		    bit(4)			/* RW CANFD 使能. 0: 不支持CANFD 报文格式; 1: 支持CANFD 报文格式. */
#define CAN_MODE_AFM			bit(3)			/* RW 接收过滤模式. 此模式使能时, 仅将过滤后的报文存入缓冲区.
                                                 *    0: 关闭接收过滤器; 1: 使能接收过滤器.
                                                 */
#define CAN_MODE_STM			bit(2)			/* RW 自测试模式. 此模式使能时, 无论是否回读到ACK 位, 均认为发送成功.
                                                 * 0: 关闭; 1: 打开.
                                                 */
#define CAN_MODE_BMM			bit(1)			/* RW 总线监听模式. Bus monitoring mode, 此模式下, 控制器仅向总线发送隐形位.
                                                 * 0: 关闭; 1: 打开.
                                                 */
#define CAN_MODE_RST			bit(0)			/* RW 软复位. 写1 打开复位; 写0 关闭复位. */

/**
 * 设置配置寄存器(CR_SET) OFFSET: 0x08
 */
#define CAN_SET_FDRF			bit(10)			/* RW 忽略远程帧使能. 使能时, 接收过滤器忽略远程帧. */
#define CAN_SET_PEX				bit(8)			/* RW 协议例外处理. 仅在CR_SET[ENA]关闭状态下可改写.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 当在r0 位回读到隐形位时, 插入错误帧; 
                                                 *    1: 当在r0 位回读到隐形位时, 进入加入总线状态. */
#define CAN_SET_NISOFD			bit(7)			/* RW NON-ISO CANFD 协议使能. 仅在CR_SET[ENA]关闭状态下可改写.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 遵守ISO CANFD 协议; 1: 遵守NON-ISO CANFD 协议. */
#define CAN_SET_ENABLE			bit(6)			/* RW 控制器使能. 0: 关闭; 1: 打开. */
#define CAN_SET_ILBP			bit(5)			/* RW 内部回绕模式. 在控制器发送报文时, 将发送的报文存入同一控制器的接收缓冲区.
                                                 *    0: 关闭; 1: 打开. */
#define CAN_SET_RTXTH_MASK		0x1E			/* RW bit[4:1] 重传阈值. 重新传输上限阈值. */
#define CAN_SET_RTXTH_SHIFT		1
#define CAN_SET_RTXLE			bit(0)			/* RW 重传阈值使能 */

/**
 * 状态寄存器(CR_STAT) OFFSET: 0x0c
 */
#define CAN_SR_STCNT			bit(16)			/* RO 传输计数支持. 0: 不支持; 1: 支持. */
#define CAN_SR_PEXS				bit(8)			/* RO 协议例外状态. 出现协议例外时置1, CR_CMD[CPEXS]写1 清除状态. */
#define CAN_SR_IDLE				bit(7)			/* RO 总线空闲标志. 当总线空闲或控制器处于BUS_OFF 状态时为1. */
#define CAN_SR_EWL				bit(6)			/* RO 传输错误阈值标志. 当TEC(TX Error Counter)或REC(RX Error Counter)达到
                                                 *    或超过EWL(Error Warning Limit)时为1. */
#define CAN_SR_TX			    bit(5)			/* RO 发送状态标志. 控制器当前处于发送状态. */
#define CAN_SR_RX			    bit(4)			/* RO 接收状态标志. 控制器当前处于接收状态. */
#define CAN_SR_EFT				bit(3)			/* RO 错误帧标志. 控制器当前正在发送错误帧. */
#define CAN_SR_DOR				bit(1)			/* RO 接收数据溢出状态. 数据溢出时置1, CR_CMD[RRB]写1 清除状态,
                                                 *    发生溢出时不保存当前接收报文.
                                                 */
#define CAN_SR_RXNE				bit(0)			/* RO 接收缓冲区非空状态 */

/**
 * 命令寄存器(CR_CMD) OFFSET: 0x10
 */
#define CAN_CMD_CPEXS			bit(7)			/* WO 例外状态清除命令. 写1 清除协议例外状态(CR_STAT[PEXS]). */
#define CAN_CMD_TXFCRST			bit(6)			/* WO 发送报文计数清除命令. 清除发送报文计数(TX_FR_CNT). */
#define CAN_CMD_RXFCRST			bit(5)			/* WO 接收报文计数清除命令. 清除接收报文计数(RX_FR_CNT). */
#define CAN_CMD_ERCRST			bit(4)			/* WO 错误计数清除命令. 用于在控制器进入BUS_OFF 后等待重新加入总线时清除错误计数.
 	 	 	 	 	 	 	 	 	 	 	 	 *    可在进入BUS_OFF 前后任意时刻写1, 生效一次后失效. */
#define CAN_CMD_CDO				bit(3)			/* WO 接收数据溢出状态清除命令. 写1 清除接收数据溢出状态. */
#define CAN_CMD_RRB				bit(2)			/* WO 接收缓冲区释放命令. 写1 清除接收缓冲区指针及接收报文计数,
                                                 *    并清除接收数据溢出状态.
                                                 */
#define CAN_CMD_RXRPMV			bit(1)			/* WO 接收缓冲区读取指移动命令. 写1 将接收缓冲区的读取指针后移1 位.
                                                 *    在CR_MODE[RXBAM]为1 时, 收缓冲区的读取指针在一次读取后自动后移1 位.
                                                 */

//-------------------------------------------------------------------------------------------------
// CAN 中断
//-------------------------------------------------------------------------------------------------

/**
 * 中断状态寄存器(INT_STAT) OFFSET: 0x14
 */
#define CAN_ISR_DMAD			bit(12)			/* RW 报文DMA 传输完成中断状态. 当一个报文被完整的从接收缓冲区读出后此位置1,
                                                 *    写1 清除中断.
                                                 */
#define CAN_ISR_OF				bit(11)			/* RW 过载帧发送中断状态. 发送过载帧时此位置1, 不在发送过载帧时写1 清除中断. */
#define CAN_ISR_TXBHC			bit(10)			/* RW 发送缓冲区硬件命令中断状态. 发送完成或失败时此位置1, 写1 清除中断. */
#define CAN_ISR_RBNE			bit(9)			/* RW 接收缓冲区非空中断状态. 接收缓冲区非空时此位置1, 为空时写1 清除中断. */
#define CAN_ISR_BS				bit(8)			/* RW 位速率切换中断状态. 位速率发生切换时此位置1, 写1 清除中断. */
#define CAN_ISR_RXF				bit(7)			/* RW 接收缓冲区满中断状态. 接收缓冲区为满时此位置1, 写1 清除中断. */
#define CAN_ISR_BE				bit(6)			/* RW 总线错误中断状态. 总线发生错误时此位置1, 写1 清除中断. */
#define CAN_ISR_AL				bit(5)			/* RW 失去仲裁中断状态. 发送状态下失去仲裁时此位置1, 写1 清除中断. */
#define CAN_ISR_FCS				bit(4)			/* RW 错误处理状态改变中断状态. 错误处理状态在ERROR_ACTIVE、ERROR_PASSIVE
                                                 *    与BUS_OFF状态间切换时产生中断, 写1 清除中断.
                                                 */
#define CAN_ISR_DO				bit(3)			/* RW 接收数据溢出中断状态. 接收缓冲区为满时继续写入数据此位置1, 写1 清除中断.
                                                 *    清除此中断前需向CR_CMD[RRB]写1 释放接收缓冲区.
                                                 */
#define CAN_ISR_EWL				bit(2)			/* RW 错误警告阈值中断状态. 在ERROR_ACTIVE状态下TEC或REC达到或跨越EWL时此位置1,
                                                 *    写1 清除中断.
                                                 */
#define CAN_ISR_TX				bit(1)			/* RW 报文发送中断状态. 报文成功发送时此位置1, 写1 清除中断. */
#define CAN_ISR_RX				bit(0)			/* RW 报文接收中断状态. 报文成功接收时此位置1, 写1 清除中断. */

/**
 * 中断使能寄存器(INT_ENA) OFFSET: 0x18
 */
/*
 * 28:16 ENA_CLR WO 中断使能清除. 写1 清除中断使能
 */
#define CAN_ICLR_DMAD			bit(28)			/* DMAD 中断使能清除; */
#define CAN_ICLR_OF				bit(27)			/* OFI 中断使能清除; */
#define CAN_ICLR_TXBHC			bit(26)			/* TXBHCI 中断使能清除; */
#define CAN_ICLR_RBNE			bit(25)			/* RBNEI 中断使能清除; */
#define CAN_ICLR_BS				bit(24)			/* BSI 中断使能清除; */
#define CAN_ICLR_RXF			bit(23)			/* RXFI 中断使能清除; */
#define CAN_ICLR_BE				bit(22)			/* BEI 中断使能清除; */
#define CAN_ICLR_AL				bit(21)			/* ALI 中断使能清除; */
#define CAN_ICLR_FCS			bit(20)			/* FCSI 中断使能清除; */
#define CAN_ICLR_DO				bit(19)			/* DOI 中断使能清除; */
#define CAN_ICLR_EWL			bit(18)			/* EWLI 中断使能清除; */
#define CAN_ICLR_TX				bit(17)			/* TXI 中断使能清除; */
#define CAN_ICLR_RX				bit(16)			/* RXI 中断使能清除. */
/*
 * 12:0 ENA_SET RW 中断使能置位. 写1 置位中断使能, 读出表示当前状态
 */
#define CAN_ISET_DMAD			bit(12)			/* DMAD 中断使能置位; */
#define CAN_ISET_OF				bit(11)			/* OFI 中断使能置位; */
#define CAN_ISET_TXBHC			bit(10)			/* TXBHCI 中断使能置位; */
#define CAN_ISET_RBNE			bit(9)			/* RBNEI 中断使能置位; */
#define CAN_ISET_BS				bit(8)			/* BSI 中断使能置位; */
#define CAN_ISET_RXF			bit(7)			/* RXFI 中断使能置位; */
#define CAN_ISET_BE				bit(6)			/* BEI 中断使能置位; */
#define CAN_ISET_AL				bit(5)			/* ALI 中断使能置位; */
#define CAN_ISET_FCS			bit(4)			/* FCSI 中断使能置位; */
#define CAN_ISET_DO			    bit(3)			/* DOI 中断使能置位; */
#define CAN_ISET_EWL			bit(2)			/* EWLI 中断使能置位; */
#define CAN_ISET_TX			    bit(1)			/* TXI 中断使能置位; */
#define CAN_ISET_RX			    bit(0)			/* RXI 中断使能置位. */

/**
 * 中断状态屏蔽寄存器(INT_MASK) OFFSET: 0x1c
 */
/*
 * 28:16 MASK_CLR WO 中断状态屏蔽清除. 写1 清除中断状态屏蔽
 */
#define CAN_IMCLR_DMAD			bit(28)			/* DMAD 中断状态屏蔽清除; */
#define CAN_IMCLR_OF			bit(27)			/* OFI 中断状态屏蔽清除; */
#define CAN_IMCLR_TXBHC		    bit(26)			/* TXBHCI 中断状态屏蔽清除; */
#define CAN_IMCLR_RBNE			bit(25)			/* RBNEI 中断状态屏蔽清除; */
#define CAN_IMCLR_BS			bit(24)			/* BSI 中断状态屏蔽清除; */
#define CAN_IMCLR_RXF			bit(23)			/* RXFI 中断状态屏蔽清除; */
#define CAN_IMCLR_BE			bit(22)			/* BEI 中断状态屏蔽清除; */
#define CAN_IMCLR_AL			bit(21)			/* ALI 中断状态屏蔽清除; */
#define CAN_IMCLR_FCS			bit(20)			/* FCSI 中断状态屏蔽清除; */
#define CAN_IMCLR_DO			bit(19)			/* DOI 中断状态屏蔽清除; */
#define CAN_IMCLR_EWL			bit(18)			/* EWLI 中断状态屏蔽清除; */
#define CAN_IMCLR_TX			bit(17)			/* TXI 中断状态屏蔽清除; */
#define CAN_IMCLR_RX			bit(16)			/* RXI 中断状态屏蔽清除. */
/*
 * 12:0 MASK_SET RW 中断状态屏蔽置位. 写1 置位中断状态屏蔽, 读出表示当前状态
 */
#define CAN_IMSET_DMAD			bit(12)			/* DMAD 中断状态屏蔽置位; */
#define CAN_IMSET_OF			bit(11)			/* OFI 中断状态屏蔽置位; */
#define CAN_IMSET_TXBHC		    bit(10)			/* TXBHCI 中断状态屏蔽置位; */
#define CAN_IMSET_RBNE			bit(9)			/* RBNEI 中断状态屏蔽置位; */
#define CAN_IMSET_BS			bit(8)			/* BSI 中断状态屏蔽置位; */
#define CAN_IMSET_RXF			bit(7)			/* RXFI 中断状态屏蔽置位; */
#define CAN_IMSET_BE			bit(6)			/* BEI 中断状态屏蔽置位; */
#define CAN_IMSET_AL			bit(5)			/* ALI 中断状态屏蔽置位; */
#define CAN_IMSET_FCS			bit(4)			/* FCSI 中断状态屏蔽置位; */
#define CAN_IMSET_DO			bit(3)			/* DOI 中断状态屏蔽置位; */
#define CAN_IMSET_EWL			bit(2)			/* EWLI 中断状态屏蔽置位; */
#define CAN_IMSET_TX			bit(1)			/* TXI 中断状态屏蔽置位; */
#define CAN_IMSET_RX			bit(0)			/* RXI 中断状态屏蔽置位. */

/**
 * 标准速率配置寄存器(BTR_NORM) OFFSET: 0x20
 */
#define CAN_BTR_NORM_SJW_MASK	(0x1F<<27)		/* RW bit[31:27] 同步补偿宽度 */
#define CAN_BTR_NORM_SJW_SHIFT	27
#define CAN_BTR_NORM_BRP_MASK	(0xFF<<19)		/* RW bit[26:19] 位速率预分频系数 */
#define CAN_BTR_NORM_BRP_SHIFT	19
#define CAN_BTR_NORM_PH2_MASK	(0x3F<<13)		/* RW bit[18:13] 相位段2 宽度 */
#define CAN_BTR_NORM_PH2_SHIFT	13
#define CAN_BTR_NORM_PH1_MASK	(0x3F<<7)		/* RW bit[12:7] 相位段1 宽度 */
#define CAN_BTR_NORM_PH1_SHIFT	7
#define CAN_BTR_NORM_PROP_MASK	0x7F			/* RW bit[6:0] 传播段宽度 */

/**
 * FD 数据速率配置寄存器(BTR_FD) OFFSET: 0x24
 */
#define CAN_BTR_FD_SJW_MASK		(0x1F<<27)		/* RW bit[31:27] 同步补偿宽度 */
#define CAN_BTR_FD_SJW_SHIFT	27
#define CAN_BTR_FD_BRP_MASK		(0xFF<<19)		/* RW bit[26:19] 位速率预分频系数 */
#define CAN_BTR_FD_BRP_SHIFT	19
#define CAN_BTR_FD_PH2_MASK		(0x3F<<13)		/* RW bit[18:13] 相位段2 宽度 */
#define CAN_BTR_FD_PH2_SHIFT	13
#define CAN_BTR_FD_PH1_MASK		(0x3F<<7)		/* RW bit[12:7] 相位段1 宽度 */
#define CAN_BTR_FD_PH1_SHIFT	7
#define CAN_BTR_FD_PROP_MASK	0x7F			/* RW bit[6:0] 传播段宽度 */

/**
 * 错误阈值配置寄存器(ERL) OFFSET: 0x28
 */
#define CAN_ERRLVL_EWL_MASK		(0xFF<<16)		/* RW bit[23:16] 错误警告阈值. 仅可在MODE[TSTM]为1时修改; 默认值为0x60. */
#define CAN_ERRLVL_EWL_SHIFT	16
#define CAN_ERRLVL_ERP_MASK		0xFF			/* RW bit[7:0] 错误被动阈值. 仅可在MODE[TSTM]为1时修改; 默认值为0x80. */

/**
 * 错误状态寄存器(FSTAT) OFFSET: 0x2c
 */
#define CAN_ERRSR_BUSOFF		bit(2)			/* RO BUS_OFF 状态 */
#define CAN_ERRSR_ERP			bit(1)			/* RO ERROR_PASSIVE 状态 */
#define CAN_ERRSR_ERA			bit(0)			/* RO ERROR_ACTIVE 状态 */

/**
 * 错误计数寄存器(ERC) OFFSET: 0x30
 */
#define CAN_ERRCNT_RX_MASK		(0x1FF<<16)		/* RO bit[24:16] 接收错误计数 */
#define CAN_ERRCNT_RX_SHIFT		16
#define CAN_ERRCNT_TX_MASK		0x1FF			/* RO bit[8:0] 发送错误计数 */

/**
 * 速率错误计数寄存器(BRE) OFFSET: 0x34
 */
#define CAN_BRERR_NORM_MASK		0xFFFF0000		/* RO bit[31:16] 常规速率错误计数 */
#define CAN_BRERR_NORM_SHIFT	16
#define CAN_BRERR_DATA_MASK		0xFFFF			/* RO bit[15:0] FD 速率错误计数 */

/**
 * 错误计数调试寄存器(CTR_PRES) OFFSET: 0x38
 */
#define CAN_CTRPRES_PRX			bit(10)  		/* WO REC 预写. 写1 时将CTR_PRES[CTPV]写入REC. */
#define CAN_CTRPRES_PTX			bit(9)			/* WO TEC 预写. 写1 时将CTR_PRES[CTPV]写入TEC. */
#define CAN_CTRPRES_CTPV_MASK	0x1FF			/* WO bit[8:0] 错误计数器预写值 */

/**
 * 错误捕捉状态寄存器(ERR_CAPT) OFFSET: 0x3c
 */
#define CAN_ECAPT_TYPE_MASK		(0x07<<5)		/* RO bit[7:5] 最近的一个错误类型. */
#define CAN_ECAPT_TYPE_SHIFT	5
#define CAN_ETYPE_BIT			0			    /*    3'd0: BIT_ERR - Bit Error; */
#define CAN_ETYPE_CRC			1			    /*    3'd1: CRC_ERR - CRC Error; */
#define CAN_ETYPE_FRM			2			    /*    3'd2: FRM_ERR - Form Error; */
#define CAN_ETYPE_ACK			3			    /*    3'd3: ACK_ERR - Acknowledge Error; */
#define CAN_ETYPE_STUP			4			    /*    3'd4: STUF_ERR - Stuff Error. */

#define CAN_ECAPT_POS_MASK		0x1F			/* RO bit[4:0] 最近的一个错误位置. */
#define CAN_EPOS_SOF			0				/*    5'b00000: POS_SOF - Error in Start of Frame; */
#define CAN_EPOS_ARB			1				/*    5'b00001: POS_ARB - Error in Arbitration Filed; */
#define CAN_EPOS_CTRL			2				/*    5'b00010: POS_CTRL - Error in Control field; */
#define CAN_EPOS_DATA			3				/*    5'b00011: POS_DATA - Error in Data Field; */
#define CAN_EPOS_CRC			4				/*    5'b00100: POS_CRC - Error in CRC Field; */
#define CAN_EPOS_ACK			5				/*    5'b00101: POS_ACK - Error in CRC delimiter, ACK field or ACK delimiter */
#define CAN_EPOS_EOF			6				/*    5'b00110: POS_EOF - Error in End of frame field; */
#define CAN_EPOS_ERR			7				/*    5'b00111: POS_ERR - Error during Error frame; */
#define CAN_EPOS_OVRL			8				/*    5'b01000: POS_OVRL - Error in Overload frame; */
#define CAN_EPOS_OTHER			0x1F			/*    5'b11111: POS_OTHER - Other position of error. */

/**
 * 重发计数寄存器(RETX_CNT) OFFSET: 0x40
 */
#define CAN_RETX_CNT_MASK		0x0F			/* RO bit[3:0] 当前发送报文重发次数 */

/**
 * 失去仲裁捕捉寄存器(ALC) OFFSET: 0x44
 */
#define CAN_ALC_ID_MASK			(0x07<<5)		/* RO bit[7:5] 失去仲裁类型. */
#define CAN_ALC_ID_SHIFT        5
#define CAN_ALC_RSVD			(0<<5)			/*    3'd0 - RSVD - 从未失去过仲裁; */
#define CAN_ALC_BASE_ID			(1<<5)			/*    3'd1 - BASE_ID - 在base identifier 段失去仲裁; */
#define CAN_ALC_SRR_RTR			(2<<5)			/*    3'd2 - SRR_RTR - 在base identifier 后的第一位失去仲裁
                                                 *                     (扩展模式的SRR 位, 标准模式的RTR 位);
                                                 */
#define CAN_ALC_IDE				(3<<5)			/*    3'd3 - IDE - 在IDE 位失去仲裁; */
#define CAN_ALC_EXTENSION		(4<<5)			/*    3'd4 - EXTENSION - 在Identifier extension 段失去仲裁; */
#define CAN_ALC_RTR				(5<<5)			/*    3'd5 - RTR - 在扩展模式的RTR 位失去仲裁. */

#define CAN_ALC_BIT_POS_MASK	0x1F			/* RO bit[4:0] 失去仲裁位置. 仅在ALC[ID_FIELD]位BASE_ID 或EXTENSION 有意义. */

/**
 * 传输延迟测量寄存器(TRV_DLY) OFFSET: 0x48
 */
#define CAN_TRV_DLY_MASK		0x7F			/* RO bit[6:0] 传输延迟测量值. 单位为一个系统时钟的周期. */

/**
 * 第二采样点配置寄存器(SSP_CFG) OFFSET: 0x4c
 */
#define CAN_SSP_CFG_SAT			bit(10)			/* RW 第二采样点延迟饱和使能. 此位为1 时, 最大延迟为255 个系统时钟. */
#define CAN_SSP_CFG_SRC_MASK	(0x03<<8)		/* RW bit[9:8] 第二采样点延迟选择. */
#define CAN_SSP_CFG_SRC_SHIFT	8
#define CAN_SSP_CFG_0			(0<<8)			/*    2'd0: 使用TRV_DLY 与SSP_CFG[SSP_OFF]之和作为第二采样点延迟; */
#define CAN_SSP_CFG_1			(1<<8)			/*    2'd1: 不启用第二采样点, 使用标准采样点作为FD 数据位速率; */
#define CAN_SSP_CFG_2			(2<<8)			/*    2'd2: 使用SSP_CFG[SSP_OFF]作为第二采样点延迟. */

#define CAN_SSP_CFG_OFF_MASK	0xFF			/* RW bit[7:0] 第二采样点偏移. 单位为一个系统时钟的周期. */

/**
 * 接收报文计数寄存器(RX_FR_CNT) OFFSET: 0x50
 */
/*
 * 31:0 FR_CNT RO 接收报文计数. 仅当CR_STAT[STCNT]时有效.
 */

/**
 * 发送报文计数寄存器(TX_FR_CNT) OFFSET: 0x54
 */
/*
 * 31:0 FR_CNT RO 发送报文计数. 仅当CR_STAT[STCNT]时有效.
 */

/**
 * 调试寄存器(DEBUG) OFFSET: 0x58
 */
#define CAN_DEBUG_SOF			bit(18)			/* RO Start of frame 段标志. 当前总线状态处于Start of frame 段. */
#define CAN_DEBUG_OVR			bit(17)			/* RO Overload 段标志. 当前总线状态处于Overload 段. */
#define CAN_DEBUG_SUSP			bit(16)			/* RO Suspend transmission 段标志. 当前总线状态处于Suspend transmission 段. */
#define CAN_DEBUG_INT			bit(15)			/* RO Intermission 段标志. 当前总线状态处于Intermission 段. */
#define CAN_DEBUG_EOF			bit(14)			/* RO End of file 段标志. 当前总线状态处于End of file 段. */
#define CAN_DEBUG_ACKD			bit(13)			/* RO ACK Delimiter 段标志. 当前总线状态处于ACK Delimiter 段. */
#define CAN_DEBUG_ACK			bit(12)			/* RO ACK 段标志. 当前总线状态处于ACK 段. */
#define CAN_DEBUG_CRCD			bit(11)			/* RO CRC Delimiter 段标志. 当前总线状态处于CRC Delimiter 段. */
#define CAN_DEBUG_CRC			bit(10)			/* RO CRC 段标志. 当前总线状态处于CRC 段. */
#define CAN_DEBUG_STC			bit(9)			/* RO Stuff Count 段标志. 当前总线状态处于Stuff Count 段. */
#define CAN_DEBUG_DAT			bit(8)			/* RO Data 段标志. 当前总线状态处于Data 段. */
#define CAN_DEBUG_CON			bit(7)			/* RO Control 段标志. 当前总线状态处于Control 段. */
#define CAN_DEBUG_ARB			bit(6)			/* RO Arbitration 段标志. 当前总线状态处于Arbitration 段. */

#define CAN_DEBUG_DSTF_CNT_MASK 0x38			/* RO bit[5:3] de-staff 计数. 当前或最近一个报文de-stuff 数余8. */
#define CAN_DEBUG_STF_CNT_MASK	0x07			/* RO bit[2:0] stuff 计数.当前或最近一个报文stuff 数余8. */

/**
 * 时间戳寄存器(TS) OFFSET: 0x5c
 */
#define CAN_TS_PSC_MASK			(0x1FF<<16)		/* RW bit[24:16] 内部时间戳分频系数 */
#define CAN_TS_PSC_SHIFT		16
#define CAN_TS_CURRENT_MASK	    0xFFFF			/* RO bit[15:0] 当前时间戳 */

/**
 * 发送报文调试寄存器(TX_FRM_TST) OFFSET: 0x60
 */
#define CAN_TX_FRM_T_TPRM_MASK	0xF8			/* RW bit[7:3] 错误参数.
 	 	 	 	 	 	 	 	 	 	 	 	 *    当TX_FRM_TST[SDLC]为1 时, 报文DLC段将在传输时被TPRM[3:0]替换;
 	 	 	 	 	 	 	 	 	 	 	 	 *    当TX_FRM_TST[FCRC]为1 时, 报文CRC 段的第TPRM 位将在传输时翻转;
 	 	 	 	 	 	 	 	 	 	 	 	 *    当TX_FRM_TST[FSTC]为1 时, 报文Stuff Count 段与TPRM[4:0]进行异或并进行传输.
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define CAN_TX_FRM_T_TPRM_SHIFT	3

#define CAN_TX_FRM_T_SDLC		bit(2)			/* RW DLC 段注错使能 */
#define CAN_TX_FRM_T_FCRC		bit(1)			/* RW CRC 段注错使能 */
#define CAN_TX_FRM_T_FSTC		bit(0)			/* RW Stuff Count 段注错使能 */

/**
 * 小数分频系数寄存器(FRC_DIV) OFFSET: 0x64
 */
#define CAN_FRC_DBT_MASK		(0xFF<<8)		/* RW bit[15:8] 数据位速率小数分频系数.
 	 	 	 	 	 	 	 	 	 	 	 	 *              位速率分频=BRP*(PH1+PH2+PROP+1+(FRC/0x100)).
 	 	 	 	 	 	 	 	 	 	 	 	 *    例: BRP=4; PH1=3; PH2=4; PROP=2; FRC=8'b01100000;
 	 	 	 	 	 	 	 	 	 	 	 	 *        DIV=4*(3+4+2+1+0.375) = 41.5.
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define CAN_FRC_DBT_SHIFT		8

#define CAN_FRC_NBT_MASK		0xFF			/* RW bit[7:0] 常规位速率小数分频系数同上 */

/**
 * 过滤器A 掩码寄存器(FLT_A_MASK) OFFSET: 0x68
 *
 * 28:0 MASK_VAL RW 过滤器A 掩码
 */

/**
 * 过滤器A 数值寄存器(FLT_A_VAL) OFFSET: 0x6c
 *
 * 28:0 FLT_VAL RW 过滤器A 数值
 */

/**
 * 过滤器B 掩码寄存器(FLT_B_MASK) OFFSET: 0x70
 *
 * 28:0 MASK_VAL RW 过滤器B 掩码
 */

/**
 * 过滤器B 数值寄存器(FLT_B_VAL) OFFSET: 0x74
 *
 * 28:0 FLT_VAL RW 过滤器B 数值
 */

/**
 * 过滤器C 掩码寄存器(FLT_C_MASK) OFFSET: 0x78
 *
 * 28:0 MASK_VAL RW 过滤器C 掩码
 */

/**
 * 过滤器C 数值寄存器(FLT_C_VAL) OFFSET: 0x7c
 *
 * 28:0 FLT_VAL RW 过滤器C 数值
 */

#define CAN_FILTER_MASK         0x1FFFFFFF

/**
 * 范围过滤器低阈值寄存器(FLT_R_LOW) OFFSET: 0x80
 *
 * 28:0 LOW_VAL RW 范围过滤器低阈值
 */

/**
 * 范围过滤器高阈值寄存器(FLT_R_HI) OFFSET: 0x84
 *
 * 28:0 HI_VAL RW 范围过滤器高阈值
 */

/**
 * 过滤器控制寄存器(FLT_CTRL) OFFSET: 0x88
 */
/* 15:12 FR RW 范围过滤器格式支持.
 */
#define CAN_FCTRL_FR_FE			bit(15)			/* FE: 范围过滤器CANFD 标准模式报文接收使能; */
#define CAN_FCTRL_FR_FB			bit(14)			/* FB: 范围过滤器CANFD 扩展模式报文接收使能; */
#define CAN_FCTRL_FR_NE			bit(13)			/* NE: 范围过滤器CAN2.0 标准模式报文接收使能; */
#define CAN_FCTRL_FR_NB			bit(12)			/* NB: 范围过滤器CAN2.0 扩展模式报文接收使能. */
/* 11:8 FC RW 过滤器C 格式支持.
 */
#define CAN_FCTRL_FC_FE			bit(11)			/* FE: 过滤器C CANFD 标准模式报文接收使能; */
#define CAN_FCTRL_FC_FB			bit(10)			/* FB: 过滤器C CANFD 扩展模式报文接收使能; */
#define CAN_FCTRL_FC_NE			bit(9)			/* NE: 过滤器C CAN2.0 标准模式报文接收使能; */
#define CAN_FCTRL_FC_NB			bit(8)			/* NB: 过滤器C CAN2.0 扩展模式报文接收使能. */
/* 7:4 FB RW 过滤器B 格式支持.
 */
#define CAN_FCTRL_FB_FE			bit(7)			/* FE: 过滤器B CANFD 标准模式报文接收使能; */
#define CAN_FCTRL_FB_FB			bit(6)			/* FB: 过滤器B CANFD 扩展模式报文接收使能; */
#define CAN_FCTRL_FB_NE			bit(5)			/* NE: 过滤器B CAN2.0 标准模式报文接收使能; */
#define CAN_FCTRL_FB_NB			bit(4)			/* NB: 过滤器B CAN2.0 扩展模式报文接收使能. */
/* 3:0 FA RW 过滤器A 格式支持.
 */
#define CAN_FCTRL_FA_FE			bit(3)			/* FE: 过滤器A CANFD 标准模式报文接收使能; */
#define CAN_FCTRL_FA_FB			bit(2)			/* FB: 过滤器A CANFD 扩展模式报文接收使能; */
#define CAN_FCTRL_FA_NE			bit(1)			/* NE: 过滤器A CAN2.0 标准模式报文接收使能; */
#define CAN_FCTRL_FA_NB			bit(0)			/* NB: 过滤器A CAN2.0 扩展模式报文接收使能. */

/**
 * 接收缓冲区信息寄存器(RX_MEM_INFO) OFFSET: 0x8c
 */
#define CAN_RX_MEM_FREE_MASK	(0x1FFF<<16)	/* RO bit[28:16] 接收缓冲区剩余深度32 位字. */
#define CAN_RX_MEM_FREE_SHIFT	16
#define CAN_RX_BUF_SIZE_MASK	0xFFF			/* RO bit[12:0] 接收缓冲区深度32 位字. */

/**
 * 接收缓冲区指针寄存器(RX_PRT) OFFSET: 0x90
 */
#define CAN_RX_RPP_MASK			(0xFFF<<16)		/* RO bit[27:16] 接收缓冲区读指针位置 */
#define CAN_RX_RPP_SHIFT		16
#define CAN_RX_WPP_MASK			0xFFF			/* RO bit[11:0] 接收缓冲区写指针位置 */

/**
 * 接收缓冲区状态寄存器(RX_STAT) OFFSET: 0x94
 */
#define CAN_RXSR_FRC_MASK		(0x7FF<<4)		/* RO bit[14:4] 接收缓冲区报文数量计数 */
#define CAN_RXSR_FRC_SHIFT		4
#define CAN_RXSR_MOF			bit(2)			/* RO 接收缓冲区数据段标志. 此位为1 时, 下一组读出数据为报文数据段. */
#define CAN_RXSR_RXF			bit(1)			/* RO 接收缓冲区满 */
#define CAN_RXSR_RXE			bit(0)			/* RO 接收缓冲区空 */

/**
 * 接收数据寄存器(RX_DATA) OFFSET: 0x98
 *
 * 31:0 DATA RO 接收缓冲区数据读出接口. 当CR_MODE[RXBAM]为1 时, 读指针自动加1; 否则需向CR_CMD[RXRPMV]写1 移动指针.
 */

/**
 * 发送缓冲区状态寄存器(TX_STAT) OFFSET: 0x9c
 */
#define CAN_TXSR_BS_MASK		0xFFFF0000		/* RO bit[31:16] 发送完成记录上一发送任务完成类型: */
#define CAN_TXSR_BS_SHIFT		16
#define CAN_TXSR_BS_NOTSEND		(0<<16)			/* 2'b00: 状态已处理或未发送; */
#define CAN_TXSR_BS_OK			(1<<16)			/* 2'b01: 发送成功; */
#define CAN_TXSR_BS_FAIL		(2<<16)			/* 2'b10: 发送失败; */
#define CAN_TXSR_BS_CANCEL		(3<<16)			/* 2'b11: 取消发送. */

#define CAN_TXSR_MASK			0x700			/* RO bit[10:8] 发送缓冲区状态 */
#define CAN_TXSR_SHIFT			8
#define CAN_TXSR_IDLE			(0<<8)			/* 3'd0: 空闲状态, 无发送请求或等待时间戳命中; */
#define CAN_TXSR_SCAN_TS		(1<<8)			/* 3'd1: 扫描时间戳状态, 扫描有效发送缓冲区中报文的时间戳; */
#define CAN_TXSR_SCAN_ID		(2<<8)			/* 3'd2: 扫描~ID~状态, 扫描有效发送缓冲区中报文的~ID; */
#define CAN_TXSR_WAIT			(3<<8)			/* 3'd3: 等待发送状态, 等待总线空闲; */
#define CAN_TXSR_SEND			(4<<8)			/* 3'd4: 发送状态, 报文发送中; */
#define CAN_TXSR_DONE			(5<<8)			/* 3'd5: 发送完成状态, 报文成功发送; */
#define CAN_TXSR_START_SCAN		(6<<8)			/* 3'd6: 开始扫描状态, 当TX_STAT[BRP]发生了变化并且发送缓冲区
                                                 *       不在发送状态时开始扫描.
                                                 */

#define CAN_TXSR_BRP_MASK		0xFF			/* RO bit[7:0] 发送请求等待. 待发送的缓冲区标志. */

/**
 * 发送命令控制寄存器(TX_CMD) OFFSET: 0xa0
 */
#define CAN_TXCMD_BSC_MASK		(0xFF<<16)		/* WO bit[23:16] 缓冲区发送记录清除. 写1 清除发送完成记录(TX_STAT[BS]). */
#define CAN_TXCMD_BSC_SHIFT		16
#define CAN_TXCMD_BCR_MASK		0xFF00			/* WO bit[15:8] 缓冲区取消请求. 取消使能了的缓冲区, 发送过程中向此位写1,
                                                 *              操作会在发送结束后生效.
                                                 */
#define CAN_TXCMD_BCR_SHIFT		8
#define CAN_TXCMD_BAR_MASK		0xFF			/* WO bit[7:0] 缓冲区添加请求. 完成向对应的缓冲区写入数据后即可向对应位
                                                 *             写1使能该缓冲区, 发送成功后自动清零.
                                                 */

/**
 * 发送缓冲区选择寄存器(TX_SEL) OFFSET: 0xa4
 */
#define CAN_TXSEL_CNT_MASK		0xF0			/* RO bit[7:4] 发送缓冲区数量. 控制器中存在的发送缓冲区数量. */
#define CAN_TXSEL_CNT_SHIFT	    4
#define CAN_TXSEL_MASK		    0x0F			/* RW bit[3:0] 发送缓冲区选择. 选择即将写入的发送缓冲区, 每个缓冲区可写入一帧报文.
                                                 */

/**
 * 发送缓冲区数据寄存器(TX_DATA) OFFSET: 0xb0～0xf4
 *
 * 仅可在MODE[TSTM]为1 时进行读取.
 *
 * 31:0 DATA RW 发送数据向TX_SEL 选择的发送缓冲区写入数据.
 */

#ifdef __cplusplus
}
#endif

#endif // _LS2K_CAN_HW_H


