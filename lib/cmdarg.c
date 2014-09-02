/*
 * ============================================================================
 *
 *       Filename:  arg.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月19日 10时04分12秒
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
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "arg.h"
#include "log.h"

#define has_arg (1)
static struct option long_arg[] = {
	{"nic", has_arg, 0, 'n'},
	{"daemon", 0, 0, 'd'},
#ifdef SERVER
	{"brditv", has_arg, 0, 'b'},
	{"port", has_arg, 0, 'p'},
#endif
#ifdef CLIENT
	{"reportitv", has_arg, 0, 'r'},
#endif
	{"msgitv", has_arg, 0, 'm'},
	{"debug", has_arg, 0, 'l'},
	{"help", 0, 0, '*'},
	{0, 0, 0 , 0},
};

#ifdef SERVER
#define SHORT_STR 	"n:db:p:m:l:"
#endif
#ifdef CLIENT
#define SHORT_STR 	"n:dm:r:l:"
#endif
#ifdef TEST
#define SHORT_STR 	"n:dm:l:"
#endif

static char *help_array[] = {
	"USAGE: ac [options]",
	"acctl server",
	"  -n, --nic 		nic which controller [required]",
	"  -d, --daemon 		daemon mode",
#ifdef SERVER
	"  -b, --brditv 	ac broadcast interval (default 30)",
	"  -p, --port 		ac listen port (default 7960)",
#endif
#ifdef CLIENT
	"  -r, --reportitv 	ap report status interval (default 30)",
#endif
	"  -m, --msgitv  	interval travel all recevied message (default 30)",
	"  -l, --debug 		enable debug (DEBUG = 1, WARN = 2, LOCK = 3, ALL = other), will auto disable daemon_mode",
	"  --help 		help info",
	NULL,
};

void help()
{
	char **ptr = &help_array[0];
	while(*ptr != NULL) {
		printf("%s\n", *ptr);
		ptr++;
	}
}

static void __early_is_debug(int argc,char *argv[])
{
	int i, ret;
	for(i = 0; i < argc; i++) {
		ret = strcmp(argv[i], "-l");
		if(ret == 0)  {
			debug = 1;
			return;
		}

		ret = strcmp(argv[i], "--debug");
		if(ret == 0) {
			debug = 1;
			return;
		}
	}

	debug = 0;
	return;
}

static void __early_init(int argc, char *argv[])
{
	if(argc < 2) {
		help();
		exit(-1);
	}
		
	memset(&argument, 0, sizeof(argument));
	__early_is_debug(argc, argv);
}

static long int __strtol(const char *nptr, char **endptr, int base)
{
	long int ret;
	errno = 0;
	ret = strtol(nptr, endptr, base);
	if(errno != 0) {
		sys_err("argument error:%s\n", nptr);
		exit(-1);
	}
	return ret;
}

void proc_cmdarg(int argc, char *argv[])
{
	int short_arg;
	__early_init(argc, argv);

	while((short_arg = getopt_long(argc, argv, SHORT_STR, long_arg, NULL)) 
		!= -1) {
		switch(short_arg) {
		case 'n':
			strncpy(&argument.nic[0], optarg, IFNAMSIZ-1);
			break;
		case 'd':
			if(!debug)
				daemon_mode = 1;
			break;
#ifdef SERVER
		case 'b':
			argument.brditv = 
				__strtol(optarg, NULL, 10);	
			break;
		case 'p':
			argument.port = 
				__strtol(optarg, NULL, 10);	
			break;
#endif
#ifdef CLIENT
		case 'r':
			argument.reportitv = 
				__strtol(optarg, NULL, 10);	
			break;
#endif
		case 'l':
			debug = __strtol(optarg, NULL, 10);	
			break;
		case 'm':
			argument.msgitv = 
				__strtol(optarg, NULL, 10);	
			break;
		case '?':
			break;
		case 'h':
			help();
			break;
		case '*':
			break;
		default:
			fprintf(stderr, "argument: %c error\n", short_arg);
			break;
		}
	}
}
