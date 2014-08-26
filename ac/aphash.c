/*
 * ============================================================================
 *
 *       Filename:  aphash.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月20日 17时09分49秒
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
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "aphash.h"
#include "thread.h"
#include "arg.h"
#include "msg.h"
#include "process.h"

struct ap_hash_t *aphead = NULL;
unsigned int conflict_count = 0;
/* all ap in net, not reg counter */
unsigned int ap_innet_cnt = 0;
/* all ap in reg */
unsigned int ap_reg_cnt = 0;

static unsigned int 
__elfhash(char* str, unsigned int len)  
{  
	unsigned int hash = 0;  
	unsigned int x    = 0;  
	unsigned int i    = 0;  
	for(i = 0; i < len; str++, i++)  
	{  
		hash = (hash << 4) + (*str);  
		if((x = hash & 0xF0000000L) != 0)  
		{  
			hash ^= (x >> 24);  
		}  
		hash &= ~x;  
	}  
	return hash;  
}

static unsigned int 
__hash_key(char *data, unsigned int len)
{
	assert(data != NULL && len > 0);
	return __elfhash(data, len) % MAX_AP;
}

static unsigned int
__hash_isconflict(char *src, char *dest, int len)
{
	while(len-- > 0) {
		if(*src++ != *dest++)
			return 1;
	}
	return 0;
}

struct ap_hash_t *hash_ap(char *mac)
{
	assert(mac != NULL);

	char *dest;
	int total = 0, key, len = 6;
	struct ap_hash_t *aphash = NULL;

	key = __hash_key(mac, len);

	do {
		if(aphash)
			pthread_mutex_unlock(&aphash->lock);

		aphash = aphead + key;

		pthread_mutex_lock(&aphash->lock);
		dest = aphash->ap.mac; 

		/* new ap */
		if(aphash->key == IDLE_AP) {
			aphash->key = key;
			memcpy(dest, mac, len);
			ap_innet_cnt++;
			pthread_mutex_unlock(&aphash->lock);
			return aphash;
		}

		conflict_count++;

		total++;
		key = (key + 1) % MAX_AP;
	} while(total <= MAX_AP && 
		__hash_isconflict(mac, dest, len));
	key = (key + MAX_AP - 1) % MAX_AP;
	assert(key >= 0 && key < MAX_AP);

	if(total > MAX_AP) {
		pthread_mutex_unlock(&aphash->lock);
		sys_warn("Hash bucket have full\n");
		return NULL;
	} else {
		/* old ap */
		assert(aphash->key == key);
		pthread_mutex_unlock(&aphash->lock);
		return aphash;
	}
}

void message_insert(struct ap_hash_t *aphash, struct message_t *msg)
{
	assert(aphash != NULL);

	msg->next = NULL;

	pthread_mutex_lock(&aphash->lock);
	*aphash->ptail = msg;
	aphash->ptail = &msg->next; 
	aphash->count++;
	pthread_mutex_unlock(&aphash->lock);
}

struct message_t *message_delete(struct ap_hash_t *aphash)
{
	if(aphash->next == NULL)
		return NULL;

	struct message_t *msg;

	pthread_mutex_lock(&aphash->lock);
	msg = aphash->next;
	aphash->next  = msg->next;
	aphash->count--;

	if(&msg->next == aphash->ptail)
		aphash->ptail = &aphash->next;
	pthread_mutex_unlock(&aphash->lock);

	return msg;
}

void message_free(struct message_t *msg)
{
	free(msg);
}

void *message_travel(void *arg)
{
	assert(aphead != NULL);

	struct ap_hash_t *aphash;
	struct message_t *msg;
	int i;
	while(1) {
		sleep(argument.msgitv);

		for(i = 0; i < MAX_AP; i++) {
			aphash = aphead + i;
			while((msg = message_delete(aphash))) {
				msg_proc(aphash, 
					(struct msg_head_t *)&msg->data[0]);
				message_free(msg);
			}
		}
	}
}

void hash_init()
{
	assert(aphead == NULL);
	_Static_assert(MAX_AP < (1ull << 32), "ap num too large\n");

	aphead = calloc(1, sizeof(struct ap_hash_t) * MAX_AP);
	if(aphead == NULL) {
		sys_err("Malloc hash bucket failed: %s\n", 
			strerror(errno));
		exit(-1);
	}
	
	int i;
	for(i = 0; i < MAX_AP; i++) {
		aphead[i].key = IDLE_AP;
		aphead[i].ptail = &aphead[i].next;
		pthread_mutex_init(&aphead[i].lock, 0);
	}

	/* create thread process all message */
	__create_pthread(message_travel, NULL);
	return;
}

