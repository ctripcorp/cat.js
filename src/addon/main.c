/*
 * main.c
 *
 *  Created on: 2015年8月11日
 *      Author: stur
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msg.h"
#include "util.h"
#include <math.h>
#include <pthread.h>
#include "manager.h"
void success_sample();
void test_log_event();
void perf();

int main(void) {
	perf();
	printf("finish");
	return EXIT_SUCCESS;
}

void test_log_event(){
 log_event("Error", "Exception1", "ERROR2", "Exception2");
}

void perf(){
	srand(time(NULL));
	int r = rand()%30000;
	char str[15];
	sprintf(str, "%d", r);
	cat_message *message = new_transaction(str,"root");
	cat_message *message1;
	for(int i=0;i<3;i++){
		cat_message *subTx=sub_transaction(str, "sub_trans", message);
		trans_complete_with_status(subTx,"0");
		sub_event("evt0","sth","0",subTx);

		r++;
		sprintf(str, "%d", r);

	}

	message1 = new_transaction(str,"standalone");
	trans_complete_with_status(message1,"0");

	trans_complete_with_status(message,"0");
}

void success_sample(){
	srand(time(NULL));
	int r = rand()%30000;
	char str[15];
	sprintf(str, "%d", r);
	cat_message *message=new_transaction(str,"name");
	//cat_message *subTx=sub_transaction(str, "sub_trans", message);
	//cat_message *event = sub_event("event1","name1",message);
	//add_data(event, "coco=girl&exception=null");
	//sleep(2);
	//trans_complete_with_status(subTx,"0");
	trans_complete_with_status(message,"0");
}
#if 0
void simple_test(){
	cat_message message=new_transaction("type1","name");
	cat_transaction *p=message.msg_transaction;
	cat_message evt0=new_event("event1","name1");
	p->messageChildren[p->message_children_size] = &evt0;
	p->message_children_size++;
	trans_complete(message.msg_transaction,"0");
}

#endif
