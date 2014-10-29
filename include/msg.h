/*
 * =====================================================================================
 *
 *       Filename:  msg.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月21日 11时55分55秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __MSG_H__
#define __MSG_H__
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <linux/if_ether.h>

#include "chap.h"
#include "arg.h"

#define UUID_LEN 	(50)

#define MSG_TYPE_START (1000)
/* msg type */
enum {
	MSG_AC_BRD  = MSG_TYPE_START,
	MSG_AC_CMD,
	MSG_AC_REG_RESP,

	MSG_AP_REG,
	MAS_AP_ERR,
	MSG_AP_RESP,
	MSG_AP_STATUS,
};

struct msg_head_t {
	uint32_t random;
	uint8_t chap[CHAP_LEN];
	char 	acuuid[UUID_LEN];
	char 	mac[ETH_ALEN];
	int  	msg_type;
}__attribute__((packed));

/* every ap will response ac broadcast */
#define msg_ap_resp_t 	msg_head_t

struct msg_ac_brd_t {
	struct msg_head_t header;
	char  takeover[UUID_LEN];
}__attribute__((packed));

struct msg_ac_cmd_t {
	struct msg_head_t header;
	char 	cmd[0];
}__attribute__((packed));

struct msg_ac_reg_resp_t {
	struct msg_head_t header;
	struct sockaddr_in acaddr;
	struct sockaddr_in apaddr;
}__attribute__((packed));

struct msg_ap_reg_t {
	struct msg_head_t header;
	struct sockaddr_in ipv4;
}__attribute__((packed));

struct msg_ap_status_t {
	struct msg_head_t header;
	char 	status[0];
}__attribute__((packed));


static void 
fill_msg_header(struct msg_head_t *msg, int msgtype, char *uuid, uint32_t random)
{
	msg->random = random;
	if(uuid)
		memcpy(&msg->acuuid[0], 
			uuid, UUID_LEN);
	memcpy(&msg->mac[0], 
		&argument.mac[0], ETH_ALEN);
	msg->msg_type = msgtype; 
}

#endif /* __MSG_H__ */
