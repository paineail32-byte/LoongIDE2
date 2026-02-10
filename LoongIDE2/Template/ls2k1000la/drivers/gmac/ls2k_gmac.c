/*
 * Copyright (C) 2021-2022 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_gmac.c
 *
 * created: 2022-03-10
 *  author: Bian
 */

/*
 * 1000M 全双工网卡
 */

#include "bsp.h"

#if BSP_USE_GMAC

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <larchintrin.h>

#include "cpu.h"
#include "ls2k1000.h"
#include "ls2k1000_irq.h"

#include "osal.h"

#include "mii.h"

#include "ls2k_drv_io.h"
#include "drv_os_priority.h"

#include "ls2k_gmac_hw.h"
#include "ls2k_gmac.h"

//***************************************************************************************

#define GMAC_DEBUG              0               // 调试
#define GMAC_TRACE_INFO         1               // print some message

//***************************************************************************************

#ifndef ETHER_MAX_LEN
#define ETHER_MAX_LEN           1518            // should defined by tcp/ip stack
#endif
#define MAX_BUF_SIZE			1536			// 48*32, reference by ETHER_MAX_LEN=1518

#define DMA_DESC_SIZE           (sizeof(GDMA_DESC_t))

#define NUM_TX_DMA_DESC 		16				// TX 描述符个数, 和 TX 缓冲区个数一致
#define NUM_RX_DMA_DESC 		16				// RX 描述符个数, 和 RX 缓冲区个数一致

#define TX_BUF_SIZE				MAX_BUF_SIZE	// TX 缓冲区大小
#define RX_BUF_SIZE				MAX_BUF_SIZE	// RX 缓冲区大小

#define MII_WR_TIMEOUT          0x0200  		// mii  write timeout
#define MII_RD_TIMEOUT          0x0200  		// mii  read  timeout
#define GDMA_RESET_TIMEOUT      0x0200  		// gdma reset timeout

//***************************************************************************************
// Hardware-specific storage, 驱动专用硬件相关的控制结构
//***************************************************************************************

typedef struct
{
	HW_GMAC_t    *hwGMAC;                       /* GMAC Hardware */
	HW_GDMA_t    *hwGDMA;
	unsigned int  phyAddr;					    /* PHY Address */
	unsigned int  gmacVersion;				    /* 设备版本号 */
    unsigned int  irqNum;	                    /* GMAC Interrupt Registers */

	int			  unitNumber;				    /* 设备号 */
    int           acceptBroadcast;			    /* Indicates configuration */
    int           autoNegotiation;              /* 是否自动协商 */
    int           autoNegoTimeout;              /* 自动协商超时 ms */

	unsigned int  LinkState;					/* Link status as reported by the Phy */
	unsigned int  DuplexMode;					/* Duplex mode of the Phy */
	unsigned int  Speed;						/* Speed of the Phy */

	unsigned int  descmode;					    /* DMA Desc Mode  chain or ring */
	RX_DESC_t *rx_desc[NUM_RX_DMA_DESC];	    /* RX 描述符指针数组 */
	TX_DESC_t *tx_desc[NUM_TX_DMA_DESC];	    /* TX 描述符指针数组 */
	int			  rx_head;
	int			  rx_tail;
	int			  tx_head;
	int			  tx_tail;

	unsigned char *rx_buf[NUM_RX_DMA_DESC];	    /* RX 缓冲区指针数组 */
	unsigned char *tx_buf[NUM_TX_DMA_DESC];	    /* TX 缓冲区指针数组 */

	osal_event_t  p_event;

	unsigned int  interrupts;	                /* Statistics 统计数 */
	unsigned int  dma_normal_intrs;
	unsigned int  dma_abnormal_intrs;
	unsigned int  dma_fatal_err;
	unsigned int  rx_interrupts;
	unsigned int  tx_interrupts;

	unsigned int  rx_pkts;
	unsigned int  rx_buffer_unavailable;
	unsigned int  rx_stopped;
	unsigned int  rx_errors;
	unsigned int  rx_length_err;
	unsigned int  rx_crc_err;
	unsigned int  rx_dropped;

	unsigned int  tx_pkts;
	unsigned int  tx_buffer_unavailable;
	unsigned int  tx_stopped;
	unsigned int  tx_errors;
	unsigned int  tx_ipheader_err;
	unsigned int  tx_playload_err;
	unsigned int  tx_defered;
	unsigned int  tx_collsions;
	unsigned int  tx_underflow;

    int           timeout;                  /* 收发超时 */
	int			  initialized;		        /* 是否初始化 */
	int           started;		            /* 是否启动 */
    char          dev_name[16];		        /* 设备名称 */
} GMAC_t;

//***************************************************************************************
// ls2k gmac device and buffer
//***************************************************************************************

#define __ALIGN(x)      __attribute__((aligned(x)))

#if BSP_USE_GMAC0
static GMAC_t ls2k_GMAC0 =
{
    .hwGMAC      = 0,
	.hwGDMA      = 0,
    .irqNum      = INTC0_GMAC0_SBD_IRQ,
    .unitNumber  = 0,
	.descmode    = CHAINMODE,       // XXX This is important
    .timeout     = 0,
    .started     = 0,
    .initialized = 0,
    .dev_name    = "gmac0",
	.acceptBroadcast = 0,
    .autoNegotiation = 1,
    .autoNegoTimeout = 5000,
};
const void *devGMAC0 = (void *)&ls2k_GMAC0;

static unsigned char tx_desc_0[(NUM_TX_DMA_DESC*DMA_DESC_SIZE)] __ALIGN(DMA_DESC_SIZE);
static unsigned char rx_desc_0[(NUM_RX_DMA_DESC*DMA_DESC_SIZE)] __ALIGN(DMA_DESC_SIZE);
static unsigned char tx_buf_0[(NUM_TX_DMA_DESC*TX_BUF_SIZE)] __ALIGN(32);
static unsigned char rx_buf_0[(NUM_RX_DMA_DESC*RX_BUF_SIZE)] __ALIGN(32);
#endif

#if BSP_USE_GMAC1
static GMAC_t ls2k_GMAC1 =
{
    .hwGMAC      = 0,
	.hwGDMA      = 0,
    .irqNum      = INTC0_GMAC1_SBD_IRQ,
    .unitNumber  = 1,
	.descmode    = CHAINMODE,       // XXX This is important
    .timeout     = 0,
    .started     = 0,
    .initialized = 0,
    .dev_name    = "gmac1",
	.acceptBroadcast = 0,
    .autoNegotiation = 1,
    .autoNegoTimeout = 5000,
};
const void *devGMAC1 = (void *)&ls2k_GMAC1;

static unsigned char tx_desc_1[(NUM_TX_DMA_DESC*DMA_DESC_SIZE)] __ALIGN(DMA_DESC_SIZE);
static unsigned char rx_desc_1[(NUM_RX_DMA_DESC*DMA_DESC_SIZE)] __ALIGN(DMA_DESC_SIZE);
static unsigned char tx_buf_1[(NUM_TX_DMA_DESC*TX_BUF_SIZE)] __ALIGN(32);
static unsigned char rx_buf_1[(NUM_RX_DMA_DESC*RX_BUF_SIZE)] __ALIGN(32);
#endif

/*
 * GMAC IO base
 */
static void ls2k1000_gmac_init_io_base(GMAC_t *pMAC)
{
    unsigned long gmac_address0;

#if (BSP_USE_GMAC0)
    if (pMAC == &ls2k_GMAC0)
    {
        gmac_address0 = READ_REG64(GMAC0_CFG_HEAD_BASE + 0x10) & ~0x0Ful;
        ls2k_GMAC0.hwGMAC = (HW_GMAC_t *)PHYS_TO_UNCACHED(gmac_address0);
        ls2k_GMAC0.hwGDMA = (HW_GDMA_t *)PHYS_TO_UNCACHED(gmac_address0 + 0x1000);
    }
#endif

#if (BSP_USE_GMAC1)
    if (pMAC == &ls2k_GMAC1)
    {
        gmac_address0 = READ_REG64(GMAC1_CFG_HEAD_BASE + 0x10) & ~0x0Ful;
        ls2k_GMAC1.hwGMAC = (HW_GMAC_t *)PHYS_TO_UNCACHED(gmac_address0);
        ls2k_GMAC1.hwGDMA = (HW_GDMA_t *)PHYS_TO_UNCACHED(gmac_address0 + 0x1000);
    }
#endif
}

//***************************************************************************************
// function prototypes
//***************************************************************************************

static int ls2k_init_tx_desc_queue(GMAC_t *pMAC);
static int ls2k_init_rx_desc_queue(GMAC_t *pMAC);

static void ls2k_gmac_init_hw(GMAC_t *pMAC, unsigned char *macAddr);
static void ls2k_gmac_do_reset(GMAC_t *pMAC);

static void ls2k_gmac_irq_handler(int vector, void *arg);

//***************************************************************************************
// functions for gdma desc
//***************************************************************************************

static inline void GDMA_INIT_RXDESC(GMAC_t *pMAC)
{
	ls2k_init_rx_desc_queue(pMAC);
	pMAC->rx_head = 0;
	pMAC->rx_tail = 0;
}

static inline void GDMA_INIT_TXDESC(GMAC_t *pMAC)
{
	ls2k_init_tx_desc_queue(pMAC);
	pMAC->tx_head = 0;
	pMAC->tx_tail = 0;
}

