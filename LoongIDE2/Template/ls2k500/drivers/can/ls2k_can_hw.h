/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_can_hw.h
 *
 * created: 2022-03-09
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

#define CAN0_BASE               0x1ff44000
#define CAN1_BASE               0x1ff45000
#define CAN2_BASE               0x1ff46000
#define CAN3_BASE               0x1ff47000

/*
 * CAN 控制器
 */
typedef struct 
{
	volatile unsigned char ctrl;							/* 0x00 控制寄存器 */
	volatile unsigned char cmd;								/* 0x01 命令寄存器 */
	volatile unsigned char status;							/* 0x02 状态寄存器 */
	volatile unsigned char intflags;						/* 0x03 中断寄存器 */

	union
	{
		struct		/* 标准模式 */
		{
			volatile unsigned char accode;					/* 0x04 验收代码寄存器 */
			volatile unsigned char acmask;					/* 0x05 验收屏蔽寄存器 */
		} std;

		struct		/* 扩展模式 */
		{
			volatile unsigned char inten;					/* 0x04 中断使能寄存器 */
			volatile unsigned char res5;					/* 0x05 */
		} ext;

	} R4;

	volatile unsigned char btr0;							/* 0x06 */
	volatile unsigned char btr1;							/* 0x07 */
	volatile unsigned char res8;							/* 0x08 */
	volatile unsigned char res9;							/* 0x09 */

	union
	{
		struct		/* 标准模式 */
		{
			struct
			{
				volatile unsigned char id[2];				/* 0x0A~0x0B:	tx ID(10-3); ID(2-0),RTR(4),DLC(3:0) */
				volatile unsigned char data[8];				/* 0x0C~0x13	tx data(1~8) */
			} tx;

			struct
			{
				volatile unsigned char id[2];				/* 0x14~0x15:	rx ID(10-3); ID(2-0),RTR(4),DLC(3:0) */
				volatile unsigned char data[8];				/* 0x16~0x1D	rx data(1~8) */
			} rx;
		} std;

		struct		/* 扩展模式 */
		{
			volatile unsigned char resA;					/* 0x0A */
			volatile unsigned char arblost;					/* 0x0B 仲裁丢失捕捉寄存器 */
			volatile unsigned char errcode;					/* 0x0C 错误代码捕捉寄存器 */
			volatile unsigned char emlimit;					/* 0x0D 错误警报限制寄存器 */
			volatile unsigned char rxerrcnt;				/* 0x0E RX错误计数寄存器 */
			volatile unsigned char txerrcnt;				/* 0x0F TX错误计数寄存器 */

			union
			{
				struct			/* 工作模式下接收区 */
				{
					volatile unsigned char frameinfo;		/* 0x10 RX帧信息 */
					union
					{
						struct	/* 扩展帧 */
						{
							volatile unsigned char id[4];	/* 0x11~0x14, RX识别码 1-4 */
							volatile unsigned char data[8];	/* 0x15~0x1C, RX数据 1-8 */
						} ext;

						struct	/* 标准帧 */
						{
							volatile unsigned char id[2];	/* 0x11~0x12, RX识别码 1-2 */
							volatile unsigned char data[8];	/* 0x13~0x1A, RX数据 1-8 */
							volatile unsigned char res1B;	/* 0x1B */
							volatile unsigned char res1C;	/* 0x1C */
						} std;
					};
				} rx;

				struct			/* 工作模式下发送区 */
				{
					volatile unsigned char frameinfo;		/* 0x10 TX帧信息 */
					union
					{
						struct	/* 扩展帧 */
						{
							volatile unsigned char id[4];	/* 0x11~0x14, TX识别码 1-4 */
							volatile unsigned char data[8];	/* 0x15~0x1C, TX数据 1-8 */
						} ext;

						struct	/* 标准帧 */
						{
							volatile unsigned char id[2];	/* 0x11~0x12, TX识别码 1-2 */
							volatile unsigned char data[8];	/* 0x13~0x1A, TX数据 1-8 */
							volatile unsigned char res1B;	/* 0x1B */
							volatile unsigned char res1C;	/* 0x1C */
						} std;
					};
				} tx;

				struct			/* 复位模式下 */
				{
					volatile unsigned char accode[4];		/* 0x10~0x13  验收代码1-4 */
					volatile unsigned char acmask[4];		/* 0x14~0x17  验收屏蔽1-4 */
				} accept;
			} msg;

			volatile unsigned char rxframecnt;				/* 0x1D RX信息计数寄存器 */

		} ext;

	} R10;

} HW_CAN_t;

