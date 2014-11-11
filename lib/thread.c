/*
 * ============================================================================
 *
 *       Filename:  thread.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年11月11日 16时15分18秒
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

#include "thread.h"
#include "log.h"

void create_pthread(void *(*start_routine) (void *), void *arg)
{
	int ret;
	pthread_t pid;
	ret = pthread_create(&pid, NULL, start_routine, arg);
	if(ret != 0) {
		sys_err("Create pthread failed: %s(%d)\n", 
			strerror(errno), errno);
		exit(-1);
	}

	ret = pthread_detach(pid);
	if(ret != 0) {
		sys_err("Create pthread failed: %s(%d)\n", 
			strerror(errno), errno);
		exit(-1);
	}
}
