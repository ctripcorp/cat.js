var addon = require('bindings')('ccat');
var sprintf=require("sprintf-js").sprintf;
var debug=true;

function Message(message_id){
	this._id=message_id;
}

Message.prototype={

	/**
	 * @api {function call} complete(status) complete
	 * @apiName complete
	 * @apiGroup Message
	 * @apiDescription complete a message with status.
	 * @apiParam {String} status message status, set "0" = success, others = fail
	 * @apiExample {curl} Example usage:
	 * message.complete();
	 * message.complete(err);
	 */
	complete : function(status){
	 	this._debugLog("message["+this._id.valueOf()+"]"+" complete invoke");

		/*
		 * if status not defined, set to "0" as success
		 */
		 if(!arguments[0]) status = "0";

		addon.glue_complete(this._id,status);
	},

	/**
	 * @api {function call} addData(key[,value]) addData
	 * @apiName addData
	 * @apiGroup Message
	 * @apiParam {String} key 如果没有传value，key是一个健值对，例如：“name=stur”
	 * @apiParam {String} value  
	 * @apiExample {curl} Example usage:
	 * message.addData("name=stur&sex=boy");
	 * message.addData("stur","boy");
	 */
	addData : function(key,value){
	 	this._debugLog("message["+this._id.valueOf()+"]"+" addData invoke");

	 	var keyValuePair;
	 	if(arguments.length==1){
	 		keyValuePair=key;
	 	}

	 	if(arguments.length==2){
	 		keyValuePair=key+"="+value;
	 	}

		//TODO
		//addon.glue_add_data(this._id,keyValuePair);
	},

	/**
	 * @api {function call} setStatus(key[,value]) setStatus
	 * @apiName setStatus
	 * @apiGroup Message
	 * @apiParam {String} status "0" as success 
	 * @apiExample {curl} Example usage:
	 * message.setStatus("ERROR");
	 */
	setStatus : function(status){
	 	this._debugLog("message["+this._id.valueOf()+"]"+" set status invoke");

		//TODO
		//addon.glue_set_status(this._id,status);
	},

	_debugLog : function(msg){
		if(debug){
			console.log(msg);
		}
	}
}

module.exports=Message;