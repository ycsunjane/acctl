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

struct sockarr_t {
	int sock;
	void *(*func) (void *);
	void *arg;
	struct sockarr_t *next;
};

void * net_recv(void *arg);
struct sockarr_t *
__insert_sockarr(int sock, void *(*func) (void *), void *arg);
int __delete_sockarr(int sock, int lock);
#endif /* __LINK_H__ */
