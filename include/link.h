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
#include <sys/epoll.h>

struct sockarr_t {
	struct epoll_event ev;

	int sock;
	void *(*func) (void *);
	void *arg;
	struct sockarr_t *next;
};

void net_epoll_init();
struct sockarr_t * insert_sockarr(int sock, void *(*func) (void *), void *arg);
int delete_sockarr(int sock);
void * net_recv(void *arg);
#endif /* __LINK_H__ */
