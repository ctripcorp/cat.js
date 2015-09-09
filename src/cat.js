var addon = require('bindings')('ccat');
var path= require('path');
var appConfig=require(path.resolve(__dirname,'./appConfig.js'));
var Span=require('./span');
var helper = require('./helper.js');

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
			addon.glue_set_domain(appConfig['domain']);
		}else{
			addon.glue_set_domain("catjs");
		}

		if (appConfig['server']){
			addon.glue_set_server(appConfig['server']);
		}else{
			addon.glue_set_server(["127.0.0.1"]);
		}

		if (appConfig['log_level']){
			addon.glue_set_log_level(appConfig['log_level']);
		}else{
			addon.glue_set_log_level(1);
		}

		/* TODO:free memory on exit
		process.stdin.resume();//so the program will not close instantly

		function exitHandler(options, err) {
		    if (options.cleanup) console.log('clean');
		    if (err) console.log(err.stack);
		    if (options.exit) process.exit();
		}

		//do something when app is closing
		process.on('exit', exitHandler.bind(null,{cleanup:true}));

		//catches ctrl+c event
		process.on('SIGINT', exitHandler.bind(null, {exit:true}));

		//catches uncaught exceptions
		process.on('uncaughtException', exitHandler.bind(null, {exit:true}));
		*/
	},

	/**
	 * @api {function call} span(type,name) span
	 * @apiName span
	 * @apiGroup Cat
	 * @apiDescription create a standalone span, usually root span , this span must explicit complete
	 * @apiParam {String} type
	 * @apiParam {String} name
	 * @apiExample {curl} Example usage:
	 * var t=cat.span("Type","Name");
	 */
	 span : function(type,name){

	 	var obj = addon.glue_new_transaction(type,name);

	 	var span = new Span(obj.pointer);

	 	/* set default timeout */
	 	if (appConfig['timeout']){
	 		span.timeout(appConfig['timeout']);
	 	}else{
	 		span.timeout(30);
	 	}

	 	return span;
	 },

	/**
	 * @api {function call} event(type,name,status[,key[,value[,...]]]) event
	 * @apiName event
	 * @apiGroup Cat
	 * @apiDescription create an event and send immediatelly
	 * @apiParam {String} type
	 * @apiParam {String} name
	 * @apiParam {String} key
	 * @apiParam {String} value
	 * @apiExample {curl} Example usage:
	 * cat.event("type","name","0","data");
	 * cat.event("type","name","0","key","value");
	 */
	 event : function(type,name,status,key,value){
	 	var keyValuePair;
	 	if(arguments.length==4){
	 		keyValuePair = key.toString();
	 	}

	 	if(arguments.length%2 != 0 && arguments.length>=5){

	 		for (var i = 3; i <arguments.length; i+=2) {
	 			var temp=arguments[i]+"="+arguments[i+1];
	 			keyValuePair+=temp;
	 		};
	
	 	}

	 	addon.glue_log_event(type,name,status,keyValuePair);
	 },

	/**
	 * @api {function call} error(error) error
	 * @apiName error
	 * @apiGroup Cat
	 * @apiDescription create an error and send immediatelly
	 * @apiParam {String} error
	 * @apiExample {curl} Example usage:
	 * cat.error(new Error("error message"));
	 */
	 error : function(err){
	 	if(err instanceof Error){
	 		var type = helper.extract_type(err);

	 		this.event("Error",type,"ERROR",err);
	 	}else{
	 		throw new Error("you can't log an error which type isn't <Error>");
	 	}
	 },

	/**
	 * @api {function} http(server) http
	 * @apiName http
	 * @apiGroup Cat
	 * @apiDescription create a root span on http request start, and auto complete on request finish
	 * @apiParam {object} server
	 * @apiExample {curl} Example usage:
	 * var http = require('http');
     * var server = http.createServer(function (request, response) {
  	 * 		response.writeHead(200, {'Content-Type': 'text/plain'});
  	 * 		response.end('Hello World\n');
     * }).listen(8888);
     * cat.http(server);
	 */
	 http : function(server){
	 	var _cat = this;
	 	server.on("request",function(req, res){
	 		var _root = _cat.span("URL",req.url);
	 		res.on("finish",function(){
	 			_root.end();
	 		});
	 	});
	 }
}

module.exports = new Cat();