static inline void GDMA_INIT_DESC(GMAC_t *pMAC)
{
	GDMA_INIT_RXDESC(pMAC);
	GDMA_INIT_TXDESC(pMAC);
}

static inline void GDMA_INIT_RXDESC_CUR(GMAC_t *pMAC)
{
	int i;
    unsigned int val;
	val = pMAC->hwGDMA->currxdesc;
	for (i=0; i<NUM_RX_DMA_DESC; i++)
	{
		if (val == VA_TO_PHYS(pMAC->rx_desc[i]))
		{
			pMAC->rx_head = i;
			pMAC->rx_tail = i;
			break;
		}
	}
}

static inline void GDMA_INIT_TXDESC_CUR(GMAC_t *pMAC)
{
	int i;
    unsigned int val;
	val = pMAC->hwGDMA->curtxdesc;
	for (i=0; i<NUM_TX_DMA_DESC; i++)
	{
		if (val == VA_TO_PHYS(pMAC->tx_desc[i]))
		{
			pMAC->tx_head = i;
			pMAC->tx_tail = i;
			break;
		}
	}
}

static inline void GDMA_INIT_DESC_CUR(GMAC_t *pMAC)
{
	GDMA_INIT_RXDESC_CUR(pMAC);
	GDMA_INIT_TXDESC_CUR(pMAC);
}

//***************************************************************************************
// MII r/w interface
//***************************************************************************************

static void mii_read_phy(GMAC_t *pMAC, unsigned char phyReg, unsigned short *val);

/*
 * detect phy addr
 */
static unsigned short mii_detect_phy_addr(GMAC_t *pMAC, unsigned int *id)
{
    unsigned char i;

    *id = 0xFFFFFFFF;

    for (i=0; i<32; i++)
    {
        unsigned short id1, id2;

        mii_read_phy(pMAC, MII_PHYIDR1, &id1);
        mii_read_phy(pMAC, MII_PHYIDR2, &id2);

        /*
         * PHYID1/PHYID2
         */
        if (((id1 != 0) && (id1 != 0xFFFF)) || ((id2 != 0) && (id2 != 0xFFFF)))
        {
            *id = (unsigned int)(id1 << 16) | id2;
        #if GMAC_DEBUG
            DBG_OUT("detect phy id=0x%08X, @phyaddr %i\r\n", *id, i);
        #endif
            return (unsigned short)i;
        }
    }

    return pMAC->phyAddr;
}

static void mii_write_phy(GMAC_t *pMAC, unsigned char phyReg, unsigned short val)
{
	int i;
    unsigned int phyVal;

    phyVal = gmac_miictrl_csr_3 |					// bits: 4-2,   CSR Clock Range
             ((pMAC->phyAddr & 0x1F) << 11) |		// bits: 15-11, PHY Address
    		 ((phyReg & 0x1F) << 6)  |				// bits: 10-6,  MII Register
    		 gmac_miictrl_wr |					    // bit:  1,     MII is Writting
    		 gmac_miictrl_busy;						// bit:  0,     MII is Busy

    pMAC->hwGMAC->miidata = (unsigned int)val & gmac_miidata_mask;
    pMAC->hwGMAC->miictrl = phyVal;

    /* wait for it to complete, become unbusy
     */
    for (i=0; i<MII_WR_TIMEOUT; i++)
    {
    	if (!(pMAC->hwGMAC->miictrl & gmac_miictrl_busy))
    	{
        	break;
        }
    }

    if (i>=MII_WR_TIMEOUT)
    {
    	printk("Error: mii write phy timeout.\n");
    }
}

static void mii_read_phy(GMAC_t *pMAC, unsigned char phyReg, unsigned short *val)
{
	int i;
    unsigned int phyVal;

    phyVal = gmac_miictrl_csr_3 |					// bits: 4-2,   CSR Clock Range
             ((pMAC->phyAddr & 0x1F) << 11) |		// bits: 15-11, PHY Address
    		 ((phyReg & 0x1F) << 6) |				// bits: 10-6,  MII Register
    		 gmac_miictrl_busy;					    // bit:  0,     MII is Busy

    pMAC->hwGMAC->miictrl = phyVal;

    /* wait for it to complete, become unbusy
     */
    for (i=0; i<MII_RD_TIMEOUT; i++)
    {
    	if (!(pMAC->hwGMAC->miictrl & gmac_miictrl_busy))
    	{
        	break;
        }
    }

    if (i<MII_RD_TIMEOUT)
    {
    	phyVal = pMAC->hwGMAC->miidata & gmac_miidata_mask;
        *val = (unsigned short)phyVal;
    }
    else
    {
    	printk("Error: mii read phy timeout.\n");
    	*val = 0;
    }
}

static int mii_get_phy_link_mode(GMAC_t *pMAC, int *eSpeed, int *eDuplex, int *eLink)
{
    int rt;
	unsigned short ctrlVal, srVal, sr1GVal=0, extsrVal=0;

	mii_read_phy(pMAC, MII_BMCR, &ctrlVal);		/* Read the phy control */
	mii_read_phy(pMAC, MII_BMSR, &srVal);		/* Read the phy status */

    if (srVal & BMSR_EXTSTAT)
    {
        mii_read_phy(pMAC, MII_100T2SR, &sr1GVal);   // 0x7800
        mii_read_phy(pMAC, MII_EXTSR, &extsrVal);    // 0x2000
    }

	pMAC->autoNegotiation = ctrlVal & BMCR_AUTOEN;

	if (pMAC->autoNegotiation)		/* autonegotiation mode */
	{
		/* Autonegotiation complete and Linkup
		 */
		if (srVal & BMSR_ACOMP)
		{
		    if ((extsrVal & EXTSR_1000TFDX) && (sr1GVal & GTSR_LP_1000TFDX))
		    {
		        *eSpeed  = SPEED1000;
		        *eDuplex = FULLDUPLEX;
		    }
		    else
		    {
			    unsigned short anVal;

                mii_read_phy(pMAC, MII_ANLPAR, &anVal);

			    /* Read the phy Autonegotiation link partner abilities
			     */
			    *eSpeed  = ((anVal & ANLPAR_TX_FD) || (anVal & ANLPAR_TX)) ? SPEED100 :
					       ((anVal & ANLPAR_10_FD) || (anVal & ANLPAR_10)) ? SPEED10 : SPEED100;
			    *eDuplex = ((anVal & ANLPAR_TX_FD) || (anVal & ANLPAR_10_FD)) ? FULLDUPLEX :
					       ((anVal & ANLPAR_TX) || (anVal & ANLPAR_10)) ? HALFDUPLEX : FULLDUPLEX;
            }
            
			*eLink   = (srVal & BMSR_LINK) ? LINKUP : LINKDOWN;
		}
		else
		{
			*eSpeed  = SPEED100;
			*eDuplex = FULLDUPLEX;
			*eLink   = LINKDOWN;
		}
	}
	else				/* special link mode */
	{
		*eSpeed  = ((ctrlVal & BMCR_SPEED(BMCR_S1000)) == BMCR_SPEED(BMCR_S1000)) ? SPEED1000 :
                   ((ctrlVal & BMCR_SPEED(BMCR_S100)) == BMCR_SPEED(BMCR_S100)) ? SPEED100 :
				   ((ctrlVal & BMCR_SPEED(BMCR_S10)) == BMCR_SPEED(BMCR_S10)) ? SPEED10 :
                   SPEED100;  // default
		*eDuplex = (ctrlVal & BMCR_FDX) ? FULLDUPLEX : HALFDUPLEX;
		*eLink   = (srVal & BMSR_LINK) ? LINKUP : LINKDOWN;
	}

    rt = (srVal & BMSR_RFAULT) ? -1 : 0;

	if (rt == 0)
	{
		pMAC->Speed      = *eSpeed;
		pMAC->DuplexMode = *eDuplex;
	}
	else				/* default */
	{
		pMAC->Speed      = SPEED100;
		pMAC->DuplexMode = FULLDUPLEX;
	}
	pMAC->LinkState = *eLink;

	return rt;
}

/*
 * 总是用自动协商, TODO 指定模式
 */
static void mii_set_phy_link_mode(GMAC_t *pMAC, int eSpeed, int eDuplex, int ePowerDown)
{
	unsigned short ctrlVal, srVal;

	mii_read_phy(pMAC, MII_BMCR, &ctrlVal);		/* Read the phy control */
	mii_read_phy(pMAC, MII_BMSR, &srVal);		/* Read the phy status */

    /*
     * 已经连接 @@ 2024.11.14
     */
    if ((ctrlVal & BMCR_AUTOEN) && (srVal & BMSR_LINK))
    {
        return;
    }

	if (pMAC->autoNegotiation)
	{
		unsigned short anarVal;

		anarVal = ANAR_FC | ANAR_TX_FD | ANAR_TX | ANAR_10_FD | ANAR_10 | ANAR_CSMA;
		mii_write_phy(pMAC, MII_ANAR, anarVal);     /* set advertise register */

		ctrlVal = BMCR_AUTOEN | BMCR_STARTNEG;      /* set restart autonegotiation */
        if (ePowerDown) ctrlVal |= BMCR_PDOWN;      /* set power saving mode */
        mii_write_phy(pMAC, MII_BMCR, ctrlVal);     /* write control register to do autonegotiation */

	    /* wait for autonegotiation done, total delay 3000ms
	     */
        if (!ePowerDown)
		{
	        int wait_ticks = 0;
		    unsigned short srVal;

            while (wait_ticks <= pMAC->autoNegoTimeout)
		    {
		    	mii_read_phy(pMAC, MII_BMSR, &srVal);
		    	if (srVal & BMSR_ACOMP)
		    	{
                	break;
                }

                osal_msleep(100);
                wait_ticks += 100;
		    }

		    if (wait_ticks > pMAC->autoNegoTimeout)
		    {
		    	printk("Error: phy autonegotiation timeout.\n");
		    }
		}

		/* TODO else - timer check autonegotiation done */
	}
	else    /* has linked yet */
	{
		ctrlVal = (eSpeed == SPEED1000) ? BMCR_SPEED(BMCR_S1000) :
                  (eSpeed == SPEED100) ? BMCR_SPEED(BMCR_S100) :
				  (eSpeed == SPEED10) ? BMCR_SPEED(BMCR_S10) : BMCR_SPEED(BMCR_S100);

		if (eDuplex) ctrlVal |= BMCR_FDX;		    /* set duplex mode */
		if (ePowerDown) ctrlVal |= BMCR_PDOWN;	    /* set power saving mode */
		mii_write_phy(pMAC, MII_BMCR, ctrlVal);		/* write control register to effect */
	}
}

