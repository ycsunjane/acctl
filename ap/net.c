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
#include <linux/if_ether.h>

#include "message.h"
#include "dllayer.h"
#include "netlayer.h"
#include "log.h"
#include "thread.h"
#include "arg.h"
#include "link.h"

/* recv dllayer */
static void *__net_dllrecv(void *arg)
{
	struct message_t *msg;
	int rcvlen;

	while(1) {
		msg = malloc(sizeof(struct message_t));
		if(msg == NULL) {
			sys_warn("malloc memory for dllayer failed: %s\n", 
				strerror(errno));
			continue;
		}

		rcvlen = dll_rcv(msg->data, DLL_PKT_DATALEN);
		if(rcvlen < sizeof(struct ethhdr)) {
			free(msg);
			continue;
		}
		msg->proto = ETH;
		message_insert(msg);
	}
	return NULL;
}

/* recv netlayer */
void *__net_netrcv(void *arg)
{
	struct message_t *msg;
	int rcvlen;

	while(1) {
		msg = malloc(sizeof(struct message_t));
		if(msg == NULL) {
			sys_warn("malloc memory for dllayer failed: %s\n", 
				strerror(errno));
			continue;
		}

		rcvlen = tcp_rcv(msg->data, DLL_PKT_DATALEN);
		if(rcvlen < 0) {
			free(msg);
			continue;
		}
		msg->proto = TCP;
		message_insert(msg);
	}
	return NULL;
}

void net_init()
{
	struct sockarr_t *rcvsock = __insert_sockarr(__net_dllrecv);
	dll_init(&argument.nic[0], &rcvsock->sock, NULL, NULL);

	/* create pthread recv msg */
	__create_pthread(net_recv, head);
	sys_debug("Create pthread recv dllayer msg\n");
}

