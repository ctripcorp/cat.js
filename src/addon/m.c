/*
 * m.c
 *
 *  Created on: 2015年8月11日
 *      Author: Stur
 */

#include "m.h"

const int KB = 1024;
const int BUFFER_SIZE = 64 * 1024;
const int CHAR_BUFFER_SIZE = 1024 * sizeof(char);
const int MB = KB * KB;
const int MAX_TRANS_CHAILD_SIZE = 1000; /* TODO:Change to server side limit if necessary */
const int EXCEPTION_CODE = -1;
const int SUCCESS_CODE = 1;
int  debug_level = 2;
FILE *dbgstream;

/*
 * cat message protocol field
 * protocol refer to: http://conf.ctripcorp.com/pages/viewpage.action?pageId=63473040
 */
const char ID[] = "PT1";
const char TAB[] = "\t";
const char LF[] = "\n";
const char SPLIT[] = "-";
const char CAT_NULL[] = "null";
const char DEFAULT_DOMAIN[] = "nodejs";

char small_buf[BUFFER_SIZE];
char *buf_ptr = small_buf;
g_context *context;

/*
 * store encode data
 */
struct byte_buf* init_buf() {
	byte_buf* buf = ZMALLOC(sizeof(struct byte_buf));
	buf->buffer = (char*) ZMALLOC(BUFFER_SIZE * (sizeof(char)));
	buf->block = 1;
	buf->ptr = 0;
	buf->size = 0;
	return buf;
}

/*
 * Purpose: write string to memory
 * Argument  IO  Description
 * --------- --  ---------------------------------
 * buf       I
 * str		 I   NULL string will be set to 'null' and string length should less than 64*KB
 */
void write_str(struct byte_buf* buf, const char* str) {
	if (str == NULL)
		str = "null";

	int len = strlen(str);

	if (buf->ptr + len > buf->block * BUFFER_SIZE) {
		_expand_buf(buf);
	}

	strncpy(&buf->buffer[buf->ptr], str, len);
	buf->ptr += len;
	buf->size += len;
}

void write_int(struct byte_buf* buf, int i) {
	/*
	 int n = log10(i) + 1;
	 char* number_buf = ZMALLOC(n*sizeof(char));
	 snprintf(number_buf, n, "%d", i);
	 write_str(buf, number_buf);
	 free(number_buf);
	 */

	char foo[30];
	sprintf(foo, "%d", i);
	write_str(buf, foo);
}

void write_long(struct byte_buf* buf, c_long l) {
	char foo[30];
	sprintf(foo, "%ld", l);
	write_str(buf, foo);
}

void write_char(struct byte_buf* buf, char c) {
	buf->buffer[buf->ptr] = c;
	buf->ptr++;
	buf->size++;
}

void _expand_buf(struct byte_buf* buf) {
	buf->block++;
	char* ptr = (char*) ZREALLOC(buf->buffer, buf->block * BUFFER_SIZE * (sizeof(char)));
	buf->buffer = ptr;
}

void free_buf(struct byte_buf* buf) {
	free(buf->buffer);
	free(buf);
	buf = NULL;
}

/*
 * Purpose: Converting an integer to 4 bytes array
 * Argument  IO  Description
 * --------- --  ---------------------------------
 * bytes     O   4 bytes char array, bytes[4]
 * i		 I
 */
void convert_int(char* bytes, int i) {
	bytes[0] = (i >> 24) & 0xFF;
	bytes[1] = (i >> 16) & 0xFF;
	bytes[2] = (i >> 8) & 0xFF;
	bytes[3] = i & 0xFF;
}

struct message* init_event() {
	message* evt = init_message();
	evt->reportType = ReportType_Event;
	return evt;
}

struct message* init_message() {
	message* msg = ZMALLOC(sizeof(struct message));
	msg->completed = 0;
	memset(msg->format_time, 0, 24);
	msg->trans = NULL;
	msg->trans_parent = NULL;
	msg->timestamp = zero();
	msg->type = (char*) ZMALLOC(CHAR_BUFFER_SIZE);
	msg->name = (char*) ZMALLOC(CHAR_BUFFER_SIZE);
	msg->status = (char*) ZMALLOC(CHAR_BUFFER_SIZE);
	msg->data = init_c_string();
	return msg;
}

void copy_string(char* to, const char* from, size_t limit) {
	size_t len = strlen(from);
	if (len > limit) {
		to = (char*) ZREALLOC(to, (len + 1) * sizeof(char));
	}
	strncpy(to, from, len);
}

struct c_string* init_c_string(){
	c_string *str = (c_string*) ZMALLOC(sizeof(c_string));
	str->block = 1;
	str->data = (char*) ZMALLOC(BUFFER_SIZE * sizeof(char));
	return str;
}

void _expand_c_string(struct c_string *str, int purpose){
	int limit = BUFFER_SIZE * str->block;
	do{
		str->block++;
		limit = BUFFER_SIZE * str->block;
	}while(limit < purpose);

	str->data = (char*) ZREALLOC(str->data, str->block * BUFFER_SIZE * sizeof(char));
}

void set_c_string(struct c_string *str, const char *data){
	int limit = BUFFER_SIZE * str->block;
	int len = strlen(data);
	if(len > limit){
		_expand_c_string(str, len);
	}
	copy_string(str->data, data, limit);
}

void cat_c_string(struct c_string *str, const char *data){
	int limit = BUFFER_SIZE * str->block;
	int len = strlen(data);
	if(len > limit){
		_expand_c_string(str, len);
	}

	strcat(str->data, data);
}

void free_c_string(struct c_string *str){
	free(str->data);
	free(str);
}

void copy_nstr(char* to, const char* from){
	copy_string(to, from, CHAR_BUFFER_SIZE);
}

