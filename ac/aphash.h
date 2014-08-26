/*
 * =====================================================================================
 *
 *       Filename:  aphash.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月20日 14时26分57秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __APHASH_H__
#define __APHASH_H__
#include <time.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <linux/if_ether.h>

#include "dllayer.h"
#include "log.h"

#ifndef MAX_AP
#define MAX_AP 		(1ul << 14)
#endif
#define IDLE_AP 	(MAX_AP + 1)

enum {
	ETH,
	TCP,
};

/* ap status */
struct ap_status_t {
};

struct ap_t {
	int 			isreg;
	char 			mac[ETH_ALEN];
	time_t 			timestamp;
	struct ap_status_t 	ap_status;
};

/* message */
struct message_t {
	/* dllayer or tcp */
	int 			proto;
	struct message_t  	*next;
	char 			data[DLL_PKT_DATALEN];
};

/* ap hash table */
struct ap_hash_t {
	unsigned int 		key;
	pthread_mutex_t 	lock; 
	struct ap_t 		ap;
	struct message_t 	*next;
	struct message_t 	**ptail;
	int 			count;
};

extern struct ap_hash_t *aphead;
extern unsigned int conflict_count;
/* all ap in net, not reg counter */
unsigned int ap_innet_cnt;
/* all ap in reg */
unsigned int ap_reg_cnt;

void hash_init();
struct ap_hash_t *hash_ap(char *mac);
void message_insert(struct ap_hash_t *aphash, struct message_t *msg);
struct message_t *message_delete(struct ap_hash_t *aphash);
void *message_travel(void *arg);
#endif /* __APHASH_H__ */
