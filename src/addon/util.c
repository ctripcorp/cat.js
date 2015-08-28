/*
 * util.c
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include <memory.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <math.h>

#include "manager.h"

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
	*buf = (char*) malloc(24 * sizeof(char));
	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, NULL);
	if ((tm = localtime(&tv.tv_sec)) != NULL) {
		strftime(fmt, sizeof fmt, "%Y-%m-%d %H:%M:%S.%%03d", tm);
		snprintf(*buf, 24, fmt, tv.tv_usec);
		//printf("tv.tv_usec:%d\n", tv.tv_usec);
	}
}

long get_tv_usec() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*1000000L+tv.tv_usec;
}

/* Subtract the ‘struct timeval’ values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0. */

int
timeval_subtract (result, x, y)
     struct timeval *result, *x, *y;
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

void str_copy(char* dest,char* source){
	strncpy(dest,source,strlen(source)+1);
}

void printIPAddress() {
	int n;
	struct ifreq ifr;
	char array[] = "eth0";

	n = socket(AF_INET, SOCK_DGRAM, 0);
	//Type of address to retrieve - IPv4 IP address
	ifr.ifr_addr.sa_family = AF_INET;
	//Copy the interface name in the ifreq structure
	strncpy(ifr.ifr_name, array, IFNAMSIZ - 1);
	ioctl(n, SIOCGIFADDR, &ifr);
	close(n);
	//display result
	printf("IP Address is %s - %s\n", array, inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr));
}

void init_ip() {
	extern g_context cat_context;
	extern g_config cat_config;
	int dns_port = 53;
	struct sockaddr_in serv;
	int sock = socket( AF_INET, SOCK_DGRAM, 0);

	//Socket could not be created
	if (sock < 0) {
		perror("Socket error");
	}

	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;

	serv.sin_addr.s_addr = inet_addr("8.8.8.8"/*cat_config.server*/);
	serv.sin_port = htons(dns_port);

	int err = connect(sock, (const struct sockaddr*) &serv, sizeof(serv));
	struct sockaddr_in name;
	socklen_t namelen = sizeof(name);
	err = getsockname(sock, (struct sockaddr*) &name, &namelen);
	//const char* p = inet_ntop(AF_INET, &name.sin_addr, cat_config.local_ip, 100);
	//printf("Local Address: %08x (%s)\n", name.sin_addr.s_addr, inet_ntoa(name.sin_addr));
	unsigned char *ip = (unsigned char *)&name.sin_addr.s_addr;
	sprintf(&cat_context.local_ip_hex[0], "%02x%02x%02x%02x", ip[0], ip[1], ip[2], ip[3]);
	cat_context.local_ip = inet_ntoa(name.sin_addr);
	sprintf(cat_context.local_ip, "%s", inet_ntoa(name.sin_addr));
	//if (p == NULL) {
	//printf("Error number : %d . Error message : %s \n", errno, strerror(errno));
	//}

	close(sock);
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

void c11_support() {
#if defined __STDC_LIB_EXIT1__
	printf("<C11 support>");
#else
	printf("<C11 not support!!!>");
#endif
}

int get_thread_id() {
	pthread_t self;
	self = pthread_self();
	return (int) self;
}

void int_to_array(unsigned int number,char** buf){
	//int n = log10(number)+1;
	char *numberArray = *buf;
	snprintf(numberArray, sizeof(numberArray), "%d", number);
}
void long_to_array(long number,char** buf){
	char *numberArray = *buf;
	snprintf(numberArray, sizeof(numberArray), "%ld", number);
}