static inline void mii_check_phy_linked(GMAC_t *pMAC, int *linked)
{
	unsigned short srVal;
	mii_read_phy(pMAC, MII_BMSR, &srVal);
	*linked = (srVal & BMSR_LINK) ? LINKUP : LINKDOWN;
}

#if 0
/******************************************************************************
 * full check the ethernet link is changed.
 */
static int mii_check_link_change(GMAC_t *pMAC)
{
	int eDuplex, eSpeed, eLink;

	if ((NULL != pMAC) && pMAC->initialized &&
		(mii_get_phy_link_mode(pMAC, &eDuplex, &eSpeed, &eLink) == 0))
	{
#if (GMAC_TRACE_INFO)
		if ((pMAC->Speed != eSpeed) || (pMAC->DuplexMode != eDuplex) || (pMAC->LinkState != eLink))
		{
	        int unitNumber = pMAC->unitNumber;
	        char *sDuplex = (eDuplex == FULLDUPLEX) ? "FULL" : "HALF";
	        char *sLinked = (eLink == LINKUP) ? "UP" : "DOWN";
	        char *sAutoNego = pMAC->autoNegotiation ? "YES" : "NO";
	        eSpeed = (eSpeed == SPEED1000) ? 1000 : (eSpeed == SPEED100) ? 100 : 10;

	        printk("GMAC%i: SPEED=%iM, DUPLEX=%s, LINK=%s. AN=%s.\r\n",
	        		unitNumber, eSpeed, sDuplex, sLinked, sAutoNego);
		}
#endif

		if ((pMAC->Speed != eSpeed) || (pMAC->DuplexMode != eDuplex))
		{
            ls2k_gmac_do_reset(pMAC);
		}

		if (pMAC->LinkState != eLink)
		{
			pMAC->LinkState = eLink;
			return 1;
		}
	}

	return 0;
}
#endif

//***************************************************************************************
// set gmac work mode
//***************************************************************************************

static void ls2k_gmac_set_workmode(GMAC_t *pMAC)
{
	unsigned int val;

	if (pMAC->DuplexMode == FULLDUPLEX)
	{
		val = pMAC->hwGMAC->config;
		val &= ~(gmac_ctrl_wd		|			// Enable Watch Dog
				 gmac_ctrl_je		|			// Disable Jumbo Frame
				 gmac_ctrl_do		|			// Enable Receive Own
				 gmac_ctrl_lm		|			// Disable Loopback mode
				 gmac_ctrl_dr		|			// Enable Retry
				 gmac_ctrl_acs		|			// Disable Pad/CRC Stripping
				 gmac_ctrl_bl_mask	|			// Set Back-Off Limit 0
				 gmac_ctrl_dc);					// Disable Deferral Check
		val |= (gmac_ctrl_jd		|			// Disable the Jabber Timer
				gmac_ctrl_be		|			// Enable Frame Burst
				gmac_ctrl_dm		|			// Set Full Duplex Mode
				gmac_ctrl_bl_0		|			//
				gmac_ctrl_te		|			// Enable Transmitter
				gmac_ctrl_re);					// Enable Receiver
    /*
        if (pMAC->Speed == SPEED1000)
        {
            val |= gmac_ctrl_tc;
        }
     */
		pMAC->hwGMAC->config = val;

		val = pMAC->hwGDMA->control;
		val |= (gdma_ctrl_rxsf	|				// Receive Store and Forward
				gdma_ctrl_txsf	|				// Transmit Store and Forward
				gdma_ctrl_ferrf);				// Forward Error Frames
		pMAC->hwGDMA->control = val;

		val = pMAC->hwGMAC->config;
		if (pMAC->Speed == SPEED1000)
		{
			val &= ~gmac_ctrl_mii;				// Port Select, 0: GMII (1000Mbps)
		}
		else
		{
			val |= gmac_ctrl_mii;				// Port Select, 1: MII (10/100Mbps)
			if (pMAC->Speed == SPEED100)
				val |= gmac_ctrl_fes; 			// Speed, 1: 100Mbps
			else
				val &= ~gmac_ctrl_fes; 			// Speed, 0: 10Mbps
		}
		pMAC->hwGMAC->config = val;

		val = pMAC->hwGMAC->framefilter;
		val &= ~(gmac_frmfilter_pcf_mask |		// Pass Control Frames, 00: GMAC过滤所有控制帧
				 gmac_frmfilter_dbf		 |		// Enable Broadcast Frames, 0:接收所有广播帧
				 gmac_frmfilter_saf		 |		// Disable Source Address Filter
				 gmac_frmfilter_pm		 |		// Pass All Multicast, 0:过滤所有多播帧
				 gmac_frmfilter_daif	 |		// DA Inverse Filtering, 0:对单播和多播帧进行正常目标地址匹配
				 gmac_frmfilter_hmc		 |		// Disable Hash Multicast
				 gmac_frmfilter_huc);			// Disable Hash Unicast
		val |= (gmac_frmfilter_ra		 |		// Disable Frame Filter
				gmac_frmfilter_pcf_0	 |		//
				gmac_frmfilter_pr);				// Enable Promiscuous Mode, 混杂模式, 接收所有以太网帧
		pMAC->hwGMAC->framefilter = val;

		val = pMAC->hwGMAC->flowctrl;
		val &= ~gmac_flowctrl_up;				// Disable Unicast Pause Frame Detect
		val |= (gmac_flowctrl_rxfcen	|		// Enable Receive Flow Control
				gmac_flowctrl_txfcen);			// Enable Transmit Flow Control
		pMAC->hwGMAC->flowctrl = val;
	}
	else	// HALFDUPLEX
	{
		val = pMAC->hwGMAC->config;
		val &= ~(gmac_ctrl_wd		|			// Enable Watch Dog
				 gmac_ctrl_jd		|			// Enable the Jabber frame
				 gmac_ctrl_je		|			// Disable Jumbo Frame
				 gmac_ctrl_do		|			// Enable Receive Own
				 gmac_ctrl_lm		|			// Disable Loopback mode
				 gmac_ctrl_dm		|			// Set Half Duplex Mode
				 gmac_ctrl_dr		|			// Enable Retry
				 gmac_ctrl_acs		|			// Disable Pad/CRC Stripping
				 gmac_ctrl_bl_mask	|			// Set Back-Off Limit 0
				 gmac_ctrl_dc);					// Disable Deferral Check
		val |= (gmac_ctrl_be	|				// Enable Frame Burst
				gmac_ctrl_bl_0);				//
		pMAC->hwGMAC->config = val;

		val = pMAC->hwGDMA->control;
		val |= (gdma_ctrl_rxsf	|				// Receive Store and Forward
				gdma_ctrl_txsf);				// Transmit Store and Forward
		pMAC->hwGDMA->control = val;

		val = pMAC->hwGMAC->config;
		val |= (gmac_ctrl_te	|				// Enable Transmitter
				gmac_ctrl_re);					// Receiver Enable
		if (pMAC->Speed == SPEED1000)
		{
			val &= ~gmac_ctrl_mii;				// Port Select, 0: GMII (1000Mbps)
		}
		else
		{
			val |= gmac_ctrl_mii;				// Port Select, 1: MII (10/100Mbps)
			if (pMAC->Speed == SPEED100)
				val |= gmac_ctrl_fes; 			// Speed, 1: 100Mbps
			else
				val &= ~gmac_ctrl_fes; 			// Speed, 0: 10Mbps
		}
		pMAC->hwGMAC->config = val;

		val = pMAC->hwGMAC->framefilter;
		val &= ~(gmac_frmfilter_ra	     |		// Enable Frame Filter
				 gmac_frmfilter_pcf_mask |		// Pass Control Frames, 00: GMAC过滤所有控制帧
				 gmac_frmfilter_dbf	     | 		// Enable Broadcast Frames, 0:接收所有广播帧
				 gmac_frmfilter_saf	     |		// Disable Source Address Filter
				 gmac_frmfilter_pm	     |		// Pass All Multicast, 0:过滤所有多播帧
				 gmac_frmfilter_daif	 |		// DA Inverse Filtering, 0:对单播和多播帧进行正常目标地址匹配
				 gmac_frmfilter_hmc	     |		// Disable Hash Multicast
				 gmac_frmfilter_huc	     |		// Disable Hash Unicast
				 gmac_frmfilter_pr);			// Disble Promiscuous Mode, 混杂模式, 接收所有以太网帧
		val |= gmac_frmfilter_pcf_0;			//
		pMAC->hwGMAC->framefilter = val;

		val = pMAC->hwGMAC->flowctrl;
		val &= ~(gmac_flowctrl_up	  |			// Disable Unicast Pause Frame Detect
				 gmac_flowctrl_rxfcen |			// Disable Receive Flow Control
				 gmac_flowctrl_txfcen);			// Disable Transmit Flow Control
		pMAC->hwGMAC->flowctrl = val;
	}
}

