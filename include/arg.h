/*
 * =====================================================================================
 *
 *       Filename:  arg.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月18日 17时37分02秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __ARG_H__
#define __ARG_H__
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <getopt.h>

struct arg_t {
#ifdef SERVER
	int  	brditv;
	int 	port;
#endif
#ifdef CLIENT
	int 	reportitv; 
#endif
	struct sockaddr_in addr;
	int 	msgitv;
	char 	nic[IFNAMSIZ];
	char 	mac[ETH_ALEN];
};

enum {
	ARG_DEBUG = 1,
	ARG_WARN,
	ARG_LOCK,
};

extern struct arg_t argument;
extern int debug;
extern int daemon_mode;

void help();
void proc_cfgarg();
void proc_cmdarg(int argc, char *argv[]);
void proc_arg(int argc, char *argv[]);
#endif /* __ARG_H__ */
