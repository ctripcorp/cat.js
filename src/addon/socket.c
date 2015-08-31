/*
 * csocket.c
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#include "socket.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <stdlib.h>
#include <winsock2.h>
#pragma  comment(lib,"ws2_32.lib")
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "socket.h"
#include <stdlib.h>
#include <fcntl.h>
#endif

void socket_send(char* buf, int sendsize){
#ifdef _WIN32
	win_send(buf,sendsize);
#else
	linux_send(buf,sendsize);
#endif
}

#ifdef _WIN32
void win_send(char* buf, int sendsize){

	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return;
	}

	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		printf("invalid socket !");
		return;
	}

	struct sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(cat_config.port);
	serAddr.sin_addr.S_un.S_addr = inet_addr(cat_config.server);
	if (connect(sclient, (struct sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("connect error !");
		closesocket(sclient);
		return;
	}

	send(sclient, buf, sendsize, 0);

	closesocket(sclient);
	WSACleanup();
}

void init_ip() {

	int dns_port = 53;
	struct sockaddr_in serv;
	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	//Socket could not be created
	if (sock < 0) {
		perror("Socket error");
	}

	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;

	serv.sin_addr.s_addr = inet_addr(/*"8.8.8.8"*/cat_config.server);
	serv.sin_port = htons(dns_port);

	int err = connect(sock, (const struct sockaddr*) &serv, sizeof(serv));
	struct sockaddr_in name;
	int namelen = sizeof(name);
	err = getsockname(sock, (struct sockaddr*) &name, &namelen);
	if (err){
		printf("Error code:%d", err);
	}
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
#else
void linux_send(char* buf, int sendsize) {
	/* debug without network*/
	//return;

	int sock_cli = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(cat_config.port);
	//10.2.6.98,10.32.21.144
	servaddr.sin_addr.s_addr = inet_addr(cat_config.server);

	///连接服务器，成功返回0，错误返回-1
	if (connect(sock_cli, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		perror("connect");
		exit(1);
	}
	//while (fgets(buf, sendsize, stdin) != NULL)
	//{
	int i = send(sock_cli, buf, sendsize, 0); ///发送
	if (i == -1) {
		printf("[info]fail to send\n");
	}
	else{
		printf("[info]send success\n");
	}
	//}

	close(sock_cli);
}

void init_ip() {
	extern g_context cat_context;
	extern g_config cat_config;
	int dns_port = 53;
	struct sockaddr_in serv;
	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	//Socket could not be created
	if (sock < 0) {
		perror("Socket error");
	}

	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;

	serv.sin_addr.s_addr = inet_addr(/*"8.8.8.8"*/cat_config.server);
	serv.sin_port = htons(dns_port);

	int err = connect(sock, (const struct sockaddr*) &serv, sizeof(serv));
	struct sockaddr_in name;
	socklen_t namelen = sizeof(name);
	err = getsockname(sock, (struct sockaddr*) &name, &namelen);
	if (err){
		printf("Error code:%d", err);
	}
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
#endif