//***************************************************************************************

#if (GMAC_DEBUG)
static void ls2k_gmac_dump_registers(GMAC_t *pMAC)
{
    DBG_OUT("GMAC registers:\r\n");
    DBG_OUT("Configuration[%08X]           = 0x%08X\r\n", (int)&pMAC->hwGMAC->config,      pMAC->hwGMAC->config);		/* 0x0000  */
    DBG_OUT("GMAC Frame Filter[%08X]       = 0x%08X\r\n", (int)&pMAC->hwGMAC->framefilter, pMAC->hwGMAC->framefilter);	/* 0x0004  */
//  DBG_OUT("Hash Table High[%08X]         = 0x%08X\r\n", (int)&pMAC->hwGMAC->hashhi,      pMAC->hwGMAC->hashhi);		/* 0x0008  */
//  DBG_OUT("Hash Table Low[%08X]          = 0x%08X\r\n", (int)&pMAC->hwGMAC->hashlo,      pMAC->hwGMAC->hashlo);		/* 0x000C  */
//  DBG_OUT("GMII Address[%08X]            = 0x%08X\r\n", (int)&pMAC->hwGMAC->miictrl,     pMAC->hwGMAC->miictrl);		/* 0x0010  */
//  DBG_OUT("GMII Data[%08X]               = 0x%08X\r\n", (int)&pMAC->hwGMAC->miidata,     pMAC->hwGMAC->miidata);		/* 0x0014  */
    DBG_OUT("Flow Control[%08X]            = 0x%08X\r\n", (int)&pMAC->hwGMAC->flowctrl,    pMAC->hwGMAC->flowctrl);	    /* 0x0018  */
//  DBG_OUT("VLAN Tag[%08X]                = 0x%08X\r\n", (int)&pMAC->hwGMAC->vlantag,     pMAC->hwGMAC->vlantag);		/* 0x001C  */
    DBG_OUT("Version[%08X]                 = 0x%08X\r\n", (int)&pMAC->hwGMAC->version,     pMAC->hwGMAC->version);		/* 0x0020  */
    DBG_OUT("Interrupt Status[%08X]        = 0x%08X\r\n", (int)&pMAC->hwGMAC->intstatus,   pMAC->hwGMAC->intstatus);	/* 0x0038  */
    DBG_OUT("Interrupt Mask[%08X]          = 0x%08X\r\n", (int)&pMAC->hwGMAC->intmask,     pMAC->hwGMAC->intmask);		/* 0x003C  */
//  DBG_OUT("Address0 High[%08X]           = 0x%08X\r\n", (int)&pMAC->hwGMAC->addr0hi,     pMAC->hwGMAC->addr0hi);		/* 0x0040  */
//  DBG_OUT("Address0 Low[%08X]            = 0x%08X\r\n", (int)&pMAC->hwGMAC->addr0lo,     pMAC->hwGMAC->addr0lo);		/* 0x0044  */
//  DBG_OUT("Address1 High[%08X]           = 0x%08X\r\n", (int)&pMAC->hwGMAC->addr1hi,     pMAC->hwGMAC->addr1hi);		/* 0x0048  */
//  DBG_OUT("Address1 Low[%08X]            = 0x%08X\r\n", (int)&pMAC->hwGMAC->addr1lo,     pMAC->hwGMAC->addr1lo);		/* 0x004C  */

    DBG_OUT("GDMA registers:\r\n");
    DBG_OUT("Bus Mode[%08X]                = 0x%08X\r\n", (int)&pMAC->hwGDMA->busmode,     pMAC->hwGDMA->busmode); 	    /* 0x1000  */
    DBG_OUT("Status[%08X]                  = 0x%08X\r\n", (int)&pMAC->hwGDMA->status,      pMAC->hwGDMA->status); 		/* 0x1014  */
    DBG_OUT("Operation Mode[%08X]          = 0x%08X\r\n", (int)&pMAC->hwGDMA->control,     pMAC->hwGDMA->control); 	    /* 0x1018  */
    DBG_OUT("Interrupt Enable[%08X]        = 0x%08X\r\n", (int)&pMAC->hwGDMA->intenable,   pMAC->hwGDMA->intenable);	/* 0x101C */
//  DBG_OUT("Transmit Poll Demand[%08X]    = 0x%08X\r\n", (int)&pMAC->hwGDMA->txpoll,      pMAC->hwGDMA->txpoll); 		/* 0x1004  */
//  DBG_OUT("Receive Poll Demand[%08X]     = 0x%08X\r\n", (int)&pMAC->hwGDMA->rxpoll,      pMAC->hwGDMA->rxpoll); 		/* 0x1008  */
    DBG_OUT("Start of Receive Descriptor List Address[%08X]  = 0x%08X\r\n", (int)&pMAC->hwGDMA->rxdesc0,   pMAC->hwGDMA->rxdesc0); 	 /* 0x100C  */
    DBG_OUT("Start of Transmit Descriptor List Address[%08X] = 0x%08X\r\n", (int)&pMAC->hwGDMA->txdesc0,   pMAC->hwGDMA->txdesc0); 	 /* 0x1010  */
//  DBG_OUT("Missed Frame and Buffer Overflow Counter[%08X]  = 0x%08X\r\n", (int)&pMAC->hwGDMA->mfbocount, pMAC->hwGDMA->mfbocount); /* 0x1020  */
    DBG_OUT("Current Host Transmit Descriptor[%08X]          = 0x%08X\r\n", (int)&pMAC->hwGDMA->curtxdesc, pMAC->hwGDMA->curtxdesc); /* 0x1048  */
    DBG_OUT("Current Host Receive Descriptor[%08X]           = 0x%08X\r\n", (int)&pMAC->hwGDMA->currxdesc, pMAC->hwGDMA->currxdesc); /* 0x104C  */
    DBG_OUT("Current Host Transmit Buffer Address[%08X]      = 0x%08X\r\n", (int)&pMAC->hwGDMA->curtxbuf,  pMAC->hwGDMA->curtxbuf);  /* 0x1050  */
    DBG_OUT("Current Host Receive Buffer Address[%08X]       = 0x%08X\r\n", (int)&pMAC->hwGDMA->currxbuf,  pMAC->hwGDMA->currxbuf);  /* 0x1054  */

    /* dump DM9161AEP registers */
    #if 0
    {
        unsigned short val;

//        pMAC->phyAddr = 17;  // LS1B==17, LS1C==19

        DBG_OUT("DM9161AEP general registers\r\n");
        MII_read_phy(pMAC, 0, &val);  DBG_OUT("0: control            = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 1, &val);  DBG_OUT("1: status             = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 2, &val);  DBG_OUT("2: phyid1             = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 3, &val);  DBG_OUT("3: phyid2             = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 4, &val);  DBG_OUT("4: auto-neg-advertise = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 5, &val);  DBG_OUT("5: link part ability  = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 6, &val);  DBG_OUT("6: auto-neg-expansion = 0x%04X\r\n", val);
        DBG_OUT("DM9161AEP special registers\r\n");
        MII_read_phy(pMAC, 16, &val);  DBG_OUT("16: specified config = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 17, &val);  DBG_OUT("17: specified status = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 18, &val);  DBG_OUT("18: 10T conf/status  = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 19, &val);  DBG_OUT("19: pwdor            = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 20, &val);  DBG_OUT("20: special config   = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 21, &val);  DBG_OUT("21: mdintr           = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 22, &val);  DBG_OUT("22: rcver            = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 23, &val);  DBG_OUT("23: dis-connect      = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 24, &val);  DBG_OUT("24: rstlh            = 0x%04X\r\n", val);
    }
    #endif
}

static void dump_1_rx_desc(int index, RX_DESC_t *desc)
{
	DBG_OUT("RX desc[%i] @0x%016lX:\r\n", index, (long)desc);
	DBG_OUT("    status   = 0x%08X\r\n", desc->status);
	DBG_OUT("    control  = 0x%08X\r\n", desc->control);
	DBG_OUT("    bufptr   = 0x%08X\r\n", desc->bufptr);
	DBG_OUT("    nextdesc = 0x%08X\r\n", desc->nextdesc);
}

static void dump_all_rx_desc(GMAC_t *pMAC)
{
    int i;
    RX_DESC_t *desc;
    
	for (i = 0; i < NUM_RX_DMA_DESC; i++)
	{
		desc = pMAC->rx_desc[i];
        dump_1_rx_desc(i, desc);
    }
    
    DBG_OUT("DMA rxdesc0   = 0x%08X\r\n", pMAC->hwGDMA->rxdesc0);
    DBG_OUT("DMA currxdesc = 0x%08X\r\n", pMAC->hwGDMA->currxdesc);
    DBG_OUT("DMA currxbuf  = 0x%08X\r\n", pMAC->hwGDMA->currxbuf);
    DBG_OUT("\r\n");
}

