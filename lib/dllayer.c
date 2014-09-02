/*
 * ============================================================================
 *
 *       Filename:  dllayer.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月19日 10时34分24秒
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
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <fcntl.h>

#include "log.h"
#include "dllayer.h"

struct dlleth_t {
	struct ethhdr 	hdr;
	char 	data[0];
}__attribute__((packed));

struct dllnic_t {
	char nic[IFNAMSIZ];
	char mac[ETH_ALEN];
	int  ifiindex;
} dllnic;

struct dllsdr_t {
	int sdrsock;
	char *sdrpkt;
	struct sockaddr_ll ll;
	pthread_mutex_t lock;
} dllsdr;

struct dllrcv_t {
	int rcvsock;
	char *rcvpkt;
	socklen_t recvlen;
	struct sockaddr_ll ll;
	pthread_mutex_t lock;
} dllrcv;

struct dllbrd_t {
	int brdsock;
	char *brdpkt;
	struct sockaddr_ll ll;
	pthread_mutex_t lock;
} dllbrd;

#define LOCK_RCV() 	pthread_mutex_lock(&dllrcv.lock)
#define LOCK_BRD() 	pthread_mutex_lock(&dllbrd.lock)
#define LOCK_SDR() 	pthread_mutex_lock(&dllsdr.lock)
#define UNLOCK_RCV() 	pthread_mutex_unlock(&dllrcv.lock)
#define UNLOCK_BRD() 	pthread_mutex_unlock(&dllbrd.lock)
#define UNLOCK_SDR() 	pthread_mutex_unlock(&dllsdr.lock)

static void __init_nic(int sock, char *nic)
{
	assert(nic != NULL);
	
	int ret;
	struct ifreq req;
	strncpy(req.ifr_name, nic, IFNAMSIZ-1);
	ret = ioctl(sock, SIOCGIFINDEX, &req);
	if(ret < 0) {
		sys_err("Can not get ifiindex of %s: %s\n", 
			nic, strerror(errno));
		exit(-1);
	}
	dllnic.ifiindex = req.ifr_ifindex;

	ret = ioctl(sock, SIOCGIFHWADDR, &req);
	if(ret < 0) {
		sys_err("Can not get mac addr of %s: %s\n", 
			nic, strerror(errno));
		exit(-1);
	}

	strncpy(dllnic.nic, nic, IFNAMSIZ-1);
	memcpy(dllnic.mac, req.ifr_hwaddr.sa_data, ETH_ALEN);
}

static void __init_pktbuf(char *localmac)
{
	assert(dllsdr.sdrpkt == NULL && dllrcv.rcvpkt == NULL);

	char *tmp;
	tmp = malloc(DLL_PKT_MAXLEN * 3);
	if(tmp == NULL) {
		sys_err("Init dllayer pkt buffer failed: %s\n", 
			strerror(errno));
		exit(-1);
	}
	dllsdr.sdrpkt = tmp;
	dllrcv.rcvpkt = tmp + DLL_PKT_MAXLEN;
	dllbrd.brdpkt = tmp + (DLL_PKT_MAXLEN << 1);

	struct ethhdr *eth = (void *)dllsdr.sdrpkt;
	memcpy(eth->h_source, localmac, ETH_ALEN);
	eth->h_proto = htons(ETH_INNO);

	eth = (void *)dllbrd.brdpkt;
	memcpy(eth->h_source, localmac, ETH_ALEN);
	memset(eth->h_dest, 0xff, ETH_ALEN);
	eth->h_proto = htons(ETH_INNO);
}

int __create_sock(int brdcst)
{
	int sock;
	sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_INNO));
	if(sock < 0) {
		sys_err("Create dllayer socket failed: %s\n", 
			strerror(errno));
		exit(-1);
	}

	if(brdcst) {
		int ret;
		ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, 
			&brdcst, sizeof(brdcst));
		if(ret < 0) {
			sys_err("Enable broadcast mode failed: %s\n",
				strerror(errno));
			exit(-1);
		}
	}
	return sock;
}

void __build_brdll()
{
	struct sockaddr_ll *ll = &dllbrd.ll;
	memset(ll, 0, sizeof(struct sockaddr_ll));

	ll->sll_family   = PF_PACKET;
	ll->sll_ifindex  = dllnic.ifiindex;
	ll->sll_halen 	 = ETH_ALEN;
	memset(ll->sll_addr, 0xff, 6);
	ll->sll_addr[6]  = 0x00;
	ll->sll_addr[7]  = 0x00;
}

void __build_rcvll(int bind)
{
	dllrcv.recvlen = sizeof(struct sockaddr_ll);

	struct sockaddr_ll *ll = &dllrcv.ll;
	memset(ll, 0, sizeof(struct sockaddr_ll));
	
	if(bind) {
		ll->sll_family   = PF_PACKET;
		ll->sll_protocol = htons(ETH_INNO);
		ll->sll_ifindex  = dllnic.ifiindex;
	} else {
		ll->sll_family   = PF_PACKET;
		ll->sll_protocol = htons(ETH_INNO);
		ll->sll_ifindex  = dllnic.ifiindex;
		ll->sll_pkttype = PACKET_HOST;
	}
}

void __build_sdrll(char *mac)
{
	struct sockaddr_ll *ll = &dllsdr.ll;
	memset(ll, 0, sizeof(struct sockaddr_ll));

	ll->sll_family   = PF_PACKET;
	ll->sll_ifindex  = dllnic.ifiindex;
	ll->sll_halen 	 = ETH_ALEN;
	memcpy(ll->sll_addr, mac, ETH_ALEN);
	ll->sll_addr[6]  = 0x00;
	ll->sll_addr[7]  = 0x00;
}

void __init_brdcast(int *sock)
{
	pthread_mutex_init(&dllbrd.lock, NULL);
	dllbrd.brdsock = __create_sock(1);
	if(sock)
		*sock = dllbrd.brdsock;
	__build_brdll();
}

void __dll_buildpkt(char *dmac, char *data, int size)
{
	assert(dmac != NULL && data != NULL && 
		size <= DLL_PKT_DATALEN);

	struct dlleth_t *deth = (void *)dllsdr.sdrpkt;
	memcpy(&deth->hdr.h_dest[0], dmac, ETH_ALEN);

	memcpy(deth->data, data, size);
}

static void __init_sdr(int *sock)
{
	pthread_mutex_init(&dllsdr.lock, NULL);
	dllsdr.sdrsock = __create_sock(0);
	if(sock)
		*sock = dllsdr.sdrsock;
}

static void __init_rcv(int *sock)
{
	int ret;

	pthread_mutex_init(&dllrcv.lock, NULL);
	dllrcv.rcvsock = __create_sock(0);
	if(sock)
		*sock = dllrcv.rcvsock;

	__build_rcvll(1);
	ret = bind(dllrcv.rcvsock, (struct sockaddr *)&dllrcv.ll, 
		dllrcv.recvlen);
	if(ret < 0) {
		sys_err("Bind recive socket failed: %s\n", 
			strerror(errno));
		exit(-1);
	}
}

int dll_sendpkt(char *dmac, char *data, int size)
{
	assert(dmac != NULL && size <= DLL_PKT_DATALEN && 
		data != NULL);
	int ret;

	LOCK_SDR();
	__dll_buildpkt(dmac, data, size);
	__build_sdrll(dmac);

	ret = sendto(dllsdr.sdrsock, dllsdr.sdrpkt, 
		size + sizeof(struct ethhdr), 0,
		(struct sockaddr *)&dllsdr.ll, sizeof(dllsdr.ll));
	if(ret < 0) {
		sys_warn("send packet failed: %s\n", 
			strerror(errno));
		UNLOCK_SDR();
		return -1;
	}
	UNLOCK_SDR();
	return 0;
}

int dll_brdcast(char *data, int size)
{
	assert(size <= DLL_PKT_DATALEN && data != NULL);

	int ret;
	struct dlleth_t *deth = (void *)dllbrd.brdpkt;

	LOCK_BRD();
	memcpy(&deth->data, data, size);

	ret = sendto(dllbrd.brdsock, dllbrd.brdpkt, 
		size + sizeof(struct ethhdr), 0, 
		(struct sockaddr *)&dllbrd.ll, sizeof(dllbrd.ll));
	if(ret < 0) {
		sys_warn("broad cast failed: %s\n", 
			strerror(errno));
		UNLOCK_BRD();
		return -1;
	}

	UNLOCK_BRD();
	return 0;
}

int dll_rcv(char *data, int size)
{
	assert(size <= DLL_PKT_DATALEN && data != NULL);

	dllrcv.recvlen = sizeof(struct sockaddr_ll);

	int ret;
	LOCK_RCV();
	__build_rcvll(0);
	ret = recvfrom(dllrcv.rcvsock, dllrcv.rcvpkt, 
		DLL_PKT_MAXLEN, 0, 
		(struct sockaddr *)&dllrcv.ll, &dllrcv.recvlen);
	if(ret < sizeof(struct ethhdr)) {
		sys_warn("recv pkt failed: %s\n", strerror(errno));
		UNLOCK_RCV();
		return -1;
	}

	struct ethhdr *hdr = (struct ethhdr *)dllrcv.rcvpkt;
	if(ntohs(hdr->h_proto) != ETH_INNO) {
		sys_warn("Reciv error dllayer packet\n");
		return -1;
	}

	memcpy(data, dllrcv.rcvpkt + sizeof(struct ethhdr), size);
	UNLOCK_RCV();
	return (ret > size) ? size: ret;
}

void dll_init(char *nic, int *rcvsock, int *sdrsock, int *brdsock)
{
	assert(nic != NULL);

	int sock = __create_sock(0);
	__init_nic(sock, nic);
	close(sock);
	__init_pktbuf(&dllnic.mac[0]);
	__init_brdcast(brdsock);
	__init_rcv(rcvsock);
	__init_sdr(sdrsock);
}

