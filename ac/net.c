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

#include "dllayer.h"
#include "netlayer.h"
#include "aphash.h"
#include "net.h"
#include "log.h"
#include "msg.h"
#include "arg.h"
#include "thread.h"
#include "process.h"
#include "link.h"

/* pthread broadcast */
static void *__net_dllbrd(void *arg)
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

/* pthread recv dllayer */
static void *__net_dllrecv(void *arg)
{
	struct message_t *msg;
	int rcvlen;
	char *mac;
	struct msg_head_t *head;
	struct ap_t *ap;
	struct ap_hash_t *aphash;

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

		head = (struct msg_head_t *)(msg->data);
		mac = &head->mac[0];
		aphash = hash_ap(mac);
		if(aphash == NULL) {
			free(msg);
			continue;
		}

		ap = &aphash->ap;
		ap->timestamp = time(NULL);

		msg->proto = TCP; 
		message_insert(aphash, msg);
	}
	return NULL;
}

/* pthread recv netlayer */
static void *__net_netrecv(void *arg)
{
	struct message_t *msg;
	int rcvlen;
	struct ethhdr *hdr;
	char *mac;
	struct ap_t *ap;
	struct ap_hash_t *aphash;

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

		hdr = (struct ethhdr *)&msg->data[0];
		mac = (char *)&hdr->h_source[0];
		aphash = hash_ap(mac);
		if(aphash == NULL) {
			free(msg);
			continue;
		}

		ap = &aphash->ap;
		ap->timestamp = time(NULL);

		msg->proto = ETH;
		message_insert(aphash, msg);
	}
	return NULL;
}

void net_init()
{
	struct sockarr_t *rcvsock = __insert_sockarr(__net_dllrecv);
	dll_init(&argument.nic[0], &rcvsock->sock, NULL, NULL);

	/* create pthread recv msg */
	__create_pthread(net_recv, __head);
	sys_debug("Create pthread recv dllayer msg\n");

	/* create pthread broadcast ac probe packet */
	__create_pthread(__net_dllbrd, NULL);
	sys_debug("Create pthread broadcast dllayer msg\n");
}