static void dump_1_tx_desc(int index, TX_DESC_t *desc)
{
	DBG_OUT("TX desc[%i] @0x%016lX:\r\n", index, (long)desc);
	DBG_OUT("    status   = 0x%08X\r\n", desc->status);
	DBG_OUT("    control  = 0x%08X\r\n", desc->control);
	DBG_OUT("    bufptr   = 0x%08X\r\n", desc->bufptr);
	DBG_OUT("    nextdesc = 0x%08X\r\n", desc->nextdesc);
}

static void dump_all_tx_desc(GMAC_t *pMAC)
{
    int i;
    TX_DESC_t *desc;

	for (i = 0; i < NUM_TX_DMA_DESC; i++)
	{
		desc = pMAC->tx_desc[i];
        dump_1_tx_desc(i, desc);
    }

    DBG_OUT("DMA txdesc0   = 0x%08X\r\n", pMAC->hwGDMA->txdesc0);
    DBG_OUT("DMA curtxdesc = 0x%08X\r\n", pMAC->hwGDMA->curtxdesc);
    DBG_OUT("DMA curtxbuf  = 0x%08X\r\n", pMAC->hwGDMA->curtxbuf);
    DBG_OUT("\r\n");
}
#endif

//***************************************************************************************
// Initialize GMAC Hardware
//***************************************************************************************

/*
 * GDMA 在复位后或者自动协商后:
 *
 *      hwGMAC->intstatus 置位: bit[0]:  RGMII PHY接口的链路状态发生变化
 *
 *      hwGDMA->status    置位: bit[26]: GMAC Line interface Interrupt
 *
 * 设置: hwGMAC->intmask  置位: bit[0] 可以屏蔽 GMAC Line interface Interrupt
 *
 */

static void ls2k_gmac_init_hw(GMAC_t *pMAC, unsigned char *macAddr)
{
	int i; 
    unsigned int val, phyID;

    /*
     * 引脚复用初始化
     */

	/* close "gmac" system interrupt, set trigger mode
	 */
    ls2k_interrupt_disable(pMAC->irqNum);

    /* reset the GMAC and DMA, and wait for it to complete
     */
    pMAC->hwGDMA->busmode = gdma_busmode_swreset;

    for (i=0; i<GDMA_RESET_TIMEOUT; i++)
    {
    	if (!(pMAC->hwGDMA->busmode & gdma_busmode_swreset))
    	{
        	break;
        }
    }

    if (i>=GDMA_RESET_TIMEOUT)
    {
    	printk("Error: gmac-dma Reset timeout.\r\n");
    }

    /* set mac address
     */
    if (macAddr != NULL)
    {
        pMAC->hwGMAC->addrhi = ((macAddr[5] <<  8) | (macAddr[4] <<  0));
        pMAC->hwGMAC->addrlo = ((macAddr[3] << 24) | (macAddr[2] << 16) |
                                (macAddr[1] <<  8) | (macAddr[0] <<  0));

        /* get the MAC address from the chip
         */
        macAddr[5] = (pMAC->hwGMAC->addrhi >>  8) & 0xFF;
        macAddr[4] = (pMAC->hwGMAC->addrhi >>  0) & 0xFF;
        macAddr[3] = (pMAC->hwGMAC->addrlo >> 24) & 0xFF;
        macAddr[2] = (pMAC->hwGMAC->addrlo >> 16) & 0xFF;
        macAddr[1] = (pMAC->hwGMAC->addrlo >>  8) & 0xFF;
        macAddr[0] = (pMAC->hwGMAC->addrlo >>  0) & 0xFF;
    }

    /* read the gmac version
     */
	pMAC->gmacVersion = pMAC->hwGMAC->version;

	/* set mii-phy CSR Clock
	 */
    /*
	val = pMAC->hwGMAC->miictrl;
	val = (val & (~gmac_miictrl_csr_mask)) | gmac_miictrl_csr_3;
	pMAC->hwGMAC->miictrl = val;
     */

	/*
     * phy initialize
	 */
    pMAC->phyAddr = mii_detect_phy_addr(pMAC, &phyID);

    if (phyID == 0x0000010A)        // YT8511
    {
		unsigned short data;

		mii_write_phy(pMAC, 0x1E, 0x0C);
		mii_read_phy(pMAC, 0x1F, &data);
		data &= 0xFFFE;
		mii_write_phy(pMAC, 0x1F, data);
		mii_read_phy(pMAC, 0x1F, &data);
    }

    #if 1
    {
        int eSpeed, eDuplex, eLink;

        pMAC->autoNegotiation = 1;          /* 自动协商吗 ? */

        mii_set_phy_link_mode(pMAC, SPEED1000, FULLDUPLEX, 0);
	    mii_get_phy_link_mode(pMAC, &eSpeed, &eDuplex, &eLink);

        int unitNumber = pMAC->unitNumber;
        char *sDuplex = (eDuplex == FULLDUPLEX) ? "FULL" : "HALF";
        char *sLinked = (eLink == LINKUP) ? "UP" : "DOWN";
        char *sAutoNego = pMAC->autoNegotiation ? "YES" : "NO";
        eSpeed = (eSpeed == SPEED1000) ? 1000 : (eSpeed == SPEED100) ? 100 : 10;

        printk("GMAC%i: SPEED=%iM, DUPLEX=%s, LINK=%s. AN=%s.\r\n",
        		unitNumber, eSpeed, sDuplex, sLinked, sAutoNego);
    }
    #endif

    /* Set the "Start of Transmit Descriptor List Address" register
     */
    pMAC->hwGDMA->txdesc0 = VA_TO_PHYS(pMAC->tx_desc[0]);		// physical address

	/* Set the "Start of Receive Descriptor List Address" register
	 */
    pMAC->hwGDMA->rxdesc0 = VA_TO_PHYS(pMAC->rx_desc[0]);		// physical address

    /* set the gmac-dma busmode register(1), bit(7)=0 使用16字节大小的描述符
     */
    val = (gdma_busmode_dsl_0 |						// 设置2个描述符间的距离, RINGMODE
    	   gdma_busmode_pbl_4);						// Dma burst length = 4
    pMAC->hwGDMA->busmode = val;

	/* Init GDMA Control (1)
	 */
	val = (gdma_ctrl_txsf |							// Transmit Store and Forward
		   gdma_ctrl_rxsf |							// Receive Store and Forward
		   gdma_ctrl_txopsecf); 					// TX Operate on Second Frame
	val &= ~(gdma_ctrl_rtc_mask |					// Receive Threshold Control.
			 gdma_ctrl_ttc_mask);					// Transmit Threshold Control
	val |= (gdma_ctrl_rtc_128 |
			gdma_ctrl_ttc_64);
	pMAC->hwGDMA->control = val;

	/* gmac default value, move to here.
	 */
	ls2k_gmac_set_workmode(pMAC);

    /**********************************************************************************
     * This is important: 屏蔽 GMAC Line interface Interrupt, 避免 GDMA status bit[26]
     **********************************************************************************/

    // pMAC->hwGMAC->intmask = gmac_intmask_rgmii;

	/* enables the pause control in Full duplex mode of operation
	 */
	val = pMAC->hwGDMA->control;
	val |= (gdma_ctrl_enhwfc | 						// Enable HW flow control
			0x00000300 | 							// Rx flow control Act. threhold (4kbytes)
			0x00400000);							// Rx flow control deact. threhold (5kbytes)
	pMAC->hwGDMA->control = val;

	val = pMAC->hwGMAC->flowctrl;
	val |= (gmac_flowctrl_rxfcen |					// Enable Receive Flow Control
			gmac_flowctrl_txfcen |					// Enable Transmit Flow Control
			0xFFFF0000);
	pMAC->hwGMAC->flowctrl = val;

	/* Clears all the pending interrupts.
	 */
	val = pMAC->hwGDMA->status;
	pMAC->hwGDMA->status = val;

	/* Enable all gdma interrupt
	 */
    GDMA_INT_EN(pMAC->hwGDMA);

#if 0
    if (pMAC->LinkState == LINKDOWN)
    {
        mii_write_phy(pMAC, MII_BMCR, BMCR_RESET);
	    mii_set_phy_link_mode(pMAC, SPEED100, FULLDUPLEX, 0);
        mii_get_phy_link_mode(pMAC, &eSpeed, &eDuplex, &eLink);
    }
#endif

	/* 关闭 MMC 相关中断
	 */
    WRITE_REG32(&pMAC->hwGMAC + 0x010C,	0xFFFFFFFF); /* mask for interrupts generated from rx counters */
    WRITE_REG32(&pMAC->hwGMAC + 0x0110,	0xFFFFFFFF); /* mask for interrupts generated from tx counters */
    WRITE_REG32(&pMAC->hwGMAC + 0x0200,	0xFFFFFFFF); /* mask for interrupts generated from rx IPC statistic counters */

    /******************************************************
	 * 硬件初始化完成
     */

	/* open "gmac" interrupt in system
	 */
    //ls2k_interrupt_enable(pMAC->irqNum);

	/* Clears all the pending interrupts again.
	 */
	val = pMAC->hwGDMA->status;
	pMAC->hwGDMA->status = val;

}

//*****************************************************************************
// Start the GMAC
//*****************************************************************************

static void ls2k_gmac_do_reset(GMAC_t *pMAC)
{
    pMAC->started = 0;
	GMAC_STOP(pMAC->hwGMAC);		    /* stop the gmac. */
	GDMA_STOP(pMAC->hwGDMA);		    /* stop the gdma. */
	GDMA_INIT_DESC(pMAC);		        /* initialize the gmac */
	ls2k_gmac_init_hw(pMAC, NULL);      /* GMAC started internal */
	GDMA_START(pMAC->hwGDMA);		    /* restart the gmac. */
	pMAC->started = 1;
}

