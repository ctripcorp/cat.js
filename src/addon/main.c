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
void test_large_trans();
void test_scenario_trans_error();

#ifdef _WIN32
void test_win_format_time();
#endif

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

void test_scenario_trans_error(){

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

	complete_message_with_status(sub_trans,"error");

	complete_message_with_status(root,"root error");

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

void test_large_trans(){
	set_debug_level(3);

	main_init();

	/* uat: "10.2.25.213" */
	const char* server[2] = {"10.2.25.213","22.22.22.22"};
	set_server(server,2);

	set_domain("345");

	message *root = new_transaction("Root", "Trans");

	for(int i=0;i<100;i++){

		message *sub_trans = sub_transaction("Addon", "SubTrans", root);

		message *evt = sub_event("Addon", "Event", "0", sub_trans);
		add_data(evt, "author=ctrip&aa=bb&cc=dd");
		add_data(evt, "sex=boy");

		for(int i=0;i<100;i++){

			message *sub_trans_inner = sub_transaction("Addon", "SubTrans", sub_trans);

			message *evt_inner = sub_event("Addon", "Event", "0", sub_trans_inner);
			add_data(evt_inner, "author=ctrip&aa=bb&cc=dd");
			add_data(evt_inner, "sex=boy");

			complete_message_with_status(sub_trans_inner,"0");
		}

		complete_message_with_status(sub_trans,"0");

		LOG(LOG_WARN,"MEMORY:%lu MB",mem_used/1024/1024/8);
		LOG(LOG_WARN,"--I:%d",i);
	}

	complete_message_with_status(root,"0");

	main_free();
}

/******************************** Function Test ********************************/

/* Win format time */
#ifdef _WIN32
void test_win_format_time(){
	char p[200];
	char* x = p;

	win_get_format_time(&x);

	printf("%s", p);
}
#endif

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





