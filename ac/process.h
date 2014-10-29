/*
 * =====================================================================================
 *
 *       Filename:  process.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月25日 18时28分04秒
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
#include <netlayer.h>
struct ac_t {
	uint32_t random;
	char 	acuuid[UUID_LEN];
};

extern struct ac_t ac;

void ac_init();
void ap_lost(int sock);
void msg_proc(struct ap_hash_t *aphash, 
	struct msg_head_t *msg, int len, int proto);
#endif /* __PROCESS_H__ */
