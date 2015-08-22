/*
 * c_config.c
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#include <pthread.h>
#include <time.h>
#include <stdlib.h>

#include "manager.h"
#include "util.h"

g_config cat_config={DEFAULT_DOMAIN,"10.2.25.213",2280};
default_message_tree tree;
g_context cat_context;

void set_domain(char* domain){
	str_copy(cat_config.domain,domain);
}

char* next_message_id() {
	char* buf = malloc(1024 * sizeof(char));
	memset(buf,0,1024);

	int timestamp = get_tv_usec()/3600000000;
	cat_context.msg_index++;

	strcpy(buf, cat_config.domain);
	strcat(buf, SPLIT);
	strncat(buf, cat_context.local_ip_hex,8);
	strcat(buf, SPLIT);
	strcat(buf, number_to_array(timestamp));
	strcat(buf, SPLIT);
	strcat(buf, number_to_array(cat_context.msg_index));
	return buf;
}

void add_message(struct cat_message* message){
	//TODO
	tree.message=message;
}

void setup(){
	init_ip();

	str_copy(tree.ThreadGroupName,THREAD_GROUP_NAME);
	gethostname(tree.HostName, 1024);
	str_copy(tree.Domain,cat_config.domain);
	tree.ThreadId = (unsigned int)pthread_self();
	str_copy(tree.ThreadName,THREAD_NAME);
	char* msg_id=next_message_id();

	str_copy(tree.MessageId,msg_id);
	str_copy(tree.ParentMessageId,STR_NULL);
	str_copy(tree.RootMessageId,STR_NULL);
	str_copy(tree.SessionToken,STR_NULL);
	str_copy(tree.IpAddress,cat_context.local_ip);
	cat_context.initialized=1;
}


