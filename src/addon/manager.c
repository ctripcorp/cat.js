/*
 * c_config.c
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "manager.h"
#include "util.h"

g_config cat_config={DEFAULT_DOMAIN,"10.2.25.213",2280};
default_message_tree tree;
g_context cat_context;

void set_domain(char* domain){
	//str_copy(cat_config.domain,domain);
}

void set_server(char* server){
	//str_copy(cat_config.server,server);
}

void inner_next_message_id(char** buffer) {
	char* buf = *buffer;
	long timestamp = get_tv_usec()/3600000000L;
	cat_context.msg_index++;

	strcpy(buf, cat_config.domain);
	strcat(buf, SPLIT);
	strncat(buf, cat_context.local_ip_hex,8);
	strcat(buf, SPLIT);

	int n1 = log10(timestamp)+1;
	char* number_buf_1 = mem(n1,sizeof(char));
	long_to_array(timestamp,&number_buf_1);
	strcat(buf, number_buf_1);
	f_mem(number_buf_1);

	strcat(buf, SPLIT);

    int n2 = log10(cat_context.msg_index)+1;
	char* number_buf_2 = mem(n2,sizeof(char));
	int_to_array(cat_context.msg_index,&number_buf_2);
	strcat(buf, number_buf_2);
	f_mem(number_buf_2);
}

void add_message(struct cat_message* message){
	next_message_id();
	tree.message=message;
}

void next_message_id(){
	char* temp_buf = mem(20*KB,sizeof(char));
	inner_next_message_id(&temp_buf);
	str_copy(tree.MessageId,temp_buf);
	f_mem(temp_buf);
}

void setup(){
	init_ip();

	str_copy(tree.ThreadGroupName,THREAD_GROUP_NAME);
	gethostname(tree.HostName, 1024);
	str_copy(tree.Domain,cat_config.domain);
	tree.ThreadId = (unsigned int)pthread_self();
	str_copy(tree.ThreadName,THREAD_NAME);

	next_message_id();

	str_copy(tree.ParentMessageId,STR_NULL);
	str_copy(tree.RootMessageId,STR_NULL);
	str_copy(tree.SessionToken,STR_NULL);
	str_copy(tree.IpAddress,cat_context.local_ip);
	cat_context.initialized=1;
}


