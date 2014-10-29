/*
 * ============================================================================
 *
 *       Filename:  message.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月26日 09时16分29秒
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
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

#include "log.h"
#include "arg.h"
#include "msg.h"
#include "thread.h"
#include "message.h"
#include "process.h"

struct message_t *head = NULL;
struct message_t **tail = &head;
pthread_mutex_t message_lock = PTHREAD_MUTEX_INITIALIZER;
#define MUTEX_LOCK(lock)  pthread_mutex_lock(&lock)
#define MUTEX_UNLOCK(lock)  pthread_mutex_unlock(&lock)

static int message_num = 0;
void message_insert(struct message_t *msg)
{
	sys_debug("message insert msg: %p\n", msg);
	msg->next = NULL;

	MUTEX_LOCK(message_lock);
	message_num++; 
	*tail = msg;
	tail = &msg->next;
	MUTEX_UNLOCK(message_lock);
}

struct message_t * message_delete()
{
	MUTEX_LOCK(message_lock);
	if(message_num == 0) {
		assert(*tail == head);
		MUTEX_UNLOCK(message_lock);
		return NULL;
	}
	
	struct message_t *tmp = head;
	message_num--;

	head = head->next;
	if(tail == &tmp->next)
		tail = &head;

	MUTEX_UNLOCK(message_lock);
	sys_debug("message delete: %p\n", tmp);
	return tmp;
}

void message_free(struct message_t *msg)
{
	free(msg);
}

void *message_travel(void *arg)
{
	struct message_t *msg;
	while(1) {
		sleep(argument.msgitv);
		if(head == NULL) continue;
		while((msg = message_delete())) {
			msg_proc((void *)msg->data, msg->len, msg->proto);
			message_free(msg);
		}
		sys_debug("Message travel pthreads (next %d second later)\n", 
			argument.msgitv);
	}
	return NULL;
}

void message_init()
{
	/* create thread to travel all message */
	__create_pthread(message_travel, NULL);
}
