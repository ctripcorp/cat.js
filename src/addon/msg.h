/*
 * msg.h
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#ifndef SRC_ADDON_MESSAGE_H_
#define SRC_ADDON_MESSAGE_H_

#include "manager.h"
#include "socket.h"

#ifdef __cplusplus
extern "C" {
#endif
extern void complete_message_with_status(struct message* msg, char* status);
void complete_message(struct message* msg);
void complete_event(struct message* evt);
void complete_trans(struct message* msg);
void set_trans_completed(struct message*);
void free_trans(struct message* trans);
void do_join(struct message* msg);
extern void timeout(struct message* msg);
extern void settimeout(struct message*);
void timeout_tree(struct message* msg);
void flush_tree(struct message* msg);
void message_flush(struct message*);
void* do_send(void *arg);
extern message* new_transaction(char* type, char* name);
extern message* sub_transaction(char* type, char* name, struct message *parent);
extern void log_event(char* type, char* name, char* status, char* data);
message* new_event(char* type, char* name, char* status, char* data);
extern message* add_data(struct message *event, char* data);
extern message* sub_event(char* type, char* name, char* status, struct message *parent);

void encode_header(struct g_context* context, struct byte_buf *buf);
void insert_int(struct byte_buf *buf, int value);
void encode_line(struct message* msg, struct byte_buf *buf, char type, enum policy policy);
void encode_message(struct message* msg, struct byte_buf *buf);
void encode(struct g_context* context, struct byte_buf *buf);
extern void set_status(struct message* msg, char* status);
void send_metric(int mem_value);

#ifdef __cplusplus
}
#endif

#endif /* SRC_ADDON_MESSAGE_H_ */

