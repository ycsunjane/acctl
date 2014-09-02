/*
 * ============================================================================
 *
 *       Filename:  link.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年09月02日 14时27分53秒
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
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
#include <errno.h>

#include "log.h"
#include "link.h"

static pthread_mutex_t sockarr_lock = PTHREAD_MUTEX_INITIALIZER;
static struct sockarr_t *__head = NULL;
static struct sockarr_t **__tail = &__head;
#define SOCKARR_LOCK() 	 			\
do { 						\
	sys_lock("sockarr lock\n"); 		\
	pthread_mutex_lock(&sockarr_lock); 	\
}while(0)

#define SOCKARR_UNLOCK() 			\
do { 						\
	sys_lock("sockarr unlock\n"); 		\
	pthread_mutex_unlock(&sockarr_lock); 	\
}while(0)

/* if add a new sock, newflag will be set 1 */
static int newflag = 0;

void * net_recv(void *arg)
{
	struct sockarr_t *sockarr, *back;

	int ret;
	int maxfd = 0;
	fd_set rcvset, backset;

	struct timeval timeout;
	timeout.tv_sec = 3;

reset:
	SOCKARR_LOCK();
	sockarr = __head;
	FD_ZERO(&rcvset);
	while(sockarr) {
		maxfd = (maxfd > sockarr->sock) ?  maxfd : sockarr->sock + 1;
		FD_SET(sockarr->sock, &rcvset);
		sockarr = sockarr->next;
	}
	newflag = 0;
	SOCKARR_UNLOCK();
	backset = rcvset;

	while(1) {
		if(newflag == 1) goto reset;
		rcvset = backset;
		ret = select(maxfd, &rcvset, NULL, NULL, &timeout);
		if(ret <= 0) {
			timeout.tv_sec = 3;
			continue;
		}

		SOCKARR_LOCK();
		sockarr = __head;
		while(sockarr && ret) {
			/* func can del sockarr with no lock */
			back = sockarr->next;
			if(FD_ISSET(sockarr->sock, &rcvset)) {
				sockarr->func(sockarr->arg);
				ret--;
			}

			sockarr = back;
		}
		SOCKARR_UNLOCK();
	}
	return NULL;	
}

struct sockarr_t *
__insert_sockarr(int sock, void *(*func) (void *), void *arg)
{
	struct sockarr_t *__sock = malloc(sizeof(struct sockarr_t));
	if(__sock == NULL) {
		sys_err("Malloc sockarr failed: %s\n", strerror(errno));
		exit(-1);
	}

	__sock->sock = sock;
	__sock->func = func;
	__sock->arg = arg;
	__sock->next = NULL;

	SOCKARR_LOCK();
	*__tail = __sock;
	__tail = &__sock->next;
	newflag = 1;
	SOCKARR_UNLOCK();

	return __sock;
}

int __delete_sockarr(int sock, int lock)
{
	struct sockarr_t *cur;
	struct sockarr_t **ppre;

	if(lock) SOCKARR_LOCK();
	cur = __head;
	ppre = &__head;

	while(cur) {
		if(cur->sock == sock) {
			*ppre = cur->next;
			if(&cur->next == __tail)
				__tail = ppre;
			newflag = 1;
			SOCKARR_UNLOCK();

			close(sock);
			free(cur);
			return 0;
		}
		ppre = &cur->next;
		cur = cur->next;
	}
	if(lock) SOCKARR_UNLOCK();
	return 0;
}

