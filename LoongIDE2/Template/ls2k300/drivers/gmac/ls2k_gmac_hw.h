/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_gmac_hw.h
 *
 * created: 2024-06-08
 *  author: Bian
 */

#ifndef _LS2K_GMAC_HW_H
#define _LS2K_GMAC_HW_H

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
// GMAC 设备
//-------------------------------------------------------------------------------------------------

#define GMAC0_BASE              0x16020000
#define GMAC1_BASE              0x16030000

typedef struct
{
	volatile unsigned int addrhi;
	volatile unsigned int addrlo;
} MAC_ADDR_t;

//-------------------------------------------------------------------------------------------------
// GMAC 控制器
//-------------------------------------------------------------------------------------------------

typedef struct
{
	volatile unsigned int config;				/* 0x0000 Configuration */
	volatile unsigned int framefilter;			/* 0x0004 GMAC Frame Filter */
	volatile unsigned int hashhi;				/* 0x0008 Hash Table High */
	volatile unsigned int hashlo;				/* 0x000C Hash Table Low */
	volatile unsigned int miictrl;				/* 0x0010 GMII Address */
	volatile unsigned int miidata;				/* 0x0014 GMII Data */
	volatile unsigned int flowctrl;				/* 0x0018 Flow Control */
	volatile unsigned int vlantag;				/* 0x001C VLAN Tag */
	volatile unsigned int version;				/* 0x0020 Version */
	volatile unsigned int rsv0;
	volatile unsigned int wakeupaddr;           /* 0x0028 wake-up frame filter adrress reg */
	volatile unsigned int pmtctrlstatus;        /* 0x002C PMT control and status register */
	volatile unsigned int rsv1[2];
	volatile unsigned int intstatus;			/* 0x0038 Interrupt Status */
	volatile unsigned int intmask;				/* 0x003C Interrupt Mask */
	volatile unsigned int addrhi;               /* 0x0040 Mac Addreee High */
	volatile unsigned int addrlo;               /* 0x0044 Mac Addreee Lo */

    MAC_ADDR_t macaddr[15];						/* 1~15 */

} HW_GMAC_t;

/*
 * GMAC Config Register 0x0000
 */
enum GMAC_Config_R
{
    gmac_ctrl_tc            = (1<<24),      /* Transmit Configuration in RGMII 使能RGMII链路信息传输
                                               =1: 将会把双工模式, 链路速度, 链路以及链路连接/断开等信息通过RGMII接口传输给PHY */

	gmac_ctrl_wd			= (1<<23),		/* Watchdog Disable, 关闭看门狗.
	 	 	 	 	 	 	 	 	 	 	   =1: GMAC将关闭接收端的看门狗定时器, 可以接收最大16384字节的以太网帧*/
	gmac_ctrl_jd			= (1<<22),		/* Jabber Disable, 关闭Jabber定时器.
											   =1: GMAC关闭发送过程中的Jabber定时器, 可以发送最大16384字节的以太网帧 */
	gmac_ctrl_be			= (1<<21),		/* Frame Burst Enable, =1: GMAC使能传输过程中的帧突发传输模式 */
	gmac_ctrl_je			= (1<<20),		/* Jumbo Frame Enable - 巨帧使能, =1: GMAC使能巨帧(最大9018字节)的接收 */
	gmac_ctrl_ifg_mask		= (0x7<<17), 	/* Inter-Frame Gap - 最小帧间距 */
	gmac_ctrl_ifg_shift		= 17,

	gmac_ctrl_dcrs			= (1<<16),		/* Disable Carrier Sense During Transmission, 传输过程中关闭载波冲突检测
											   =1: GMAC忽略半双工模式下CRS信号的检测 */
	gmac_ctrl_mii			= (1<<15),		/* Port Select, 0: GMII (1000Mbps), 1: MII (10/100Mbps) */
	gmac_ctrl_fes			= (1<<14),		/* Speed, 0: 10Mbps, 1: 100Mbps */
	gmac_ctrl_do			= (1<<13),		/* Disable Receive Own, 关闭接收自己发出的以太网帧.
											   =1: GMAC不接收半双工模式下gmii_txen_o有效的以太网帧 */
	gmac_ctrl_lm			= (1<<12),		/* Loopback Mode,  =1: GMII/MII工作在环回模式下 */
	gmac_ctrl_dm			= (1<<11),		/* Duplex Mode, 使能全双工模式
											   =1: GMAC工作在全双工模式下, 在全双工模式下可以同时发送和接收以太网帧 */
	gmac_ctrl_ipc			= (1<<10),		/* Checksum Offload, 校验和卸载使能
											   =1: GMAC硬件计算接收到以太网帧的负载(payload). 还检查IPV4头的校验和是否正确 */
	gmac_ctrl_dr			= (1<<9),		/* Disable Retry, 关闭重传
											   =1: GMAC在遇到冲突时不重传发送冲突的以太网帧, 而只报告冲突错误 */
	gmac_ctrl_lud			= (1<<8),		/* Link Up/Down, 0: 链路断开, 1: 链路连接  */
	gmac_ctrl_acs			= (1<<7),		/* Automatic Pad/CRC Stripping, 以太网帧Pad/CRC自动去除
											   =1: GMAC中去除接收到的以太网帧的Pad和FCS */
	gmac_ctrl_bl_mask		= (0x03<<5), 	/* Back-Off Limit - 回退限制, 回退限制决定基于slot的延迟时间 */
	gmac_ctrl_bl_shift		= 5,
	gmac_ctrl_bl_0			= (0<<5),		// 00: k=min(n,10)
	gmac_ctrl_bl_1			= (1<<5),		// 01: k=min(n,8)
	gmac_ctrl_bl_2			= (2<<5),		// 10: k=min(n,4)
	gmac_ctrl_bl_3			= (3<<5),		// 11: k=min(n,1)

	gmac_ctrl_dc			= (1<<4),		/* Deferral Check, =1: 使能deferral检测功能 */
	gmac_ctrl_te			= (1<<3),		/* Transmitter Enable, =1: 使能GMAC传输功能 */
	gmac_ctrl_re			= (1<<2),		/* Receiver Enable, =1: 使能GMAC接收功能 */
};

/*
 * GMAC Frame Filter Register 0x0004
 */
enum GMAC_FrameFilter_R
{
	gmac_frmfilter_ra		= (1<<31),		/* Receive All, =1: GMAC接收模块把接收到的所有帧都发给应用程序, 忽略源地址/目标地址过滤机制 */
	gmac_frmfilter_hpf		= (1<<10),		/* Hash or Perfect Filter, 哈希或者完全过滤
											   =1: 在哈希/完全过滤机制中匹配的以太网帧发送给应用.
											   =0: 只有在哈希过滤机制中匹配的以太网帧才发送给应用.  */
	gmac_frmfilter_saf		= (1<<9),		/* Source Address Filter Enable, 源地址过滤使能
											   GMAC CORE比较比较接收到以太网帧的源地址域和在SA寄存器中的值, 如果匹配, 接收状态寄存器中的SAMatch位设置为高.
											   =1: 源地址匹配失败, GMAC CORE将丢弃该以太网帧.
											   =0: 不管源地址匹配结果GMAC CORE都接收此帧, 而匹配结果写入接收状态寄存器. */
	gmac_frmfilter_saif		= (1<<8),		/* SA Inverse Filtering, 源地址反转过滤.
											   =1: 和SA寄存器中源地址匹配的以太网帧将会标记为源地址匹配失败.
											   =0: 和SA寄存器中源地址不匹配的以太网帧将会标记为源地址匹配失败.  */
	gmac_frmfilter_pcf_mask	 = (0x03<<6),	/* bits: 7-6, Pass Control Frames, 接收控制帧 */
	gmac_frmfilter_pcf_shift = 6,
	gmac_frmfilter_pcf_0	 = (0<<6),		// 00: GMAC过滤所有控制帧;
	gmac_frmfilter_pcf_1	 = (1<<6),		// 01: GMAC接收除了pause帧以外的所有控制帧;
	gmac_frmfilter_pcf_2	 = (2<<6),		// 10: GMAC接收所有控制帧;
	gmac_frmfilter_pcf_3	 = (3<<6),		// 11: GMAC根据地址过滤情况接收控制帧 */

