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

struct _ip_t {
	char   apmac[ETH_ALEN];
	struct sockaddr_in ipv4;
	struct _ip_t *next;
};

struct _ippool_t {
	struct _ip_t *pool;
	struct _ip_t *alloc;
	int 	total;
	int 	left;
};

extern struct _ippool_t *ippool;

struct _ip_t *res_ip_alloc(struct sockaddr_in *addr, char *mac);
int res_ip_conflict(struct sockaddr_in *addr, char *mac);
void resource_init();
#endif /* __RESOURCE_H__ */
