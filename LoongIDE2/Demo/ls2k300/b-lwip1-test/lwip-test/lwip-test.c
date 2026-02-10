/*
 * net-test.c
 *
 * created: 2022-03-19
 *  author: 
 */

//-----------------------------------------------------------------------------
// BSP
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <string.h>

#include "bsp.h"
#include "ls2k_gmac.h"
#include "lwip_test.h"

#if BSP_USE_OS

/******************************************************************************
 * LWIP RTOS 测试代码
 ******************************************************************************/

#if BSP_USE_GMAC0
char *local_IP  = "192.168.1.123";
char *remote_IP = "192.168.1.111";
#endif

#if BSP_USE_GMAC1
char *local_IP  = "192.168.1.124";
char *remote_IP = "192.168.1.111";
#endif

extern void ls2k_initialize_lwip(unsigned char *ip0, unsigned char *ip1);

void lwip_test(void)
{
	#if BSP_USE_GMAC
	{
        #if LIB_LWIP
        {
            ls2k_gmac_init(NULL, NULL);     // 引用 devGMAC0
        }
        #endif

        ls2k_initialize_lwip(NULL, NULL);

        #if (TEST_TCP_SERVER)
        {
            tcp_server_init();
        }
        #endif

        #if (TEST_TCP_CLIENT)
        {
            tcp_client_init();
        }
        #endif

        #if (TEST_UDP_SERVER)
        {
            udp_server_init();
        }
        #endif

        #if (TEST_UDP_CLIENT)
        {
            udp_client_init();
        }
        #endif

    }
    #endif

}

#else // #if BSP_USE_OS

/******************************************************************************
 * LWIP 裸机测试代码
 ******************************************************************************/

#include <stdio.h>

#ifdef LS2K500
#include "ls2k500.h"
#elif defined(LS2K1000)
#include "ls2k1000.h"
#endif

#if BSP_USE_GMAC0

  #include "lwip/init.h"
 #if LWIP_VERSION_MAJOR == 1
  #include "lwip/tcp_impl.h"
  #include "lwip/ip_frag.h"
 #elif LWIP_VERSION_MAJOR == 2
  #include "lwip/tcp.h"
  #include "lwip/ip.h"
 #else
  #error "unknown lwIP version."
 #endif
  #include "netif/etharp.h"
  #include "lwip/netif.h"
  
  extern void tcp_tmr(void);

  extern struct netif *p_gmac0_netif;

  extern void lwip_init(void);
  extern void ls2k_initialize_lwip(unsigned char *lip, unsigned char *ip1);
  extern void ethernetif_input(struct netif *netif);
#endif

//-----------------------------------------------------------------------------
// 裸机主循环
//-----------------------------------------------------------------------------

static int tcpclient_count = 0;

