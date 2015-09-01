/*
 * util.h
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#ifndef SRC_ADDON_UTIL_H_
#define SRC_ADDON_UTIL_H_

#define KB 1024

#ifdef _WIN32
#include "win32.h"
#endif

int write_to_buffer_raw(char *buf, char * data, int length, int index);
int setint_to_buffer_begin(char* buffer, int i);
void getFormatTime(char** buf);
long get_tv_usec();
void str_copy(char* dest, char* source);
void* mem(int count, int eltsize);
void f_mem(void* p);
int c_get_threadid();
void c_exit_thread();

#endif /* SRC_ADDON_UTIL_H_ */
