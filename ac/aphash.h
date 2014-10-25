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
#include "msg.h"

/* 
 * statistical information
 */
/* all ap in net, not reg counter */
extern unsigned int ap_innet_cnt;
/* all ap in reg */
extern unsigned int ap_reg_cnt;


extern struct ap_hash_head_t *aphead;
extern unsigned int conflict_count;
#ifndef MAX_BUCKET
#define MAX_BUCKET 		(1ul << 14)
#endif
#define IDLE_AP 	(MAX_BUCKET + 1)

/* message */
struct message_t {
	/* dllayer or tcp */
	int 			proto;
	struct message_t  	*next;
	char 			data[0];
};

struct ap_status_t {
	int 			isreg;
	/* set when ap reg in other ac */
	char 			reguuid[UUID_LEN];

	int 			level;
};

struct ap_t {
	/* key and dllayer mac */
	char 			mac[ETH_ALEN];

	/* tcp sock */
	int 			sock;
	struct ap_status_t 	ap_status;
};

struct ap_hash_t {
	int 			key;
	struct ap_t 		ap;

	pthread_mutex_t 	lock; 

	struct message_t 	*next;
	struct message_t 	**ptail;
	time_t 			timestamp;
	int 			count;

	struct ap_hash_t 	*apnext;
};

/* ap hash table */
struct ap_hash_head_t {
	pthread_mutex_t 	lock; 
	struct ap_hash_t 	aphash;
};

void hash_init();
struct ap_hash_t *hash_ap(char *mac);

void message_travel_init();
void message_insert(struct ap_hash_t *aphash, struct message_t *msg);
#endif /* __APHASH_H__ */
