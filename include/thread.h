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
#include "log.h"

static void __create_pthread(void *(*start_routine) (void *), void *arg)
{
	int ret;
	pthread_t pid;
	ret = pthread_create(&pid, NULL, start_routine, arg);
	if(ret != 0) {
		sys_err("Create pthread failed: %s\n", strerror(errno));
		exit(-1);
	}

	ret = pthread_detach(pid);
	if(ret != 0) {
		sys_err("Create pthread failed: %s\n", strerror(errno));
		exit(-1);
	}
}
#endif /* __THREAD_H__ */
