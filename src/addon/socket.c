/*
 * csocket.c
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#include "socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

void linux_send(char* buf, int sendsize) {
	/* debug without network
	 return;*/

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
	}else{
		printf("[info]send success\n");
	}
	//}

	close(sock_cli);
}
