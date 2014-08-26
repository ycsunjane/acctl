/*
 * =====================================================================================
 *
 *       Filename:  link.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月27日 09时43分55秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __LINK_H__
#define __LINK_H__
#include <sys/select.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "log.h"

struct sockarr_t;

static struct sockarr_t *__head = NULL;
static struct sockarr_t **__tail = &__head;
/* if add a new sock, newflag will be set 1 */
static int newflag = 0;

struct sockarr_t {
	int sock;
	void *(*func) (void *);
	struct sockarr_t *next;
};

static void * net_recv(void *arg)
{
	struct sockarr_t *sockarr;

	int ret;
	int maxfd = 0;
	fd_set rcvset, backset;

reset:
	sockarr = arg;
	FD_ZERO(&rcvset);
	while(sockarr) {
		maxfd = (maxfd > sockarr->sock) ?  maxfd : sockarr->sock + 1;
		FD_SET(sockarr->sock, &rcvset);
		sockarr = sockarr->next;
	}
	backset = rcvset;

	while(1) {
		if(newflag == 1) goto reset;
		rcvset = backset ;
		ret = select(maxfd, &rcvset, NULL, NULL, NULL);
		if(ret <= 0)
			continue;

		sockarr = arg;
		while(sockarr && ret--) {
			if(FD_ISSET(sockarr->sock, &rcvset))
				sockarr->func(NULL);
			sockarr = sockarr->next;
		}
	}
	return NULL;	
}

static struct sockarr_t *__insert_sockarr(void *(*func) (void *))
{
	struct sockarr_t *sock = malloc(sizeof(struct sockarr_t));
	if(sock == NULL) {
		sys_err("Malloc sockarr failed: %s\n", strerror(errno));
		exit(-1);
	}

	newflag = 1;
	sock->func = func;
	sock->next = NULL;

	*__tail = sock;
	__tail = &sock->next;
	return sock;
}

#endif /* __LINK_H__ */