//*****************************************************************************
// Start the GMAC
//*****************************************************************************

static void ls2k_gmac_set_start(GMAC_t *pMAC)
{
    if (!pMAC->started)
    {
        GMAC_START(pMAC->hwGMAC);
        GDMA_START(pMAC->hwGDMA);
        ls2k_interrupt_enable(pMAC->irqNum);
        pMAC->started = 1;
    }
}

//*****************************************************************************
// Stop the GMAC
//*****************************************************************************

static void ls2k_gmac_set_stop(GMAC_t *pMAC)
{
    if (pMAC->started)
    {
    	ls2k_interrupt_disable(pMAC->irqNum);
        GDMA_STOP(pMAC->hwGDMA);
        GMAC_STOP(pMAC->hwGMAC);
        pMAC->started = 0;
    }
}

//***************************************************************************************
// Initialize Tx Descriptors
//***************************************************************************************

static int ls2k_init_tx_desc_queue(GMAC_t *pMAC)
{
	int i;
	unsigned long desc_ptr, buf_ptr;
	TX_DESC_t *desc = NULL;

#if BSP_USE_GMAC0
    if (pMAC == &ls2k_GMAC0)
    {
	    desc_ptr = (unsigned long)&tx_desc_0[0];
	    buf_ptr  = (unsigned long)&tx_buf_0[0];
    }
    else
#endif
#if BSP_USE_GMAC1
    if (pMAC == &ls2k_GMAC1)
    {
	    desc_ptr = (unsigned long)&tx_desc_1[0];
	    buf_ptr  = (unsigned long)&tx_buf_1[0];
    }
    else
#endif
    {
        printk("parameter error while init tx desc!\r\n");
        return -1;
    }

	/* initializing desc and buffer
	 */
	for (i = 0; i < NUM_TX_DMA_DESC; i++)
	{
		/* tx descriptor
		 */
		desc = (TX_DESC_t *)desc_ptr;
		memset((void *)desc, 0, DMA_DESC_SIZE);
		pMAC->tx_desc[i] = desc;

		if (pMAC->descmode == CHAINMODE)
		{
        #if ENH_DESC
			desc->status = txdesc0_stat_tch;
        #else
            desc->control = txdesc1_ctrl_tch;
        #endif
			if (i > 0)
			{
				pMAC->tx_desc[i-1]->nextdesc = VA_TO_PHYS(desc);  // physical address
			}

			if (i == NUM_TX_DMA_DESC - 1)
			{
				desc->nextdesc = VA_TO_PHYS(pMAC->tx_desc[0]);     // physical address
			}
		}
		else if (i == NUM_TX_DMA_DESC - 1)	// RINGMODE
		{
        #if ENH_DESC
			desc->status = txdesc0_stat_ter;
		#else
            desc->control = txdesc1_ctrl_ter;
		#endif
		}

		/* tx buffer
		 */
    	pMAC->tx_desc[i]->bufptr = VA_TO_PHYS(buf_ptr);		// 发送缓存物理地址
    	pMAC->tx_buf[i] = (void *)buf_ptr;					// 发送缓冲区数组

		desc_ptr += DMA_DESC_SIZE;
    	buf_ptr  += TX_BUF_SIZE;
	}

	return 0;
}

//***************************************************************************************
// Initialize Rx Descriptors
//***************************************************************************************

static int ls2k_init_rx_desc_queue(GMAC_t *pMAC)
{
	int i;
	unsigned long desc_ptr, buf_ptr;
	RX_DESC_t *desc = NULL;

#if BSP_USE_GMAC0
    if (pMAC == &ls2k_GMAC0)
    {
	    desc_ptr = (unsigned long)&rx_desc_0[0];
	    buf_ptr  = (unsigned long)&rx_buf_0[0];
    }
    else
#endif
#if BSP_USE_GMAC1
    if (pMAC == &ls2k_GMAC1)
    {
	    desc_ptr = (unsigned long)&rx_desc_1[0];
	    buf_ptr  = (unsigned long)&rx_buf_1[0];
    }
    else
#endif
    {
        printk("parameter error while init rx desc!\r\n");
        return -1;
    }

	/* initializing desc and buffer
	 */
	for (i = 0; i < NUM_RX_DMA_DESC; i++)
	{
		/* rx descriptor
		 */
		desc = (RX_DESC_t *)desc_ptr;
		memset((void *)desc, 0, DMA_DESC_SIZE);
		pMAC->rx_desc[i] = desc;

		if (pMAC->descmode == CHAINMODE)
		{
			desc->control = rxdesc1_ctrl_rch;
			if (i > 0)
			{
				pMAC->rx_desc[i-1]->nextdesc = VA_TO_PHYS(desc);  // physical address
			}

			if (i == NUM_RX_DMA_DESC - 1)
			{
				desc->nextdesc = VA_TO_PHYS(pMAC->rx_desc[0]);	  // physical address
			}
		}
		else if (i == NUM_RX_DMA_DESC - 1)	// RINGMODE
		{
			desc->control = rxdesc1_ctrl_rer;
		}

		/* rx buffer
		 */
    	pMAC->rx_desc[i]->bufptr = VA_TO_PHYS(buf_ptr);		// 接收缓存物理地址
    	pMAC->rx_buf[i] = (void *)buf_ptr;					// 接收缓冲区数组

        pMAC->rx_desc[i]->control |= ETHER_MAX_LEN;
        pMAC->rx_desc[i]->status = rxdesc0_stat_own;        // set as owned by dma
 
        desc_ptr += DMA_DESC_SIZE;
    	buf_ptr  += RX_BUF_SIZE;
	}

	return 0;
}

//***************************************************************************************
// initialize the device
//***************************************************************************************

extern int ls2k_gmac_init_hook(const void *dev);

STATIC_DRV int GMAC_initialize(const void *dev, void *arg)
{
	GMAC_t *pMAC = (GMAC_t *)dev;

	if (dev == NULL)
    {
        return -1;
    }

    /* This is for stuff that only gets done once
     */
    if (!pMAC->initialized)
    {
        ls2k1000_gmac_init_io_base(pMAC);

        ls2k_gmac_init_hook(dev);
        
    	/* initialize gmac registers
    	 */
    	GDMA_INIT_DESC(pMAC);
        ls2k_gmac_init_hw(pMAC, (unsigned char *)arg);

        pMAC->p_event = osal_event_create(pMAC->dev_name, 0);
        if (NULL == pMAC->p_event)
        {
            printk("create GMAC event fail.\r\n");
            return -1;
        }

        /* install the interrupt handler
         */
        ls2k_install_irq_handler(pMAC->irqNum, ls2k_gmac_irq_handler, (void *)pMAC);
        ls2k_set_irq_routeip(pMAC->irqNum, INT_ROUTE_IP0);
        
        ls2k_set_irq_triggermode(pMAC->irqNum, INT_TRIGGER_LEVEL);
        
        /* flag as initialized.
         */
        pMAC->initialized = 1;
    }

    /*
     * gdma start to receive and transmit, or not?
     *
     * TODO Some problem when FreeRTOS
     */
    GDMA_STOP(pMAC->hwGDMA);
    GMAC_STOP(pMAC->hwGMAC);
    pMAC->started = 0;

    printk("GMAC%i controller initialized, version %04X. \r\n",
#if BSP_USE_GMAC0
           (pMAC == devGMAC0) ? 0 :
#endif
#if BSP_USE_GMAC1
           (pMAC == devGMAC1) ? 1 :
#endif
           -1, pMAC->gmacVersion);

    return 0;
}

//***************************************************************************************
// Receive one packet
//***************************************************************************************

static int ls2k_gmac_recv_packet_internal(GMAC_t *pMAC, unsigned char *buf, int size)
{
    int rx_len;
    unsigned long buf_ptr;
    unsigned int status;

    status = pMAC->rx_desc[pMAC->rx_head]->status;
    if (status & rxdesc0_stat_own)
    {
        /**************************************************
         * Info user while no packet in
         */
        DBG_OUT("rxdesc_own error rx_head=%i\r\n", pMAC->rx_head);
        return 0;
    }

    // DBG_OUT("rx_head = %i\r\n", pMAC->rx_head);

    /******************************************************
     * When current receice desc is not ownered by DMA
     */

    if (status & rxdesc0_stat_es)   /* RX Error Summary */
    {
        pMAC->rx_errors++;
    }

    if (status & rxdesc0_stat_le)   /* RX Length Error */
    {
        pMAC->rx_length_err++;
    }

    if (status & rxdesc0_stat_ce)  /* RX CRC Error */
    {
        pMAC->rx_crc_err++;
    }

    buf_ptr = (unsigned long)pMAC->rx_buf[pMAC->rx_head];
    
    /******************************************************
     * If no errors, accept packet
     */
    if (((status & rxdesc0_stat_es) == 0) &&
        ((status & rxdesc0_stat_fs) == rxdesc0_stat_fs) &&
        ((status & rxdesc0_stat_ls) == rxdesc0_stat_ls))
    {
        pMAC->rx_pkts++;

        /* get total length of receive data.
         */
        rx_len = (status & rxdesc0_stat_fl_mask) >> 16;

        rx_len = rx_len > size ? size : rx_len;

        /**************************************************
         * 用户自定义缓冲区, 需要复制; 否则直接使用
         */
        if (buf_ptr != (unsigned long)buf)
        {
            printk("gmac rx error!\r\n");
            // memcpy((void *)buf, (void *)buf_ptr, rx_len);
        }
    }
    else	// has error!
    {
        printk("gmac rx dropped!, desc[%i]->status=0x%08x\r\n", pMAC->rx_head, status);
        pMAC->rx_dropped++;
        rx_len = 0;
    }

    /**************************************************
     * set up the receive dma buffer
     */

    pMAC->rx_desc[pMAC->rx_head]->control = ETHER_MAX_LEN + 18;
    if (pMAC->descmode == CHAINMODE)
    {
    	pMAC->rx_desc[pMAC->rx_head]->control |= rxdesc1_ctrl_rch;
    }
    else if (pMAC->rx_head == NUM_RX_DMA_DESC - 1)
    {
        pMAC->rx_desc[pMAC->rx_head]->control |= rxdesc1_ctrl_rer;
    }

    pMAC->rx_desc[pMAC->rx_head]->status = rxdesc0_stat_own;        // set as owned by dma
    pMAC->rx_desc[pMAC->rx_head]->bufptr = VA_TO_PHYS(buf_ptr);	    // Physical address

    /* Now force the DMA to start receive
     */
    pMAC->hwGDMA->rxpoll = 1;

    /*
     * increment the buffer index
     */
    pMAC->rx_head++;
    if (pMAC->rx_head >= NUM_RX_DMA_DESC)
    {
        pMAC->rx_head = 0;
    }

    return rx_len;
}

