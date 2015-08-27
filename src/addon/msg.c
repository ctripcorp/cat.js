/*
 * msg.c
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "msg.h"
#include "manager.h"

cat_message *root;

int write_to_buffer(struct channel_buffer *buf, char * data) {

	if (data == NULL) {
		data = "null";
	}
	int length = strlen(data);
	write_to_buffer_raw(buf->buffer, data, length, buf->buffer_pointer);
	buf->buffer_pointer += length;
	return length;
}

int write_char_to_buffer(struct channel_buffer *buf, char data) {
	buf->buffer[buf->buffer_pointer] = data;
	buf->buffer_pointer++;
	return 1;
}

void trans_complete_with_status(struct cat_message* message, char* status) {
	message->Status = status;

	/* returned value used for clear timeout which is set in node */
	trans_complete(message);
}

void trans_complete(struct cat_message* message) {

	if(message->msg_transaction != NULL){
		/* transaction has explicit call complete */
		/* if all sub trans has complete this trans will auto complete */
		message->msg_transaction->docomplete = 1;
	}else{
		 /* message is not a transaction */
		return;
	}

	if(message->complete == 1) return; // message already complete

	if (message->msg_transaction->ts_parent == NULL/* this is a root message */) {
		if(message->msg_transaction->count_fork == 0/* message is transaction and its child has complete */){
			set_complete(message);
			send_tree(message);
		}
	} else if(--message->msg_transaction->ts_parent->msg_transaction->count_fork == 0){
		/* message is last complete child, auto complete its parent */
		set_complete(message);
		if(message->msg_transaction->ts_parent->msg_transaction->docomplete == 1){
			trans_complete(message->msg_transaction->ts_parent);
		}
	}
}

void timeout(struct cat_message* message) {
	if(message->complete == 1) return; // message has complete

	if (message->msg_transaction->ts_parent == NULL) {
		 /* this is a root message */
		set_complete(message);
		send_tree(message);
	} else if(--message->msg_transaction->ts_parent->msg_transaction->count_fork == 0){
		/* message is last complete child, auto complete its parent */
		set_complete(message);
		trans_complete(message->msg_transaction->ts_parent);
	}
}

void set_complete(struct cat_message* message){
	message->complete = 1;
	printf("--Transaction[%p] has complete\n",message);

	if(message->msg_transaction->t_start == 1){
	    pthread_t id = pthread_self();

	    if(pthread_equal(id,message->msg_transaction->tid))
	    {
	        /* complete running in timeout thread */
	        int ret1  = 100;
	        pthread_exit(&ret1);
	    }else{
	    	cancel_timeout(message);
	    }

	    message->msg_transaction->t_start = 0;
	}
}

void send_tree(struct cat_message* message) {
	pthread_t tid;
	int err;
    err = pthread_create(&tid, NULL, &do_send, message);
    if (err != 0)
        printf("\ncan't create thread :[%s]", strerror(err));
}

void* do_send(void *arg){
	struct cat_message* message = arg;
	add_message(message);
	struct channel_buffer buf;
	encode(&tree, &buf);
	printf("[info]Raw Text:\n");
	for (int i = 0; i < buf.buffer_pointer; i++) {
		printf("%c", buf.buffer[i]);
	}

#if 0
	printf("Raw Binary:\n");
	for (int i = 0; i < buf.buffer_pointer; i++) {
		char c=buf.buffer[i];
		if(c==9) {
			printf(" TAB ");
		}
		else if(c==10) {
			printf("\n");
		}
		else if((c>=65&&c<=90)||(c>=97&&c<=122)||(c>=48&&c<=57)||c=='.'||c=='-'||c==':'||c==32||i<4) {
			printf("%d ", buf.buffer[i]);
		} else {
			printf("[%d] ", buf.buffer[i]);
			printf("(%c)", buf.buffer[i]);
		}

	}
	printf("\n");
#endif

	linux_send(buf.buffer, buf.writen_size);
	return NULL;
}

