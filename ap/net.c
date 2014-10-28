/*
 * ============================================================================
 *
 *       Filename:  net.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/26/14 14:18:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * ============================================================================
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <linux/if_ether.h>

#include "message.h"
#include "dllayer.h"
#include "netlayer.h"
#include "log.h"
#include "thread.h"
#include "arg.h"
#include "link.h"
#include "process.h"

/* recv dllayer */
static void *net_dllrecv(void *arg)
{
	struct message_t *msg;
	int rcvlen;

	msg = malloc(sizeof(struct message_t) + 
		DLL_PKT_DATALEN);
	if(msg == NULL) {
		sys_warn("malloc memory for dllayer failed: %s\n", 
			strerror(errno));
		goto err;
	}

	rcvlen = dll_rcv(msg->data, DLL_PKT_DATALEN);
	if(rcvlen < (int)sizeof(struct ethhdr)) {
		free(msg);
		goto err;
	}
	msg->proto = MSG_PROTO_ETH;
	message_insert(msg);
err:
	return NULL;
}

/* recv netlayer */
void *__net_netrcv(void *arg)
{
	struct sockarr_t *sockarr = arg;
	unsigned int events = sockarr->retevents;
	int clisock = sockarr->sock;

	if(events & EPOLLRDHUP ||
		events & EPOLLERR ||
		events & EPOLLHUP) {
		sys_debug("Epool get err: %s(%d)\n", strerror(errno), errno);
		ac_lost();
		return NULL;
	}

	struct message_t *msg;
	msg = malloc(sizeof(struct message_t) + NET_PKT_DATALEN);
	if(msg == NULL) {
		sys_warn("malloc memory for dllayer failed: %s\n", 
			strerror(errno));
		goto err;
	}

	int rcvlen;
	struct nettcp_t tcp;
	tcp.sock = clisock;
	rcvlen = tcp_rcv(&tcp, msg->data, NET_PKT_DATALEN);
	if(rcvlen <= 0) {
		ac_lost();
		free(msg);
		goto err;
	}
	msg->proto = MSG_PROTO_TCP;
	message_insert(msg);
err:
	return NULL;
}

void net_init()
{
	int sock;
	/* init epoll */
	net_epoll_init();
	
	dll_init(&argument.nic[0], &sock, NULL, NULL);
	insert_sockarr(sock, net_dllrecv, NULL);

	/* create pthread recv msg */
	__create_pthread(net_recv, head);
	sys_debug("Create pthread recv dllayer msg\n");
}