//***************************************************************************************
// ls2k_gmac_read()
//***************************************************************************************

STATIC_DRV int GMAC_read(const void *dev, void *buf, int size, void *arg)
{
	GMAC_t *pMAC = (GMAC_t *)dev;

    if ((pMAC == NULL) || (buf == NULL))
    {
        return -1;
    }

    return ls2k_gmac_recv_packet_internal(pMAC, (unsigned char *)buf, size);
}

//***************************************************************************************
// Send one packet
//***************************************************************************************

static int ls2k_gmac_send_packet_internal(GMAC_t *pMAC, unsigned char *buf, int size)
{
    int tx_len = size;
    unsigned long buf_ptr;
    unsigned int status;

    status = pMAC->tx_desc[pMAC->tx_head]->status;
    if (status & txdesc0_stat_own)
    {
        /**************************************************
         * Info user while no buffer to send
         */
        DBG_OUT("tx desc own error tx_head=%i\r\n", pMAC->tx_head);
        return 0;
    }

    // DBG_OUT("tx_head = %i\r\n", pMAC->tx_head);

    /******************************************************
     * Has GMAC Transmit Queue to become available.
     */

    /* Copy data to send buffer
     */
    if (tx_len > ETHER_MAX_LEN)
        tx_len = ETHER_MAX_LEN;

    buf_ptr = (unsigned long)pMAC->tx_buf[pMAC->tx_head];

    /******************************************************
     * 用户自定义缓冲区, 需要复制; 否则直接使用
     */
    if (buf_ptr != (unsigned long)buf)
    {
        printk("gmac tx error!\r\n");
        // memcpy((void *)buf_ptr, (void *)buf, tx_len);
    }

    /* send it off
     */
    pMAC->tx_desc[pMAC->tx_head]->control = tx_len & txdesc1_ctrl_bs1_mask;
    pMAC->tx_desc[pMAC->tx_head]->status = txdesc0_stat_own;
#if ENH_DESC
    pMAC->tx_desc[pMAC->tx_head]->status |= txdesc0_stat_ic |		// Interrption on Complete
    									    txdesc0_stat_ls |		// Last  Segment
    									    txdesc0_stat_fs;		// First Segment
#else
    pMAC->tx_desc[pMAC->tx_head]->control |= txdesc1_ctrl_ic |		// Interrption on Complete
    									     txdesc1_ctrl_ls |		// Last  Segment
    									     txdesc1_ctrl_fs;		// First Segment
#endif
	if (pMAC->descmode == CHAINMODE)							    // chain mode
	{
#if ENH_DESC
    	pMAC->tx_desc[pMAC->tx_head]->status |= txdesc0_stat_tch;
#else
        pMAC->tx_desc[pMAC->tx_head]->control |= txdesc1_ctrl_tch;
#endif
    }
	else if (pMAC->tx_head == NUM_TX_DMA_DESC - 1)
	{
#if ENH_DESC
    	pMAC->tx_desc[pMAC->tx_head]->status |= txdesc0_stat_ter;	// last of ring mode
#else
        pMAC->tx_desc[pMAC->tx_head]->control |= txdesc1_ctrl_ter;	// last of ring mode
#endif
    }
	pMAC->tx_desc[pMAC->tx_head]->bufptr = VA_TO_PHYS(buf_ptr);		// Physical address

#if GMAC_DEBUG
    DBG_OUT("tramsfer desc[%i], datalen=%i\r\n", pMAC->tx_head, tx_len);
#endif

    pMAC->tx_pkts++;

    /* Now force the DMA to start transmission
     */
    pMAC->hwGDMA->txpoll = 1;

    /******************************************************
     * 上面语句结束后, DMA 立即进入中断处理.
     ******************************************************/

    /*
     * increment the buffer index
     */
    pMAC->tx_head++;
    if (pMAC->tx_head >= NUM_TX_DMA_DESC)
    {
        pMAC->tx_head = 0;
    }

    return tx_len;
}

//***************************************************************************************
// ls2k_gmac_write()
//***************************************************************************************

STATIC_DRV int GMAC_write(const void *dev, void *buf, int size, void *arg)
{
	GMAC_t *pMAC = (GMAC_t *)dev;

    if ((pMAC == NULL) || (buf == NULL))
    {
        return -1;
    }

    return ls2k_gmac_send_packet_internal(pMAC, (unsigned char *)buf, size);
}

//***************************************************************************************
// Show interface statistics
//***************************************************************************************

static void ls2k_gmac_stats(GMAC_t *pMAC)
{
    printk("Interrupts:%-8u\r\n", pMAC->interrupts);
    printk("    RX Interrupts:%-8u\r\n", pMAC->rx_interrupts);
    printk("    TX Interrupts:%-8u\r\n", pMAC->tx_interrupts);
    printk("RX Packets:%-8u\r\n", pMAC->rx_pkts);
    printk("    RX Error Summary:%-8u\r\n", pMAC->rx_errors);
    printk("    RX Length Error:%-8u\r\n", pMAC->rx_length_err);
    printk("    RX CRC Error:%-8u\r\n", pMAC->rx_crc_err);
    printk("    RX dropped:%-8u\r\n", pMAC->rx_dropped);
    printk("TX Packets:%-8u\r\n", pMAC->tx_pkts);
    printk("    TX Error Summary:%-8u\r\n", pMAC->tx_errors);
    printk("    TX IP Header Error:%-8u\r\n", pMAC->tx_ipheader_err);
    printk("    TX Payload Checksum Error:%-8u\r\n", pMAC->tx_playload_err);
    printk("    TX Defered Bit Error:%-8u\r\n", pMAC->tx_defered);
    printk("    TX Collsion Error:%-8u\r\n", pMAC->tx_collsions);
    printk("Fatal Error:%-8u\r\n", pMAC->dma_fatal_err);
}

//***************************************************************************************
// 等待接收事件
//***************************************************************************************

int ls2k_gmac_wait_receive_packet(const void *dev, unsigned char **bufptr)
{
    unsigned int status, recv_event;
	GMAC_t *pMAC = (GMAC_t *)dev;

    if ((pMAC == NULL) || (bufptr == NULL))
    {
        return -1;
    }

    /*
     * Exsisting received packet, return immediately
     */
    status = READ_REG32(&pMAC->rx_desc[pMAC->rx_head]->status);
    if ((status & rxdesc0_stat_own) == 0)
    {
        *bufptr = pMAC->rx_buf[pMAC->rx_head];              /* receive buffer */
        return (status & rxdesc0_stat_fl_mask) >> 16;       /* length of receive data. */
    }

    *bufptr = NULL;

    /*
     * Waiting Rx Event...
     */
    recv_event = osal_event_receive(pMAC->p_event,
                                    GMAC_RX_EVENT,
                                    OSAL_EVENT_FLAG_AND | OSAL_EVENT_FLAG_CLEAR,
                                    OSAL_WAIT_FOREVER);

    if (recv_event != GMAC_RX_EVENT)
    {
        return -1;
    }

    /*
     * Confirm received packet
     */
    status = READ_REG32(&pMAC->rx_desc[pMAC->rx_head]->status);
    if ((status & rxdesc0_stat_own) == 0)
    {
        *bufptr = pMAC->rx_buf[pMAC->rx_head];              /* receive buffer */
        return (status & rxdesc0_stat_fl_mask) >> 16;       /* length of receive data. */
    }

    return 0;  // IS'S OK
}

//***************************************************************************************
// 等待发送事件
//***************************************************************************************

