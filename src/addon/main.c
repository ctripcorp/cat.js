/*
 * main.c
 *
 *  Created on: 2015年8月11日
 *      Author: stur
 */
#include <stdio.h>
#include <stdlib.h>
#include "m.h"
#include "msg.h"
#include "manager.h"

void test_LOG();
void test_new_transaction();
void test_main_init();
void test_mark();
void test_new_event();
void test_scenario_trans();
void test_scenario_multi_message();

int main(void) {

	/* Replace this function to test specified function */
	test_scenario_trans();

#ifdef _WIN32
	system("pause");
#endif

	return EXIT_SUCCESS;
}

/******************************** Scenario Test ********************************/

void test_scenario_trans(){

	set_debug_level(4);

	main_init();

	/* uat: "10.2.25.213" */
	const char* server[2] = {"10.2.25.213","22.22.22.22"};
	set_server(server,2);

	set_domain("winnode");

	message *root = new_transaction("Addon", "Trans");

	c_sleep(1); // do some business

	message *sub_trans = sub_transaction("Addon", "SubTrans", root);

	c_sleep(1); // do some business

	message *evt = sub_event("Addon", "Event", "0", sub_trans);
	add_data(evt, "author=ctrip");
	add_data(evt, "sex=boy");

	complete_message_with_status(sub_trans,"0");

	complete_message_with_status(root,"0");

	main_free();
}

void test_scenario_multi_message(){

	set_debug_level(4);

	main_init();

	/* uat: "10.2.25.213" */
	const char* server[2] = {"10.2.25.213","22.22.22.22"};
	set_server(server,2);

	set_domain("cat-nodejs");

	message *root = new_transaction("Addon1", "Trans");

	c_sleep(1); // do some business

	message *standalone = new_transaction("Addon2", "Trans");

	c_sleep(1); // do some business

	complete_message_with_status(standalone,"0");

	complete_message_with_status(root,"0");

	main_free();
}

/******************************** Function Test ********************************/

/* LOG */
void test_LOG() {
	char name[] = "stur";
	LOG(LOG_WARN, "hello %s", name);	/* This should print */

	debug_level = 1;
	LOG(LOG_WARN, "warn %s", name);	/* This should not print */
	LOG(LOG_FATAL, "fatal %s", name);	/* This should print */
}

/* New Transaction */
void test_new_transaction(){
	main_init();

	message *msg = new_transaction("Addon","Trans");

	c_sleep(1); // do some business

	complete_message_with_status(msg,"0");

	main_free();
}

/* New Event */
void test_new_event(){
	main_init();

	log_event("Addon","Event","0","author=ctrip");

	main_free();
}

/* Init Context */
void test_main_init(){
	main_init();
	main_free();
}

/* Mark */
void test_mark(){
	int i = 123;
	char data[30];
	sprintf(data, "%d", i);
	mark(data);
	int c = read_mark();
	LOG(LOG_INFO,"read int:%d",c);
}





