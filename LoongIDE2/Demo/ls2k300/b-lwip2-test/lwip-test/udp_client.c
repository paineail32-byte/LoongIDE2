/*
 * udp_client.c
 *
 * created: 2021/6/24
 *  author: 
 */
 
#include "lwip_test.h"

#if TEST_UDP_CLIENT

#include "bsp.h"
#include <string.h>

#if BSP_USE_OS

#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/sys.h"

static char udp_msg[] = "this is a UDP test package";

static void udp_client_thread(void *arg)
{
	struct sockaddr_in remote_addr;
	int sock_fd;
	int err;
	int count = 0;

	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd == -1)
    {
		printk("failed to create sock_fd!\n");
		return;
	}

	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr(remote_IP);     // 192.168.1.111:9063
	remote_addr.sin_port = htons(UDP_REMOTE_PORT);

	while (1)
    {
		err = sendto(sock_fd,
                     (char *)udp_msg,
                     sizeof(udp_msg),
                     0,
					 (struct sockaddr *)&remote_addr,
                     sizeof(remote_addr));
		count++;
		printk("==> send count %d\r\n", count);
		
		delay_ms(500);
	}
}

void udp_client_init(void)
{
	sys_thread_new("udp_client_thread",
                    udp_client_thread,
                    NULL,
                    DEFAULT_THREAD_STACKSIZE,
            #ifdef OS_FREERTOS
                    DEFAULT_THREAD_PRIO - 1
            #else
                    DEFAULT_THREAD_PRIO + 1
            #endif
                   );
}

#else // #if BSP_USE_OS

#include <string.h>

#include "lwip/init.h"
#include "lwip/udp.h"

//---------------------------------------------------------------------------------------

static struct udp_pcb *m_udpcli_pcb = NULL;         // 定义一个UDP的协议控制块

static unsigned int m_udpcli_flag = 0;

static char udpcli_rx_buf[UDP_CLIENT_BUFSIZE];      // 定义用来发送数据的缓存
static char udpcli_tx_buf[UDP_CLIENT_BUFSIZE];      // 定义用来接收数据的缓存

//---------------------------------------------------------------------------------------

/*
 * 接收到数据包将要调用的函数
 */
static void udp_client_recv_callback(void *arg,
                                     struct udp_pcb *upcb,
                                     struct pbuf *p,
                                #if LWIP_VERSION_MAJOR == 1
                                     struct ip_addr *addr,
                                #else
                                     ip_addr_t *addr,
                                #endif
                                     uint16_t port)
{
    if (p != NULL)
    {
        if ((p->tot_len) >= UDP_CLIENT_BUFSIZE)         // 如果收的的数据大于缓存
        {      
            memcpy(udpcli_rx_buf, p->payload, UDP_CLIENT_BUFSIZE);
            udpcli_rx_buf[UDP_CLIENT_BUFSIZE-1] = 0;
        }
        else
        {
            memcpy(udpcli_rx_buf, p->payload, p->tot_len);
            udpcli_rx_buf[p->tot_len] = 0;
        }
        
        m_udpcli_flag |= LWIP_NEW_DATA;                 // 收到了新的数据
        pbuf_free(p);
    }
}

/*
 * 发送数据
 */
static void udp_client_send_data(void)
{
    err_t err;
    struct pbuf *tmpbuf;

    if ((m_udpcli_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA)
    {
        int thislen = strlen((char *)udpcli_tx_buf);

        tmpbuf = pbuf_alloc(PBUF_TRANSPORT, thislen, PBUF_RAM);
        tmpbuf->payload = udpcli_tx_buf;

        err = udp_send(m_udpcli_pcb, tmpbuf);           // 发送数据
        if (err != ERR_OK)
        {
            printf("UDP client send fail!");
        }
        
        m_udpcli_flag &= ~LWIP_SEND_DATA;               // 清除发送数据的标志
        pbuf_free(tmpbuf);
    }
}

/*
 * 初始化UDP客户端
 */
void udp_client_initialize(unsigned char *lip, unsigned char *rip)
{
    err_t err;
    ip_addr_t local_ip, remote_ip;

    IP4_ADDR(&local_ip, lip[0], lip[1], lip[2], lip[3]);
    IP4_ADDR(&remote_ip, rip[0], rip[1], rip[2], rip[3]);

    m_udpcli_pcb = udp_new();                           // 新建一个UDP协议控制块
    if (m_udpcli_pcb != NULL)
    {
        udp_bind(m_udpcli_pcb, &local_ip, UDP_LOCAL_PORT);
        err = udp_connect(m_udpcli_pcb, &remote_ip, UDP_REMOTE_PORT);   // 设置连接到远程主机
        if (err == ERR_OK)
        {
            udp_recv(m_udpcli_pcb, udp_client_recv_callback, NULL);     // 注册接收回调函数
        }
        else
        {
            udp_remove(m_udpcli_pcb);
        }
    }
}

//---------------------------------------------------------------------------------------

int udpcli_send_data(unsigned char *buf, int buflen)
{
    int thislen = buflen < UDP_CLIENT_BUFSIZE-1 ? buflen : UDP_CLIENT_BUFSIZE-1;
    memcpy(udpcli_tx_buf, buf, thislen);
    udpcli_tx_buf[thislen] = 0;
    m_udpcli_flag |= LWIP_SEND_DATA;            // 标记有数据需要发送
    udp_client_send_data();

    return thislen;
}

int udpcli_recv_data(unsigned char *buf, int buflen)
{
    if ((m_udpcli_flag & LWIP_NEW_DATA) == LWIP_NEW_DATA)
    {
        int thislen = strlen(udpcli_rx_buf);
        thislen = thislen < buflen ? thislen : buflen;
        memcpy(buf, udpcli_rx_buf, thislen);
        buf[thislen] = 0;
        m_udpcli_flag &= ~LWIP_NEW_DATA;        // 清除接收数据的标志
        return thislen;
    }

    return 0;
}

#endif // #if BSP_USE_OS

#endif // #if TEST_UDP_CLIENT


