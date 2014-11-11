/*
 * =====================================================================================
 *
 *       Filename:  thread.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月25日 17时19分58秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __THREAD_H__
#define __THREAD_H__

#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

#define THREAD 	pthread_mutex_t

#define INIT_LOCK(name) \
	THREAD name = PTHREAD_MUTEX_INITIALIZER

#define LOCK(name) \
	pthread_mutex_lock(name)

#define TRYLOCK(name) \
	pthread_mutex_lock(name)

#define UNLOCK(name) \
	pthread_mutex_unlock(name)

static inline void LOCK_INIT(THREAD *name)
{
	pthread_mutex_init(name, NULL);
}

void create_pthread(void *(*start_routine) (void *), void *arg);
#endif /* __THREAD_H__ */
