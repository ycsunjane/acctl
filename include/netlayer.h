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

#define NET_PKT_DATALEN 	(2048)

struct nettcp_t {
	int sock;
	struct sockaddr_in addr;
};

int tcp_connect(struct nettcp_t *tcp);
int tcp_rcv(struct nettcp_t *tcp, char *data, int size);
int tcp_sendpkt(struct nettcp_t *tcp, char *data, int size);
void tcp_close(struct nettcp_t *tcp);
#ifdef SERVER
int tcp_listen(struct nettcp_t *tcp);
int tcp_accept(struct nettcp_t *tcp, void *func(void *));
#endif
#endif /* __NETLAYER_H__ */