struct message* init_transaction() {
	transaction *trans = (transaction*) ZMALLOC(sizeof(transaction));
	trans->children_size = 0;
	trans->standalone = 0;
	trans->duration = 0;
	trans->count_fork = 0;
	trans->docomplete = 0;
	trans->endtime = zero();
	memset(trans->end_format_time, 0, 24);
	trans->is_root = 0;	//0 means this is not root
	trans->timeout = -1;	//-1 means timeout not set
	trans->flush = 0;	//0 means has not flushed

	message* message = init_message();
	message->reportType = ReportType_Transaction;
	message->trans = trans;

	return message;
}

void free_transaction(message* message) {
	free(message->trans);
	free(message);
}

void free_message(message* message) {
	free(message->type);
	free(message->status);
	free(message->name);
	free_c_string(message->data);
	free(message);
}

struct g_context* setup_context() {
	g_context *context = (g_context*) ZMALLOC(sizeof(g_context));
	init_ip(context);
	context->domain = ZMALLOC(CHAR_BUFFER_SIZE);
	copy_string(context->domain, &DEFAULT_DOMAIN[0], CHAR_BUFFER_SIZE);

	context->hostname = ZMALLOC(CHAR_BUFFER_SIZE);
	context->msg_id = ZMALLOC(CHAR_BUFFER_SIZE);
	context->msg = NULL;
	gethostname(context->hostname, 1024);

	LOG(LOG_INFO,"host name:%s",context->hostname);

	context->msg_index = read_mark();
	context->initialized = 1;
	context->serv = ZMALLOC(sizeof(server));
	context->serv->server =ZMALLOC(sizeof(char*)*4);
	int i;
	for(i=0;i<4;i++){
		context->serv->server[i] = ZMALLOC(sizeof(char)* 16);
	}
	copy_string(context->serv->server[0], "0.0.0.0", 16);
	context->serv->len = 1;

	return context;
}

void free_context(struct g_context* context) {
	free(context->msg_id);
	free(context->hostname);
	free(context->domain);
	free(context);
}

c_long zero() {
#ifdef _WIN32
	return 0LL;
#else
	return 0L;
#endif
}

void get_format_time(char** buf) {
	char fmt[64];
	struct timeval tv;
	struct tm *tm;

	gettimeofday(&tv, NULL);
	if ((tm = localtime(&tv.tv_sec)) != NULL) {
		strftime(fmt, sizeof fmt, "%Y-%m-%d %H:%M:%S.%%03d", tm);
		snprintf(*buf, 24, fmt, tv.tv_usec);
	}
}

c_long get_tv_usec() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000L + tv.tv_usec;
}

/* not used yet */
void c11_support() {
#if defined __STDC_LIB_EXIT1__
	printf("<C11 support>");
#else
	printf("<C11 not support!!!>");
#endif
}

int c_get_threadid() {
#ifdef _WIN32
	int x = GetCurrentThreadId();
	return x;
#else
	return (int) pthread_self();
#endif
}

void c_exit_thread() {
	int ret1 = 100;
#ifdef _WIN32
	ExitThread(ret1);
	return 1;
#else
	pthread_exit(&ret1);
#endif
}

/* sleep by seconds */
void c_sleep(unsigned int sec){
#ifdef _WIN32
	Sleep(sec * 1000);
#else
	sleep(sec);
#endif
}

void *zmalloc(const char *file, int line, int size) {
	void *ptr = malloc(size);

	if (!ptr) {
		LOG(LOG_FATAL,"Could not allocate");
		//TODO this may exit main thread, so do something left and disable logging
		exit(1);
	}

	return (ptr);
}

void *zrealloc(const char *file, int line, void* ptr, int size) {
	void *_ptr = realloc(ptr, size);

	if (!_ptr) {
		LOG(LOG_FATAL,"Could not reallocate");
		//TODO this may exit main thread, so do something left and disable logging
		exit(1);
	}

	return (_ptr);
}

void mark(const char* data){
   FILE *fp;

   fp = fopen("mark.txt", "w+");
   fprintf(fp, "%s", data);
   fclose(fp);
}

int read_mark(){
   FILE *fp;
   char buff[255];
   fp = fopen("mark.txt", "r");
   if(fp){
	   fgets(buff, 255, (FILE*)fp);
	   int i;
	   sscanf(buff, "%d", &i);
	   fclose(fp);
	   return i;
   }

   LOG(LOG_WARN,"fail to read mark");

   return 0;
}

#ifdef _WIN32
char* win_get_format_time() {
	/*
	 struct timeval2 tv;
	 struct timezone2 tz;
	 gettimeofday(&tv, &tz);
	 struct tm *tm;
	 if ((tm = localtime(&tv.tv_sec)) != NULL) {
	 strftime(fmt, sizeof fmt, "%Y-%m-%d %H:%M:%S.%%03d", tm);
	 sprintf(*buf, fmt, tv.tv_usec);
	 }
	 */

	//TODO workaround for above solution, the precision not correct
	char fmt[64];
	char *buf = (char*) ZMALLOC(24 * sizeof(char));
	time_t timer;
	char buffer[20];
	struct tm* tm_info;

	time(&timer);
	tm_info = localtime(&timer);

	strftime(buffer, 24, "%Y-%m-%d %H:%M:%S", tm_info);
	strcpy(buf, buffer);
	strcat(buf, ".000");
	return buf;
}

c_long win_get_tv_usec() {
	struct timeval2 tv;
	struct timezone2 tz;
	gettimeofday(&tv, &tz);
	fx_long temp = tv.tv_sec * 1000000LL + tv.tv_usec;
	return temp;
}
#endif