	gmac_frmfilter_dbf		= (1<<5),		/* Disable Broadcast Frames, 关闭广播帧. =1: 过滤所有接收的广播帧. =0: 接收所有广播帧.  */
	gmac_frmfilter_pm		= (1<<4),		/* Pass All Multicast, 接收所有多播帧. =1: 接收所有多播帧. =0: 过滤所有多播帧.  */
	gmac_frmfilter_daif		= (1<<3),		/* DA Inverse Filtering, 目标地址反转过滤.
											   =1: 对单播和多播帧进行反向目标地址匹配.
											   =0: 对单播和多播帧进行正常目标地址匹配.  */
	gmac_frmfilter_hmc		= (1<<2),		/* Hash Multicast, 哈希多播过滤, =1: 对接收到的多播帧根据哈希表的内容进行目标地址过滤 */
	gmac_frmfilter_huc		= (1<<1),		/* Hash Unicast, 哈希单播过滤; =1: 对接收到的单播帧根据哈希表的内容进行目标地址过滤 */
	gmac_frmfilter_pr		= (1<<0),		/* Promiscuous Mode, 混杂模式, =1: 接收所有以太网帧 */
};

/*
 * GMAC Flow Control Register 0x0018
 */
enum GMAC_FlowControl_R
{
	gmac_flowctrl_pt_mask	= (0xFFFF<<16),		/* bits31~16, Pause Time, 暂停时间, 此域保存了需要填入传输控制帧中的暂停时间域 */
	gmac_flowctrl_pt_shift  = 16,
	gmac_flowctrl_plt_mask	= (0x03<<4),		/* bits5~4, Pause Low Threshold, 用于设置暂停时间的阈值 */
	gmac_flowctrl_plt_shift = 4,
	gmac_flowctrl_plt_0		= (0<<4),			/* 00: 暂停时间减少4个时间槽 */
	gmac_flowctrl_plt_1		= (1<<4),			/* 01: 暂停时间减少28个时间槽 */
	gmac_flowctrl_plt_2		= (2<<4),			/* 10: 暂停时间减少144个时间槽 */
	gmac_flowctrl_plt_3		= (3<<4),			/* 11: 暂停时间减少256个时间槽 */
												/* 一个时间槽为在GMII/MII接口上传输512比特或者64字节的时间 */
	gmac_flowctrl_up		= (1<<3),			/* Unicast Pause Frame Detect, 单播的暂停帧探测,
												   =1: GMAC将会根据GMAC地址0指定的本站单播地址来探测暂停帧 */
	gmac_flowctrl_rxfcen	= (1<<2),			/* Receive Flow Control Enable, 接收流控使能,
												   =1: GMAC将会解析接收到的暂停帧, 并且按照暂停帧指定的时间暂停帧的发送 */
	gmac_flowctrl_txfcen	= (1<<1),			/* Transmit Flow Control Enable, 发送流控使能,
												   =1: 在全双工模式下, GMAC使能暂停帧的发送; 在半双工模式下, GMAC使能反压操作.  */
	gmac_flowctrl_fcb_bpa	= (1<<0),			/* Flow Control Busy/Backpressure Activate, 流控忙/反压激活,
												   =1: 在全双工模式下发起暂停控制帧的发送或在半双工模式下启动反压操作 */
};

/*
 * GMAC Interrupt Status Register 0x0038
 */
enum GMAC_IntStatus_R
{
	gmac_intstat_ts				= (1<<9),		/* Set if int generated due to TS (Read Time Stamp Status Register to know details) */
	gmac_intstat_mmcerr			= (1<<7),		/* MMC校验和卸载寄存器产生任何中断产生时, 此位设置为1 */
	gmac_intstat_mmctx			= (1<<6),		/* MMC传输中断寄存器产生任何中断时, 此位设置为1 */
	gmac_intstat_mmcrx			= (1<<5),		/* MMC接收中断寄存器产生任何中断时, 此位设置为1 */
	gmac_intstat_mmc			= (1<<4),		/* MMC中断状态. 7:5的任何位为高时, 此位设置为1 */
	gmac_intstat_pmt			= (1<<3),		/* 电源管理中断状态, 在PowerDown或者WakeUp时, 此位设置为1 */
	gmac_intstat_ancomp			= (1<<2),		/* RGMII PHY接口自动协商完成时, 此位设置为1 */
	gmac_intstat_linkstatus		= (1<<1),		/* RGMII PHY接口的链路状态发生任何变化时, 此位设置为1 */
	gmac_intstat_rgmii			= (1<<0),		/* RGMII PHY接口的链路状态发生任何变化时, 此位设置为1 */
};

/*
 * GMAC Interrupt Mask Register 0x003C
 */
enum GMAC_IntMask_R
{
	gmac_intmask_timestamp		= (1<<9),		/* =1: 禁止时间戳发生的中断 */
	gmac_intmask_pmt			= (1<<3),		/* =1: 禁止电源管理引起的中断 */
	gmac_intmask_ancomp			= (1<<2),		/* =1: 禁止PCS自动协商完成中断 */
	gmac_intmask_linkchange		= (1<<1),		/* =1: 禁止由于PCS链路状态变化引起的中断 */
	gmac_intmask_rgmii			= (1<<0),		/* =1: 禁止RGMII引起的中断 */
};

/*
 * GMAC Address0 High Register x0040
 */

/*
 * GMAC Address0 Low Register 0x0044
 */

/*
 * GMAC Address1 High Register 0x0048
 */

/*
 * GMAC Address1 Low Register 0x004C
 */

/*
 * AN Control Register 0x00C0
 */

/*
 * AN Status Register 0x00C4
 */

/*
 * Auto-Negotiation Advertisement Register 0x00C8
 */

/*
 * Auto-Negotiation Link Partner Ability Register 0x00CC
 */

/*
 * Auto-Negotiation Expansion Register 0x00D0
 */

/*
 * SGMII/RGMII Status Register 0x00D8
 */

/*
 * GMII Address Register 0x0010
 * GMII Data Register 0x0014
 */
