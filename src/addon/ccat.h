#ifndef CCAT_H
#define CCAT_H
 
extern "C"
{
	#include "msg.h"
	#include "manager.h"

	extern void ts_fork(struct cat_message* ts);
	extern void ts_join(struct cat_message* ts);
	extern void ts_complete(struct cat_message* ts);
	
	extern cat_message* new_transaction(char* type, char* name);
	extern cat_message* sub_transaction(char* type, char* name, struct cat_message *parent);
	extern cat_message* new_event(char* type, char* name, struct cat_message *parent);
}

#endif
