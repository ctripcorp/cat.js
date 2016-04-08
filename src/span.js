var util = require("util");
var addon = require('bindings')('ccat');
var Message = require('./message');
var helper = require('./helper.js');

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

var spanProto=Span.prototype;

/**
 * @api {function call} span(type,name) span
 * @apiName span
 * @apiGroup Span
 * @apiDescription create a sub span from self.
 * @apiParam {String} type
 * @apiParam {String} name
 * @apiExample {curl} Example usage:
 * var t = cat.span("Type","Name");
 * var t_sub=t.span("Type","Name");
 */
 spanProto.span= function(type,name){
 	if(this._end) return;

 	var obj=addon.glue_subTransaction(this._id,type,name);

 	var span=new Span(obj.pointer);

 	return span;
 }

/**
 * @api {function call} event(type,name,status,[,data]) event
 * @apiName event
 * @apiGroup Span
 * @apiDescription create an event on current span
 * @apiParam {String} type
 * @apiParam {String} name
 * @apiParam {String} status
 * @apiParam {String} data
 * @apiExample {curl} Example usage:
 * var t = cat.span("Type","Name");
 * t.event("Type","Name");
 */
 spanProto.event= function(type, name, status, data){
 	if(this._end) return;

 	var obj = addon.glue_new_event(this._id, type, name, status);
 	var cat_event = new Message(obj.pointer);
 	if(arguments.length == 4){
 		cat_event.addData(data);
 	}
 	return cat_event;
 }

/**
 * @api {function call} error(error) error
 * @apiName error
 * @apiGroup Span
 * @apiDescription log an error in this span
 * @apiParam {String} error
 * @apiExample {curl} Example usage:
 * var t=cat.span("Type","Name");
 * t.error(new Error("error message"));
 */
 spanProto.error= function(err){
 	if(this._end) return;

 	if(err instanceof Error){
 		var type = helper.extract_type(err);

 		return this.event(type,err.message,"0",err);
 	}else{
 		throw new Error("you can't log an error which type isn't Error");
 	}
 }

/**
* @api {function call} timeout(timeout) timeout
* @apiName timeout 
* @apiGroup Span
* @apiDescription set the timeout in millisecond
* @apiParam {int} timeout
* @apiExample {curl} Example usage:
* var t = cat.span("Type","Name");
* t.timeout(3);
*/
spanProto.timeout =function(timeout_sec){
	if(this._end) return;
	
	if(this._timer){
		clearTimeout(this._timer);
	}

	var trans = this._id;
	this._timer = setTimeout(function() {
		addon.glue_timeout(trans);
	}, timeout_sec * 1000);
	
	addon.glue_settimeout(trans);
}

module.exports=Span;