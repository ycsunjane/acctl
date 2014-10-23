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
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <fcntl.h>
#include <signal.h>

#include "log.h"
#include "link.h"
#include "netlayer.h"

static int __tcp_alive(struct nettcp_t *tcp)
{

	int optval = 1;
	int optlen = sizeof(optval);
	if(setsockopt(tcp->sock, SOL_SOCKET, 
			SO_KEEPALIVE, &optval, optlen) < -1) {
		sys_err("Set tcp keepalive failed: %s\n",
			strerror(errno));
		return -1;
	}

	optval = 30;
	if(setsockopt(tcp->sock, SOL_TCP, 
			TCP_KEEPCNT, &optval, optlen) < -1) {
		sys_err("Set tcp_keepalive_probes failed: %s\n",
			strerror(errno));
		return -1;
	}

	optval = 30;
	if(setsockopt(tcp->sock, SOL_TCP, 
			TCP_KEEPIDLE, &optval, optlen) < -1) {
		sys_err("Set tcp_keepalive_time failed: %s\n",
			strerror(errno));
		return -1;
	}

	optval = 5;
	if(setsockopt(tcp->sock, SOL_TCP, 
			TCP_KEEPINTVL, &optval, optlen) < -1) {
		sys_err("Set tcp_keepalive_intvl failed: %s\n",
			strerror(errno));
		return -1;
	}

	return 0;
}

int tcp_connect(struct nettcp_t *tcp)
{
	int ret;

	tcp->sock = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp->sock < 0) {
		sys_err("Create tcp sock failed: %s\n",
			strerror(errno));
		return -1;
	}
	ret = __tcp_alive(tcp);
	if(ret < 0) {
		sys_err("Set tcp alive failed\n");
		tcp_close(tcp);
		return -1;
	}

	socklen_t addr_len = sizeof(struct sockaddr_in);
	ret = connect(tcp->sock, (struct sockaddr *)&tcp->addr, addr_len);
	if(ret < 0) {
		tcp_close(tcp);
		sys_err("Connect ac failed: %s\n", strerror(errno));
		return -1;
	}

	return tcp->sock;
}

int tcp_rcv(struct nettcp_t *tcp, char *data, int size)
{
	assert(data != NULL && tcp->sock >= 0);

	int recvlen = 0, len;

	while(1) {
		len = recv(tcp->sock, data, size, 0);
		if(len > 0) {
			data += len;
			size -= len;
			recvlen += len;
			continue;
		} else if(len <= 0) {
			if(errno == EINTR)
				continue;
			if(errno == EAGAIN)
				break;
			sys_err("tcp recv failed: %s(%d)\n", 
				strerror(errno), errno);
			break;
		}
	}

	return recvlen;
}

int tcp_sendpkt(struct nettcp_t *tcp, char *data, int size)
{
	assert(data != NULL && size <= NET_PKT_DATALEN);

	if(tcp->sock == -1) return -1;

	int sdrlen;
	while(1) {
		sdrlen = send(tcp->sock, data, size, 0);
		if(sdrlen <= 0) {
			if(errno == EAGAIN || errno == EINTR)
				continue;
			sys_err("tcp send failed: %s(%d)\n", 
				strerror(errno), errno);
			break;
		}
	}

	return sdrlen;
}

void tcp_close(struct nettcp_t *tcp)
{
	close(tcp->sock);
	tcp->sock = -1;
}

#ifdef SERVER
#define BACKLOG 	(512)
int tcp_listen(struct nettcp_t *tcp)
{
	int ret;

	tcp->sock = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp->sock < 0) {
		sys_err("Create tcp sock failed: %s\n",
			strerror(errno));
		return -1;
	}

	int reuseaddr = 1;
	ret = setsockopt(tcp->sock, SOL_SOCKET, SO_REUSEADDR,
		&reuseaddr, sizeof(reuseaddr));
	if(ret < 0) {
		sys_err("set sock reuse failed: %s\n",
			strerror(errno));
		return -1;
	}

	socklen_t socklen = sizeof(struct sockaddr_in);
	ret = bind(tcp->sock, 
		(struct sockaddr *)&tcp->addr, socklen);
	if(ret < 0) {
		sys_err("Bind tcp sock failed: %s\n",
			strerror(errno));
		tcp_close(tcp);
		return -1;
	}

	ret = listen(tcp->sock, BACKLOG);
	if(ret < 0) {
		sys_err("Bind tcp sock failed: %s\n",
			strerror(errno));
		tcp_close(tcp);
		return -1;
	}

	return tcp->sock;
}

static int _sock_nonblock(int socket)
{
	int flags;

	flags = fcntl(socket, F_GETFL, 0);
	if(flags < 0) {
		sys_err("Get socket flags failed: %s(%d)\n", 
			strerror(errno), errno);
		return -1;
	}

	if(fcntl(socket, F_SETFL, flags | O_NONBLOCK) < 0) {
		sys_err("Set socket flags failed: %s(%d)\n", 
			strerror(errno), errno);
		return -1;
	}

	return 0;
}

int tcp_accept(struct nettcp_t *tcp, void *func(void *))
{
	int clisock;
	clisock = accept(tcp->sock, NULL, NULL);
	if(clisock < 0) {
		sys_err("Accept tcp sock failed: %s\n",
			strerror(errno));
		return -1;
	}

	if(_sock_nonblock(clisock) < 0)
		return -1;

	sys_debug("New client:%d\n", clisock);
	insert_sockarr(clisock, func, NULL);
	return clisock;
}
#endif
