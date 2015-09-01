/*
 * msg.h
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#ifndef SRC_ADDON_MESSAGE_H_
#define SRC_ADDON_MESSAGE_H_

#include "manager.h"
#include "util.h"
#include "socket.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef _WIN32
typedef int pthread_t;
#include "win32.h"
#include <process.h>
#else
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#endif



#define MAX_BUFFER_SIZE 8192
#define TRANSACTION_CHILD_SIZE 1000
#define EXCEPTION_CODE -1
#define ID  "PT1"
#define TAB "\t"
#define LF "\n"

enum report_type {
	ReportType_Event,
	ReportType_Transaction,
	ReportType_Heartbeat,
	ReportType_Metric
};
typedef enum report_type report_type;

enum policy {
	Policy_DEFAULT, Policy_WITHOUT_STATUS, Policy_WITH_DURATION
};
typedef enum policy policy;

typedef struct channel_buffer{
    char buffer[MAX_BUFFER_SIZE];
	int buffer_pointer;
	int writen_size;
} channel_buffer;

typedef struct cat_message cat_message;
typedef struct cat_transaction cat_transaction;
typedef struct flush_stack flush_stack;

struct cat_message{
	char* Data;
	char* Name;
	char* Status;
	long Timestamp;
	char* format_time;
	char* Type;
	report_type reportType;
    cat_transaction *msg_transaction;
    int complete;
};

struct flush_stack{
	cat_message* stk[MAX_BUFFER_SIZE];
	int top;
};

void push(cat_message* msg);
cat_message*  pop(void);

struct cat_transaction{
	struct cat_message* messageChildren[TRANSACTION_CHILD_SIZE];
	int message_children_size;
	int count_fork;
	unsigned int Standalone;
	long duration;
	long endtime;
	char* end_format_time;
	cat_message *ts_parent;
	int docomplete;//root trans must explicit set to 1, even if all sub trans has complete
	int timeout;//sec
	pthread_t tid;
	int t_start; // 1 = timeout thread already start
};


extern int write_to_buffer_raw(char *buf, char * data,int length, int index);
extern int setint_to_buffer_begin(char* buffer, int i);
extern void getFormatTime(char** buf);
extern g_context cat_context;
extern g_config cat_config;
extern default_message_tree tree;
extern void setup();
extern void add_message(cat_message* message);
extern long get_tv_usec();
extern void str_copy(char* dest,char* source);

int write_to_buffer(struct channel_buffer *buf, char * data);
int write_char_to_buffer(struct channel_buffer *buf, char data);
void send_tree(struct cat_message* message);
int buf_write_int(struct channel_buffer *buf,int i);
int buf_write_long(struct channel_buffer *buf,long i);

int encode_header(struct default_message_tree* tree, struct channel_buffer *buf);
int encode_line(struct cat_message* message, struct channel_buffer *buf, char type, enum policy policy);
int encode_message(struct cat_message* message, struct channel_buffer *buf);
void encode(struct default_message_tree* tree, struct channel_buffer *buf);
cat_message* new_event(char* type, char* name,char* status,char* data);
void cancel_timeout(struct cat_message* message);
void set_complete(struct cat_message* message);
void settimeout(struct cat_message* message, int sec);
void* do_timeout(void *arg);
void* do_send(void *arg);
void free_trans(struct cat_message *trans);
void free_event(struct cat_message *event);
void free_tree(struct cat_message *root);

/*export for glue code*/
void set_status(struct cat_message* message, char* status);
void trans_complete(struct cat_message* message);
void trans_complete_with_status(struct cat_message* message, char* status);
void timeout(struct cat_message* message);
cat_message* new_transaction(char* type, char* name);
cat_message* sub_transaction(char* type, char* name, struct cat_message *parent);
void log_event(char* type, char* name, char* status, char* data);
cat_message* sub_event(char* type, char* name, char* status,struct cat_message *parent);
cat_message* add_data(struct cat_message *event, char* data);

#endif /* SRC_ADDON_MESSAGE_H_ */

