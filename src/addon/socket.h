/*
 * csocket.h
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#ifndef SRC_ADDON_SOCKET_H_
#define SRC_ADDON_SOCKET_H_

#include "m.h"

extern const int cat_port;

void init_ip(struct g_context *context);
void win_send(char* buf, int sendsize);
void linux_send(char* buf, int sendsize);
void socket_send(char* buf, int sendsize);

#ifdef _WIN32
SOCKET win_client();
#endif

#endif /* SRC_ADDON_SOCKET_H_ */