enum GMAC_MIICtrl_R
{
	gmac_miictrl_phyaddr_mask	= (0x1F<<11),	/* bits15~11, PHY Address. 此域选择需要访问32个PHY中的哪个 */
	gmac_miictrl_phyaddr_shift	= 11,
	gmac_miictrl_gmiireg_mask	= (0x1F<<6),	/* bits10~6, GMII Register. 此域选择需要访问的的PHY的哪个GMII配置寄存器 */
	gmac_miictrl_gmiireg_shift	= 6,
	gmac_miictrl_csr_mask		= (0x07<<2),	/* bits4~2, CSR Clock Range. 此域决定MDC时钟是clk_csr_i时钟频率比例 */
	gmac_miictrl_csr_shift		= 2,			/* (CR)CSR Clock Range: */
	gmac_miictrl_csr_5			= 0x00000014,	/* 250-300 MHz */
	gmac_miictrl_csr_4			= 0x00000010,	/* 150-250 MHz */
	gmac_miictrl_csr_3			= 0x0000000C,	/* 5-60 MHz   */
	gmac_miictrl_csr_2			= 0x00000008,	/* 20-35 MHz   */
	gmac_miictrl_csr_1			= 0x00000004,	/* 100-150 MHz */
	gmac_miictrl_csr_0			= 0x00000000,	/* 60-100 MHz  */
	gmac_miictrl_wr				= (1<<1),		/* GMII Write. =1: 通过GMII 数据寄存器对PHY进行写操作, =0: 通过GMII数据寄存器对PHY进行读操作 */
	gmac_miictrl_busy			= (1<<0),		/* GMII Busy. 对寄存器4和寄存器5写之前, 此位应为0. 在写寄存器4之前此位必须先置0.
                                                   - 在访问PHY 的寄存器时, 应用程序需要将此位设置为1, 表示GMII接口上有写或者读操作正在进行. */

    gmac_miidata_mask			= 0xFFFF,		/* 此域保存了对PHY进行管理读访问操作的16位数据, 或者对PHY进行管理写访问的16位数据. */
};

//-------------------------------------------------------------------------------------------------
// IEEE1588 Registers of GMAC
//-------------------------------------------------------------------------------------------------

typedef struct
{
	volatile unsigned int stamp_ctrl;				/* 0x0700 Time Stamp Control Register */
	volatile unsigned int sub_second_inc; 			/* 0x0704 Sub-Second Increment Register */
	volatile unsigned int systm_second; 			/* 0x0708 System Time - Seconds Register */
	volatile unsigned int systm_nanosecond; 		/* 0x070C System Time - Nanoseconds Register */
	volatile unsigned int systm_second_upd; 		/* 0x0710 System Time - Seconds Update Register */
	volatile unsigned int systm_nanosecond_upd; 	/* 0x0714 System Time - Nanoseconds Update Register */
	volatile unsigned int ts_addend;	 			/* 0x0718 Time Stamp Addend Register */
	volatile unsigned int tgttm_second; 			/* 0x071C Target Time Seconds Register */
	volatile unsigned int tgttm_nanosecond; 		/* 0x0720 Target Time Nanoseconds Register */
	volatile unsigned int systm_hisecond; 			/* 0x0724 System Time - Higher Word Seconds Register */
	volatile unsigned int ts_status; 				/* 0x0728 Time Stamp Status Register */
} HW_IEEE1588_t;

//-------------------------------------------------------------------------------------------------
// DMA registers of GMAC
//-------------------------------------------------------------------------------------------------

#define GDMA0_BASE			    0x16021000
#define GDMA1_BASE			    0x16031000

typedef struct
{
	volatile unsigned int busmode; 			/* 0x1000 Bus Mode */
	volatile unsigned int txpoll; 			/* 0x1004 Transmit Poll Demand */
	volatile unsigned int rxpoll; 			/* 0x1008 Receive Poll Demand */
	volatile unsigned int rxdesc0; 			/* 0x100C Start of Receive Descriptor List Address */
	volatile unsigned int txdesc0; 			/* 0x1010 Start of Transmit Descriptor List Address */
	volatile unsigned int status; 			/* 0x1014 Status */
	volatile unsigned int control; 			/* 0x1018 Operation Mode */
	volatile unsigned int intenable; 		/* 0x101C Interrupt Enable */
	volatile unsigned int mfbocount; 		/* 0x1020 Missed Frame and Buffer Overflow Counter */
	volatile unsigned int rsv2[9];
	volatile unsigned int curtxdesc; 		/* 0x1048 Current Host Transmit Descriptor */
	volatile unsigned int currxdesc; 		/* 0x104C Current Host Receive Descriptor */
	volatile unsigned int curtxbuf; 		/* 0x1050 Current Host Transmit Buffer Address */
	volatile unsigned int currxbuf; 		/* 0x1054 Current Host Receive Buffer Address */
} HW_GDMA_t;

/*
 * Bus Mode Register of GMAC's DMA 	Offset: 0x00
 */
enum GDMA_BusMode_R
{
	gdma_busmode_fb			= (1<<16),		/* Fixed Burst 定长突发传输长度 *//* 用户不用关心此位设置 */
	gdma_busmode_pr_mask	= (0x03<<14),	/* Rx:Tx priority ratio, RxDMA与TxDMA优先级比例, 在DA位为1时起作用  */
	gdma_busmode_pr_shift   = 14,
	gdma_busmode_pr_1		= (0<<14),		/* (PR)TX:RX DMA priority ratio 1:1 */
	gdma_busmode_pr_2		= (1<<14),		/* (PR)TX:RX DMA priority ratio 2:1 */
	gdma_busmode_pr_3		= (2<<14),		/* (PR)TX:RX DMA priority ratio 3:1 */
	gdma_busmode_pr_4		= (3<<14),		/* (PR)TX:RX DMA priority ratio 4:1 */
	gdma_busmode_pbl_mask	= (0x3f<<8),	/* Programmable Burst Length 可编程突发传输长度 *//* 用户不用关心此位设置 */
	gdma_busmode_pbl_shift	= 8,
	gdma_busmode_pbl_256	= 0x01002000,	/* (DmaBurstLengthx8 | DmaBurstLength32) = 256 */
	gdma_busmode_pbl_128	= 0x01001000,	/* (DmaBurstLengthx8 | DmaBurstLength16) = 128 */
	gdma_busmode_pbl_64		= 0x01000800,	/* (DmaBurstLengthx8 | DmaBurstLength8)  = 64 */
	gdma_busmode_pbl_32		= (1<<13), 		/* Dma burst length = 32 */
	gdma_busmode_pbl_16		= (1<<12), 		/* Dma burst length = 16 */
	gdma_busmode_pbl_8		= (1<<11), 		/* Dma burst length = 8 */
	gdma_busmode_pbl_4		= (1<<10), 		/* Dma burst length = 4 */
	gdma_busmode_pbl_2		= (1<<9),		/* Dma burst length = 2 */
	gdma_busmode_pbl_1		= (1<<8),		/* Dma burst length = 1 */
	gdma_busmode_pbl_0		= 0,			/* Dma burst length = 0 */
	gdma_busmode_desc_8		= (1<<7),		/* Enh Descriptor size, =1: 使用32字节大小的描述符, =0: 使用16字节大小的描述符 */
	gdma_busmode_dsl_mask	= (0x1F<<2),	/* Descriptor Skip Length, 设置2个描述符间的距离. 但此值为0时, 默认为DMA描述符大小 */
	gdma_busmode_dsl_shift  = 2,
	gdma_busmode_dsl_16		= (1<<6),
	gdma_busmode_dsl_8		= (1<<5),
	gdma_busmode_dsl_4		= (1<<4),
	gdma_busmode_dsl_2		= (1<<3),
	gdma_busmode_dsl_1		= (1<<2),
	gdma_busmode_dsl_0		= 0,
	gdma_busmode_das		= (1<<1),		/* DMA Arbitration scheme, =0: 在RxDMA和TxDMA间采用轮转仲裁机制, =1: RxDMA优先级高于TxDMA优先级(PR值) */
	gdma_busmode_swreset	= (1<<0),		/* 此位置高DMA控制器将复位GMAC内部寄存器和逻辑. 当复位结束时该位自动清零 */
};

