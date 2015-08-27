#include <node.h>
#include <v8.h>
#include <string.h>
#include <stdio.h>
#include "ccat.h"
using namespace v8;

char *parse_args(v8::Local<v8::Value> value);
void callback(void* p);
/*
 * args[0] <Number> pointer of transaction to complete
 * args[1] <String> transaction status
 */
void glue_complete(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if (args.Length() != 2) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
		return;
	}

	if(!args[0]->IsNumber() || !args[1]->IsString()){
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments type")));
		return;
	}

	long ptr=args[0]->NumberValue();
	char* status=parse_args(args[1]);
	
	cat_message* p= reinterpret_cast<cat_message*>(ptr);
	trans_complete_with_status(p, status);

	args.GetReturnValue().Set(Number::New(isolate,0));
}

void glue_add_data(const FunctionCallbackInfo<Value>& args) {

}

void glue_set_status(const FunctionCallbackInfo<Value>& args) {

}

void glue_subTransaction(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if (args.Length() != 3) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Wrong number of arguments")));
		return;
	}
	long ptr=args[0]->NumberValue();
	
	cat_message* p= reinterpret_cast<cat_message*>(ptr);
	char* type=parse_args(args[1]);
	char* name=parse_args(args[2]);
	cat_message *message = sub_transaction(type,name,p);

	long rPtr= reinterpret_cast<long>(message);

	Local<Object> obj = Object::New(isolate);
	//return pointer address
	obj->Set(String::NewFromUtf8(isolate, "pointer"), Number::New(isolate,rPtr));
	args.GetReturnValue().Set(obj);
}

void glue_new_event(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if (args.Length() != 3) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Wrong number of arguments")));
		return;
	}

	if(!args[0]->IsNumber() || !args[1]->IsString() || !args[2]->IsString()){
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments type")));
		return;
	}

	long ptr=args[0]->NumberValue();
	
	cat_message* p= reinterpret_cast<cat_message*>(ptr);
	char* type=parse_args(args[1]);
	char* name=parse_args(args[2]);
	cat_message *message = sub_event(type,name,p);
	
	long rPtr= reinterpret_cast<long>(message);
	Local<Object> obj = Object::New(isolate);
	obj->Set(String::NewFromUtf8(isolate, "pointer"), Number::New(isolate,rPtr));
	args.GetReturnValue().Set(obj);
}

void glue_set_domain(const FunctionCallbackInfo<Value>& args) {

}

void glue_set_server(const FunctionCallbackInfo<Value>& args) {

}

/*
 * args[0] <String> transaction type
 * args[1] <String> transaction name
 */
void glue_new_transaction(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if (args.Length() != 2) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
		return;
	}

	if(!args[0]->IsString() || !args[1]->IsString()){
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments type")));
		return;
	}

	char* type=parse_args(args[0]);
	char* name=parse_args(args[1]);

	cat_message *message = new_transaction(type,name);
	
	long ptr= reinterpret_cast<long>(message);

	Local<Object> obj = Object::New(isolate);
	obj->Set(String::NewFromUtf8(isolate, "pointer"), Number::New(isolate,ptr));
	args.GetReturnValue().Set(obj);
}

void glue_log_event(const FunctionCallbackInfo<Value>& args) {

}

void glue_log_error(const FunctionCallbackInfo<Value>& args) {

}

void glue_settimeout(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if (args.Length() != 2) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
		return;
	}

	if(!args[0]->IsNumber() || !args[0]->IsNumber()){
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments type")));
		return;
	}

	long ptr = args[0]->NumberValue();
	int sec = args[1]->NumberValue();
	cat_message* p= reinterpret_cast<cat_message*>(ptr);
	settimeout(p,sec);

	args.GetReturnValue().Set(Number::New(isolate,0)); // return 0 as success
}

void Init(Handle<Object> exports) {

	/* Method Glue */
    /* glue nodejs and c modules and check paramter illegal */
	/* message.js */
	NODE_SET_METHOD(exports, "glue_complete", glue_complete);
	NODE_SET_METHOD(exports, "glue_add_data", glue_add_data);
	NODE_SET_METHOD(exports, "glue_set_status", glue_set_status);

	/* span.js */
	NODE_SET_METHOD(exports, "glue_subTransaction", glue_subTransaction);
	NODE_SET_METHOD(exports, "glue_new_event", glue_new_event);

	/* cat.js */
	NODE_SET_METHOD(exports, "glue_set_domain", glue_set_domain);
	NODE_SET_METHOD(exports, "glue_set_server", glue_set_server);
	NODE_SET_METHOD(exports, "glue_new_transaction", glue_new_transaction);
	NODE_SET_METHOD(exports, "glue_log_event", glue_log_event);
	NODE_SET_METHOD(exports, "glue_log_error", glue_log_error);
	NODE_SET_METHOD(exports, "glue_settimeout", glue_settimeout);
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

