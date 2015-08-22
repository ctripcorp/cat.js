var addon = require('bindings')('ccat');
var path= require('path');
var appConfig=require(path.resolve(__dirname,'./appConfig.js'));
var Transaction=require('./transaction');

/**
 * @api {constructor} Cat([optoin]) constructor
 * @apiName Cat
 * @apiGroup Cat
 * @apiDescription create a root transaction in express middleware, and auto complete on request finish
 * @apiParam {String} option, always 'combined' 
 * @apiExample {curl} Example usage:
 * var express = require('express');
 * var app = express();
 * //自动在请求的结束埋点，
 * var rootTS = cat.newTransaction('type','name')
 *		app.use(rootTS('combined')); *	app.get('/', function (req, res) {
 *		res.send('hello, world!')
 *	});
 */
function Cat(option) {
	this._initConfig();

	/* use for express 自动埋点
	if(arguments[0]=='combined'){
		var rootTS = addon.glue_new_transaction("type",name);
		return log(req,res,next){
			req.on('socket',function(){
				rootTS.complete();
			});
		};
	}
	*/
}

Cat.prototype={

	_initConfig : function(){
		if (appConfig['domain']){
			console.log("domain:"+appConfig['domain']);
			//TODO
			//addon.glue_set_domain(appConfig['domain']);
		}

		if (appConfig['server']){
			console.log("server:"+appConfig['server'][0]);
			//TODO
			//addon.glue_set_server(appConfig['server'][0]);
		}
	},

	/**
	 * @api {function call} newTransaction(type,name) newTransaction
	 * @apiName newTransaction
	 * @apiGroup Cat
	 * @apiDescription create a standalone transaction, usually root transaction , this transaction must explicit complete
	 * @apiParam {String} type
	 * @apiParam {String} name
	 * @apiExample {curl} Example usage:
	 * var ts_root=cat.newTransaction("Type","Name");
	 */
	newTransaction : function(type,name){
		var obj=addon.glue_new_transaction("type",name);
		var msg_id=obj.pointer;
		console.log("Message ID:",msg_id);
		var transaction=new Transaction(msg_id);
		return transaction;
	},

	/**
	 * @api {function call} logEvent(type,name[,key[,value]]) logEvent
	 * @apiName logEvent
	 * @apiGroup Cat
	 * @apiDescription create an event and send immediatelly
	 * @apiParam {String} type
	 * @apiParam {String} name
	 * @apiParam {String} key
	 * @apiParam {String} value
	 * @apiExample {curl} Example usage:
	 * cat.logEvent("type","name");
	 * cat.logEvent("type","name","key","value");
	 */
	logEvent : function(type,name,key,value){
		var keyValuePair;
	 	if(arguments.length==3){
	 		keyValuePair=key;
	 	}

	 	if(arguments.length==4){
	 		keyValuePair=key+"="+value;
	 	}

		//TODO
		//addon.glue_log_event(type,name,keyValuePair);
	},

	/**
	 * @api {function call} logError(error) logError
	 * @apiName logError
	 * @apiGroup Cat
	 * @apiDescription create an error and send immediatelly
	 * @apiParam {String} error
	 * @apiExample {curl} Example usage:
	 * cat.logError("exception");
	 */
	logError : function(error){
		var exceptionType="Exception";
		//TOTO
		//read exception type from error
		//TODO
		//addon.glue_log_error(exceptionType,keyValuePair);
	}
}

module.exports=new Cat();