cat_message* new_transaction(char* type, char* name) {
	root = sub_transaction(type, name, NULL);
	return root;
}

void cancel_timeout(struct cat_message* message){
	pthread_cancel(message->msg_transaction->tid);
	message->msg_transaction->t_start = 0;
}

void settimeout(struct cat_message* message, int sec){
	message->msg_transaction->timeout = sec;
	if(message->msg_transaction->t_start == 1)
		cancel_timeout(message);
	int err;
    err = pthread_create(&message->msg_transaction->tid, NULL, &do_timeout, message);
    if (err != 0)
        printf("\ncan't create thread :[%s]", strerror(err));
    else
    	message->msg_transaction->t_start = 1;
}

void* do_timeout(void *arg)
{
	struct cat_message* message = arg;
	sleep(message->msg_transaction->timeout);
	timeout(message);
    return NULL;
}

cat_message* sub_transaction(char* type, char* name, struct cat_message *parent) {
	if (!cat_context.initialized) {
		setup();
	}

	cat_transaction *trans_temp = (cat_transaction*) malloc(sizeof(cat_transaction));
	cat_message *message_temp = (cat_message*) malloc(sizeof(cat_message));
	message_temp->Type = type;
	message_temp->Name = name;
	message_temp->reportType = ReportType_Transaction;
	message_temp->Status = "0";
	message_temp->Data = "";
	message_temp->complete = 0;
	trans_temp->Standalone = 0;
	getFormatTime(&message_temp->format_time);
	message_temp->Timestamp = get_tv_usec();

	trans_temp->message_children_size = 0;
	trans_temp->_mDurationInMicro = -1;
	trans_temp->count_fork = 0;
	trans_temp->docomplete = 0;
	trans_temp->ts_parent = parent;

	message_temp->msg_transaction = trans_temp;

	if (parent != NULL) {
		parent->msg_transaction->count_fork++;
		parent->msg_transaction->messageChildren[parent->msg_transaction->message_children_size] = message_temp;
		parent->msg_transaction->message_children_size++;
	}

	printf("--Transaction[%p] created\n",message_temp);
	return message_temp;
}

void log_event(char* type, char* name, char* status, char* data) {
	if (!cat_context.initialized) {
		setup();
	}

	cat_message* evt0 = new_event(type,name);
	evt0->Status=status;
	evt0->Data =data;
	send_tree(evt0);
}

cat_message* new_event(char* type, char* name) {
	cat_message* evt0 = (cat_message*) malloc(sizeof(cat_message));
	char* buf = malloc(1024 * 1024 * sizeof(char));
	memset(buf,0,1024 * 1024);
	strcpy(buf, "");
	evt0->Type = type;
	evt0->Name = name;
	evt0->reportType = ReportType_Event;
	evt0->Data = buf;
	evt0->Status = "0";
	evt0->Timestamp = 0;
	getFormatTime(&evt0->format_time);
	evt0->msg_transaction = NULL;

	return evt0;
}

void free_event(struct cat_message *event){
	free(event->Data);
	free(event);
}

cat_message* add_data(struct cat_message *event, char* data){

	if(strlen(event->Data)>0)
		strcat(event->Data, "&");
	strcat(event->Data, data);

	return event;
}

cat_message* sub_event(char* type, char* name, struct cat_message *parent) {
	cat_message* evt0 = new_event(type,name);

	parent->msg_transaction->messageChildren[parent->msg_transaction->message_children_size] = evt0;
	parent->msg_transaction->message_children_size++;

	return evt0;
}