static void lwip_access_net(void *arg, int *next_interval)
{
    int rdbytes, wrbytes;

#if BSP_USE_GMAC0

#ifndef ETHERNETIF_INPUT_TMR
    ethernetif_input(p_gmac0_netif);
#endif

    tcp_tmr();      // lwip 裸机编程必须调用函数 tcp_tmr()

#if TEST_TCP_SERVER
    char tcpsvrBuf[TCP_SERVER_BUFSIZE];

    memset(tcpsvrBuf, 0, TCP_SERVER_BUFSIZE);

    if (tcpsvr_recv_data((unsigned char *)tcpsvrBuf, TCP_SERVER_BUFSIZE) > 0)
    {
        printk("FROM CLIENT: %s\r\n", tcpsvrBuf);

        sprintf(tcpsvrBuf, "REPLAY: tick count=%i\r\n", (unsigned int)get_clock_ticks());
        wrbytes = strlen(tcpsvrBuf);
        tcpsvr_send_data((unsigned char *)tcpsvrBuf, wrbytes);
    }
#endif

#if TEST_TCP_CLIENT
    char tcpcliBuf[TCP_CLIENT_BUFSIZE];

    if (tcpclient_count < 100)
    {
        /*
         * 本次发送...
         */
        memset(tcpcliBuf, 0, TCP_CLIENT_BUFSIZE);
        sprintf(tcpcliBuf, "2K tick count=%i\r\n", get_clock_ticks());
        wrbytes = strlen(tcpcliBuf);
        wrbytes = tcpcli_send_data((unsigned char *)tcpcliBuf, wrbytes);
        // printk("SEND: %s", tcpcliBuf);

        tcp_tmr();

        /*
         * 上一次发送的回答
         */
        memset(tcpcliBuf, 0, TCP_CLIENT_BUFSIZE);
        rdbytes = tcpcli_recv_data((unsigned char *)tcpcliBuf, TCP_CLIENT_BUFSIZE);
        if (rdbytes > 0)
        {
            tcpclient_count++;
            printk("SERVER REPLAY: %s\r\n", tcpcliBuf);
        }
    }
    else
    {
        tcpcli_disconnect();
    }
#endif

#if TEST_UDP_SERVER
    char udpsvrBuf[UDP_SERVER_BUFSIZE];

    memset(udpsvrBuf, 0, UDP_SERVER_BUFSIZE);

    if (udpsvr_recv_data((unsigned char *)udpsvrBuf, UDP_SERVER_BUFSIZE) > 0)
    {
        printk("RECV: %s\r\n", udpsvrBuf);

        sprintf(udpsvrBuf, "REPLAY: tick count=%i\r\n", (unsigned int)get_clock_ticks());
        wrbytes = strlen(udpsvrBuf);
        udpsvr_send_data((unsigned char *)udpsvrBuf, wrbytes);
    }
#endif

#if TEST_UDP_CLIENT
    char udpcliBuf[UDP_CLIENT_BUFSIZE];

    memset(udpcliBuf, 0, UDP_CLIENT_BUFSIZE);

    sprintf(udpcliBuf, "tick count=%i", get_clock_ticks());
    wrbytes = strlen(udpcliBuf);
    wrbytes = udpcli_send_data((unsigned char *)udpcliBuf, wrbytes);
    //printk("SEND: %s", udpcli_tx_buf);

    tcp_tmr();

    rdbytes = udpcli_recv_data((unsigned char *)udpcliBuf, UDP_CLIENT_BUFSIZE);
    if (rdbytes > 0)
    {
        printk("RECV: %s\r\n", udpcliBuf);
    }

#endif

#endif // #if BSP_USE_GMAC0

    (void)rdbytes;
    (void)wrbytes;
    (void)tcpclient_count;
}

//-----------------------------------------------------------------------------
// 测试程序
//-----------------------------------------------------------------------------

int lwip_test(void)
{
	#if BSP_USE_GMAC0
	{
	    unsigned char lip[4] = {192, 168, 1, 123};
	    unsigned char rip[4] = {192, 168, 1, 111};

        #if LIB_BSP
            ls2k_gmac_init(NULL, NULL);
        #endif

        lwip_init();                        // Initilaize the LwIP stack
        ls2k_initialize_lwip(lip, NULL);    // Initilaize the LwIP & GMAC0 glue

        #if TEST_TCP_SERVER
            tcp_server_initialize(lip);
        #endif

        #if TEST_TCP_CLIENT
            tcp_client_initialize(lip, rip);
        #endif

        #if TEST_UDP_SERVER
            udp_server_initialize(lip);
        #endif

        #if TEST_UDP_CLIENT
            udp_client_initialize(lip, rip);
        #endif

        #if (TEST_TCP_SERVER || TEST_TCP_CLIENT) && (defined(TCP_TMR))
            ls2k_start_lwip_timer();
        #endif

        // ftpd_init();

        (void)lip;
        (void)rip;
    }
	#endif

    //-----------------------------------------------------
    // 使用 periodic function
    //-----------------------------------------------------

    #ifdef LS2K300
    {
        extern int periodic_func_add(const char *fname, void (*func)(void *, int *), void *arg, int interval);

        #if TEST_TCP_CLIENT || TEST_UDP_CLIENT
        {
            periodic_func_add("lwiptest", lwip_access_net, devGMAC0, 100);
        }
        #else // #elif TEST_TCP_SERVER || TEST_UDP_SERVER
        {
            periodic_func_add("lwiptest", lwip_access_net, devGMAC0, 10);
        }
        #endif
    }
    #else
    {
        for (;;)
        {
            lwip_access_net(NULL, NULL);

            delay_ms(100);

            tcp_tmr();
        }
    }
    #endif

    /*
     * Never goto here!
     */
    return 0;
}

#endif // #if BSP_USE_OS

/*
 * @@ End
 */
