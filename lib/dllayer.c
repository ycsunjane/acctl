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

#include "log.h"

#define ETH_INNO 		(ETH_P_ALL)
#define DLL_PKT_MAXLEN 		(512)
#define DLL_PKT_DATALEN 	(DLL_PKT_MAXLEN - sizeof(struct ethhdr))

struct dllnic_t {
	char nic[IFNAMSIZ];
	char mac[ETH_ALEN];
	int  ifiindex;
} dllnic;

struct dllsdr_t {
	int sdrsock;
	char *sdrpkt;
	struct sockaddr_ll ll;
} dllsdr;

struct dllrcv_t {
	int rcvsock;
	char *rcvpkt;
	socklen_t recvlen;
	struct sockaddr_ll ll;
} dllrcv;

struct dllbrd_t {
	int brdsock;
	char *brdpkt;
	struct sockaddr_ll ll;
} dllbrd;

struct dlleth_t {
	struct ethhdr 	hdr;
	char 	data[0];
}__attribute__((packed));

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
	sock = socket(PF_PACKET, SOCK_RAW, ETH_INNO);
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

	ll->sll_family   = PF_PACKET;
	ll->sll_protocol = htons(ETH_INNO);
	ll->sll_ifindex  = dllnic.ifiindex;
	ll->sll_hatype   = ARPHRD_ETHER;
	ll->sll_pkttype  = PACKET_BROADCAST;
	ll->sll_halen 	 = ETH_ALEN;
	memset(ll->sll_addr, 0xff, 6);
	ll->sll_addr[6]  = 0x00;
	ll->sll_addr[7]  = 0x00;
}

void __init_brdcast()
{
	dllbrd.brdsock = __create_sock(1);
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

void __build_sdrsock(char *mac)
{
	struct sockaddr_ll *ll = &dllbrd.ll;

	ll->sll_family   = PF_PACKET;
	ll->sll_protocol = htons(ETH_INNO);
	ll->sll_ifindex  = dllnic.ifiindex;
	ll->sll_hatype   = ARPHRD_ETHER;
	ll->sll_pkttype  = PACKET_OUTGOING;
	ll->sll_halen 	 = ETH_ALEN;
	memcpy(ll->sll_addr, mac, ETH_ALEN);
	ll->sll_addr[6]  = 0x00;
	ll->sll_addr[7]  = 0x00;
}

int dll_sendpkt(char *dmac, char *data, int size)
{
	assert(dmac != NULL && size <= DLL_PKT_DATALEN && 
		data != NULL);
	__dll_buildpkt(dmac, data, size);
	__build_sdrsock(dmac);

	int ret;
	ret = sendto(dllsdr.sdrsock, dllsdr.sdrpkt, size + ETH_ALEN, 0,
		(struct sockaddr *)&dllsdr.ll, sizeof(dllsdr.ll));
	if(ret < 0) {
		sys_warn("send packet failed: %s\n", 
			strerror(errno));
		return -1;
	}
	return 0;
}

int dll_brdcast(char *data, int size)
{
	assert(size <= DLL_PKT_DATALEN && data != NULL);

	int ret;
	struct dlleth_t *deth = (void *)dllbrd.brdpkt;
	memcpy(&deth->data, data, size);

	ret = sendto(dllbrd.brdsock, dllbrd.brdpkt, size + ETH_ALEN, 0, 
		(struct sockaddr *)&dllbrd.ll, sizeof(dllbrd.ll));
	if(ret < 0) {
		sys_warn("broad cast failed: %s\n", 
			strerror(errno));
		return -1;
	}

	return 0;
}

int dll_rcv(char *data, int size)
{
	dllrcv.recvlen = sizeof(struct sockaddr_ll);

	int ret;
	ret = recvfrom(dllrcv.rcvsock, dllrcv.rcvpkt, 
		DLL_PKT_MAXLEN, 0, 
		(struct sockaddr *)&dllrcv.ll, &dllrcv.recvlen);
	if(ret < 0) {
		sys_warn("recv pkt failed: %s\n", strerror(errno));
		return -1;
	}
	memcpy(data, dllrcv.rcvpkt, size);
	return 0;
}

void dll_init(char *nic)
{
	assert(nic != NULL);
	dllsdr.sdrsock = __create_sock(0);
	dllrcv.rcvsock = __create_sock(1);

	__init_nic(dllsdr.sdrsock, nic);
	__init_pktbuf(&dllnic.mac[0]);
	__init_brdcast();
}

