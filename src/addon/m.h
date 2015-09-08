/*
 * m.h
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 *     Purpose: Define macro, struct, const, common functions
 */

#ifndef SRC_ADDON_M_H_
#define SRC_ADDON_M_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "win32.h"

#define ZMALLOC(theSize) zmalloc(__FILE__, __LINE__, theSize)
#define ZREALLOC(ptr, theSize) zrealloc(__FILE__, __LINE__, ptr, theSize)

#define LOG_FATAL    (1)
#define LOG_ERR      (2)
#define LOG_WARN     (3)
#define LOG_INFO     (4)

#define LOG(level, ...) do {  \
                            if (level <= debug_level) { \
                            	switch(level){	\
								case 1: printf("[FATAL]");break;	\
								case 2: printf("[ERROR]");break;	\
								case 3: printf("[WARN]");break;	\
								case 4: printf("[INFO]");break;	\
								}	\
                                printf("%s:%d:", __FILE__, __LINE__); \
                                printf(__VA_ARGS__); \
                                printf("\n"); \
                            } \
                        } while (0)

#define LOG1(level, ...) do {  \
                            if (level <= debug_level) { \
                                printf(__VA_ARGS__); \
                            } \
                        } while (0)

extern FILE *dbgstream;
extern int  debug_level;

extern const int KB;
extern const int BUFFER_SIZE;
extern const int CHAR_BUFFER_SIZE;
extern const int MB;
extern const int MAX_TRANS_CHAILD_SIZE;
extern const int EXCEPTION_CODE;
extern const int SUCCESS_CODE;

/*
 * cat message protocol field
 * protocol refer to: http://conf.ctripcorp.com/pages/viewpage.action?pageId=63473040
 */
extern const char ID[];
extern const char TAB[];
extern const char LF[];
extern const char SPLIT[];
extern const char CAT_NULL[];
extern const char DEFAULT_DOMAIN[];

typedef struct byte_buf byte_buf;
typedef struct transaction transaction;
typedef struct flush_stack flush_stack;
typedef struct message message;
typedef struct c_string c_string;
typedef struct server server;

enum report_type {
	ReportType_Event = 10, ReportType_Transaction = 11, ReportType_Heartbeat = 12, ReportType_Metric = 13
};
typedef enum report_type report_type;

enum policy {
	Policy_DEFAULT = 21, Policy_WITHOUT_STATUS = 22, Policy_WITH_DURATION = 23
};
typedef enum policy policy;

typedef struct g_context {
	unsigned int initialized;
	char *domain;
	server* serv;
	char local_ip[16];
	char local_ip_hex[8];
	unsigned int msg_index;
	char *hostname;
	message *msg;
	char *msg_id;
} g_context;

struct byte_buf {
	char *buffer;
	int ptr; /* pointer to current location of buffer */
	int size;
	int block; /* buffer block number, per block have 64KB */
};

struct c_string {
	char *data;
	int block;
};

struct server{
	char **address;
	int len;
};

struct message {
	char* type;
	char* name;
	c_string* data;
	char* status;
	c_long timestamp;
	char format_time[24];
	report_type reportType;
	transaction *trans;
	message *trans_parent;
	int completed; /* complete flag, 1 = completed success */
};

struct transaction {
	struct message* children[1000];
	int children_size;
	int count_fork;
	unsigned int standalone;
	c_long duration;
	c_long endtime;
	char end_format_time[24];
	int is_root; /* is root transaction flag, if this is root transaction is_root = 1  */
	int docomplete; /* root transaction must explicit set to 1, even if all sub transaction has complete */
	int timeout; /* timeout in seconds, -1 = Not Set */
	pthread_t tid; /* -1 = Not Set */
	int flush; /* 1 = flushed */
};

/* reusable buffer */
extern char small_buf[];
extern char *buf_ptr;

/* global context */
extern g_context *context;
extern void init_ip(struct g_context *context);

struct c_string* init_c_string();
void set_c_string(struct c_string *str, const char *data);
void cat_c_string(struct c_string *str, const char *data);
void free_c_string(struct c_string *str);
void _expand_c_string(struct c_string *str, int purpose);
c_long zero();
struct byte_buf* init_buf();
struct g_context* setup_context();
void free_context();
void _expand_buf(struct byte_buf*);
void write_str(struct byte_buf* buf, const char* str);
void write_int(struct byte_buf* buf, int i);
void write_long(struct byte_buf* buf, c_long l);
void write_char(struct byte_buf* buf, char c);
void free_buf(struct byte_buf*);
void convert_int(char* bytes, int i);
void ilog(char*);
struct message* init_message();
void free_message(message* message);
struct message* init_transaction();
void free_transaction(message* message);
struct message* init_event();
void *zmalloc(const char *file, int line, int size);
void *zrealloc(const char *file, int line, void* ptr, int size);
c_long get_tv_usec();
int c_get_threadid();
void c_exit_thread();
void copy_string(char* to, const char* from, size_t limit);
void copy_nstr(char* to, const char* from);
void mark(const char*);
int read_mark();
void c_sleep(unsigned int sec);

#ifdef _WIN32
void win_get_format_time(char** buf);
c_long win_get_tv_usec();
#else
void get_format_time(char** buf);
#endif

#endif /* SRC_ADDON_M_H_ */
