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

int main(void) {
	success_sample();
	return EXIT_SUCCESS;
}

void success_sample(){
	srand(time(NULL));
	int r = rand()%30000;
	char str[15];
	sprintf(str, "%d", r);
	cat_message *message=new_transaction(str,"name");
	sleep(2);
	new_event("event1","name1",message);
	ts_complete(message);
}
#if 0
void simple_test(){
	cat_message message=new_transaction("type1","name");
	cat_transaction *p=message.msg_transaction;
	cat_message evt0=new_event("event1","name1");
	p->messageChildren[p->message_children_size] = evt0;
	p->message_children_size++;
	ts_complete(message.msg_transaction);
}

#endif
