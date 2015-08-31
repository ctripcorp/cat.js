/*
 * csocket.h
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#ifndef SRC_ADDON_SOCKET_H_
#define SRC_ADDON_SOCKET_H_
#include "manager.h"
extern g_config cat_config;
extern g_context cat_context;
void init_ip();

void win_send(char* buf, int sendsize);
void linux_send(char* buf,int sendsize);

void socket_send(char* buf, int sendsize);

#endif /* SRC_ADDON_SOCKET_H_ */
