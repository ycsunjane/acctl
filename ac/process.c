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
#include <linux/if_ether.h>

#include "log.h"
#include "msg.h"
#include "aphash.h"

char 	acuuid[UUID_LEN];

#define X86_UUID 	"cat /sys/class/dmi/id/product_uuid"

#define pr_ap(mac, uuid) 					\
do { 								\
	printf("%02x%02x%02x%02x%02x%02x reg in: %s\n", 	\
		(unsigned char) mac[0], 			\
		(unsigned char) mac[1], 			\
		(unsigned char) mac[2], 			\
		(unsigned char) mac[3], 			\
		(unsigned char) mac[4], 			\
		(unsigned char) mac[5], 			\
		uuid); 						\
} while(0)

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
	return;
}

void msg_init()
{
	__get_cmd_stdout(X86_UUID, acuuid, UUID_LEN-1);
	acuuid[UUID_LEN - 1] = 0;
}

static int __uuid_equ(char *src, char *dest)
{
	return !strncmp(src, dest, UUID_LEN - 1);
}

static void __ap_reg(struct ap_t *ap, struct msg_ap_reg_t *msg)
{
	if(ap->isreg) {
		sys_warn("ap repeat register\n");
		return;
	}

	ap->isreg = 1;
	memcpy(&ap->mac[0], &msg->header.mac[0], ETH_ALEN);
	ap_reg_cnt++;
}

void msg_proc(struct ap_hash_t *aphash, struct msg_head_t *msg)
{
	char *ap = msg->mac;

	if(!__uuid_equ(msg->acuuid, acuuid) 
		&& (msg->msg_type == MSG_AP_RESP)) {
			pr_ap(ap, msg->acuuid);
			return;
	}

	switch(msg->msg_type) {
	case MSG_AP_REG:
		__ap_reg(&aphash->ap, (struct msg_ap_reg_t *)msg);
		break;
	case MSG_AP_STATUS:
		break;
	default:
		sys_err("Invaild msg type\n");
		break;
	}
}