/*
 * CAN Control Register
 */
#define CAN_CTRL_OVERFLOWIEN_STD        (1<<4)			/* 溢出中断使能 */
#define CAN_CTRL_ERRORIEN_STD           (1<<3)			/* 错误中断使能 */
#define CAN_CTRL_TXIEN_STD		        (1<<2)			/* 发送中断使能 */
#define CAN_CTRL_RXIEN_STD		        (1<<1)			/* 接收中断使能 */

#define CAN_CTRL_SLEEP_EXT		        (1<<4)			/* 睡眠模式 */
#define CAN_CTRL_AFILTER_EXT	        (1<<3)			/* 滤波器模式.   =1: 单; =0: 双. */
#define CAN_CTRL_STANDWORK_EXT	        (1<<2)			/* 正常工作模式. =1: 正常. */
#define CAN_CTRL_LISTENONLY_EXT	        (1<<1)			/* 只听模式.     =1: 只听. */

#define CAN_CTRL_RESET 			        (1<<0)			/* 复位请求 */

#define CAN_STD_INTERRUPTS              (CAN_CTRL_OVERFLOWIEN_STD | CAN_CTRL_ERRORIEN_STD | \
						                 CAN_CTRL_TXIEN_STD | CAN_CTRL_RXIEN_STD)

/*
 * CAN Command Register.
 */
#define CAN_CMD_STANDARD    			0
#define CAN_CMD_EXTENDED			    (1<<7)			/* 扩展模式 -> CAN 2.0 A/B, 0=standard */
#define CAN_CMD_SLEEP_STD			    (1<<4)			/* Standard: 睡眠 */
#define CAN_CMD_SELFRXREQUEST_EXT	    (1<<4)			/* 自接收请求, =1: 当前信息可被同时发送和接收 */
#define CAN_CMD_CLEARDATAOVERFLOW       (1<<3)			/* 清除数据溢出 */
#define CAN_CMD_RELEASERXBUF		    (1<<2)			/* 释放接收缓冲器 */
#define CAN_CMD_TXABORT				    (1<<1)			/* 中止发送, =1: 当前, 如果不是正在处理, 等待中的发送请求被取消 */
#define CAN_CMD_TXREQUEST 			    (1<<0)			/* 发送请求 */

/*
 * CAN Status Register
 */
#define CAN_STATUS_BUS				    (1<<7)			/* 总线状态. =1: 总线关闭; =0: 总线开启. */
#define CAN_STATUS_ERROR			    (1<<6)			/* 出错状态. 	=1: 至少出现一个错误计数器满或超过CPU报警限制. */
#define CAN_STATUS_TX				    (1<<5)			/* 发送状态. 	=1: 发送; =0: 空闲. */
#define CAN_STATUS_RX				    (1<<4)			/* 接收状态. 	=1: 接收; =0: 空闲. */
#define CAN_STATUS_TXCOMPLETE		    (1<<3)			/* 发送完毕状态. =1: 完毕; =0: 未完毕. */
#define CAN_STATUS_TXBUF			    (1<<2)			/* 发送缓存器状态. =1: 释放; =0: 锁定. */
#define CAN_STATUS_DATAOVERFLOW		    (1<<1)			/* 数据溢出状态. =1: 溢出, 信息丢失, 因为RXFIFO中没有足够的空间来存储它. */
#define CAN_STATUS_RXBUF 			    (1<<0)			/* 接收缓存器状态. =1: 满, RXFIFO中有可用信息. */

