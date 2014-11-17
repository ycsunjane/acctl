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
#include <sys/ioctl.h>
#include <net/if.h>
#include <assert.h>

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

#define SYSSTAT_LOCK() 	(pthread_mutex_lock(&sysstat.lock))
#define SYSSTAT_UNLOCK() (pthread_mutex_unlock(&sysstat.lock))

struct sysstat_t sysstat = {
	.acuuid = {0},
	.isreg = 0,
	.sock = -1,
	.dmac = {0},
	.lock = PTHREAD_MUTEX_INITIALIZER,
};

static void ac_reconnect()
{
	if(!sysstat.server.sin_addr.s_addr)
		return;

	int ret;
	struct nettcp_t tcp;
	SYSSTAT_LOCK();
	/* other process have reconnect */
	if(sysstat.sock >= 0)
		goto unlock;

	tcp.addr = sysstat.server;
	ret = tcp_connect(&tcp);
	if(ret < 0)
		goto unlock;

	pr_ipv4(&tcp.addr);
	sysstat.sock = ret;
	SYSSTAT_UNLOCK();
	sys_debug("connect success: %d\n", sysstat.sock);
	insert_sockarr(tcp.sock, __net_netrcv, NULL);
	return;

unlock:
	SYSSTAT_UNLOCK();
	return;
}

static void *report_apstatus(void *arg)
{
	int ret;
	struct apstatus_t *ap;

	int bufsize = sizeof(struct msg_ap_status_t) +
		sizeof(struct apstatus_t);
	char *buf = calloc(1, bufsize);
	if(buf == 0) {
		sys_err("Malloc for report apstatus failed:%s ", 
			strerror(errno));
		exit(-1);
	}

	int proto;
	/* tcp do not use chap to protect */
	fill_msg_header((void *)buf, MSG_AP_STATUS, NULL, 0);

	/* ap will first connect remote ac, but if 
	 * find local ac, ap will connect to local ac */
	while(1) {
		proto = (sysstat.sock >= 0) ? MSG_PROTO_TCP : MSG_PROTO_ETH;
		if(proto == MSG_PROTO_ETH) {
			ac_reconnect();
			goto wait;
		}

		ap = get_apstatus();
		memcpy(buf + sizeof(struct msg_ap_status_t),
			ap, sizeof(struct apstatus_t));

		ret = net_send(proto, sysstat.sock, 
			sysstat.dmac, buf, bufsize);
		if(ret <= 0 && proto == MSG_PROTO_TCP) {
			ac_lost();
			ac_reconnect();
		}
wait:
		sys_debug("report ap status (next %d seconds later)\n", 
			argument.reportitv);
		sleep(argument.reportitv);
	}
	return NULL;
}

static int __uuid_equ(char *src, char *dest)
{
	return !strncmp(src, dest, UUID_LEN - 1);
}

/* XXX: only use in !reg stat,
 * there is impossible have 10 ac in local */
#define LOCAL_AC_MAX 	(10)
struct mac_random_map_t {
	uint32_t random;
	char mac[ETH_ALEN];
};
static struct mac_random_map_t random_map[LOCAL_AC_MAX] = {{0}};

static int offset = 0;
static uint32_t new_random(char *mac)
{
	uint32_t random;
	random_map[offset].random = chap_get_random();
	memcpy(random_map[offset].mac, mac, ETH_ALEN);
	random = random_map[offset].random;
	offset = (offset + 1) % LOCAL_AC_MAX;
	return random;
}

static uint32_t get_random(char *mac)
{
	int i, j;
	/* offset should be check first */
	for(i = offset - 1, j = 0; j < LOCAL_AC_MAX; i++, j++) {
		if(!memcmp(mac, random_map[i % LOCAL_AC_MAX].mac, ETH_ALEN))
			return random_map[i % LOCAL_AC_MAX].random;
	}
	return 0;
}

static void _proc_brd(struct msg_ac_brd_t *msg, int len, int proto)
{
	/* send current ipv4 */
	struct msg_ap_reg_t *resp = 
		malloc(sizeof(struct msg_ap_reg_t));
	if(resp == NULL) {
		sys_warn("Malloc for response failed:%s\n", 
			strerror(errno));
		return;
	}

	/* generate random1 */
	fill_msg_header((void *)resp, MSG_AP_REG, 
		msg->header.acuuid, new_random(msg->header.mac));
	resp->ipv4 = argument.addr;

	/* calculate chap: md5sum1 = packet + random0 + password */
	chap_fill_msg_md5((void *)resp, sizeof(*resp), msg->header.random);
	net_send(proto, -1, &msg->header.mac[0], 
		(void *)resp, sizeof(struct msg_ap_reg_t));
	free(resp);
}

