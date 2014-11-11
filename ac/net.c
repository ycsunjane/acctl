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
#include "chap.h"

static void *__net_dllrecv(void *arg)
{
	struct message_t *msg;
	msg = malloc(sizeof(struct message_t) + DLL_PKT_DATALEN);
	if(msg == NULL) {
		sys_warn("malloc memory for dllayer failed: %s\n", 
			strerror(errno));
		goto err;
	}

	int rcvlen;
	rcvlen = dll_rcv(msg->data, DLL_PKT_DATALEN);
	if(rcvlen < (int)sizeof(struct ethhdr)) {
		free(msg);
		goto err;
	}
	msg->len = rcvlen;

	sys_debug("datalink layer recive a packet\n");
	char *mac;
	struct msg_head_t *head;
	head = (struct msg_head_t *)(msg->data);
	mac = &head->mac[0];

	struct ap_hash_t *aphash;
	aphash = hash_ap(mac);
	if(aphash == NULL) {
		free(msg);
		goto err;
	}

	memcpy(aphash->ap.mac, mac, ETH_ALEN);
	msg->proto = MSG_PROTO_ETH; 
	message_insert(aphash, msg);
err:
	return NULL;
}

/* pthread recv netlayer */
static void *__net_netrcv(void *arg)
{
	struct sockarr_t *sockarr = arg;
	unsigned int events = sockarr->retevents;
	int clisock = sockarr->sock;

	if(events & EPOLLRDHUP ||
		events & EPOLLERR ||
		events & EPOLLHUP) {
		sys_debug("Epool get err: %s(%d)\n", strerror(errno), errno);
		ap_lost(clisock);
		return NULL;
	}

	if(!(events & EPOLLIN)) {
		sys_warn("Epoll unknow events: %u\n", events);
		return NULL;
	}

	struct message_t *msg;
	msg = malloc(sizeof(struct message_t) + NET_PKT_DATALEN);
	if(msg == NULL) {
		sys_warn("Malloc memory for message failed: %s(%d)\n", 
			strerror(errno), errno);
		goto err;
	}

	sys_debug("net layer recive a packet\n");
	struct nettcp_t tcp;
	tcp.sock = clisock;
	int rcvlen;
	rcvlen = tcp_rcv(&tcp, msg->data, NET_PKT_DATALEN);
	if(rcvlen <= 0) {
		ap_lost(clisock);
		free(msg);
		goto err;
	}
	msg->len = rcvlen;

	char *mac;
	struct msg_head_t *head;
	head = (struct msg_head_t *)(msg->data);
	mac = &head->mac[0];

	struct ap_hash_t *aphash;
	aphash = hash_ap(mac);
	if(aphash == NULL) {
		free(msg);
		goto err;
	}

	aphash->ap.sock = clisock;
	msg->proto = MSG_PROTO_TCP;
	message_insert(aphash, msg);
err:
	return NULL;
}

/* pthread broadcast */
static void *net_dllbrd(void *arg)
{
	struct msg_ac_brd_t *reqbuf = 
		malloc(sizeof(struct msg_ac_brd_t));
	if(reqbuf == NULL) {
		sys_err("Malloc broadcast message failed: %s(%d)\n",
			strerror(errno), errno);
		return NULL;
	}

	while(1) {
		ac.random = chap_get_random();
		sys_debug("Send a broadcast probe msg, random: %u (next %d second later)\n", 
			ac.random, argument.brditv);

		/* generate random0 */
		fill_msg_header(&reqbuf->header, MSG_AC_BRD, 
			&ac.acuuid[0], ac.random);

		/* first broad cast packet, there is no need calculate chap */
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

	while(1)
		tcp_accept(&tcplisten, __net_netrcv);
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
	create_pthread(net_recv, NULL);
	sys_debug("Create pthread net_recv msg\n");

	/* create pthread tcp listen */
	create_pthread(net_netlisten, NULL);
	sys_debug("Create pthread tcp listen\n");

	/* create pthread broadcast ac probe packet */
	create_pthread(net_dllbrd, NULL);
	sys_debug("Create pthread broadcast dllayer msg\n");
}

