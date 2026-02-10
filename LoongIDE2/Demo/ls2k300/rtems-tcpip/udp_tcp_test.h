/*
 * udp_tcp_test.h
 *
 * created: 2023-01-17
 *  author: 
 */

#ifndef _UDP_TCP_TEST_H
#define _UDP_TCP_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#define UDP_TSET    0
#if UDP_TSET
extern int udp_server_task_create(void);
#endif

#if !UDP_TSET
#define TCP_TSET    1
#if TCP_TSET
extern int tcp_server_task_create(void);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif // _UDP_TCP_TEST_H