static void 
_proc_brd_isreg(struct msg_ac_brd_t *msg, int len, int proto)
{
	if(!__uuid_equ(&msg->header.acuuid[0], &sysstat.acuuid[0])) {
		if(__uuid_equ(&msg->takeover[0], &sysstat.acuuid[0])) {
			if(sysstat.sock >= 0) {
				close(sysstat.sock);
				sysstat.sock = -1;
			}
			_proc_brd(msg, len, proto);
		} else {
			/* tell the broadcast ac, ap have reg in other ac */
			struct msg_ap_resp_t *resp = 
				malloc(sizeof(struct msg_ap_resp_t));
			if(resp == NULL) {
				sys_warn("Malloc for response failed:%s\n", 
					strerror(errno));
				return;
			}
			fill_msg_header(resp, MSG_AP_RESP, 
				msg->header.acuuid, 
				new_random(msg->header.mac));

			/* calculate chap */
			chap_fill_msg_md5(resp, sizeof(*resp), 
				msg->header.random);
			net_send(proto, -1, &msg->header.mac[0], 
				(void *)resp, sizeof(struct msg_ap_resp_t));
			free(resp);
		}
	}
}

/*
 * reponse_brd recv broadcast msg from ac and update sysstat
 * */
static void proc_brd(struct msg_ac_brd_t *msg, int len, int proto)
{
	assert(proto == MSG_PROTO_ETH);

	sys_debug("receive ac broadcast packet\n");
	if(len < sizeof(*msg)) {
		sys_err("receive error msg ac broadcast packet\n");
		return;
	}

	if(sysstat.isreg)
		_proc_brd_isreg(msg, len, proto);
	else
		_proc_brd(msg, len, proto);
}

static int addr_equ(struct sockaddr_in *addr)
{
	if(addr->sin_addr.s_addr == argument.addr.sin_addr.s_addr)
		return 1;
	return 0;
}

static int setaddr(struct sockaddr *addr)
{
	struct ifreq req;
	strncpy(req.ifr_name, argument.nic, IFNAMSIZ);
	req.ifr_addr = *addr;
	req.ifr_addr.sa_family = AF_INET;

	sys_debug("Set client nic ip: %s, addr: %s\n", req.ifr_name, 
		inet_ntoa(((struct sockaddr_in *)&req.ifr_addr)->sin_addr));

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) {
		sys_err("Create socket failed: %s(%d)\n",
			strerror(errno), errno);
		return 0;
	}

	int ret;
	ret = ioctl(sockfd, SIOCSIFADDR, &req);
	if(ret < 0) {
		sys_err("Set ap addr failed: %s(%d)\n",
			strerror(errno), errno);
		close(sockfd);
		return 0;
	}
	close(sockfd);
	return 1;
}

static void 
proc_reg_resp(struct msg_ac_reg_resp_t *msg, int len, int proto)
{
	sys_debug("Receive ac reg response packet\n");
	if(len < sizeof(*msg)) {
		sys_err("Receive error msg ac reg response packet\n");
		return;
	}

	/* md5sum3 = packet + random1 + password */
	if(chap_msg_cmp_md5((void *)msg, sizeof(*msg), 
			get_random(msg->header.mac))) {
		sys_err("ac reg response packet chap error\n");
		return;
	}

	strncpy(sysstat.acuuid, msg->header.acuuid, UUID_LEN);
	memcpy(sysstat.dmac, msg->header.mac, ETH_ALEN);
	if(msg->acaddr.sin_addr.s_addr)
		sysstat.server = msg->acaddr;
	if(msg->apaddr.sin_addr.s_addr) 
		setaddr((void *)&msg->apaddr);

	pr_ipv4(&msg->acaddr);

	/* if have connect to remote ac, close it. 
	 * and connect to local ac */
	if(sysstat.sock >= 0)
		close(sysstat.sock);

	ac_reconnect();
	sysstat.isreg = 1;
}

static void __exec_cmd(struct msg_ac_cmd_t *cmd)
{
	sys_debug("receive ac command packet\n");
	printf("cmd: %s\n", cmd->cmd);
}

static int is_mine(struct msg_head_t *msg)
{
	return 1;
}

void msg_proc(struct msg_head_t *msg, int len, int proto)
{
	if(!is_mine(msg))
		return;

	switch(msg->msg_type) {
	case MSG_AC_BRD:
		proc_brd((void *)msg, len, proto);
		break;
	case MSG_AC_REG_RESP:
		proc_reg_resp((void *)msg, len, proto);
		break;
	case MSG_AC_CMD:
		__exec_cmd((struct msg_ac_cmd_t *)msg);
		break;
	default:
		break;
	}
}

void ac_lost()
{
	sys_debug("ac lost\n");
	delete_sockarr(sysstat.sock);
	sysstat.sock = -1;
}

void init_report()
{
	/* init remote ac address */
	sysstat.server = argument.acaddr;
	create_pthread(report_apstatus, NULL);
}
