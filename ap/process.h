/*
 * =====================================================================================
 *
 *       Filename:  process.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月26日 10时18分54秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __PROCESS_H__
#define __PROCESS_H__
#include <pthread.h>
#include "msg.h"
struct sysstat_t {
	char acuuid[UUID_LEN];
	int isreg;

	int sock;
	char dmac[ETH_ALEN];
	struct sockaddr_in server;
};

void ac_lost();
void init_report();
extern struct sysstat_t sysstat;
void msg_proc(struct msg_head_t *msg, int proto);
#endif /* __PROCESS_H__ */