int ls2k_gmac_wait_transmit_idle(const void *dev, unsigned char **bufptr)
{
    unsigned int status, recv_event;
	GMAC_t *pMAC = (GMAC_t *)dev;

    if ((pMAC == NULL) || (bufptr == NULL))
    {
        return -1;
    }

    /*
     * Exsisting idle tx buffer, return immediately
     */
    status = READ_REG32(&pMAC->tx_desc[pMAC->tx_head]->status);
    if ((status & txdesc0_stat_own) == 0)
    {
        *bufptr = pMAC->tx_buf[pMAC->tx_head];
        return ETHER_MAX_LEN;
    }

    *bufptr = NULL;

    /*
     * Waiting Tx Event...
     */
    recv_event = osal_event_receive(pMAC->p_event,
                                    GMAC_TX_EVENT,
                                    OSAL_EVENT_FLAG_AND | OSAL_EVENT_FLAG_CLEAR,
                                    OSAL_WAIT_FOREVER);

    if (recv_event != GMAC_TX_EVENT)
    {
        return -1;
    }

    /*
     * Confirm idle tx buffer
     */
    status = READ_REG32(&pMAC->tx_desc[pMAC->tx_head]->status);
    if ((status & txdesc0_stat_own) == 0)
    {
        *bufptr = pMAC->tx_buf[pMAC->tx_head];
        return ETHER_MAX_LEN;
    }

    return 0; // IS'S OK
}

//***************************************************************************************
// Driver ioctl handler
//***************************************************************************************

STATIC_DRV int GMAC_ioctl(const void *dev, int cmd, void *arg)
{
    int rt = 0;
	GMAC_t *pMAC = (GMAC_t *)dev;

    if (dev == NULL)
    {
        return -1;
    }

    switch (cmd)
    {
    	case IOCTL_GMAC_START:              /* start the GMAC hardware */
            ls2k_gmac_set_start(pMAC);
            delay_us(100);
            break;

        case IOCTL_GMAC_STOP:               /* stop the GMAC hardware */
            ls2k_gmac_set_stop(pMAC);
            delay_us(100);
            break;

        case IOCTL_GMAC_RESET:              /* stop then reset GMAC */
            ls2k_gmac_do_reset(pMAC);
            break;

        case IOCTL_GMAC_SET_MACADDR:
            rt = -2;
            break;

        case IOCTL_GMAC_SET_TIMEOUT:        /* set transmit/receive time out */
            pMAC->timeout = (long)arg;
            break;

        case IOCTL_GMAC_IS_RUNNING:         /* GMAC is running? started or not */
            rt = pMAC->started;
            break;

    	case IOCTL_GMAC_SHOW_STATS:
    		ls2k_gmac_stats(pMAC);
    		break;

    	default:
    		break;
    }

    return rt;
}

//***************************************************************************************
// interrupt handler
//***************************************************************************************

static void ls2k_gmac_irq_handler(int vector, void *arg)
{
	GMAC_t *pMAC = (GMAC_t *)arg;
	volatile unsigned int dma_status;
    volatile int rx_flag=0, tx_flag=0;

    if (pMAC == NULL)
    {
        printk("%s", "gmac interrupt error: arg == NULL!\r\n");
        return;
    }

    pMAC->interrupts++;

	/*
     * 读 GMAC-DMA 状态寄存器, 判断中断是否由本设备(DMA)产生, 如果不是, 返回
	 */
    dma_status = pMAC->hwGDMA->status;
	pMAC->hwGDMA->status = dma_status;      // & ~0x1FFFF;

	if (0 == dma_status)
	{
    	return;
    }

    /*
     * GMAC Line interface Interrupt
     */
    if (dma_status & gdma_status_lifi)      
    {
        pMAC->hwGMAC->intmask = gmac_intmask_rgmii;
        
        // TODO Link status change
        
        return;
    }

	GDMA_INT_DIS(pMAC->hwGDMA);	            /* disable gdma interrupt */

	/******************************************************
	 * 开始处理  GMAC-DMA 中断
	 */

	/* Fatal Bus Error Interrutp
	 */
	if (dma_status & gdma_status_fbei)
	{
#if (GMAC_DEBUG)
		DBG_OUT("%s", "Fatal Error: gmac-dma Bus Error Interrutp! Restart it.\r\n");
#endif
		pMAC->dma_fatal_err++;
        ls2k_gmac_do_reset(pMAC);
	    return;
	}

	if (dma_status & gdma_status_nis)
	{
    	pMAC->dma_normal_intrs++;
    }

	if (dma_status & gdma_status_ais)
	{
    	pMAC->dma_abnormal_intrs++;
    }

	/******************************************************
	 * 接收部分的处理
	 */

	/* Normal Receive Interrupt
	 */
	if (dma_status & gdma_status_rxi)
	{
		/* Must prevent the dead loop: Flag "reserved" bit as "done"
		 */
	    while (((READ_REG32(&pMAC->rx_desc[pMAC->rx_tail]->status ) & rxdesc0_stat_own) == 0) &&
	    	   ((READ_REG32(&pMAC->rx_desc[pMAC->rx_tail]->control) & rxdesc1_ctrl_UDF) == 0))
	    {
	        pMAC->rx_interrupts++;
	        
	        OR_REG32(&pMAC->rx_desc[pMAC->rx_tail]->control, rxdesc1_ctrl_UDF);

	        pMAC->rx_tail++;

	        if (pMAC->rx_tail >= NUM_RX_DMA_DESC)
	        {
                pMAC->rx_tail = 0;
            }

            rx_flag = 1;
	    }

	    if (rx_flag)
	    {
	        osal_event_send(pMAC->p_event, GMAC_RX_EVENT);
	    }
	}

	/* Receive Buffer Unavailable
	 */
	if (dma_status & gdma_status_rxbufu)
	{
    	pMAC->rx_buffer_unavailable++;
    }

	/* Receive Process Stopped
	 */
	if (dma_status & gdma_status_rxstop)
	{
#if (GMAC_DEBUG)
		DBG_OUT("%s", "Warning: gmac-dma Receive Process Stopped!\n");
#endif
		pMAC->rx_stopped++;
#if !defined(DUAL_NIC_REDUNDANCY)
		GDMA_STOP_RX(pMAC->hwGDMA);
		GDMA_INIT_RXDESC(pMAC);
		GDMA_INIT_RXDESC_CUR(pMAC);
		GDMA_START_RX(pMAC->hwGDMA);
#endif
	}

	/******************************************************
	 * 发送部分的处理
	 */

	/* Normal Transmit Interrupt
	 */
	if (dma_status & gdma_status_txi)
	{
	    unsigned int sr;

	    while ((((sr = READ_REG32(&pMAC->tx_desc[pMAC->tx_tail]->status)) & txdesc0_stat_own) == 0) &&
	    	   ((READ_REG32(&pMAC->tx_desc[pMAC->tx_tail]->control) & txdesc1_ctrl_bs1_mask) != 0))
	    {
	        WRITE_REG32(&pMAC->tx_desc[pMAC->tx_tail]->control, 0);
	        
	        pMAC->tx_interrupts++;

	        if (sr & txdesc0_stat_es)			// TX Error Summary
	        {
            	pMAC->tx_errors++;
            }

	        if (sr & txdesc0_stat_ihe)			// TX IP Header Error
	        {
            	pMAC->tx_ipheader_err++;
            }

	        if (sr & txdesc0_stat_pce)			// Payload Checksum Error
	        {
            	pMAC->tx_playload_err++;
            }

	        if (sr & txdesc0_stat_db)			// Defered Bit
	        {
            	pMAC->tx_defered++;
            }

	        if (sr & txdesc0_stat_cc_mask)		// bits: 6-3, Collsion Count
	        {
            	pMAC->tx_collsions += (sr & txdesc0_stat_cc_mask) >> 3;
            }

	        pMAC->tx_tail++;

	        if (pMAC->tx_tail >= NUM_TX_DMA_DESC)
	        {
                pMAC->tx_tail = 0;
            }

	        tx_flag = 1;
	    }

	    if (tx_flag)
	    {
	        osal_event_send(pMAC->p_event, GMAC_TX_EVENT);
        }
	}

	/* Transmit Underflow
	 */
	if (dma_status & gdma_status_txunf)
	{
		pMAC->tx_underflow++;
	}

	/* Transmit Buffer Unavailable, 该中断是否取消 ?
	 */
	if (dma_status & gdma_status_txbufu)
	{
    	pMAC->tx_buffer_unavailable++;
    }

	/* Transmit Process Stopped
	 */
	if (dma_status & gdma_status_txstop)
	{
#if (GMAC_DEBUG)
		DBG_OUT("%s", "Warning: gmac-dma Transmit Process Stopped!\r\n");
#endif
		pMAC->tx_stopped++;
#if !defined(DUAL_NIC_REDUNDANCY)
		GDMA_STOP_TX(pMAC->hwGDMA);
		GDMA_INIT_TXDESC(pMAC);
		GDMA_INIT_TXDESC_CUR(pMAC);
		GDMA_START_TX(pMAC->hwGDMA);
#endif
	}

	/******************************************************
	 * GMAC 中断处理 结束
	 */

	/* open gdma interrupt
	 */
	GDMA_INT_EN(pMAC->hwGDMA);

	return;
}

//---------------------------------------------------------------------------------------

#if (PACK_DRV_OPS)
/******************************************************************************
 * GMAC driver operators
 */
static const driver_ops_t ls2k_gmac_drv_ops =
{
    .init_entry  = GMAC_initialize,
    .open_entry  = NULL,
    .close_entry = NULL,
    .read_entry  = GMAC_read,
    .write_entry = GMAC_write,
    .ioctl_entry = GMAC_ioctl,
};

const driver_ops_t *gmac_drv_ops = &ls2k_gmac_drv_ops;
#endif

/******************************************************************************
 * device name
 */
const char *ls2k_gmac_get_device_name(const void *pMAC)
{
    return ((GMAC_t *)pMAC)->dev_name;
}

#endif // #if BSP_USE_GMAC

/*
 * @@ END
 */
 
