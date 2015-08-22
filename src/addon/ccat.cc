#include <node.h>
#include <v8.h>
#include <string.h>
#include <stdio.h>
#include "ccat.h"
using namespace v8;

char *parse_args(v8::Local<v8::Value> value);
void glue_inner(const FunctionCallbackInfo<Value>& args,void (*foo)(struct cat_message* ts));

void glue_complete(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if (args.Length() != 2) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Wrong number of arguments")));
		return;
	}

	//long ptr=args[0]->NumberValue();
	//TODO set status
	//char* status=parse_args(args[1]);
	printf("%s\n", "glue_complete");
	//cat_message* p= reinterpret_cast<cat_message*>(ptr);
	//ts_complete(p);

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

	printf("%s\n", "glue_subTransaction");

	//return pointer address
	long rPtr= reinterpret_cast<long>(message);
	Local<Object> obj = Object::New(isolate);
	obj->Set(String::NewFromUtf8(isolate, "pointer"), Number::New(isolate,rPtr));
	args.GetReturnValue().Set(obj);
}

void glue_inner(const FunctionCallbackInfo<Value>& args,void (*foo)(struct cat_message* ts)) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if (args.Length() != 1) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Wrong number of arguments")));
		return;
	}

	printf("%s\n", "glue_inner");
	long ptr=args[0]->NumberValue();
	cat_message* p= reinterpret_cast<cat_message*>(ptr);
	foo(p);

	args.GetReturnValue().Set(Number::New(isolate,0));
}

void glue_fork(const FunctionCallbackInfo<Value>& args) {
	glue_inner(args,&ts_fork);
}

void glue_join(const FunctionCallbackInfo<Value>& args) {
	glue_inner(args,&ts_join);
}

void glue_new_event(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if (args.Length() != 3) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Wrong number of arguments")));
		return;
	}

	printf("%s\n", "glue_new_event");
	long ptr=args[0]->NumberValue();
	cat_message* p= reinterpret_cast<cat_message*>(ptr);
	char* type=parse_args(args[1]);
	char* name=parse_args(args[2]);
	cat_message *message = new_event(type,name,p);

	long rPtr= reinterpret_cast<long>(message);
	Local<Object> obj = Object::New(isolate);
	obj->Set(String::NewFromUtf8(isolate, "pointer"), Number::New(isolate,rPtr));
	args.GetReturnValue().Set(obj);
}

void glue_set_domain(const FunctionCallbackInfo<Value>& args) {

}

void glue_set_server(const FunctionCallbackInfo<Value>& args) {

}

void glue_new_transaction(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if (args.Length() != 2) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Wrong number of arguments")));
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

void Init(Handle<Object> exports) {
	/* Method Glue */
	/* message.js */
	NODE_SET_METHOD(exports, "glue_complete", glue_complete);
	NODE_SET_METHOD(exports, "glue_add_data", glue_add_data);
	NODE_SET_METHOD(exports, "glue_set_status", glue_set_status);

	/* transaction.js */
	NODE_SET_METHOD(exports, "glue_subTransaction", glue_subTransaction);
	NODE_SET_METHOD(exports, "glue_fork", glue_fork);
	NODE_SET_METHOD(exports, "glue_join", glue_join);
	NODE_SET_METHOD(exports, "glue_new_event", glue_new_event);

	/* cat.js */
	NODE_SET_METHOD(exports, "glue_set_domain", glue_set_domain);
	NODE_SET_METHOD(exports, "glue_set_server", glue_set_server);
	NODE_SET_METHOD(exports, "glue_new_transaction", glue_new_transaction);
	NODE_SET_METHOD(exports, "glue_log_event", glue_log_event);
	NODE_SET_METHOD(exports, "glue_log_error", glue_log_error);
}

char *parse_args(v8::Local<v8::Value> value) {
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

