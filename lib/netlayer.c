/*
 * ============================================================================
 *
 *       Filename:  netlayer.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月26日 15时55分51秒
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
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>

#include "log.h"
struct nettcp_t {
	int sock;
	int loop;
	struct sockaddr_in addr;
};

struct nettcp_t tcp = {0};

int tcp_connect(struct sockaddr_in addr, int reconnect)
{
	int ret;

	if(!reconnect) {
		tcp.sock = socket(AF_INET, SOCK_STREAM, 0);
		if(tcp.sock < 0) {
			sys_err("Create tcp sock failed: %s\n",
				strerror(errno));
			return -1;
		}

		tcp.addr = addr;
	}

	socklen_t addr_len = sizeof(struct sockaddr_in);
	ret = connect(tcp.sock, (struct sockaddr *)&addr, addr_len);
	if(ret < 0) {
		sys_err("Connect ac failed: %s\n", strerror(errno));
		return -1;
	}
	tcp.loop = 3;
	return tcp.sock;
}

int tcp_rcv(char *data, int size)
{
	assert(data != NULL);

	int recvlen;
retry:
	recvlen = recv(tcp.sock, data, size, 0);
	if(recvlen < 0) {
		sys_err("tcp recv failed: %s\n", strerror(errno));
		if(errno == EINTR)
			goto retry;
		if(errno == ETIMEDOUT)
			goto recnt;
	}
	return recvlen;
recnt:
	if(!tcp.loop) return -1;
	tcp_connect(tcp.addr, 1);
	tcp.loop--;
	goto retry;
}

int tcp_sendpkt(char *data, int size)
{
	assert(data != NULL);

	int sdrlen;
retry:
	sdrlen = send(tcp.sock, data, size, 0);
	if(sdrlen < 0) {
		sys_err("tcp send failed: %s\n", strerror(errno));
		if(errno == EINTR)
			goto retry;
		if(errno == EPIPE)
			goto recnt;
	}
	return sdrlen;
recnt:
	if(!tcp.loop) return -1;
	tcp_connect(tcp.addr, 1);
	tcp.loop--;
	goto retry;
}