int encode_header(struct default_message_tree* tree, struct channel_buffer *buf) {

	int count = 0;
	count += write_to_buffer(buf, ID);
	count += write_to_buffer(buf, TAB);
	count += write_to_buffer(buf, tree->Domain);
	count += write_to_buffer(buf, TAB);
	count += write_to_buffer(buf, tree->HostName);
	count += write_to_buffer(buf, TAB);
	count += write_to_buffer(buf, tree->IpAddress);
	count += write_to_buffer(buf, TAB);
	count += write_to_buffer(buf, tree->ThreadGroupName);
	count += write_to_buffer(buf, TAB);
	count += write_to_buffer(buf, number_to_array(tree->ThreadId));
	count += write_to_buffer(buf, TAB);
	count += write_to_buffer(buf, tree->ThreadName);
	count += write_to_buffer(buf, TAB);
	count += write_to_buffer(buf, tree->MessageId);
	count += write_to_buffer(buf, TAB);
	count += write_to_buffer(buf, tree->ParentMessageId);
	count += write_to_buffer(buf, TAB);
	count += write_to_buffer(buf, tree->RootMessageId);
	count += write_to_buffer(buf, TAB);
	count += write_to_buffer(buf, tree->SessionToken);
	count += write_to_buffer(buf, LF);

	return count;
}

int encode_line(struct cat_message* message, struct channel_buffer *buf, char type, enum policy policy) {
	int count = 0;

	count += write_char_to_buffer(buf, type);

	char* time;
	getFormatTime(&time);

	if (type == 'T' && message->reportType == ReportType_Transaction) {
		//long duration = message->msg_transaction->DurationInMillis;

		count += write_to_buffer(buf, time);
	} else {
		count += write_to_buffer(buf, message->format_time);
	}

	count += write_to_buffer(buf, TAB);
	count += write_to_buffer(buf, message->Type);
	count += write_to_buffer(buf, TAB);
	count += write_to_buffer(buf, message->Name);
	count += write_to_buffer(buf, TAB);

	if (policy != Policy_WITHOUT_STATUS) {
		count += write_to_buffer(buf, message->Status);
		count += write_to_buffer(buf, TAB);

		char* data = message->Data;

		if (policy == Policy_WITH_DURATION && message->reportType == ReportType_Transaction) {
			long current = get_tv_usec();
			//TODO int->long
			int duration = (int) (current - message->Timestamp);
			count += write_to_buffer(buf, number_to_array(duration));

			count += write_to_buffer(buf, "us");
			count += write_to_buffer(buf, TAB);
		}

		count += write_to_buffer(buf, data);
		count += write_to_buffer(buf, TAB);
	}

	count += write_to_buffer(buf, LF);

	return count;
}

int encode_message(struct cat_message* message, struct channel_buffer *buf) {

	if (message->reportType == ReportType_Event) {
		int r = encode_line(message, buf, 'E', Policy_DEFAULT);
		free_event(message);
		return r;
	}
	if (message->reportType == ReportType_Transaction) {
		cat_transaction *transaction_temp = message->msg_transaction;
		int len = transaction_temp->message_children_size;

		if (len == 0) {
			return encode_line(message, buf, 'A', Policy_WITH_DURATION);
		}
		int count = 0;
		count += encode_line(message, buf, 't', Policy_WITHOUT_STATUS);

		for (int i = 0; i < len; i++) {
			struct cat_message* child = transaction_temp->messageChildren[i];
			count += encode_message(child, buf);
		}

		count += encode_line(message, buf, 'T', Policy_WITH_DURATION);
		//free(transaction_temp->messageChildren);
		return count;
	}
	if (message->reportType == ReportType_Heartbeat) {
		return encode_line(message, buf, 'H', Policy_DEFAULT);
	}
	if (message->reportType == ReportType_Metric) {
		return encode_line(message, buf, 'M', Policy_DEFAULT);
	}

	return EXCEPTION_CODE;
}

void encode(struct default_message_tree* tree, struct channel_buffer *buf) {

	int count = 0;
	buf->buffer_pointer = 4; // place-holder
	count += encode_header(tree, buf);
	if (tree->message != NULL) {
		count += encode_message(tree->message, buf);
	}
	setint_to_buffer_begin(buf->buffer, count);
	buf->writen_size = count + 4;
	return;
}