/*
 * Status Register of GMAC's DMA 	Offset: 0x14
 */
enum GDMA_Status_R
{
	gdma_status_pmti		= (1<<28),		/* GMAC PMT Interrupt, 电源管理模块触发中断. 只读 */
	gdma_status_mmci		= (1<<27),		/* GMAC MMC Interrupt, MMC模块触发中断. 只读 */
	gdma_status_lifi		= (1<<26),		/* GMAC Line interface Interrupt, GMAC模块的PCS或者RGMII模块触发中断. 只读 */
	gdma_status_errbit2		= (1<<25),		/* =1: 描述符访问错误; =0: 数据缓存访问错误 */
	gdma_status_errbit1		= (1<<24),		/* =1: 读传输错误; =0: 写传输错误 */
	gdma_status_errbit0		= (1<<23),		/* =1: TxDMA数据传输过程中发生错误; =0: RxDMA数据传输过程中发生错误 */

	gdma_status_txs_mask	= (0x07<<20),	/* Transmit Process State, 传输过程状态(参考编码表) */
	gdma_status_txs_shift   = 20,
	gdma_status_txs_0		= (0<<20),		// 000: 传输停止; 复位或者停止命令发送
	gdma_status_txs_1		= (1<<20),		// 001: 正在进行; 获取传输描述符
	gdma_status_txs_2		= (2<<20),		// 010: 正在进行; 等待传输状态
	gdma_status_txs_3		= (3<<20),		// 011: 正在进行; 从发送缓存读取数据并发送到传输FIFO(TxFIFO)
	gdma_status_txs_4		= (4<<20),		// 100: 写入时间戳状态
	gdma_status_txs_5		= (5<<20),		// 101: 保留
	gdma_status_txs_6		= (6<<20),		// 110: 挂起; 传输描述符不可用或者传输缓存下溢.
	gdma_status_txs_7		= (7<<20),		// 111: 运行; 关闭传输描述符.

	gdma_status_rxs_mask	= (0x07<<17),	/* bits: 19-17, Receive Process State, 接收过程状态(参考编码表) */
	gdma_status_rxs_shift   = 17,
	gdma_status_rxs_0		= (0<<17),		// 000: 停止; 复位或者接收到停止命令
	gdma_status_rxs_1		= (1<<17),		// 001: 运行; 获取接收描述符.
	gdma_status_rxs_2		= (2<<17),		// 010: 保留;
	gdma_status_rxs_3		= (3<<17),		// 011: 运行; 等待接收包.
	gdma_status_rxs_4		= (4<<17),		// 100: 暂停; 接收描述符不可用.
	gdma_status_rxs_5		= (5<<17),		// 101: 运行; 关闭接收描述符.
	gdma_status_rxs_6		= (6<<17),		// 110: 时间戳写状态.
	gdma_status_rxs_7		= (7<<17),		// 111: 运行; 将包内容从接收缓存传输到系统内存.

	gdma_status_nis			= (1<<16),		/* Normal Interrupt Summary, 正常中断汇总, 提示系统是否存在正常中断 */
	gdma_status_ais			= (1<<15),		/* Abnormal Interrupt Summary, 异常中断汇总, 提示系统是否存在异常中断 */
	gdma_status_erxi		= (1<<14),		/* Early Receive Interrupt, 提前接收中断, 提示DMA控制器已经把包的第一个数据写入接收缓存 */
	gdma_status_fbei		= (1<<13),		/* Fatal Bus Error Interrupt, 总线错误中断, 提示总线错误, 具体信息见[25:23]. 当此位设置后DMA引擎停止总线访问操作 */
	gdma_status_etxi		= (1<<10),		/* Early Transmit Interrupt, 提前发送中断, 提示需要传输的以太网帧已经完全传输到MTL模块中的传输 FIFO */
	gdma_status_rxwt		= (1<<9),		/* Receive Watchdog Timeout, 接收看门狗超时, 提示接收到一个大小超过2048字节的以太网帧 */
	gdma_status_rxstop		= (1<<8),		/* Receive Process Stopped, 接收过程停止 */
	gdma_status_rxbufu		= (1<<7),		/* Receive Buffer Unavailable, 接收缓存不可用 */
	gdma_status_rxi			= (1<<6),		/* Receive Interrupt, 接收中断, 指示帧接收完成, 帧接收的状态信息已经写入接收描述符, 接收处于运行状态 */
	gdma_status_txunf		= (1<<5),		/* Transmit Underflow, 传输缓存下溢, 指示帧发送过程中产生接收缓存下溢 */
	gdma_status_rxovf		= (1<<4),		/* Receive Overflow, 接收缓存上溢, 指示帧接收过程中接收缓存上溢 */
	gdma_status_txjt		= (1<<3),		/* Transmit Jabber Timeout */
	gdma_status_txbufu		= (1<<2),		/* Transmit Buffer Unavailable, 传输缓存不可用, 提示传输列表中的下一个描述符不能被DMA控制器访问 */
	gdma_status_txstop		= (1<<1),		/* Transmit Process Stopped, 传输过程停止 */
	gdma_status_txi			= (1<<0),		/* Transmit Interrupt, 传输完成中断, 提示帧传输完成并且第一个描述符的31位置位 */
};

/*
 * Operation Mode Register of GMAC's DMA 	Offset: 0x18
 */
enum GDMA_Ctrl_R
{
	gdma_ctrl_notdroptcpcse	= (1<<26),		/* 关闭丢弃TCP/IP Checksum错误以太网帧的功能, =1: GMAC将不丢弃checksum错误的以太网帧 */
	gdma_ctrl_rxsf			= (1<<25),		/* Receive Store and Forward, 接收存储转发, =1: MTL模块只接收已经全部存储在接收FIFO中的以太网帧 */
	gdma_ctrl_txsf			= (1<<21),		/* Transmit Store and Forward, 发送存储转发, =1: 帧的发送只在帧的内容已经全部进入MTL的传输FIFO中 */
	gdma_ctrl_ftxfifo		= (1<<20),		/* Flush Transmit FIFO, 冲刷传输FIFO, =1: 传输控制逻辑复位为默认值, 并且会导致发送FIFO里面的数据全部丢失 */
	gdma_ctrl_ttc_mask		= (0x7<<14),	/* Transmit Threshold Control, 传输阈值控制, 当帧大小超过此值时MTL将会传输该帧 */
	gdma_ctrl_ttc_shift		= 14,
	gdma_ctrl_ttc_64		= (0<<14),		// 000: 64  字节
	gdma_ctrl_ttc_128		= (1<<14),		// 001: 128 字节
	gdma_ctrl_ttc_192		= (2<<14),		// 010: 192 字节
	gdma_ctrl_ttc_256		= (3<<14),		// 011: 256 字节
	gdma_ctrl_ttc_40		= (4<<14),		// 100: 40  字节
	gdma_ctrl_ttc_32		= (5<<14),		// 101: 32  字节
	gdma_ctrl_ttc_24		= (6<<14),		// 110: 24  字节
	gdma_ctrl_ttc_16		= (7<<14),		// 111: 16  字节
	gdma_ctrl_txstart		= (1<<13),		/* Start/Stop Transmission Command, =1: 传输进入运行状态, =0: 传输进入停止状态 */

