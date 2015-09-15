var addon = require('bindings')('ccat');
var path= require('path');
var appConfig=require(path.resolve(__dirname,'./appConfig.js'));
var Span=require('./span');
var helper = require('./helper.js');

function Cat(option) {
	this._initConfig();
}

Cat.prototype={

	_initConfig : function(){
		
		var _domain = appConfig['domain'] || "catjs";
		addon.glue_set_domain(_domain);

		var _server = appConfig['server'] || ["127.0.0.1"];
		addon.glue_set_server(_server);

		var _level = appConfig['log_level'] || 1;
		addon.glue_set_log_level(_level);

		var _enable = appConfig['enable'] || 1;
		addon.glue_disable(_enable);
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
	 * @api {function call} error(err) error
	 * @apiName error
	 * @apiGroup Cat
	 * @apiDescription create an error and send immediatelly
	 * @apiParam {String} err
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
	 * @api {function call} http(server) http
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

