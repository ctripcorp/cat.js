var addon = require('bindings')('ccat');
var debug=true;

function Message(message_id){
	this._id = message_id;
}

Message.prototype={

	/**
	 * @api {function call} end(status) end
	 * @apiName end
	 * @apiGroup Message
	 * @apiDescription end a message with status.
	 * @apiParam {String} status message status, set "0" = success, others = fail
	 * @apiExample {curl} Example usage:
	 * message.end();
	 * message.end(err);
	 */
	end : function(status){

		/*
		 * if status not defined, set to "0" as success
		 */
		if(!arguments[0]) status = "0";
	
		return addon.glue_complete(this._id,status);
	},

	/**
	 * @api {function call} addData(key[,value[,...]]) addData
	 * @apiName addData
	 * @apiGroup Message
	 * @apiParam {String} key 如果没有传value，key是一个健值对，例如：“name=stur”
	 * @apiParam {String} value  
	 * @apiExample {curl} Example usage:
	 * message.addData("name=stur&sex=boy");
	 * message.addData("stur","boy","coco","girl");
	 */
	addData : function(key,value){
	 	var keyValuePair;
	 	if(arguments.length==1){
	 		keyValuePair=key.toString();
	 	}

	 	if(arguments.length%2 == 0 && arguments.length>=2){

	 		for (var i = 0; i <arguments.length; i+=2) {
	 			var temp=arguments[i]+"="+arguments[i+1];
	 			keyValuePair+=temp;
	 		};
	
	 	}

		addon.glue_add_data(this._id, keyValuePair);
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
		addon.glue_set_status(this._id,status);
	}
}

module.exports=Message;