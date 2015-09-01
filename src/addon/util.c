/*
 * util.c
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#ifdef _WIN32
#include "win32.h"
#else
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#endif

#include "manager.h"
#include "util.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <sys/types.h>
#include <errno.h>
#include <math.h>

int write_to_buffer_raw(char *buf, char * data, int length, int index) {
	strncpy(&buf[index], data, length);
	return length;
}

int setint_to_buffer_begin(char* buffer, int i) {
	//memcpy(buffer, &i, 4);

	buffer[0] = (i >> 24) & 0xFF;
	buffer[1] = (i >> 16) & 0xFF;
	buffer[2] = (i >> 8) & 0xFF;
	buffer[3] = i & 0xFF;

	return 4;
}

void getFormatTime(char** buf) {
	char fmt[64];
	*buf = (char*)malloc(24 * sizeof(char));

#ifdef _WIN32
	struct timeval2 tv;
#else
	struct timeval tv;
#endif
	struct tm *tm;

	gettimeofday(&tv, NULL);
	if ((tm = localtime(&tv.tv_sec)) != NULL) {
		strftime(fmt, sizeof fmt, "%Y-%m-%d %H:%M:%S.%%03d", tm);
		snprintf(*buf, 24, fmt, tv.tv_usec);
	}
}

long get_tv_usec() {
#ifdef _WIN32
	struct timeval2 tv;
	struct timezone2 tz;
	gettimeofday(&tv, &tz);
	return tv.tv_sec*1000000L+tv.tv_usec;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*1000000L+tv.tv_usec;
#endif
}

void str_copy(char* dest,char* source){
	strncpy(dest,source,strlen(source)+1);
}

void* mem(int count, int eltsize){
	void* p = calloc(count,eltsize);
	if(p == NULL){
		printf("Not enough memory");
	}
	return p;
}
void f_mem(void* p){
	free(p);
	p=NULL;
}

/* not used yet */
void c11_support() {
#if defined __STDC_LIB_EXIT1__
	printf("<C11 support>");
#else
	printf("<C11 not support!!!>");
#endif
}

int c_get_threadid(){
#ifdef _WIN32
	return GetCurrentThreadId();
#else
	return (int)pthread_self();
#endif
}

void c_exit_thread(){
	int ret1 = 100;
#ifdef _WIN32
	ExitThread(ret1);
	return 1;
#else
	pthread_exit(&ret1);
#endif
}


