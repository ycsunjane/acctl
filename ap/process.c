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

#include "msg.h"
#include "log.h"
#include "dllayer.h"
#include "netlayer.h"
#include "link.h"
#include "net.h"
#include "arg.h"

struct sysstat_t {
	int isreg;
	char acuuid[UUID_LEN];
};

struct sysstat_t sysstat = {0};

static void __response_brd(struct msg_ac_brd_t *msg)
{
	int ret;

	if(sysstat.isreg) {
		struct msg_ap_resp_t *data = 
			malloc(sizeof(struct msg_ap_resp_t));
		if(data == NULL) {
			sys_warn("Malloc for response failed:%s\n", 
				strerror(errno));
			return;
		}
		dll_sendpkt(&msg->header.mac[0], 
			(char *)data, sizeof(struct msg_ap_resp_t));
		free(data);
	} else {
		ret = tcp_connect(msg->ipv4, 0);
		if(ret < 0) {
			sys_warn("Connect ac failed: %s\n",
				strerror(errno));
			return;
		}
		struct sockarr_t *tcpsock =
			__insert_sockarr(__net_netrcv);
		tcpsock->sock = ret;

		struct msg_ap_reg_t *data =
			malloc(sizeof(struct msg_ap_reg_t));
		if(data == NULL) {
			sys_warn("Malloc for reg failed:%s\n", 
				strerror(errno));
			return;
		}
		memcpy(&data->header.acuuid[0], 
			&msg->header.acuuid[0], UUID_LEN);
		memcpy(&data->header.mac[0], 
			&argument.mac[0], ETH_ALEN);
		data->header.msg_type = MSG_AP_REG; 
		ret = tcp_sendpkt((char *)data, 
			sizeof(struct msg_ap_reg_t));
		if(ret < 0) return;
		sysstat.isreg = 1;
	}
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
		break;
	default:
		break;
	}
}
