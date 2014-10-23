/*
 * ============================================================================
 *
 *       Filename:  net.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月20日 14时25分36秒
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
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <netinet/in.h>

#include "dllayer.h"
#include "aphash.h"
#include "net.h"
#include "log.h"
#include "msg.h"
#include "arg.h"
#include "thread.h"
#include "process.h"
#include "link.h"

static void *__net_dllrecv(void *arg)
{
	struct message_t *msg;
	int rcvlen;
	char *mac;
	struct msg_head_t *head;
	struct ap_t *ap;
	struct ap_hash_t *aphash;

	msg = malloc(sizeof(struct message_t) + DLL_PKT_DATALEN);
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

	head = (struct msg_head_t *)(msg->data);
	mac = &head->mac[0];
	aphash = hash_ap(mac);
	if(aphash == NULL) {
		free(msg);
		goto err;
	}

	ap = &aphash->ap;
	ap->timestamp = time(NULL);

	msg->proto = TCP; 
	message_insert(aphash, msg);

err:
	return NULL;
}

/* pthread recv netlayer */
static void *__net_netrcv(void *ptr)
{
	struct sockarr_t *sockarr = ptr;
	unsigned int events = sockarr->ev.events;
	int clisock = sockarr->sock;

	if(events & EPOLLRDHUP ||
		events & EPOLLERR ||
		events & EPOLLHUP) {
		ap_lost(clisock);
		return NULL;
	}

	if(!(events & EPOLLIN)) {
		sys_warn("Epoll unknow events: %u\n", events);
		return NULL;
	}

#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"

	struct message_t *msg;
	msg = malloc(sizeof(struct message_t) + NET_PKT_DATALEN);
	if(msg == NULL) {
		sys_warn("Malloc memory for message failed: %s(%d)\n", 
			strerror(errno), errno);
		goto err;
	}

	struct nettcp_t tcp;
	tcp.sock = clisock;
	tcp_rcv(&tcp, msg->data, NET_PKT_DATALEN);

	struct ethhdr *hdr;
	char *mac;
	hdr = (struct ethhdr *)&msg->data[0];
	mac = (char *)&hdr->h_source[0];

	struct ap_hash_t *aphash;
	aphash = hash_ap(mac);
	if(aphash == NULL) {
		free(msg);
		goto err;
	}

	struct ap_t *ap;
	ap = &aphash->ap;
	ap->timestamp = time(NULL);
	ap->sock = clisock;

	msg->proto = ETH;
	message_insert(aphash, msg);
err:
	return NULL;
}

/* pthread broadcast */
static void *net_dllbrd(void *arg)
{
	struct msg_ac_brd_t *reqbuf = 
		malloc(sizeof(struct msg_ac_brd_t));
	strncpy(reqbuf->header.acuuid, acuuid, UUID_LEN-1);
	reqbuf->header.msg_type = MSG_AC_BRD;
	memcpy(&reqbuf->header.mac[0], &argument.mac[0], ETH_ALEN);
	reqbuf->ipv4 = argument.addr;

	while(1) {
		sys_debug("Send a broadcast probe msg (next %d second later)\n", 
			argument.brditv);
		dll_brdcast((char *)reqbuf, sizeof(struct msg_ac_brd_t));
		sleep(argument.brditv);
	}
	return NULL;
}

static void *net_netlisten(void *arg)
{
	int ret;
	struct nettcp_t tcplisten;
	tcplisten.addr.sin_family = AF_INET;
	tcplisten.addr.sin_addr.s_addr = htonl(INADDR_ANY);
	tcplisten.addr.sin_port = htons(argument.port);
	ret = tcp_listen(&tcplisten);
	if(ret < 0) {
		sys_err("Create listen tcp failed\n");
		exit(-1);
	}

	while(1) {
		tcp_accept(&tcplisten, __net_netrcv);
		sys_debug("accept connect\n");
	}
}

void net_init()
{
	int sock;

	/* init epoll */
	net_epoll_init();

	/* init datalink layer */
	dll_init(&argument.nic[0], &sock, NULL, NULL);
	insert_sockarr(sock, __net_dllrecv, NULL);

	/* create pthread recv msg */
	__create_pthread(net_recv, NULL);
	sys_debug("Create pthread net_recv msg\n");

	/* create pthread tcp listen */
	__create_pthread(net_netlisten, NULL);
	sys_debug("Create pthread tcp listen\n");

	/* create pthread broadcast ac probe packet */
	__create_pthread(net_dllbrd, NULL);
	sys_debug("Create pthread broadcast dllayer msg\n");
}

