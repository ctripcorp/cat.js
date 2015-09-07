/*
 * csocket.c
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#include "socket.h"

const int cat_port = 2280;

void socket_send(char* buf, int sendsize) {
	return;
#ifdef _WIN32
	win_send(buf,sendsize);
#else
	linux_send(buf, sendsize);
#endif
}

#ifdef _WIN32

SOCKET win_client() {
	int i;
	for (i = 0; i < context->serv->len; i++) {
		WORD sockVersion = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(sockVersion, &data) != 0)
		{
			return NULL;
		}

		SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sclient == INVALID_SOCKET)
		{
			LOG(LOG_ERR, "invalid socket !");
			return NULL;
		}

		struct sockaddr_in serAddr;
		serAddr.sin_family = AF_INET;
		serAddr.sin_port = htons(cat_config.port);
		serAddr.sin_addr.S_un.S_addr = inet_addr(context->serv->server[i]);
		int err = connect(sclient, (struct sockaddr *)&serAddr, sizeof(serAddr));
		if(err == SOCKET_ERROR)
		{
			LOG(LOG_ERR,"connect error, error code:%d", err);
		} else {
			break;
		}
	}
	return sclient;
}

void win_send(char* buf, int sendsize) {

	SOCKET sclient = win_client();

	send(sclient, buf, sendsize, 0);

	closesocket(sclient);
	WSACleanup();
}

void init_ip(struct g_context *context) {

	SOCKET sclient = win_client();

	struct sockaddr_in name;
	int namelen = sizeof(name);
	int err = getsockname(sclient, (struct sockaddr*) &name, &namelen);
	if (err) {
		LOG(LOG_ERR,"connect error, error code:%d", err);
	}

	unsigned char *ip = (unsigned char *)&name.sin_addr.s_addr;
	sprintf(&context->local_ip_hex[0], "%02x%02x%02x%02x", ip[0], ip[1], ip[2], ip[3]);
	copy_string(context->local_ip, inet_ntoa(name.sin_addr), CHAR_BUFFER_SIZE);
	closesocket(sclient);
	WSACleanup();
}
#else
void linux_send(char* buf, int sendsize) {
	int sock_cli;
	int i, j = 0;
	for (i = 0; i < context->serv->len; i++) {
		sock_cli = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in servaddr;
		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(cat_port);

		servaddr.sin_addr.s_addr = inet_addr(context->serv->server[i]);

		int err = connect(sock_cli, (const struct sockaddr*) &servaddr, sizeof(servaddr));
		if (err) {
			LOG(LOG_ERR, "connect %s error, error code:%d", context->serv->server[i], err);
		} else {
			j = 1;
			break;
		}
	}
	if (!j)	/* fail to connect server */
		return;

	int r = send(sock_cli, buf, sendsize, 0);
	if (r == -1) {
		LOG(LOG_FATAL, "fail to send");
	} else {
		LOG(LOG_INFO, "send success\n");
	}

	close(sock_cli);
}

/*
 * Purpose: initialization IP field in context struct
 * Argument  					IO
 * --------- 					--
 * context   					I
 * context->local_ip_hex		O
 * context->local_ip			O
 */
void init_ip(struct g_context *context) {
	struct sockaddr_in serv;
	int err = -1;

	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (sock < 0) {
		LOG(LOG_ERR, "socket error");
		return;
	}

	LOG(LOG_INFO, "socket descriptor: %d", sock);

	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = inet_addr("8.8.8.8");
	serv.sin_port = htons(53);

	socklen_t sock_len = sizeof(serv);
	err = connect(sock, (const struct sockaddr*) &serv, sock_len);
	if (err) {
		LOG(LOG_ERR, "connect error, error code:%d", err);
	}

	err = getsockname(sock, (struct sockaddr*) &serv, &sock_len);
	if (err) {
		LOG(LOG_ERR, "connect error, error code:%d", err);
	}

	char* ptr_local_ip = inet_ntoa(serv.sin_addr);

	LOG(LOG_INFO, "Local Address: %08x (%s)", serv.sin_addr.s_addr, ptr_local_ip);

	unsigned char *ip = (unsigned char *) &serv.sin_addr.s_addr;
	sprintf(&context->local_ip_hex[0], "%02x%02x%02x%02x", ip[0], ip[1], ip[2], ip[3]);
	copy_string(context->local_ip, ptr_local_ip, CHAR_BUFFER_SIZE);

	close(sock);
}

#endif

