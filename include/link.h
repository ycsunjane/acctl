/*
 * =====================================================================================
 *
 *       Filename:  link.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月27日 09时43分55秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __LINK_H__
#define __LINK_H__
#include <sys/epoll.h>
#include <stdint.h>
#include "dllayer.h"
#include "netlayer.h"

enum {
	MSG_PROTO_ETH = 8000,
	MSG_PROTO_TCP,
};

struct sockarr_t {
	struct epoll_event ev;

	int sock;
	uint32_t retevents;
	void *(*func) (void *);
	void *arg;

	struct sockarr_t *next;
};

void net_epoll_init();

int delete_sockarr(int sock);
struct sockarr_t * insert_sockarr(int sock, void *(*func) (void *), void *arg);

void * net_recv(void *arg);
int net_send(int proto, int sock, char *dmac, char *msg, int size);
#endif /* __LINK_H__ */
