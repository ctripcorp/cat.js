var util = require("util");

var addon = require('bindings')('ccat');
var Message=require('./message');

/**
 * @api {constructor} Span(message_id) constructor
 * @apiName Span
 * @apiGroup Span
 * @apiDescription set span id
 * @apiParam {String} message_id, if message_id = 'combined' this span will atuo complete on request end, this scenario is use in experess 
 * @apiExample {curl} Example usage:
 * var express = require('express');
 * var app = express();
 *	//自动在请求的开始和结束埋点
 *	app.use(cat('combined'));
 *	app.get('/', function (req, res) {
 *		res.send('hello, world!')
 *	});
 */
function Span(message_id){
	Message.apply(this,arguments);

	/* use for express 自动埋点
	if(arguments[0]=='combined'){
		return log(req,res,next){
			req.on('socket',function(){
				rootTS.complete();
			});
		};
	}
	*/
}

util.inherits(Span, Message);

var tsProto=Span.prototype;

/**
 * @api {function call} subSpan(type,name) subSpan
 * @apiName subSpan
 * @apiGroup Span
 * @apiDescription create a sub span from self.
 * @apiParam {String} type
 * @apiParam {String} name
 * @apiExample {curl} Example usage:
 * var ts_root=cat.newTransaction("Type","Name");
 * var ts_sub=ts_root.subSpan("Type","Name");
 */
tsProto.subSpan= function(type,name){
	this._debugLog("Span["+this._id+"]"+" create subSpan");
	
	var sub_ts_id=0;

	sub_ts_id=addon.glue_subTransaction(this._id,type,name);
	var subSpan=new Span(sub_ts_id);
	return subSpan;
}

/**
 * @apiDefine fork_join
 *
 * @apiDescription do fork() before an asynchronize call and do join() after callback finish.
 *	fork() 和 join() 总是成对出现
 * @apiExample {curl} Example usage:
 *	var ts_root=cat.new_transaction("Type","Name");
 *	for (var i = 0; i < 10; i++) {
 *		ts_root.fork();
 *		fs.readFile('../src/ccat.cc', function(err, data) {			
 *			//if all branch have join, ts_root will complete self
 *			ts_root.join();
 *		});
 *	}	
 *	//ts_root won't send here, it will wait all fork branch finish
 *	ts_root.complete();
 */

/**
 * @api {function call} fork() fork
 * @apiName fork
 * @apiGroup Span
 * @apiUse fork_join
 */
tsProto.fork= function(){
	this._debugLog("Span["+this._id+"]"+" fork invoke");
	addon.glue_fork(this._id);
}

/**
 * @api {function call} join() join
 * @apiName join
 * @apiGroup Span
 * @apiUse fork_join
 */
tsProto.join= function(){
	this._debugLog("Span["+this._id+"]"+" join invoke");

	addon.glue_join(this._id);
}

/**
 * @api {function call} logEvent(type,name[,key[,value]]) logEvent
 * @apiName logEvent
 * @apiGroup Span
 * @apiDescription create an event on current span
 * @apiParam {String} type
 * @apiParam {String} name
 * @apiParam {String} key
 * @apiParam {String} value
 * @apiExample {curl} Example usage:
 * var ts_root=cat.new_transaction("Type","Name");
 * ts_root.logEvent("Type","Name");
 */
 tsProto.logEvent= function(type,name,key,value){
	this._debugLog("Span["+this._id+"]"+" log event");
	var event_id=0;
	
	event_id=addon.glue_new_event(this._id,type,name);
	var cat_event=new Message(event_id);
	if(arguments.length==3){
		cat_event.addData(key);
	}
	if(arguments.length==4){
		cat_event.addData(key,value);
	}
	return cat_event;
}

/**
 * @api {function call} logError(error) logError
 * @apiName logError
 * @apiGroup Span
 * @apiDescription log an error in this span
 * @apiParam {String} error
 * @apiExample {curl} Example usage:
 * var ts_root=cat.new_transaction("Type","Name");
 * ts_root.logError("Exception");
 */
 tsProto.logError= function(error){
	this._debugLog("Span["+this._id+"]"+" log event");
	var exceptionType="Exception";
	
	//read exception type from error
	this.logEvent("Error",exceptionType,"0",error);
	this.setStatus(exceptionType);
}

module.exports=Span;