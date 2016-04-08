/*
 * c_config.c
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#include "manager.h"

void main_init() {
	context = setup_context();
}

void main_free() {
	char data[30];
	sprintf(data, "%d", context->msg_index);
	mark(data);
	free_context(context);
}

void set_domain(char* domain) {
	copy_nstr(context->domain, domain);
}

void identify_prod(char* ip)
{
	int i, len;
	char* servers[] = { "10.168.", "192.168.", "172.22.", "10.11.", "10.99.", "10.15.", "10.9." };

	len = sizeof(servers) / sizeof(servers[0]);
	for (i = 0; i < len;i++){
		if (startsWith(ip, servers[i])){
			context->env_type = "pro";
			return;
		}
	}

	context->env_type = "fws";
}

char* config_server_url()
{
	if (strcmp(context->env_type, "lpt") == 0)
	{
		return "http://ws.config.framework.lpt.qa.nt.ctripcorp.com/configws/serviceconfig/configinfoes/get/999999/1";
	}
	if (strcmp(context->env_type, "fws") == 0)
	{
		return "http://ws.config.framework.fws.qa.nt.ctripcorp.com/configws/serviceconfig/configinfoes/get/999999/1";
	}
	if (strcmp(context->env_type, "uat") == 0)
	{
		return "http://ws.config.framework.uat.qa.nt.ctripcorp.com/configws/serviceconfig/configinfoes/get/999999/1";
	}
	else if (strcmp(context->env_type, "pro") == 0)
	{
		return "http://ws.config.framework.ctripcorp.com/configws/serviceconfig/configinfoes/get/999999/1";
	}
	else
	{
		return "http://ws.config.framework.fws.qa.nt.ctripcorp.com/configws/serviceconfig/configinfoes/get/999999/1";
	}
}

char* get_config_server_url(){
	char* config_url;

	identify_prod(context->env_type);

	config_url = config_server_url();

	return config_url;
}

void set_server(const char* serv[], int len) {

	int i;

	if (len > 4)
		len = 4;

	for (i = 0; i < len; i++) {
		strncpy(context->serv->address[i], serv[i], strlen(serv[i]));
	}

	context->serv->len = len;
}

void set_debug_level(int level) {
	debug_level = level;
}

void toggle_send(int flag){
	context->send_on = flag;
}

void next_message_id(char** buffer) {
	char* buf = *buffer;
	char foo[30];
	c_long x, timestamp;

	foo[0] = '\0';
	x = get_tv_usec();

#ifdef _WIN32
	timestamp = x / 3600000000LL;
#else
	timestamp = x / 3600000000L;
#endif

	context->msg_index++;

	strcpy(buf, context->domain);
	strcat(buf, SPLIT);
	strncat(buf, context->local_ip_hex, 8);
	strcat(buf, SPLIT);

#ifdef _WIN32
	sprintf(foo, "%lld", timestamp);
#else
	sprintf(foo, "%ld", timestamp);
#endif

	strcat(buf, foo);
	strcat(buf, SPLIT);

	sprintf(foo, "%d", context->msg_index);
	strcat(buf, foo);
}

void add_message(struct message* msg) {
	char* ptr = small_buf;
	next_message_id(&ptr);
	copy_string(context->msg_id, small_buf, CHAR_BUFFER_SIZE);
	context->msg = msg;
}

