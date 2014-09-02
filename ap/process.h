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
	int isreg;
	int islost;
	char acuuid[UUID_LEN];
	pthread_mutex_t lock;
};

void ac_lost(int lock);
extern struct sysstat_t sysstat;
void msg_proc(struct msg_head_t *msg);
#endif /* __PROCESS_H__ */
