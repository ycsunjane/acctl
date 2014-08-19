/*
 * ============================================================================
 *
 *       Filename:  dllser.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年08月19日 15时53分51秒
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

#include "dllayer.h"
int debug = 1;
int main()
{
	dll_init("enp3s0");
	char *data = "wwirelesserwirelesserwirelesserwirelesserwirelesserwirelesserwirelesserwirelesserwirelesserwirelesserirelesser";
	int size = strlen(data);

	dll_brdcast(data, size);
	return 0;
}
