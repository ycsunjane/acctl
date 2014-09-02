/*
 * =====================================================================================
 *
 *       Filename:  message.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月26日 09时16分35秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#include "dllayer.h"

enum {
	ETH,
	TCP,
}; 

/* message */
struct message_t {
	/* dllayer or tcp */
	int 			proto;
	struct message_t  	*next;
	char 			data[0];
};

extern int message_num;
extern struct message_t *head, **tail;

void message_init();
void message_insert(struct message_t *msg);
#endif /* __MESSAGE_H__ */
