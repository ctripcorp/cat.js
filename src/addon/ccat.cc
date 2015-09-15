#include <node.h>
#include <v8.h>
#include <string.h>
#include <stdio.h>

#include "m.h"
#include "msg.h"
#include "manager.h"

using namespace v8;

char *parse_args(v8::Local<v8::Value> value);
Isolate* prepareIsolate(const FunctionCallbackInfo<Value>& args, int len);
void glue_set_inner(const FunctionCallbackInfo<Value>& args, void (*foo)(char*));
int checkAndGetInteger(Isolate* isolate, v8::Local<v8::Value> value);
long checkAndGetLong(Isolate* isolate, v8::Local<v8::Value> value);
char* checkAndGetString(Isolate* isolate, v8::Local<v8::Value> value);
static int isLogging;

/*
 * args[0] <Number> pointer of transaction to complete
 * args[1] <String> transaction status
 */
 void glue_complete(const FunctionCallbackInfo<Value>& args) {
 	if(isLogging == 0)return;

 	Isolate* isolate = prepareIsolate(args, 2);

 	long ptr = checkAndGetLong(isolate, args[0]);
 	char* status = checkAndGetString(isolate, args[1]);

 	message* p= reinterpret_cast<message*>(ptr);
 	complete_message_with_status(p, status);

 	args.GetReturnValue().Set(Number::New(isolate,0));
 }

 void glue_add_data(const FunctionCallbackInfo<Value>& args) {
 	if(isLogging == 0)return;

 	Isolate* isolate = prepareIsolate(args, 2);

 	long ptr = checkAndGetLong(isolate, args[0]);
 	char* data = checkAndGetString(isolate, args[1]);

 	message* p= reinterpret_cast<message*>(ptr);
 	add_data(p, data);

 	args.GetReturnValue().Set(Number::New(isolate,0));
 }

 void glue_set_status(const FunctionCallbackInfo<Value>& args) {
 	if(isLogging == 0)return;

 	Isolate* isolate = prepareIsolate(args, 2);

 	long ptr = checkAndGetLong(isolate, args[0]);
 	char* status = checkAndGetString(isolate, args[1]);

 	message* p= reinterpret_cast<message*>(ptr);
 	set_status(p, status);

 	args.GetReturnValue().Set(Number::New(isolate,0));
 }

 void glue_subTransaction(const FunctionCallbackInfo<Value>& args) {
 	if(isLogging == 0)return;

 	Isolate* isolate = prepareIsolate(args, 3);
 	long ptr = checkAndGetLong(isolate, args[0]);

 	message* p= reinterpret_cast<message*>(ptr);
 	char* type = checkAndGetString(isolate, args[1]);
 	char* name = checkAndGetString(isolate, args[2]);
 	message *message = sub_transaction(type,name,p);

 	long rPtr= reinterpret_cast<long>(message);

 	Local<Object> obj = Object::New(isolate);
	//return pointer address
 	obj->Set(String::NewFromUtf8(isolate, "pointer"), Number::New(isolate,rPtr));
 	args.GetReturnValue().Set(obj);
 }

 void glue_new_event(const FunctionCallbackInfo<Value>& args) {
 	if(isLogging == 0)return;

 	Isolate* isolate = prepareIsolate(args, 4);

 	long ptr = checkAndGetLong(isolate, args[0]);

 	message* p= reinterpret_cast<message*>(ptr);
 	char* type = checkAndGetString(isolate, args[1]);
 	char* name = checkAndGetString(isolate, args[2]);
 	char* status = checkAndGetString(isolate, args[3]);
 	message *msg = sub_event(type,name,status,p);

 	long rPtr= reinterpret_cast<long>(msg);
 	Local<Object> obj = Object::New(isolate);
 	obj->Set(String::NewFromUtf8(isolate, "pointer"), Number::New(isolate,rPtr));
 	args.GetReturnValue().Set(obj);
 }

 void glue_set_inner(const FunctionCallbackInfo<Value>& args, void (*foo)(char*)){
 	if(isLogging == 0)return;

 	Isolate* isolate = prepareIsolate(args, 1);

 	char* value = checkAndGetString(isolate, args[0]);

 	foo(value);

 	args.GetReturnValue().Set(Number::New(isolate,0));
 }

 void glue_set_domain(const FunctionCallbackInfo<Value>& args) {
 	if(isLogging == 0)return;

 	Isolate* isolate = prepareIsolate(args, 1);

 	char* value = checkAndGetString(isolate, args[0]);

 	set_domain(value);

 	args.GetReturnValue().Set(Number::New(isolate,0));
 }

 void glue_set_server(const FunctionCallbackInfo<Value>& args) {
 	if(isLogging == 0)return;

 	Isolate* isolate = Isolate::GetCurrent();
 	HandleScope scope(isolate);

 	if (args.Length() != 1) {
 		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
 		return;
 	}

 	if(!args[0]->IsArray()){
 		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments type")));
 		return;
 	}

 	Local<Array> arr= Local<Array>::Cast(args[0]);
 	int len = arr->Length();
 	const char* value[4];

 	for(int i=0;i<len;i++){
 		Local<Value> item = arr->Get(i);
 		value[i] = parse_args(item);
 	}

 	set_server(value, len);

 	args.GetReturnValue().Set(Number::New(isolate,0));
 }

