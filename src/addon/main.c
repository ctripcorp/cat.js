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
#include "manager.h"

int main(void) {
	srand(time(NULL));
	int r = rand() % 30000;
	char str[15];
	sprintf(str, "%d", r);
	cat_message *message = new_transaction(str, "root");
	trans_complete_with_status(message, "0");
	printf("finish");
#ifdef _WIN32
	system("pause");
#endif
	return EXIT_SUCCESS;
}
