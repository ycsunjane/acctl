/*
 * =====================================================================================
 *
 *       Filename:  apstatus.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年09月01日 15时28分40秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jianxi sun (jianxi), ycsunjane@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __APSTATUS_H__
#define __APSTATUS_H__
#define SSID_MAXLEN 	(200)

struct ssid_t {
	char ssid[SSID_MAXLEN];
	int  power;
};

struct apstatus_t {
	int  ssidnum;
	struct ssid_t ssid0;
	struct ssid_t ssid1;
	struct ssid_t ssid2;
};

struct apstatus_t *get_apstatus();
#endif /* __APSTATUS_H__ */
