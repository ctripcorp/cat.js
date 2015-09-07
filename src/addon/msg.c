/*
 * msg.c
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#include "msg.h"

void complete_message_with_status(struct message* msg, char* status) {
	copy_string(msg->status, status, CHAR_BUFFER_SIZE);
	complete_message(msg);
}

void complete_message(struct message* msg) {
	switch (msg->reportType) {
	case ReportType_Transaction:
		complete_trans(msg);
		break;
	case ReportType_Event:
		complete_event(msg);
		break;
	default:
		break;
	}
}

void complete_event(struct message* evt) {
	evt->completed = 1;
}

void complete_trans(struct message* msg) {
	transaction *ptr_trans = msg->trans;

	ptr_trans->docomplete = 1; /* transaction has explicit call completed */

	if (msg->completed == 1)
		return; /* message already completed */

	if (ptr_trans->is_root == 1) { /* message is root transaction */

		if (ptr_trans->count_fork == 0) { /* all its child has completed */
			set_trans_completed(msg);
			message_flush(msg);
		}

		/* else: do nothing, just wait for sub transaction complete */

	} else {
		set_trans_completed(msg);
		do_join(msg);
	}
}

void do_join(struct message* msg) {
	message *ptr_parent = msg->trans_parent;

	if (--ptr_parent->trans->count_fork == 0 /* do join */) {
		/* this transaction is last completed child, */
		if (ptr_parent->trans->is_root) {
			if (ptr_parent->trans->docomplete == 1) {
				/* root transaction must explicit call complete */
				complete_trans(ptr_parent);
			}
		} else {
			/* auto completed its parent  */
			complete_trans(ptr_parent);
		}

	}

	/* else: Transaction is not last child */
}

void timeout(struct message* msg) {
	transaction *ptr_trans;

	if (msg->completed == 1)
		return; /* message already completed */

	ptr_trans = msg->trans;

	copy_string(msg->status, "TIMEOUT", CHAR_BUFFER_SIZE);

	if (ptr_trans->is_root == 1) { /* message is root transaction */
		set_trans_completed(msg);
		message_flush(msg);
	} else {
		set_trans_completed(msg);
		do_join(msg);
	}
}

void set_trans_completed(struct message* msg) {
	c_long current;

	msg->completed = 1;
	get_format_time(&buf_ptr);
	strncpy(msg->trans->end_format_time, small_buf, 24);

	LOG(LOG_INFO, "-- Transaction[%p] complete at time:%s", msg, msg->trans->end_format_time);

	current = get_tv_usec();
	msg->trans->endtime = current;
	msg->trans->duration = current - msg->timestamp;
}

void message_flush(struct message* msg) {
	do_send(msg);
#if 0 //TODO resolve thread issue
#ifdef _WIN32
	do_send(message);
	/*
	 int val = 0;
	 HANDLE handle;
	 handle = (HANDLE)_beginthread(do_send, 0, &val); // create thread
	 WaitForSingleObject(handle, INFINITE);
	 */
#else
	pthread_t tid;
	int err;
	err = pthread_create(&tid, NULL, &do_send, message);
	if (err != 0)
	printf("\ncan't create thread :[%s]", strerror(err));

	void* status;
	pthread_join(tid, &status);
#endif

	//TODO free may not corrent
	/*
	 while (1){
	 message* temp = pop();
	 if (temp == NULL)
	 break;
	 else
	 free_tree(message);
	 }*/
#endif
}

void* do_send(void *arg) {
	struct message* msg = arg;
	int i;
	struct byte_buf *buf ;
	add_message(msg);
	buf = init_buf();
	encode(context, buf);

	/* print raw message data */
	LOG1(LOG_INFO, "[INFO]Raw Text:\n");

	for (i = 0; i < buf->ptr; i++) {
		LOG1(LOG_INFO, "%c", buf->buffer[i]);
	}

#if 0	/* manual check binary error, comment this on release */
	printf("Raw Binary:\n");
	for (int i = 0; i < buf->ptr; i++) {
		char c=buf->buffer[i];
		if(c==9) {
			printf(" TAB ");
		}
		else if(c==10) {
			printf("\n");
		}
		else if((c>=65&&c<=90)||(c>=97&&c<=122)||(c>=48&&c<=57)||c=='.'||c=='-'||c==':'||c==32||i<4) {
			printf("%d ", buf->buffer[i]);
		} else {
			printf("[%d] ", buf->buffer[i]);
			printf("(%c)", buf->buffer[i]);
		}

	}
	printf("\n");
#endif

	socket_send(buf->buffer, buf->size);

	return NULL;
}

message* new_transaction(char* type, char* name) {
	message* root_trans = sub_transaction(type, name, NULL);
	root_trans->trans->is_root = 1;
	return root_trans;
}

#if 0
void cancel_timeout(struct message* message) {

#ifdef _WIN32
	//TODO
#else
	pthread_cancel(message->trans->tid);
#endif

	message->trans->flush = 0;

}

void settimeout(struct message* message, int sec) {
	message->trans->timeout = sec;
	if ((int) message->trans->flush == 1)
	cancel_timeout(message);
#ifdef _WIN32
	int val = 0;
	HANDLE handle;
	handle = (HANDLE)_beginthread(do_send, 0, &val); // create thread
	WaitForSingleObject(handle, INFINITE);

	message->trans->flush = 1;
#else
	int err;
	err = pthread_create(&message->trans->tid, NULL, &do_timeout, message);
	if (err != 0)
	printf("\ncan't create thread :[%s]", strerror(err));
	else
	message->trans->flush = 1;
#endif
}

