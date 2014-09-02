/*
 * ============================================================================
 *
 *       Filename:  process.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月26日 10时04分23秒
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
#include <unistd.h>
#include <arpa/inet.h>

#include "msg.h"
#include "log.h"
#include "dllayer.h"
#include "netlayer.h"
#include "link.h"
#include "net.h"
#include "arg.h"
#include "thread.h"
#include "process.h"
#include "apstatus.h"

struct sysstat_t sysstat = {
	.isreg = 0,
	.islost = 0,
	.acuuid = {0},
	.lock = PTHREAD_MUTEX_INITIALIZER,
};

static pthread_mutex_t __report_thread_lock = PTHREAD_MUTEX_INITIALIZER;

static void __fill_msg_header(struct msg_head_t *msg, int msgtype)
{
	memcpy(&msg->acuuid[0], 
		&sysstat.acuuid[0], UUID_LEN);
	memcpy(&msg->mac[0], 
		&argument.mac[0], ETH_ALEN);
	msg->msg_type = msgtype; 
}

static void __debug_ipv4(struct sockaddr_in *addr)
{
	char buf[20];
	inet_ntop(AF_INET, &addr->sin_addr, buf, 19);
	sys_debug("addr:%s, port:%d\n", buf, ntohs(addr->sin_port));
}

static void *report_apstatus(void *arg)
{
	int ret;
	struct apstatus_t *ap;

	int bufsize = sizeof(struct msg_ap_status_t) +
		sizeof(struct apstatus_t);
	char *buf = malloc(bufsize);
	if(buf == 0) {
		sys_err("Malloc for report apstatus failed:%s ", 
			strerror(errno));
		exit(-1);
	}

	struct msg_ap_status_t *msg = (void *)buf;
	__fill_msg_header(&msg->header, MSG_AP_STATUS);

	while(1) {
		if(sysstat.islost) goto exit;
		if(!sysstat.isreg) goto wait;

		ap = get_apstatus();
		memcpy(buf + sizeof(struct msg_ap_status_t),
			ap, sizeof(struct apstatus_t));
		ret = tcp_sendpkt(&tcp, buf, bufsize);
		if(ret <= 0) goto exit;
wait:
		sleep(argument.reportitv);
		sys_debug("wait: %d\n", argument.reportitv);
	}
exit:
	free(buf);
	pthread_mutex_unlock(&__report_thread_lock);
	sys_warn("Tcp lost, pthread(report_apstatus) exit\n");
	return NULL;
}

static int __uuid_equ(char *src, char *dest)
{
	return !strncmp(src, dest, UUID_LEN - 1);
}

static void __lost_reconnect()
{
	sys_warn("Try reconnect ac\n");

	int ret;
	pthread_mutex_lock(&sysstat.lock);
	ret = tcp_connect(&tcp);
	if(ret < 0) {
		pthread_mutex_unlock(&sysstat.lock);
		return;
	}
	__insert_sockarr(tcp.sock, __net_netrcv, NULL);
	sysstat.islost = 0;
	sysstat.isreg = 1;
	pthread_mutex_unlock(&sysstat.lock);
}

static void __send_response(struct msg_ac_brd_t *msg)
{
	struct msg_ap_resp_t *data = 
		malloc(sizeof(struct msg_ap_resp_t));
	if(data == NULL) {
		sys_warn("Malloc for response failed:%s\n", 
			strerror(errno));
		return;
	}

	__fill_msg_header(data, MSG_AP_RESP);
	dll_sendpkt(&msg->header.mac[0], 
		(char *)data, sizeof(struct msg_ap_resp_t));
	free(data);
}

static void __response_brd(struct msg_ac_brd_t *msg)
{
	int ret;

	if(sysstat.isreg) {
		if(__uuid_equ(&msg->header.acuuid[0], &sysstat.acuuid[0])) {
			/* broadcast pkt from ac */
			if(sysstat.islost) {
				__lost_reconnect();
				goto report;
			}
		} else {
			/* broadcast pkt from other ac */
			__send_response(msg);
		}
		return;
	} else {
		tcp.addr = msg->ipv4;
		__debug_ipv4(&msg->ipv4);
		ret = tcp_connect(&tcp);
		if(ret < 0) {
			sys_warn("Connect ac failed: %s\n",
				strerror(errno));
			return;
		}

		pthread_mutex_lock(&sysstat.lock);
		__insert_sockarr(tcp.sock, __net_netrcv, NULL);
		sysstat.islost = 0;
		sysstat.isreg = 1;
		pthread_mutex_unlock(&sysstat.lock);

		memcpy(&sysstat.acuuid[0],
			&msg->header.acuuid[0], UUID_LEN);

		struct msg_ap_reg_t *data =
			malloc(sizeof(struct msg_ap_reg_t));
		if(data == NULL) {
			sys_warn("Malloc for reg failed:%s\n", 
				strerror(errno));
			return;
		}
		__fill_msg_header(&data->header, MSG_AP_REG);

		ret = tcp_sendpkt(&tcp, (char *)data, 
			sizeof(struct msg_ap_reg_t));
		free(data);
		if(ret < 0) return;
	}
report:
	if(pthread_mutex_trylock(&__report_thread_lock) == 0)
		__create_pthread(report_apstatus, NULL);
}

static void __exec_cmd(struct msg_ac_cmd_t *cmd)
{
	printf("cmd: %s\n", cmd->cmd);
}

static void __exec_takeover()
{
	return;
}

void msg_proc(struct msg_head_t *msg)
{
	if(!sysstat.isreg) {
		sys_debug("Recive msg: %s\n", msg->acuuid);
		if(msg->msg_type != MSG_AC_BRD) {
			sys_warn("Recive error msg\n");
			return;
		}
	}

	switch(msg->msg_type) {
	case MSG_AC_BRD:
		__response_brd((struct msg_ac_brd_t *)msg);
		break;
	case MSG_AC_CMD:
		__exec_cmd((struct msg_ac_cmd_t *)msg);
		break;
	case MSG_AC_TAKEOVER:
		__exec_takeover();
		break;
	default:
		break;
	}
}

void ac_lost(int lock)
{
	pthread_mutex_lock(&sysstat.lock);
	__delete_sockarr(tcp.sock, lock);
	tcp_close(&tcp);
	sysstat.islost = 1;
	pthread_mutex_unlock(&sysstat.lock);
}
