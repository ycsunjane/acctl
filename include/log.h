/*
 * =====================================================================================
 *
 *       Filename:  log.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年06月16日 09时59分21秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __LOG_H__
#define __LOG_H__
#include <syslog.h>
#include <pthread.h>

#include "arg.h"

extern int debug;

#define SYSLOG_ERR 	(LOG_ERR|LOG_USER)
#define SYSLOG_WARN 	(LOG_WARNING|LOG_USER)
#define SYSLOG_DEBUG 	(LOG_DEBUG|LOG_USER)
#define SYSLOG_INFO 	(LOG_INFO|LOG_USER)
#define SYSLOG_LOCK 	(LOG_NOTICE|LOG_USER)

#define __sys_log2(fmt, ...) 						\
	do { 								\
		syslog(SYSLOG_INFO, fmt, ##__VA_ARGS__); 		\
		if(debug) fprintf(stderr, fmt,  ##__VA_ARGS__); 	\
	} while(0)

#define __sys_log(LEVEL, fmt, ...) 										\
	do { 													\
		int __debug = 0; 										\
		syslog(LEVEL, "(%u)%s +%d %s(): "fmt,  								\
			(unsigned int)pthread_self(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); 	\
		switch(debug) { 										\
		case ARG_DEBUG: 											\
			__debug = (LEVEL == SYSLOG_DEBUG) ? 1: 0; 						\
			break; 											\
		case ARG_WARN: 											\
			__debug = (LEVEL == SYSLOG_WARN) ? 1: 0; 						\
			break; 											\
		case ARG_LOCK: 											\
			__debug = (LEVEL == SYSLOG_LOCK) ? 1: 0; 						\
			break; 											\
		default: 											\
			__debug = 1; 										\
		} 												\
		if(__debug) 											\
			fprintf(stderr, "(%u)%s +%d %s(): "fmt, 							\
				(unsigned int)pthread_self(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); 	\
	} while(0)

#define sys_err(fmt, ...) 	__sys_log(SYSLOG_ERR, "ERR: "fmt, ##__VA_ARGS__)

#ifdef DEBUG
#define sys_warn(fmt, ...) 	__sys_log(SYSLOG_WARN, "WARNING: "fmt, ##__VA_ARGS__)
#define sys_debug(fmt, ...) 	__sys_log(SYSLOG_DEBUG, "DEBUG: "fmt, ##__VA_ARGS__)
#define sys_lock(fmt, ...) 	__sys_log(SYSLOG_LOCK, "LOCK: "fmt, ##__VA_ARGS__)
#define pure_info(fmt, ...) 	__sys_log2(fmt, ##__VA_ARGS__)
#else
#define sys_warn(fmt, ...) 	NULL
#define sys_debug(fmt, ...)  	NULL
#define sys_lock(fmt, ...) 	NULL
#define pure_info(fmt, ...) 	NULL
#endif

#ifdef DEBUG
#define pr_ap(mac, uuid) 	__pr_ap(mac, uuid)
#else
#define pr_ap(mac, uuid) 	NULL
#endif
static void __pr_ap(char *mac, char *uuid)
{
	printf("%02x%02x%02x%02x%02x%02x reg in: %s\n",
		(unsigned char) mac[0],
		(unsigned char) mac[1],
		(unsigned char) mac[2],
		(unsigned char) mac[3],
		(unsigned char) mac[4],
		(unsigned char) mac[5],
		uuid);
}

#endif /* __LOG_H__ */