	gdma_rxflowctrl_deact   = 0x00401800,   /* (RFD)Rx flow control deact. threhold             [22]:12:11 */
	gdma_rxflowctrl_deact1K = 0x00000000,   /* (RFD)Rx flow control deact. threhold (1kbytes)   [22]:12:11 */
	gdma_rxflowctrl_deact2K = 0x00000800,   /* (RFD)Rx flow control deact. threhold (2kbytes)   [22]:12:11 */
	gdma_rxflowctrl_deact3K = 0x00001000,   /* (RFD)Rx flow control deact. threhold (3kbytes)   [22]:12:11 */
	gdma_rxflowctrl_deact4K = 0x00001800,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11 */
	gdma_rxflowctrl_deact5K = 0x00400000,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11 */
	gdma_rxflowctrl_deact6K = 0x00400800,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11 */
	gdma_rxflowctrl_deact7K = 0x00401000,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11 */
	gdma_rxflowctrl_act     = 0x00800600,   /* (RFA)Rx flow control Act. threhold               [23]:10:09 */
	gdma_rxflowctrl_act1K   = 0x00000000,   /* (RFA)Rx flow control Act. threhold (1kbytes)     [23]:10:09 */
	gdma_rxflowctrl_act2K   = 0x00000200,   /* (RFA)Rx flow control Act. threhold (2kbytes)     [23]:10:09 */
	gdma_rxflowctrl_act3K   = 0x00000400,   /* (RFA)Rx flow control Act. threhold (3kbytes)     [23]:10:09 */
	gdma_rxflowctrl_act4K   = 0x00000300,   /* (RFA)Rx flow control Act. threhold (4kbytes)     [23]:10:09 */
	gdma_rxflowctrl_act5K   = 0x00800000,   /* (RFA)Rx flow control Act. threhold (5kbytes)     [23]:10:09 */
	gdma_rxflowctrl_act6K   = 0x00800200,   /* (RFA)Rx flow control Act. threhold (6kbytes)     [23]:10:09 */
	gdma_rxflowctrl_act7K   = 0x00800400,   /* (RFA)Rx flow control Act. threhold (7kbytes)     [23]:10:09 */

	gdma_ctrl_enhwfc		= (1<<8),		/* Enable HW flow control, =1: 基于接收FIFO利用率的硬件流控电路生效 */
	gdma_ctrl_ferrf			= (1<<7),		/* Forward Error Frames, 传输错误帧, =1: 接收错误帧(错误帧包括:CRC错误,冲突错误,巨帧,看门狗超时,溢出等) */
	gdma_ctrl_fuszf			= (1<<6),		/* Forward Undersized Frames, =1: 接收FIFO将会接收没有错误但小于64字节的以太网帧 */

	gdma_ctrl_rtc_mask		= (0x3<<3),		/* Receive Threshold Control, 接收阈值控制, 当帧大小超过此值时MTL将会接收该帧 */
	gdma_ctrl_rtc_shift		= 3,
	gdma_ctrl_rtc_64		= (0<<3),		// 00: 64  字节
	gdma_ctrl_rtc_32		= (1<<3),		// 01: 32  字节
	gdma_ctrl_rtc_96		= (2<<3),		// 10: 96  字节
	gdma_ctrl_rtc_128		= (3<<3),		// 11: 128 字节

	gdma_ctrl_txopsecf		= (1<<2),		/* TX Operate on Second Frame, =1: DMA在第一个以太网帧的状态尚未写回时即可以开始处理第二个以太网帧 */
	gdma_ctrl_rxstart		= (1<<1),		/* Start/Stop Receive, =1: 接收进入运行状态, =0: 接收进入停止状态 */
};

/*
 * Interrupt Enable Register of GMAC's DMA 	Offset: 0x1C
 */
enum GDMA_IEN_R
{
	gdma_ienable_nis		= (1<<16),		/* Normal Interrupt Summary Enable, =1: 正常中断使能, =0: 正常中断不使能 */
	gdma_ienable_ais		= (1<<15),		/* Abnormal Interrupt Summary Enable, =1: 非正常中断使能, =0: 非正常中断不使能 */
	gdma_ienable_erxi		= (1<<14),		/* Early Receive Interrupt Enable, 早期接收中断使能, =1: 早期接收中断使能 */
	gdma_ienable_fbei		= (1<<13),		/* Fatal Bus Error Enable, =1: 总线致命错误中断使能 */
	gdma_ienable_etxi		= (1<<10),		/* Early Transmit Interrupt Enable, =1: 使能早期传输中断 */
	gdma_ienable_rxwt		= (1<<9),		/* Receive Watchdog Timeout Enable, =1: 使能接收看门狗超时中断 */
	gdma_ienable_rxstop		= (1<<8),		/* Receive Stopped Enable, =1: 使能接收停止中断 */
	gdma_ienable_rxbufu		= (1<<7),		/* Receive Buffer Unavailable Enable, =1: 使能接收缓冲区不可用中断 */
	gdma_ienable_rxi		= (1<<6),		/* Receive Interrupt Enable, =1: 使能接收完成中断 */
	gdma_ienable_txunf		= (1<<5),		/* Underflow Interrupt Enable, =1: 使能传输FIFO下溢中断 */
	gdma_ienable_rxovf		= (1<<4),		/* Overflow Interrupt Enable, =1: 使能接收FIFO上溢中断 */
	gdma_ienable_txjt		= (1<<3),		/* Transmit Jabber Timeout Enable, =1: 使能Jabber超时中断 */
	gdma_ienable_txbufu		= (1<<2),		/* Transmit Buffer Unavailable Enable, =1: 使能传输缓存不可用中断 */
	gdma_ienable_txstop		= (1<<1),		/* Transmit Stopped Enable, =1: 使能传输停止中断 */
	gdma_ienable_txi		= (1<<0),		/* Transmit Interrupt Enable, =1: 使能传输完成中断 */

	gdma_ien_base = (gdma_ienable_nis | gdma_ienable_ais | gdma_ienable_fbei),
	gdma_ien_rx	  = (gdma_ienable_rxstop | gdma_ienable_rxi | gdma_ienable_rxbufu),
	gdma_ien_tx	  = (gdma_ienable_txstop | gdma_ienable_txi | gdma_ienable_txbufu | gdma_ienable_txunf),
};

//-------------------------------------------------------------------------------------------------
// DMA Descriptor
//-------------------------------------------------------------------------------------------------

#define ENH_DESC        1

/*
 * Receive Descriptor RDES0 - Status
 */