/*
 * args[0] <String> transaction type
 * args[1] <String> transaction name
 */
 void glue_new_transaction(const FunctionCallbackInfo<Value>& args) {
 	if(isLogging == 0)return;

 	Isolate* isolate = prepareIsolate(args, 2);
 	
 	char* type = checkAndGetString(isolate, args[0]);
 	char* name = checkAndGetString(isolate, args[1]);

 	message *message = new_transaction(type,name);

 	long ptr= reinterpret_cast<long>(message);

 	Local<Object> obj = Object::New(isolate);
 	obj->Set(String::NewFromUtf8(isolate, "pointer"), Number::New(isolate,ptr));
 	args.GetReturnValue().Set(obj);
 }

 void glue_log_event(const FunctionCallbackInfo<Value>& args) {
 	if(isLogging == 0)return;

 	Isolate* isolate = prepareIsolate(args, 4);

 	char* type = checkAndGetString(isolate, args[0]);
 	char* name = checkAndGetString(isolate, args[1]);
 	char* status = checkAndGetString(isolate, args[2]);
 	char* data = checkAndGetString(isolate, args[3]);
 	log_event(type,name,status,data);

 	args.GetReturnValue().Set(Number::New(isolate,0)); 
 }

 void glue_timeout(const FunctionCallbackInfo<Value>& args) {
 	if(isLogging == 0)return;

 	Isolate* isolate = prepareIsolate(args, 1);

 	long ptr = checkAndGetLong(isolate, args[0]);
 	message* p= reinterpret_cast<message*>(ptr);
 	timeout(p);

	args.GetReturnValue().Set(Number::New(isolate,0)); // return 0 as success
}

void glue_settimeout(const FunctionCallbackInfo<Value>& args) {
	if(isLogging == 0)return;

	Isolate* isolate = prepareIsolate(args, 1);

	long ptr = checkAndGetLong(isolate, args[0]);
	message* p= reinterpret_cast<message*>(ptr);
	settimeout(p);

	args.GetReturnValue().Set(Number::New(isolate,0)); // return 0 as success
}

void glue_set_log_level(const FunctionCallbackInfo<Value>& args) {
	if(isLogging == 0)return;

	Isolate* isolate = prepareIsolate(args, 1);

	int log_level = checkAndGetInteger(isolate, args[0]);
	set_debug_level(log_level);

	args.GetReturnValue().Set(Number::New(isolate,0)); // return 0 as success
}

void glue_disable(const FunctionCallbackInfo<Value>& args) {
	if(isLogging == 0)return;

	Isolate* isolate = prepareIsolate(args, 1);

	isLogging = checkAndGetInteger(isolate, args[0]);
	printf("%d\n", isLogging);

	args.GetReturnValue().Set(Number::New(isolate,0)); // return 0 as success
}


void Init(Handle<Object> exports) {

	isLogging = 1;

	/* Method Glue */
    /* glue nodejs and c modules and check paramter illegal */
	/* message.js */
	NODE_SET_METHOD(exports, "glue_complete"       , glue_complete);
	NODE_SET_METHOD(exports, "glue_add_data"       , glue_add_data);
	NODE_SET_METHOD(exports, "glue_set_status"     , glue_set_status);

	/* span.js */
	NODE_SET_METHOD(exports, "glue_subTransaction" , glue_subTransaction);
	NODE_SET_METHOD(exports, "glue_new_event"      , glue_new_event);

	/* cat.js */
	NODE_SET_METHOD(exports, "glue_set_domain"     , glue_set_domain);
	NODE_SET_METHOD(exports, "glue_set_server"     , glue_set_server);
	NODE_SET_METHOD(exports, "glue_new_transaction", glue_new_transaction);
	NODE_SET_METHOD(exports, "glue_log_event"      , glue_log_event);
	NODE_SET_METHOD(exports, "glue_timeout"        , glue_timeout);
	NODE_SET_METHOD(exports, "glue_settimeout"     , glue_settimeout);

	/*
	 * LOG_FATAL    (1)
     * LOG_ERR      (2)
     * LOG_WARN     (3)
     * LOG_INFO     (4)
	 */
     NODE_SET_METHOD(exports, "glue_set_log_level" , glue_set_log_level);
     NODE_SET_METHOD(exports, "glue_disable", glue_disable);

     main_init();
 }

 Isolate* prepareIsolate(const FunctionCallbackInfo<Value>& args, int len){
 	Isolate* isolate = Isolate::GetCurrent();
 	HandleScope scope(isolate);

 	if (args.Length() != len) {
 		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));	
 	}

 	return isolate;
 }

 int checkAndGetInteger(Isolate* isolate, v8::Local<v8::Value> value){
 	if(!value->IsNumber()){
 		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments type")));
 	}

 	int intValue = value->NumberValue();
 	return intValue;
 }

 long checkAndGetLong(Isolate* isolate, v8::Local<v8::Value> value){
 	if(!value->IsNumber()){
 		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments type")));
 	}

 	long longValue = value->NumberValue();
 	return longValue;
 }

 char* checkAndGetString(Isolate* isolate, v8::Local<v8::Value> value){
 	if(!value->IsString()){
 		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments type")));
 	}

 	return parse_args(value);
 }

/*
 * extract char* from v8::Value
 */
 char* parse_args(v8::Local<v8::Value> value) {
 	if (value->IsString()) {
 		String::Utf8Value str_temp(value);
 		char *str = (char *) malloc(str_temp.length() + 1);
 		strcpy(str, *str_temp);
 		return str;
 	}
 	const char *fallback = "";
 	char *str = (char *) malloc(strlen(fallback) + 1);
 	strcpy(str, fallback);
 	return str;
 }

 NODE_MODULE(ccat, Init)

