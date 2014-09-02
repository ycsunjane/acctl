/*
 * =====================================================================================
 *
 *       Filename:  net.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/26/14 14:26:10
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __NET_H__
#define __NET_H__

extern struct nettcp_t tcp;
void net_init();
void *__net_netrcv(void *arg);
#endif /* __NET_H__ */
