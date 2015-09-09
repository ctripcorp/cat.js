/*
 * c_config.h
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#ifndef SRC_ADDON_DEPS_C_CONFIG_H_
#define SRC_ADDON_DEPS_C_CONFIG_H_

#include <math.h>
#include "m.h"

#ifdef __cplusplus
extern "C" {
#endif

void add_message(struct message* msg);
void next_message_id(char** buffer);

extern void set_domain(char* domain);
extern void set_server(const char* server[], int len);
extern void main_init();
extern void main_free();
extern void set_debug_level(int level);
extern void toggle_send(int flag);

#ifdef __cplusplus
}
#endif
#endif /* SRC_ADDON_DEPS_C_CONFIG_H_ */

