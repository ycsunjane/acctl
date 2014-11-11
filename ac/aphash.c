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

struct ap_hash_head_t *aphead = NULL;
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
	return __elfhash(data, len) % MAX_BUCKET;
}

struct ap_hash_t *hash_ap(char *mac)
{
	assert(mac != NULL);

	char *dest;
	int key, len = 6;

	struct ap_hash_head_t *head = NULL;
	struct ap_hash_t **pprev, *aphash = NULL;
	pprev = &aphash;

	key = __hash_key(mac, len);

	head = aphead + key;
	pthread_mutex_lock(&head->lock);
	aphash = &head->aphash;
	dest = aphash->ap.mac; 

	/* aphash head is null */
	if(aphash->key == IDLE_AP)
		goto new;

	/* travel list */
	while(aphash) {
		pprev = &aphash;
		if(!strncmp(aphash->ap.mac, mac, ETH_ALEN))
			goto ret;
		aphash = aphash->apnext;
		conflict_count++;
	}

	/* calloc new */
	aphash = calloc(1, sizeof(struct ap_hash_t));
	if(aphash == NULL) {
		sys_err("Calloc failed: %s\n", 
			strerror(errno));
		return NULL;
	}
	(*pprev)->apnext = aphash;
	goto new;

new:
	pthread_mutex_init(&aphash->lock, 0);
	aphash->key = key;
	aphash->ptail = &aphash->next;
	memcpy(dest, mac, len);
	ap_innet_cnt++;
ret:
	pthread_mutex_unlock(&head->lock);
	pr_hash(key, aphash, mac);
	return aphash;
}

void hash_init()
{
	assert(aphead == NULL);
	_Static_assert(MAX_BUCKET < (1ull << 32), "ap num too large\n");

	aphead = calloc(1, sizeof(struct ap_hash_head_t) * MAX_BUCKET);
	if(aphead == NULL) {
		sys_err("Malloc hash bucket failed: %s\n", 
			strerror(errno));
		exit(-1);
	}
	sys_debug("Max support ap: %lu\n", MAX_BUCKET);

	int i;
	for(i = 0; i < MAX_BUCKET; i++) {
		aphead[i].aphash.key = IDLE_AP;
		aphead[i].aphash.ptail = &aphead[i].aphash.next;
		pthread_mutex_init(&aphead[i].lock, 0);
	}

	return;
}

void message_insert(struct ap_hash_t *aphash, struct message_t *msg)
{
	assert(aphash != NULL);

	msg->next = NULL;

	sys_debug("message insert aphash: %p, msg: %p\n", aphash, msg);
	pthread_mutex_lock(&aphash->lock);
	*(aphash->ptail) = msg;
	aphash->ptail = &msg->next; 
	aphash->count++;
	pthread_mutex_unlock(&aphash->lock);
}

static struct message_t *message_delete(struct ap_hash_t *aphash)
{
	if(aphash->next == NULL)
		return NULL;

	sys_debug("message delete aphash: %p\n", aphash);
	struct message_t *msg;

	pthread_mutex_lock(&aphash->lock);
	msg = aphash->next;
	aphash->next = msg->next;
	aphash->count--;

	if(&msg->next == aphash->ptail)
		aphash->ptail = &aphash->next;
	pthread_mutex_unlock(&aphash->lock);

	return msg;
}

static void message_free(struct message_t *msg)
{
	free(msg);
}

static void *message_travel(void *arg)
{
	assert(aphead != NULL);

	struct ap_hash_t *aphash;
	struct message_t *msg;
	int i;
	while(1) {
		sleep(argument.msgitv);

		/* travel hash bucket */
		for(i = 0; i < MAX_BUCKET; i++) {
			aphash = &(aphead[i].aphash);
			/* travel hash list */
			while(aphash) {
				/* travel message */
				while((msg = message_delete(aphash))) {
					msg_proc(aphash, 
						(void *)&msg->data[0], 
						msg->len,
						msg->proto);
					message_free(msg);
				}
				aphash = aphash->apnext;
			}
		}
	}
}

void message_travel_init()
{
	/* create thread process all message */
	create_pthread(message_travel, NULL);
}

