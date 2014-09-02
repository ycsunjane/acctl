/*
 * ============================================================================
 *
 *       Filename:  apstatus.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年09月01日 15时27分34秒
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

#include "apstatus.h"

struct apstatus_t ap = {0};

struct apstatus_t *get_apstatus()
{
	ap.ssidnum = 1;
	strcpy(&ap.ssid0.ssid[0], "jianxi-pri");

	return &ap;
}
