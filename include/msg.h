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
#include <netinet/in.h>
#include <linux/if_ether.h>

#define UUID_LEN 	(50)

#define MSG_TYPE_START (1000)
enum {
	MSG_AC_BRD  = MSG_TYPE_START,
	MSG_AC_CMD,

	MSG_AP_REG,
	MSG_AP_RESP,
	MSG_AP_STATUS,
};

struct msg_head_t {
	char 	acuuid[UUID_LEN];
	char 	mac[ETH_ALEN];
	int  	msg_type;
};

/* every ap will response ac broadcast */
#define msg_ap_resp_t 	msg_head_t

struct msg_ac_brd_t {
	struct msg_head_t header;
	struct sockaddr_in ipv4;
};

struct msg_ap_reg_t {
	struct msg_head_t header;
};

struct msg_ac_cmd_t {
	struct msg_head_t header;
	char 	cmd[0];
};

struct msg_ap_status_t {
	struct msg_head_t header;
	char 	status[0];
};
#endif /* __MSG_H__ */
