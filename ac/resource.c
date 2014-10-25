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

struct _ippool_t *ippool;

struct _ip_t *res_ip_alloc(struct sockaddr_in *addr, char *mac)
{
	assert(ippool != NULL);

	if(ippool->left == 0) {
		sys_warn("No ip in pool\n");
		return NULL;
	}
	assert(ippool->pool != NULL);

	struct _ip_t *new, **prev;
alloc_new:
	new = ippool->pool;
	if(addr == NULL || addr->sin_addr.s_addr == 0) {
		ippool->pool = new->next;
		new->next = ippool->alloc->next;
		ippool->alloc = new;
		memcpy(new->apmac, mac, ETH_ALEN);
		ippool->left--;
		return new;
	} else {
		/* if specify addr will alloc addr first */
		prev = &(ippool->pool);
		while(new) {
			if((addr->sin_addr.s_addr ==
					new->ipv4.sin_addr.s_addr)) {
				*prev = new->next;
				new->next = ippool->alloc->next;
				ippool->alloc = new;
				memcpy(new->apmac, mac, ETH_ALEN);
				ippool->left--;
				return new;
			}
			prev = &(new->next);
			new = new->next;
		}

		addr = NULL;
		goto alloc_new;
	}

	return NULL;
}

int res_ip_conflict(struct sockaddr_in *addr, char *mac)
{
	assert(ippool != NULL);
	if(addr->sin_addr.s_addr == 0)
		return 0;

	struct _ip_t *ip = ippool->alloc;
	while(ip) {
		if((addr->sin_addr.s_addr == ip->ipv4.sin_addr.s_addr)) {
			if(memcmp(mac, ip->apmac, ETH_ALEN))
				return 1;
			else
				return 0;
		}
		ip = ip->next;
	}
	return 0;
}

static void res_ip_init()
{
	int total = 0; /* get ip num from database */
	ippool = calloc(1, 
		sizeof(struct _ippool_t) + sizeof(struct _ip_t) * total);
	if(ippool == NULL) {
		sys_err("Calloc memory for ip pool failed: %s(%d)\n", 
			strerror(errno), errno);
		exit(-1);
	}
	ippool->total = total;
	ippool->left = total;
}

void resource_init()
{
	res_ip_init();
}
