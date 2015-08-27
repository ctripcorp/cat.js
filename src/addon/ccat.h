#ifndef CCAT_H
#define CCAT_H
 
extern "C"
{
	#include "msg.h"
	#include "manager.h"

	extern void trans_complete_with_status(struct cat_message* message, char* status);
	extern void settimeout(struct cat_message* message, int sec);
	extern cat_message* new_transaction(char* type, char* name);
	extern cat_message* sub_transaction(char* type, char* name, struct cat_message *parent);
	extern void log_event(char* type, char* name, char* status, char* data);
	extern cat_message* sub_event(char* type, char* name, struct cat_message *parent);
	extern cat_message* add_data(struct cat_message *event, char* data);
}

#endif
