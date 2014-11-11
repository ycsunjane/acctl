/*
 * ============================================================================
 *
 *       Filename:  resource.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年10月24日 15时10分07秒
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
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <linux/if_ether.h>

#include "log.h"
#include "resource.h"

struct _ippool_t *ippool = NULL;

struct _ip_t *res_ip_alloc(struct sockaddr_in *addr, char *mac)
{
	assert(ippool != NULL);

	if(!ippool->left && !list_empty(&ippool->pool)) {
		sys_warn("No ip in pool\n");
		return NULL;
	}

	struct _ip_t *new;
alloc_new:
	if(addr == NULL || addr->sin_addr.s_addr == 0) {
		LOCK(&ippool->lock);
		new = list_first_entry(&ippool->pool, 
			struct _ip_t, list);
		memcpy(new->apmac, mac, ETH_ALEN);
		list_move(&new->list, &ippool->alloc);
		ippool->left--;
		UNLOCK(&ippool->lock);
		return new;
	} else {
		/* if specify addr will alloc addr first */
		LOCK(&ippool->lock);
		list_for_each_entry(new, &ippool->pool, list) {
			if((addr->sin_addr.s_addr ==
					new->ipv4.sin_addr.s_addr)) {
				list_move(&new->list, &ippool->alloc);
				memcpy(new->apmac, mac, ETH_ALEN);
				ippool->left--;
				UNLOCK(&ippool->lock);
				return new;
			}
		}
		UNLOCK(&ippool->lock);

		/* have no addr */
		addr = NULL;
		goto alloc_new;
	}

	return NULL;
}

static int 
addrequ(struct sockaddr_in *src, struct sockaddr_in *dest)
{
	if(src->sin_addr.s_addr == dest->sin_addr.s_addr)
		return 1;
	return 0;
}

int res_ip_conflict(struct sockaddr_in *addr, char *mac)
{
	assert(ippool != NULL);

	if(addr->sin_addr.s_addr == 0)
		return 0;

	int ret = 0;
	struct _ip_t *ip;
	LOCK(&ippool->lock);
	list_for_each_entry(ip, &ippool->alloc, list) {
		if(addrequ(&ip->ipv4, addr)) { 
			if(memcmp(mac, ip->apmac, ETH_ALEN))
				ret = 1;
			else
				ret = 0;
			goto end;
		}
	}
end:
	UNLOCK(&ippool->lock);
	return ret;
}

static int res_ip_repeat(struct sockaddr_in *addr)
{
	assert(ippool != NULL);

	struct _ip_t *pos;

	list_for_each_entry(pos, &ippool->pool, list) {
		if(addrequ(&pos->ipv4, addr))
			return 1;
	}

	list_for_each_entry(pos, &ippool->alloc, list) {
		if(addrequ(&pos->ipv4, addr))
			return 1;
	}
	return 0;
}

int res_ip_add(struct sockaddr_in *addr)
{
	assert(ippool != NULL);

	LOCK(&ippool->lock);
	if(res_ip_repeat(addr)) 
		goto err;

	struct _ip_t *ip = 
		calloc(1, sizeof(struct _ip_t));
	if(!ip) {
		sys_err("Calloc ip_t failed: %s(%d)\n", 
			strerror(errno), errno);
		goto err;
	}

	ip->ipv4 = *addr;
	list_add_tail(&ip->list, &ippool->pool);
	ippool->total++;
	ippool->left++;
	UNLOCK(&ippool->lock);
	return 0;
err:
	UNLOCK(&ippool->lock);
	return -1;
}

void res_ip_clear()
{
	assert(ippool == NULL);

	struct _ip_t *pos, *tmp;

	LOCK(&ippool->lock);

	list_for_each_entry_safe(pos, tmp, &ippool->pool, list)
		free(pos);

	list_for_each_entry_safe(pos, tmp, &ippool->alloc, list)
		free(pos);

	ippool->total = 0;
	ippool->left = 0;

	UNLOCK(&ippool->lock);
}

static void res_ip_init()
{
	assert(ippool == NULL);
	ippool = calloc(1, sizeof(struct _ippool_t));
	if(ippool == NULL) {
		sys_err("Calloc memory for ip pool failed: %s(%d)\n", 
			strerror(errno), errno);
		exit(-1);
	}
	
	LOCK_INIT(&ippool->lock);
	INIT_LIST_HEAD(&ippool->pool);
	INIT_LIST_HEAD(&ippool->alloc);
	return;
}

void resource_init()
{
	res_ip_init();
}