enum RDESC0_Status
{
	rxdesc0_stat_own			= (1<<31),		/* OWN, =1: 描述符当前属于DMA控制, =0: 属于主机控制. 当DMA模块完成一次传输时, 会将该位主动清0 */
	rxdesc0_stat_afm			= (1<<30),		/* Destination Address Filter Fail, 目标地址过滤错误. =1: 当前数据帧目标地址不符合GMAC内部的帧目标地址过滤器 */
	rxdesc0_stat_fl_mask    	= (0x3FFF<<16), /* bit29~16, Frame length 帧长度, 表示接收当前帧的长度, 当ES位为0时有效 */
	rxdesc0_stat_fl_shift   	= 16,
	rxdesc0_stat_es      		= (1<<15),		/* Error Summary 总体错误信息, 指示当前帧是否出错, 其值为RDES[0,1,3,4,6,7,11,14]各位作或运算(OR)的结果 */
	rxdesc0_stat_de				= (1<<14),		/* Descriptor Error 描述符错误, =1: 当前描述符所指向的buffer与帧不相符或者OWN为0(主机控制) */
	rxdesc0_stat_saf			= (1<<13),		/* Source Address Filter Fail 源地址过滤错误, =1: 当前数据帧的源地址不符合GMAC内部的帧源地址过滤器 */
	rxdesc0_stat_le				= (1<<12),		/* Length Error 长度错误, =1: 当前接收帧长度与默认长度不符. 当Frame Type位为1且CRC Error位为0时有效 */
	rxdesc0_stat_oe				= (1<<11),		/* Over Flow Error 溢出错误, =1: 接收该帧时GMAC内部RxFIFO溢出 */
	rxdesc0_stat_vlan			= (1<<10),		/* VLAN Tag VLAN标志, =1: 该帧的类型为VLAN */
	rxdesc0_stat_fs				= (1<<9),		/* First Desciptor 第一个描述符, =1: 当前描述符所指向的buffer为当前接收帧的第一个保存buffer */
	rxdesc0_stat_ls				= (1<<8),		/* Last Desciptor 最后一个描述符, =1: 当前描述符所指向的buffer为当前接收帧的最后一个保存buffer */
	rxdesc0_stat_ipce_gf		= (1<<7),		/* IPC Checksum Error/Giant Frame 校验错误/超长帧.
	 	 	 	 	 	 	 	 	 	 	 	   =1: 如果IPC校验功能启用则表示当前帧的IPv4头校验值与帧内部校验域的值不相符.
	 	 	 	 	 	 	 	 	 	 	 	 	 - 如果未启用则表示当前帧为一个超长帧(长度大于1518字节) */
	rxdesc0_stat_lc				= (1<<6),		/* Late Collision 后期冲突, =1: 在半双工模式下, 当前帧接收时发生了一个后期冲突 */
	rxdesc0_stat_ft				= (1<<5),		/* Frame Type 帧类型, =1: 当前帧为一个以太网格式帧, =0: 当前帧为一个IEEE802.3格式帧 */
	rxdesc0_stat_rwt			= (1<<4),		/* Receive Watchdog Timeout, =1: 当前时钟值超过了接收模块看门狗电路时钟的值, 既接收帧超时 */
	rxdesc0_stat_re				= (1<<3),		/* Receive Error 接收错误, =1: 接收当前帧时内部模块出错. 内部信号rxer置1且rxdv置1 */
	rxdesc0_stat_dbe			= (1<<2),		/* Dribble bit Error 奇数位错误, =1: 接收帧长度不是整数, 即总长度为奇数位, 该位只有在mii模式下有效 */
	rxdesc0_stat_ce				= (1<<1),		/* CRC Error 接收CRC校验错误, =1: 接收当前帧时内部CRC校验出错. 该位只有在last descriptor(RDES0[8])为1时有效 */
#if ENH_DESC
    rxdesc0_stat_extsts   		= (1<<0),   	/* Extended Status Available (RDES4) */
#else
	rxdesc0_stat_rmpce			= (1<<0),		/* RX GMAC Checksum/payload Checksum Error 接受校验/负载校验错误.
												   =1: 接收当前帧时内部RX GMAC寄存器组1-15中存在一个匹配当前帧目的地址.
												   =0: RX GMAC 寄存器组0匹配接受帧目的地址. 如果Full Checksum Offload Engine启用时,
												   =1: 该帧TCP/UDP/ICMP校验错误. 该位为1时也可能表示当前帧实际接受长度与帧内部记载长度不相符. */
#endif
};

/*
 * Receive Descriptor RDES1 - Control, Address
 */
enum RDESC1_Control
{
	rxdesc1_ctrl_di				= (1<<31),		/* (Disable Rx int on completion) */
	rxdesc1_ctrl_UDF    		= (1<<30),
#if ENH_DESC
	rxdesc1_ctrl_bs2_mask	    = 0x1FFF0000,   /* (TBS2) 	Buffer 2 size, [28:16] */
	rxdesc1_ctrl_bs2_shift      = 16,
	rxdesc1_ctrl_rer       	    = (1<<15),   	/* (RER)	End of descriptors ring */
	rxdesc1_ctrl_rch            = (1<<14),   	/* (RCH)	Second buffer address is chain address */
	rxdesc1_ctrl_bs1_mask	    = 0x00001FFF,   /* (TBS1) 	Buffer 1 size, [12:0] */
#else
	rxdesc1_ctrl_rer			= (1<<25),		/* Receive End of Ring, =1: 该描述符为环型描述符链表的最后一个, 下一个描述符的地址为接收描述符链的基址 */
	rxdesc1_ctrl_rch			= (1<<24),		/* Second Address Chained, =1: 描述符中的第二个buffer地址指向的是下一个描述符的地址 */
	rxdesc1_ctrl_bs2_mask		= (0x07FF<<11),	/* bits: 21-11, Receive Buffer Size 2, 该域表示数据buffer2的大小 */
	rxdesc1_ctrl_bs2_shift	 	= 11,
	rxdesc1_ctrl_bs1_mask		= (0x07FF<<0),	/* bits: 10-0, Receive Buffer Size 1, 该域表示数据buffer1的大小 */
#endif
};

/*
 * Transmit Descriptor TDES0 - Status
 */
