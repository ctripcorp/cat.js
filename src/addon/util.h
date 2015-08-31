/*
 * util.h
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#ifndef SRC_ADDON_UTIL_H_
#define SRC_ADDON_UTIL_H_

#define KB 1024

int write_to_buffer_raw(char *buf, char * data, int length, int index);
int setint_to_buffer_begin(char* buffer, int i);
void getFormatTime(char** buf);
int get_thread_id();
void init_ip();
long get_tv_usec();
void str_copy(char* dest, char* source);
void* mem(int count, int eltsize);
void f_mem(void* p);
#endif /* SRC_ADDON_UTIL_H_ */
