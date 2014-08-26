/*
 * =====================================================================================
 *
 *       Filename:  netlayer.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月26日 15时55分53秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __NETLAYER_H__
#define __NETLAYER_H__
#include <netinet/in.h>

extern struct nettcp_t tcp;

int tcp_connect(struct sockaddr_in addr, int reconnect);
int tcp_rcv(char *data, int size);
int tcp_sendpkt(char *data, int size);
#endif /* __NETLAYER_H__ */