enum TDESC0_Status
{
    txdesc0_stat_own			= (1<<31),
#if ENH_DESC
	txdesc0_stat_ic 			= (1<<30),		/* (IC)		Tx - interrupt on completion */
	txdesc0_stat_ls				= (1<<29),		/* (LS)		Tx - Last segment of the frame */
	txdesc0_stat_fs				= (1<<28),		/* (FS)		Tx - First segment of the frame */
	txdesc0_stat_dc				= (1<<27),		/* (DC)		Tx - Add CRC disabled (first segment only) */
	txdesc0_stat_dp				= (1<<26),		/* (DP)		Tx - Disable padding */
	txdesc0_stat_tten			= (1<<25),		/* Time Stamp Enable */
	txdesc0_stat_cic_mask		= 0x00c00000,   /* Tx checksum offloading control mask [23:22] */
	txdesc0_stat_cic_shift		= 22,
	txdesc0_stat_cic_bypass		= 0x00000000,   /* Checksum bypass */
	txdesc0_stat_ipv4	  		= 0x00400000,	/* IPv4 header checksum */
	txdesc0_stat_tcp			= 0x00800000,	/* TCP/UDP/ICMP checksum. Pseudo header checksum is assumed to be present */
	txdesc0_stat_full			= 0x00c00000,	/* TCP/UDP/ICMP checksum fully in hardware including pseudo header */
	txdesc0_stat_ter		    = (1<<21),		/* (TER)End of descriptors ring */
	txdesc0_stat_tch			= (1<<20),   	/* (TCH)Second buffer address is chain address */
#endif
	txdesc0_stat_UDF    		= (1<<17),
	txdesc0_stat_ihe			= (1<<16),		/* IP Header Error, =1: 表示内部校验模块发现该发送帧的IP头出错, 并且不会对该域做任何修改 */
	txdesc0_stat_es				= (1<<15),		/* Error Summary, 指示当前帧是否出错, 其值为TDES[1,2,8,9,10,11,13,14]各位作或运算(OR)的结果 */
	txdesc0_stat_jt				= (1<<14),		/* Jabber Timeout, =1: 表示GMAC发送模块遇到了Jabber超时 */
	txdesc0_stat_ff				= (1<<13),		/* Frame Flushed, =1: 表示软件发出了一个刷新命令导致DMA/MTL将其内部的帧刷新掉 */
	txdesc0_stat_pce			= (1<<12),		/* Payload Checksum Error, =1: 表示内部负载校验模块再向发送帧中插入校验数据时出错. 当负载校验模块启用时, 该位有效 */
	txdesc0_stat_lc				= (1<<11),		/* Loss of Carrier, =1: 表示在发送该帧过程中载波丢失(gmii_crs信号多个周期未置起) */
	txdesc0_stat_nc				= (1<<10),		/* No Carrier, =1: 表示在发送过程中, PHY的载波信号一直未置起 */
	txdesc0_stat_lco			= (1<<9),		/* Late Collision, =1: 表示在半双工模式下, 当前帧接收时发生了一个后期冲突 */
	txdesc0_stat_ec				= (1<<8),		/* Excessive Collison, =1: 表示在发送当前帧的时候连续出现了16次冲突 */
	txdesc0_stat_vf				= (1<<7),		/* VLAN Frame, =1: 表示当前发送帧为一个VLAN帧 */
	txdesc0_stat_cc_mask		= (0x0F<<3),	/* bits: 6-3, Collsion Count, 该域表示当前帧在成功发送之前所遇到冲突次数的总数 */
	txdesc0_stat_cc_shift   	= 3,
	txdesc0_stat_ed				= (1<<2),		/* Excessive Deferral, =1: 表示当前帧传输结束 */
	txdesc0_stat_uf				= (1<<1),		/* Underflow Error, =1: 表示当前帧传输时发生了溢出错误, 即数据传输buffer过小或不可用 */
	txdesc0_stat_db				= (1<<0),		/* Defered Bit, =1: 表示此次发送被延迟, 只有在半双工模式下有效 */
};

/*
 * Transmit Descriptor TDES1 - Control, Address
 */
enum TDESC1_Control
{
#if ENH_DESC
	txdesc1_ctrl_bs2_mask	    = 0x1FFF0000,   /* (TBS2)     Buffer 2 size, [28:16] */
	txdesc1_ctrl_bs2_shift      = 16,
	txdesc1_ctrl_bs1_mask	    = 0x00001FFF,   /* (TBS1)     Buffer 1 size, [12:0] */
#else
	txdesc1_ctrl_ic				= (1<<31),		/* Interrption on Complete, =1: 表示该帧接发送完成后将会置起STATUS寄存器中TI位(CSR5[0]) */
	txdesc1_ctrl_ls				= (1<<30),		/* Last Segment, =1: 表示当前buffer包含的是一帧数据的最后一段(如果帧分为多个段) */
	txdesc1_ctrl_fs				= (1<<29),		/* First Segment, =1: 表示当前buffer包含的是一帧数据的第一段(如果帧分为多个段) */
	txdesc1_ctrl_cic_mask		= (0x02<<27),	/* bits: 28-27, Checksum Insertion Control, 该域控制内部模块是否在发送帧中填充校验数据 */
	txdesc1_ctrl_cic_shift  	= 27,
	txdesc1_ctrl_cic_ipv4		= (1<<27),
	txdesc1_ctrl_cic_tcp		= (1<<28),
	txdesc1_ctrl_cic_all		= (0x02<<27),
	txdesc1_ctrl_dc				= (1<<26),		/* =1, Disable CRC, =1: GMAC硬件不在每个发送帧的结尾添加CRC校验数据 */
	txdesc1_ctrl_ter			= (1<<25),		/* Transmit End of Ring, =1: 表示该描述符为环型描述符链表的最后一个, 下一个描述符的地址为发送描述符链的基址 */
	txdesc1_ctrl_tch			= (1<<24),		/* Second Address Chained, =1: 表示描述符中的第二个buffer地址指向的是下一个描述符的地址  */
	txdesc1_ctrl_dp				= (1<<23),		/* Dissable Pading, =1: 表示GMAC将不会对长度小于64字节的数据包进行空数据填充 */
	txdesc1_ctrl_ttse			= (1<<22),		/* Transmit Time Stamp Enable, =1: 表示将启用内部模块计算IEEE1588硬件时间戳计算, 在TDES1[29]为1时有效 */
	txdesc1_ctrl_bs2_mask		= (0x07FF<<11),	/* bits: 21-11, Transmit Buffer Size 2, 该域表示数据buffer2的大小. 当TDES1[24]为1时, 该域无效 */
	txdesc1_ctrl_bs2_shift 	    = 11,
	txdesc1_ctrl_bs1_mask		= (0x07FF<<0),	/* bits: 10-0, Transmit Buffer Size 1, 该域表示数据buffer1的大小. 该域一直有效 */
#endif
};

/*
 * GMAC DMA Receive and Transmit Descriptor
 * __attribute__((packed))的作用: 告诉编译器取消结构在编译过程中的优化对齐,按照实际占用字节数进行对齐---是GCC特有的语法
 */

typedef struct
{
	volatile unsigned int status;     	/* Status */
	volatile unsigned int control;		/* 31-22: Control; 21-11: Buffer 2 lenght; 10-0: Buffer 1 length */
	volatile unsigned int bufptr;		/* Buffer 1 pointer */
	volatile unsigned int nextdesc;		/* Next descriptor pointer (Dma-able) in chain structure */
	volatile unsigned int tmp0;
	volatile unsigned int tmp1;
	volatile unsigned int tmp2;
	volatile unsigned int tmp3;
} GDMA_DESC_t; // __attribute__((packed));

typedef GDMA_DESC_t 	RX_DESC_t;
typedef GDMA_DESC_t 	TX_DESC_t;

#if 0
struct dma_desc
{
	/* Receive descriptor */
	union
	{
		struct
		{
			/* RDES0 */
			unsigned int reserved1:1;
			unsigned int crc_error:1;
			unsigned int dribbling:1;
			unsigned int mii_error:1;
			unsigned int receive_watchdog:1;
			unsigned int frame_type:1;
			unsigned int collision:1;
			unsigned int frame_too_long:1;
			unsigned int last_descriptor:1;
			unsigned int first_descriptor:1;
			unsigned int multicast_frame:1;
			unsigned int run_frame:1;
			unsigned int length_error:1;
			unsigned int partial_frame_error:1;
			unsigned int descriptor_error:1;
			unsigned int error_summary:1;
			unsigned int frame_length:14;
			unsigned int filtering_fail:1;
			unsigned int own:1;

