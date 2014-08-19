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

#include "arg.h"

#define has_arg (1)
struct option long_arg[] = {
	{"nic", has_arg, 0, 'n'},
	{"daemon", 0, 0, 'd'},
	{"debug", 0, 0, '*'},
	{"help", 0, 0, '*'},
	{0, 0, 0 , 0},
};
#define SHORT_STR 	"n:d"

static char *help_array[] = {
	"USAGE: ac [options]",
	"acctl server",
	"  -n, --nic 		nic which controller [required]",
	"  -d, --daemon 	daemon mode",
	"  --debug 	enable debug, will auto disable daemon_mode",
	"  --help 	help info",
	NULL,
};

static void help()
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
		ret = strcmp(argv[i], "-d");
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
	if(argc < 2)
		help();
		
	memset(&argument, 0, sizeof(argument));
	__early_is_debug(argc, argv);
}

static void __check_arg()
{
}


void proc_arg(int argc, char *argv[])
{
	int short_arg;
	__early_init(argc, argv);

	while((short_arg = getopt_long(argc, argv, SHORT_STR, long_arg, NULL)) 
		!= -1) {
		switch(short_arg) {
		case 'n':
			strncpy(&argument.nic[0], optarg,IFNAMSIZ-1);
			break;
		case 'd':
			if(!debug)
				daemon_mode = 1;
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

	__check_arg();
}