/*
 * CAN Interrupt Register
 */
#define CAN_IFLAG_BUSERROR_EXT		    (1<<7)		    /* 总线错误中断 */
#define CAN_IFLAG_ARBITRATELOST_EXT	    (1<<6)		    /* 仲裁丢失中断 */
#define CAN_IFLAG_ERRORPASSIVE_EXT	    (1<<5)		    /* 错误消极中断 */
#define CAN_IFLAG_WAKEUP				(1<<4)		    /* 唤醒中断 */
#define CAN_IFLAG_DATAOVERFLOW		    (1<<3)		    /* 数据溢出中断 */
#define CAN_IFLAG_ERROR				    (1<<2)		    /* 错误中断 */
#define CAN_IFLAG_TX					(1<<1)		    /* 发送中断 */
#define CAN_IFLAG_RX 				    (1<<0)		    /* 接收中断 */

/*
 * CAN Interrupt Enable Register
 */
#define CAN_IEN_BUSERROR_EXT		    (1<<7)			/* 总线错误中断 */
#define CAN_IEN_ARBITRATELOST_EXT	    (1<<6)			/* 仲裁丢失中断 */
#define CAN_IEN_ERRORPASSIVE_EXT	    (1<<5)			/* 错误消极中断 */
#define CAN_IEN_WAKEUP_EXT		        (1<<4)			/* 唤醒中断 */
#define CAN_IEN_DATAOVERFLOW_EXT	    (1<<3)			/* 数据溢出中断 */
#define CAN_IEN_ERROR_EXT			    (1<<2)			/* 错误中断 */
#define CAN_IEN_TX_EXT			        (1<<1)			/* 发送中断 */
#define CAN_IEN_RX_EXT 			        (1<<0)			/* 接收中断 */
#define CAN_IEN_ALL_EXT			        0xFF			/* Enable All */

/*
 * CAN Bus Timer Register 0
 */
#define CAN_BTR0_SJW_MASK		        0xC0			/* bit(7:6), 同步跳转宽度 */
#define CAN_BTR0_SJW_SHIFT		        6
#define CAN_BTR0_BRP_MASK		        0x3F			/* bit(5:0), 波特率分频系数 */

/*
 * CAN Bus Timer Register 1
 */
#define CAN_BTR1_SAMPLE_MASK            0x80			/* bit(7), =1 时三次采样, 否则是一次采样 */
#define CAN_BTR1_SAMPLE_SHIFT           7
#define CAN_BTR1_TSEG2_MASK             0x70			/* bit(6:4), 一个bit中的时间段 2 的计数值 */
#define CAN_BTR1_TSEG2_SHIFT            4
#define CAN_BTR1_TSEG1_MASK             0x0F			/* bit(3:0), 一个bit中的时间段 1 的计数值 */

/*
 * CAN Arbitrate Lost Register
 */
#define CAN_ARBLOST_MASK_EXT            0x1F

/*
 * CAN Error Code Register
 */
#define CAN_ERRCODE_MASK_EXT	 	    0xC0			/* bit(7:6), */
#define CAN_ERRCODE_BIT_EXT			    0x00			/* 00: 位错 */
#define CAN_ERRCODE_FORM_EXT		    0x40			/* 01: 格式错 */
#define CAN_ERRCODE_STUFF_EXT	 	    0x80			/* 10: 填充错 */
#define CAN_ERRCODE_OTHER_EXT	 	    0xC0			/* 11: 其它错误 */
#define CAN_ERRCODE_DIR_EXT		 	    0x20			/* bit(5), =1: RX Error; =0: TX Error */
#define CAN_ERRCODE_SEG_MASK_EXT 	    0x1F

#ifdef __cplusplus
}
#endif

#endif // _LS2K_CAN_HW_H

