/*
 * ============================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月18日 17时30分36秒
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
#include <pthread.h>
#include <signal.h>

#include "arg.h"
#include "net.h"
#include "msg.h"
#include "aphash.h"
#include "process.h"
#include "netlayer.h"

void ui()
{
	printf("1 system status\n");

	int num;
	while(1) {
		printf("Input num: ");
		scanf("%d", &num);

		switch(num) {
		case 1: {
			}
		}
	}
}

int main(int argc, char *argv[])
{
	proc_arg(argc, argv);

	msg_init();
	hash_init();
	net_init();

	ui();
	return 0;
}
