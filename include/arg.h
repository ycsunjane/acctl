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
#include <net/if.h>

struct arg_t {
	char nic[IFNAMSIZ];
};

extern struct arg_t argument;
extern int debug;
extern int daemon_mode;

void proc_arg(int argc, char *argv[]);
#endif /* __ARG_H__ */
