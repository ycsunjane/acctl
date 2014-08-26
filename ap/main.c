/*
 * ============================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月26日 09时05分59秒
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

#include "arg.h"
#include "message.h"
#include "process.h"
#include "msg.h"
#include "net.h"

void ui();

int main(int argc, char *argv[])
{
	proc_arg(argc, argv);
	/* create recv pthread */
	net_init();
	/* create message loop travel pthread */
	message_init();

	ui();
	return 0;
}

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
