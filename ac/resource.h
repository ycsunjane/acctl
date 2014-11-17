/*
 * =====================================================================================
 *
 *       Filename:  resource.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年10月24日 15时10分10秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include "thread.h"
#include "list.h"
#include "mjson.h"

#define IPLEN 	(20)
struct res_t {
	char ip_start[IPLEN];
	char ip_end[IPLEN];
	char ip_mask[IPLEN];
	char bak_start[IPLEN];
	char bak_end[IPLEN];
	char bak_mask[IPLEN];
} resource;

static const struct json_attr_t json_attrs[] = {
	{"ip_start", t_string, .addr.string = resource.ip_start, .len = IPLEN},
	{"ip_end", t_string, .addr.string = resource.ip_end, .len = IPLEN},
	{"ip_mask", t_string, .addr.string = resource.ip_mask, .len = IPLEN},
	{NULL},
};

struct _ip_t {
	char   apmac[ETH_ALEN];
	struct sockaddr_in ipv4;
	struct list_head list;
};

struct _ippool_t {
	THREAD lock;
	struct list_head pool;
	struct list_head alloc;
	int 	total;
	int 	left;
};

extern struct _ippool_t *ippool;

void resource_init();
struct _ip_t *res_ip_alloc(struct sockaddr_in *addr, char *mac);
int res_ip_conflict(struct sockaddr_in *addr, char *mac);
int res_ip_add(struct sockaddr_in *addr);
void res_ip_clear();
#endif /* __RESOURCE_H__ */
