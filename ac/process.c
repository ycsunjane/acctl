/*
 * ============================================================================
 *
 *       Filename:  process.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月25日 14时31分55秒
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
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>

#include "net.h"
#include "log.h"
#include "msg.h"
#include "aphash.h"
#include "arg.h"
#include "netlayer.h"
#include "apstatus.h"
#include "link.h"
#include "process.h"
#include "resource.h"

static void __get_cmd_stdout(char *cmd, char *buf, int len)
{
	FILE *fp;
	int size;
	fp = popen(cmd, "r"); 
	if(fp != NULL) {
		size = fread(buf, 1, len, fp);
		if(size <= 0)
			goto err;
		/* skip \n */
		buf[size - 1] = 0;
		pclose(fp);
		return;
	}
err:
	buf[0] = 0;
	sys_err("Exec %s failed: %s\n", cmd, strerror(errno));
	exit(0);
}

static int __uuid_equ(char *src, char *dest)
{
	return !strncmp(src, dest, UUID_LEN - 1);
}

static char *__buildcmd()
{
	char *cmd;
	cmd = malloc(100);
	strcpy(cmd, "cmdtest;");
	return cmd;
}

static void __ap_status(struct ap_t *ap, struct msg_ap_reg_t *msg)
{
	sys_debug("recive ap report status packet\n");

	int ret;
	struct apstatus_t *status = (void *)((char *)msg + 
		sizeof(struct msg_ap_status_t));
	printf("ssidnum:%d, ssid:%s \n", status->ssidnum, status->ssid0.ssid);

	char *cmd = __buildcmd();
	if(cmd == NULL) return;
	int cmdlen = strlen(cmd);
	int totallen = sizeof(struct msg_ac_cmd_t) + cmdlen;
	assert(totallen <= NET_PKT_DATALEN);

	struct msg_ac_cmd_t *data = malloc(totallen);
	if(data == NULL) {
		sys_warn("malloc msg for cmd failed: %s\n",
			strerror(errno));
		return;
	}
	fill_msg_header(&data->header, MSG_AC_CMD, 
		&ac.acuuid[0], msg->header.random);
	strncpy((char *)data + sizeof(struct msg_ac_cmd_t), cmd, cmdlen);

	struct nettcp_t tcp;
	tcp.sock = ap->sock;
	ret = tcp_sendpkt(&tcp, (char *)data, totallen);
	if(ret <= 0)
		ap_lost(ap->sock);
	free(data);
	free(cmd);
}


static void __ap_reg(struct ap_t *ap, 
	struct msg_ap_reg_t *msg, int len, int proto)
{
	sys_debug("recive ap reg packet\n");
	if(len < sizeof(*msg)) {
		sys_err("receive wrong ap reg packet\n");
		return;
	}

	/* XXX:first random is broadcast random, 
	 * ac broadcast random will change every argument.brditv
	 * so ap reg packet must be recive in argument.brditv */
	/* calculate: md5sum2 = packet + random0 + password
	 * compare md5sum1 with md5sum2*/
	if(chap_msg_cmp_md5((void *)msg, sizeof(*msg), ac.random)) {
		sys_err("receive packet chap check failed\n");
		return;
	}

	struct _ip_t *ip;
	struct sockaddr_in *addr;
	if(res_ip_conflict(&(msg->ipv4), msg->header.mac))
		addr = NULL;
	else
		addr = &(msg->ipv4);

	pr_ipv4(addr);
	ip = res_ip_alloc(addr, msg->header.mac);
	pr_ipv4(&ip->ipv4);

	struct msg_ac_reg_resp_t *resp = 
		calloc(1, sizeof(struct msg_ac_reg_resp_t));
	if(resp == NULL) {
		sys_err("Calloc memory failed: %s(%d)\n", 
			strerror(errno), errno);
		return;
	}

	/* generate random2 */
	ap->random = chap_get_random();
	fill_msg_header((void *)resp, MSG_AC_REG_RESP, 
		&ac.acuuid[0], ap->random);

	resp->acaddr = argument.addr;
	if(ip != NULL)
		resp->apaddr = ip->ipv4;

	/* calculate chap: md5sum3 = packet + random1 + password */
	chap_fill_msg_md5((void *)resp, sizeof(*resp), msg->header.random);
	net_send(proto, ap->sock, msg->header.mac, 
		(void *)resp, sizeof(struct msg_ac_reg_resp_t));
	free(resp);
	ap_reg_cnt++;
}

struct ac_t ac;
#define X86_UUID 	"cat /sys/class/dmi/id/product_uuid"
void ac_init()
{
	memset(ac.acuuid, 0, UUID_LEN);
	__get_cmd_stdout(X86_UUID, ac.acuuid, UUID_LEN-1);
}

void ap_lost(int sock)
{
	sys_debug("ap lost sock: %d\n", sock);
	delete_sockarr(sock);
}

int is_mine(struct msg_head_t *msg, int len)
{
	/* check packet len */
	if(len < sizeof(*msg)) {
		sys_err("receive ultrashort packet\n");
		return 0;
	}

	/* ap have reg in other ac */
	char *ap = msg->mac;
	if(!__uuid_equ(msg->acuuid, ac.acuuid) 
		&& (msg->msg_type == MSG_AP_RESP)) {
		pr_ap(ap, msg->acuuid);
		return 0;
	}

	return 1;
}

void msg_proc(struct ap_hash_t *aphash, 
	struct msg_head_t *msg, int len, int proto)
{
	if(!is_mine(msg, len)) return;

	switch(msg->msg_type) {
	case MSG_AP_REG:
		__ap_reg(&aphash->ap, (void *)msg, len, proto);
		break;
	case MSG_AP_STATUS:
		/* only received by tcp */
		__ap_status(&aphash->ap, (void *)msg);
		break;
	default:
		sys_err("Invaild msg type\n");
		break;
	}
}
