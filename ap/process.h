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
#include <stdint.h>
#include <pthread.h>
#include "msg.h"
struct sysstat_t {
	/* only used by local ac */
	char acuuid[UUID_LEN];
	char dmac[ETH_ALEN];
	int isreg;

	/* used by local ac and remote ac */
	int sock;
	struct sockaddr_in server;
	pthread_mutex_t lock;
};

void ac_lost();
void init_report();
extern struct sysstat_t sysstat;
void msg_proc(struct msg_head_t *msg, int len, int proto);
#endif /* __PROCESS_H__ */
