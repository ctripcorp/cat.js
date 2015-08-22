/*
 * util.h
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#ifndef SRC_ADDON_UTIL_H_
#define SRC_ADDON_UTIL_H_

int write_to_buffer_raw(char *buf, char * data, int length, int index);
int setint_to_buffer_begin(char* buffer, int i);
void getFormatTime(char** buf);
int get_thread_id();
void init_ip();
long get_tv_usec();
char * number_to_array(unsigned int number);

void str_copy(char* dest,char* source);
#endif /* SRC_ADDON_UTIL_H_ */
