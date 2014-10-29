/*
 * ============================================================================
 *
 *       Filename:  chap.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年10月28日 16时14分55秒
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
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include "chap.h"
#include "msg.h"
#include "log.h"

static char passwd[] = PASSWD;

void chap_get_md5(uint8_t *data, int len, uint32_t random, uint8_t *decrypt)
{
	pr_pkt(data, len);

	MD5_CTX md5;
	MD5Init(&md5);
	MD5Update(&md5, data, len);
	MD5Update(&md5, (void *)&random, sizeof(uint32_t));
	MD5Update(&md5, (void *)&passwd[0], strlen(passwd));
	MD5Final(&md5, decrypt);
	sys_debug("data: %p, len: %d, random: %u\n", data, len, random);
	pr_md5(decrypt);

	return;
}

int chap_cmp_md5(uint8_t *data, int len, uint32_t random, uint8_t *oldmd5)
{
	uint8_t *decrypt = malloc(CHAP_LEN);
	if(!decrypt) {
		sys_err("Malloc decrypt failed: %s(%d)\n", 
			strerror(errno), errno);
		return -1;
	}

	chap_get_md5(data, len, random, decrypt);

	int ret;
	ret = memcmp(decrypt, oldmd5, CHAP_LEN);
	free(decrypt);

	return ret;
}

int chap_msg_cmp_md5(struct msg_head_t *msg, int len, uint32_t random)
{
	uint8_t oldmd5[CHAP_LEN];
	memcpy(oldmd5, msg->chap, CHAP_LEN);
	memset(msg->chap, 0x0, CHAP_LEN);
	return chap_cmp_md5((void *)msg, len, random, oldmd5);
}

void chap_fill_msg_md5(struct msg_head_t *msg, int len, int random)
{
	memset(msg->chap, 0, CHAP_LEN);
	chap_get_md5((void *)msg, len, random, msg->chap);
}

uint32_t chap_get_random()
{
	uint32_t random;
	int fd = open("/dev/random", O_RDONLY);
	read(fd, &random, sizeof(random));
	close(fd);

	sys_debug("Generate a random: %u\n", random);
	return random;
}