void* do_timeout(void *arg) {
	struct message* message = arg;
	csleep(message->trans->timeout);
	timeout(message);
	c_exit_thread();
	return NULL;
}
#endif

message* sub_transaction(char* type, char* name, struct message *parent) {
	message *msg = init_transaction();

	copy_nstr(msg->type, type);
	copy_nstr(msg->name, name);

	get_format_time(&buf_ptr);
	strncpy(msg->format_time, small_buf, 24);

	LOG(LOG_INFO,"transaction create time:%s",msg->format_time);

	msg->timestamp = get_tv_usec();

#ifdef _WIN32
	LOG(LOG_INFO,"transaction timestamp:%lld",msg->timestamp);
#else
	LOG(LOG_INFO,"transaction timestamp:%ld",msg->timestamp);
#endif

	msg->trans_parent = parent;

	if (parent != NULL) {
		parent->trans->count_fork++;
		parent->trans->children[parent->trans->children_size] = msg;
		parent->trans->children_size++;
	}

	LOG(LOG_INFO, "-- Transaction[%p] created", msg);
	return msg;
}

void log_event(char* type, char* name, char* status, char* data) {
	message* evt = new_event(type, name, status, data);
	message_flush(evt);
}

message* new_event(char* type, char* name, char* status, char* data) {
	message* evt = init_event();
	set_c_string(evt->data, data);
	copy_string(evt->type, type, CHAR_BUFFER_SIZE);
	copy_string(evt->name, name, CHAR_BUFFER_SIZE);
	copy_string(evt->status, status, CHAR_BUFFER_SIZE);
	evt->timestamp = get_tv_usec();
	get_format_time(&buf_ptr);
	strncpy(evt->format_time, small_buf, 24);
	return evt;
}

message* add_data(struct message *event, char* data) {

	if (strlen(event->data->data) > 0)
		cat_c_string(event->data, "&");

	cat_c_string(event->data, data);

	return event;
}

message* sub_event(char* type, char* name, char* status, struct message *parent) {
	message* evt = new_event(type, name, status, "");

	parent->trans->children[parent->trans->children_size] = evt;
	parent->trans->children_size++;

	return evt;
}

void encode_header(struct g_context* context, struct byte_buf *buf) {
	write_str(buf, ID);
	write_str(buf, TAB);
	write_str(buf, context->domain);
	write_str(buf, TAB);
	write_str(buf, context->hostname);
	write_str(buf, TAB);
	write_str(buf, context->local_ip);
	write_str(buf, TAB);
	write_str(buf, CAT_NULL);
	write_str(buf, TAB);
	write_str(buf, "0");
	write_str(buf, TAB);
	write_str(buf, CAT_NULL);
	write_str(buf, TAB);
	write_str(buf, context->msg_id);
	write_str(buf, TAB);
	write_str(buf, CAT_NULL);
	write_str(buf, TAB);
	write_str(buf, CAT_NULL);
	write_str(buf, TAB);
	write_str(buf, CAT_NULL);
	write_str(buf, LF);
}

void insert_int(struct byte_buf *buf, int value) {
	int len = 4, i = 0;
	char bytes[4];

	convert_int(bytes, value);
	do {
		buf->buffer[i] = bytes[i];
	} while (++i < len);
}

void encode_line(struct message* msg, struct byte_buf *buf, char type, enum policy policy) {
	char* data;
	write_char(buf, type);

	if (type == 'T' && msg->reportType == ReportType_Transaction) {
		write_str(buf, msg->trans->end_format_time);
	} else {
		write_str(buf, msg->format_time);
	}

	write_str(buf, TAB);
	write_str(buf, msg->type);
	write_str(buf, TAB);
	write_str(buf, msg->name);
	write_str(buf, TAB);

	if (policy != Policy_WITHOUT_STATUS) {
		write_str(buf, msg->status);
		write_str(buf, TAB);

		data = msg->data->data;

		if (policy == Policy_WITH_DURATION && msg->reportType == ReportType_Transaction) {
			write_long(buf, msg->trans->duration);

			write_str(buf, "us");
			write_str(buf, TAB);
		}

		write_str(buf, data);
		write_str(buf, TAB);
	}

	write_str(buf, LF);
}

void encode_message(struct message* msg, struct byte_buf *buf) {
	int i;

	if (msg->reportType == ReportType_Event) {
		encode_line(msg, buf, 'E', Policy_DEFAULT);
		return;
	}
	if (msg->reportType == ReportType_Transaction) {
		transaction *transaction_temp = msg->trans;
		int len = transaction_temp->children_size;

		if (len == 0) {
			encode_line(msg, buf, 'A', Policy_WITH_DURATION);
			return;
		}
		encode_line(msg, buf, 't', Policy_WITHOUT_STATUS);

		for (i = 0; i < len; i++) {
			struct message* child = transaction_temp->children[i];
			encode_message(child, buf);
		}

		encode_line(msg, buf, 'T', Policy_WITH_DURATION);
	}
	if (msg->reportType == ReportType_Heartbeat) {
		encode_line(msg, buf, 'H', Policy_DEFAULT);
		return;
	}
	if (msg->reportType == ReportType_Metric) {
		encode_line(msg, buf, 'M', Policy_DEFAULT);
		return;
	}
}

void encode(struct g_context* context, struct byte_buf *buf) {
	buf->ptr = 4; // place-holder
	encode_header(context, buf);
	if (context->msg != NULL) {
		encode_message(context->msg, buf);
	}
	insert_int(buf, buf->size);
	buf->size += 4;
	return;
}

void set_status(struct message* msg, char* status) {
	copy_string(msg->status, status, CHAR_BUFFER_SIZE);
}

