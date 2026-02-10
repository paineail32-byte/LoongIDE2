/*
 * Network configuration for LS2K in RTEMS Application
 */

#ifndef _RTEMS_NETWORKCONFIG_H_
#define _RTEMS_NETWORKCONFIG_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * The following will normally be set by the BSP if it supports
 * a single network device driver.  In the event, it supports
 * multiple network device drivers, then the user's default
 * network device driver will have to be selected by a BSP
 * specific mechanism.
 */

/* #define RTEMS_USE_BOOTP */

#include <bsp.h>

/*
 * Define RTEMS_SET_ETHERNET_ADDRESS if you want to specify the
 * Ethernet address here.  If RTEMS_SET_ETHERNET_ADDRESS is not
 * defined the driver will choose an address.
 */

static char ethernet_address[6] = { 0x56, 0x58, 0x00, 0x38, 0xFE, 0x22 };

#ifdef RTEMS_USE_LOOPBACK
/*
 * Loopback interface
 */
extern void rtems_bsdnet_loopattach();
static struct rtems_bsdnet_ifconfig loopback_config =
{
    "lo0",                          /* name */
    rtems_bsdnet_loopattach,        /* attach function */
    NULL,                           /* link to next interface */
    "127.0.0.1",                    /* IP address */
    "255.0.0.0",                    /* IP net mask */
};
#endif

/*
 * see "bsp.h"
 */
#define RTEMS_BSP_NETWORK_DRIVER_NAME       LS2K_NETWORK_DRIVER_NAME
#define RTEMS_BSP_NETWORK_DRIVER_ATTACH     LS2K_NETWORK_DRIVER_ATTACH

/*
 * Default network interface
 */
static struct rtems_bsdnet_ifconfig netdriver_config =
{
    RTEMS_BSP_NETWORK_DRIVER_NAME,      /* name */
    RTEMS_BSP_NETWORK_DRIVER_ATTACH,    /* attach function */

#ifdef RTEMS_USE_LOOPBACK
    &loopback_config,                   /* link to next interface */
#else
    NULL,                               /* No more interfaces */
#endif

#if 0
    NULL,                               /* BOOTP supplies IP address */
    NULL,                               /* BOOTP supplies IP net mask */
#else
    "192.168.1.123",                    /* IP address */
    "255.255.255.0",                    /* IP net mask */
#endif /* !RTEMS_USE_BOOTP */

    ethernet_address,                   /* Ethernet hardware address */
    0,                                  /* ignore broadcast */
    0,                                  /* mtu */
    0,                                  /* rbuf_count */
    0,                                  /* xbuf_count */
    0x300,                              /* port */
    9,                                  /* irq */
    0,                                  /* bpar */
    NULL                                /* driver control pointer */
};

/*
 * Network configuration
 */
struct rtems_bsdnet_config rtems_bsdnet_config =
{
    &netdriver_config,

#if (defined (RTEMS_USE_BOOTP))
    rtems_bsdnet_do_bootp,
#else
    NULL,
#endif

    8,                                  /* Default network task priority */
    1024 * 1024,                        /* Default mbuf capacity */
    1024 * 1024,                        /* Default mbuf cluster capacity */
    "ls2k",                             /* Host name */
    "ls2k",                             /* Domain name */
    "192.168.1.1",                      /* Gateway */
    "192.168.1.1",                      /* Log host */
    {"192.168.1.1"},                    /* Name server(s) */
    {"192.168.1.1"},                    /* NTP server(s) */
    0,                                  /* efficiency */
    0,                                  /* udp TX buffer size */
    0,                                  /* udp RX buffer size */
    0,                                  /* tcp TX buffer size */
    0,                                  /* tcp RX buffer size */
};

/*
 * For TFTP test application
 */
#if (defined (RTEMS_USE_BOOTP))
#define RTEMS_TFTP_TEST_HOST_NAME   "BOOTP_HOST"
#define RTEMS_TFTP_TEST_FILE_NAME   "BOOTP_FILE"
#else
#define RTEMS_TFTP_TEST_HOST_NAME   "XXX.YYY.ZZZ.XYZ"
#define RTEMS_TFTP_TEST_FILE_NAME   "tftptest"
#endif

/*
 * For NFS test application
 *
 * NFS server/path to mount and a directory to ls once mounted
 */
#define RTEMS_NFS_SERVER            "192.168.1.220"
#define RTEMS_NFS_SERVER_PATH       "/home"
#define RTEMS_NFS_LS_PATH           "/mnt/nfstest"

#ifdef  __cplusplus
}
#endif

#endif /* _RTEMS_NETWORKCONFIG_H_ */
