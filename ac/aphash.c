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

#include "aphash.h"

struct ap_hash_t *aphead = NULL;
unsigned int conflict_count = 0;

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

struct ap_hash_t *hash_ap(char *data, int type)
{
	assert(data != NULL && (type == ETH || type = TCP));

	char *dest;
	int total = 0, key, len;
	struct ap_hash_t *aphash = NULL;

	key = __hash_key(data, len);

	do {
		if(aphash)
			pthread_mutex_unlock(&aphash->lock);

		aphash = aphead + key;

		pthread_mutex_lock(&aphash->lock);
		dest = (type == ETH) ? aphash->ap.mac : 
			aphash->ap.ip;
		len = (type == ETH) ? 6 : 4;

		if(aphash->key == IDLE_AP) {
			aphash->key = key;
			memcpy(dest, data, len);
			pthread_mutex_unlock(&aphash->lock);
			return aphash;
		}

		conflict_count++;

		total++;
		key = (key + 1) % MAX_AP;
	} while(__hash_isconflict(data, dest, len)
		&& total < MAX_AP);
	key--;

	if(total == MAX_AP) {
		pthread_mutex_unlock(&aphash->lock);
		sys_warn("Hash bucket have full\n");
		return NULL;
	} else {
		aphash->key = key;
		memcpy(dest, data, len);
		pthread_mutex_unlock(&aphash->lock);
		return aphash;
	}
}

void hash_init()
{
	assert(aphead == NULL);
	_Static_assert(MAX_AP < (1ul << 32), "ap num too large\n");

	aphead = malloc(sizeof(struct ap_hash_t) * HASH_BUCKET_LEN);
	if(aphead == NULL) {
		sys_err("Malloc hash bucket failed: %s\n", 
			strerror(errno));
		exit(-1);
	}
	
	int i;
	for(i = 0; i < MAX_AP; i++) {
		aphead[i].key = IDLE_AP;
		pthread_mutex_init(&aphead[i].lock, 0);
	}
	return;
}

