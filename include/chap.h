/*
 * =====================================================================================
 *
 *       Filename:  chap.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年10月28日 17时10分56秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __CHAP_H__
#define __CHAP_H__

#include <stdint.h>
#include "md5.h"

struct msg_head_t;

#define CHAP_LEN 	(16)
void chap_get_md5(uint8_t *data, int len, uint32_t random, uint8_t *decrypt);
int chap_cmp_md5(uint8_t *data, int len, uint32_t random, uint8_t *oldmd5);
int chap_msg_cmp_md5(struct msg_head_t *msg, int len, uint32_t random);
void chap_fill_msg_md5(struct msg_head_t *msg, int len, int random);
uint32_t chap_get_random();
#endif /* __CHAP_H__ */