			/* RDES1 */
			unsigned int buffer1_size:11;
			unsigned int buffer2_size:11;
			unsigned int reserved2:2;
			unsigned int second_address_chained:1;
			unsigned int end_ring:1;
			unsigned int reserved3:5;
			unsigned int disable_ic:1;
		} rx;

		struct		/* -- enhanced -- */
		{
			/* RDES0 */
			unsigned int payload_csum_error:1;
			unsigned int crc_error:1;
			unsigned int dribbling:1;
			unsigned int error_gmii:1;
			unsigned int receive_watchdog:1;
			unsigned int frame_type:1;
			unsigned int late_collision:1;
			unsigned int ipc_csum_error:1;
			unsigned int last_descriptor:1;
			unsigned int first_descriptor:1;
			unsigned int vlan_tag:1;
			unsigned int overflow_error:1;
			unsigned int length_error:1;
			unsigned int sa_filter_fail:1;
			unsigned int descriptor_error:1;
			unsigned int error_summary:1;
			unsigned int frame_length:14;
			unsigned int da_filter_fail:1;
			unsigned int own:1;

			/* RDES1 */
			unsigned int buffer1_size:13;
			unsigned int reserved1:1;
			unsigned int second_address_chained:1;
			unsigned int end_ring:1;
			unsigned int buffer2_size:13;
			unsigned int reserved2:2;
			unsigned int disable_ic:1;
		} erx;		/* -- enhanced -- */

		/* Transmit descriptor */
		struct
		{
			/* TDES0 */
			unsigned int deferred:1;
			unsigned int underflow_error:1;
			unsigned int excessive_deferral:1;
			unsigned int collision_count:4;
			unsigned int heartbeat_fail:1;
			unsigned int excessive_collisions:1;
			unsigned int late_collision:1;
			unsigned int no_carrier:1;
			unsigned int loss_carrier:1;
			unsigned int reserved1:3;
			unsigned int error_summary:1;
			unsigned int reserved2:15;
			unsigned int own:1;
			/* TDES1 */
			unsigned int buffer1_size:11;
			unsigned int buffer2_size:11;
			unsigned int reserved3:1;
			unsigned int disable_padding:1;
			unsigned int second_address_chained:1;
			unsigned int end_ring:1;
			unsigned int crc_disable:1;
			unsigned int reserved4:2;
			unsigned int first_segment:1;
			unsigned int last_segment:1;
			unsigned int interrupt:1;
		} tx;

		struct 		/* -- enhanced -- */
		{
			/* TDES0 */
			unsigned int deferred:1;
			unsigned int underflow_error:1;
			unsigned int excessive_deferral:1;
			unsigned int collision_count:4;
			unsigned int vlan_frame:1;
			unsigned int excessive_collisions:1;
			unsigned int late_collision:1;
			unsigned int no_carrier:1;
			unsigned int loss_carrier:1;
			unsigned int payload_error:1;
			unsigned int frame_flushed:1;
			unsigned int jabber_timeout:1;
			unsigned int error_summary:1;
			unsigned int ip_header_error:1;
			unsigned int time_stamp_status:1;
			unsigned int reserved1:2;
			unsigned int second_address_chained:1;
			unsigned int end_ring:1;
			unsigned int checksum_insertion:2;
			unsigned int reserved2:1;
			unsigned int time_stamp_enable:1;
			unsigned int disable_padding:1;
			unsigned int crc_disable:1;
			unsigned int first_segment:1;
			unsigned int last_segment:1;
			unsigned int interrupt:1;
			unsigned int own:1;

			/* TDES1 */
			unsigned int buffer1_size:13;
			unsigned int reserved3:3;
			unsigned int buffer2_size:13;
			unsigned int reserved4:3;
		} etx;		/* -- enhanced -- */
	} des01;
	unsigned int des2;
	unsigned int des3;
};

/* Transmit checksum insertion control */
enum tdes_csum_insertion
{
	cic_disabled        = 0,	/* Checksum Insertion Control */
	cic_only_ip         = 1,	/* Only IP header */
	cic_no_pseudoheader = 2,	/* IP header but pseudoheader is not calculated */
	cic_full            = 3,	/* IP header and pseudoheader */
};
#endif

//-------------------------------------------------------------------------------------------------
// MII
//-------------------------------------------------------------------------------------------------

/*
 * mii status defination
 */
typedef enum
{
	LINKDOWN	= 0,
	LINKUP		= 1,
} mii_sr_t;

typedef enum
{
	HALFDUPLEX	= 1,
	FULLDUPLEX	= 2,
} link_sr_t;

typedef enum
{
	SPEED10     = 1,
	SPEED100    = 2,
	SPEED1000   = 3,
} speed_t;

/*
 * mac-dma desc mode
 */
typedef enum
{
	RINGMODE	= 0,
	CHAINMODE	= 1,
} desc_mode_t;

//-------------------------------------------------------------------------------------------------
// macro for operate
//-------------------------------------------------------------------------------------------------

/*
 * start gmac-dma
 */
#define GDMA_START_RX(hwGDMA) do { hwGDMA->control |= gdma_ctrl_rxstart; } while (0)
#define GDMA_START_TX(hwGDMA) do { hwGDMA->control |= gdma_ctrl_txstart; } while (0)
#define GDMA_START(hwGDMA)    do { hwGDMA->control |= (gdma_ctrl_txstart | gdma_ctrl_rxstart); } while (0)

/*
 * stop gmac-dma
 */
#define GDMA_STOP_RX(hwGDMA) do { hwGDMA->control &= ~gdma_ctrl_rxstart; } while (0)
#define GDMA_STOP_TX(hwGDMA) do { hwGDMA->control &= ~gdma_ctrl_txstart; } while (0)
#define GDMA_STOP(hwGDMA)    do { hwGDMA->control &= ~(gdma_ctrl_txstart | gdma_ctrl_rxstart); } while (0)

/*
 * gmac-dma interrupt
 */
#define GDMA_INT_EN(hwGDMA)  do { hwGDMA->intenable = (gdma_ien_base | gdma_ien_rx | gdma_ien_tx); } while (0)
#define GDMA_INT_DIS(hwGDMA) do { hwGDMA->intenable = 0; } while (0)

/*
 * gmac start
 */
#define GMAC_START_TX(hwGMAC) do { hwGMAC->config |= gmac_ctrl_te; } while (0)
#define GMAC_START_RX(hwGMAC) do { hwGMAC->config |= gmac_ctrl_re; } while (0)
#define GMAC_START(hwGMAC)    do { hwGMAC->config |= (gmac_ctrl_te | gmac_ctrl_re); } while (0)

/*
 * gmac stop
 */
#define GMAC_STOP_TX(hwGMAC) do { hwGMAC->config &= ~gmac_ctrl_te; } while (0)
#define GMAC_STOP_RX(hwGMAC) do { hwGMAC->config &= ~gmac_ctrl_re; } while (0)
#define GMAC_STOP(hwGMAC)    do { hwGMAC->config &= ~(gmac_ctrl_te | gmac_ctrl_re); } while (0)

#ifdef __cplusplus
}
#endif

#endif // _LS2K_GMAC_HW_H

/*
 * @@ END
 */


