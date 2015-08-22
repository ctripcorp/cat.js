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
#include "msg.h"
#include "manager.h"

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

void ts_fork(struct cat_message* message) {
	message->msg_transaction->count_fork++;
}

void ts_join(struct cat_message* message) {
	if (--message->msg_transaction->count_fork == 0) {
		ts_complete(message);
	}
}

void ts_complete(struct cat_message* message) {
	if (message->msg_transaction->ts_parent != NULL) {
		ts_join(message->msg_transaction->ts_parent);
	} else {
		//TODO
		//set status as success if not set
		send_tree(message);
	}
}

void send_tree(struct cat_message* message) {
	add_message(message);
	struct channel_buffer buf;
	encode(&tree, &buf);
	printf("Text:\n");
	for (int i = 0; i < buf.buffer_pointer; i++) {
		printf("%c", buf.buffer[i]);
	}

	printf("Raw Binary:\n");
	for (int i = 0; i < buf.buffer_pointer; i++) {
		char c=buf.buffer[i];
		if(c==9){
			printf(" TAB ");
		}
		else if(c==10){
			printf("\n");
		}
		else if((c>=65&&c<=90)||(c>=97&&c<=122)||(c>=48&&c<=57)||c=='.'||c=='-'||c==':'||c==32||i<4){
			printf("%d ", buf.buffer[i]);
		}else{
			printf("[%d] ", buf.buffer[i]);
			printf("(%c)", buf.buffer[i]);
		}

	}
	printf("\n");
	linux_send(buf.buffer, buf.writen_size);
}

cat_message* new_transaction(char* type, char* name) {
	return sub_transaction(type, name, NULL);
}

cat_message* sub_transaction(char* type, char* name, struct cat_message *parent) {
	if (!cat_context.initialized) {
		setup();
	}

	cat_transaction *ts_temp=(cat_transaction*)malloc(sizeof(cat_transaction));
	cat_message *message_temp=(cat_message*)malloc(sizeof(cat_message));
	message_temp->Type = type;
	message_temp->Name = name;
	message_temp->reportType=ReportType_Transaction;
	message_temp->Status="0";
	message_temp->Data="";

	ts_temp->ts_parent = parent;
	ts_temp->message_children_size = 0;
	ts_temp->Standalone = 1;
	ts_temp->_mDurationInMicro = -1;

	message_temp->msg_transaction = ts_temp;
	getFormatTime(&message_temp->format_time);
	message_temp->Timestamp=get_tv_usec();

	return message_temp;
}

struct cat_message* log_event(char* type, char* name) {
	cat_message* evt0=(cat_message*)malloc(sizeof(cat_message));
	evt0->Type=type;
	evt0->Name=name;
	evt0->reportType=ReportType_Event;
	evt0->Data="";
	evt0->Status="0";
	evt0->Timestamp=0;
	getFormatTime(&evt0->format_time);
	evt0->msg_transaction=NULL;
	return evt0;
}

cat_message* new_event(char* type, char* name, struct cat_message *parent){
	cat_message* evt0=(cat_message*)malloc(sizeof(cat_message));
	evt0->Type=type;
	evt0->Name=name;
	evt0->reportType=ReportType_Event;
	evt0->Data="";
	evt0->Status="0";
	evt0->Timestamp=0;
	getFormatTime(&evt0->format_time);
	evt0->msg_transaction=NULL;

	parent->msg_transaction->messageChildren[parent->msg_transaction->message_children_size] = *evt0;
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
			long current=get_tv_usec();
			//TODO int->long
			int duration=(int)(current-message->Timestamp);
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
		return encode_line(message, buf, 'E', Policy_DEFAULT);
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
			struct cat_message *child = &((transaction_temp->messageChildren)[i]);
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

