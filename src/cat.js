var addon = require('bindings')('ccat');
var path= require('path');
var http = require('http');
var appConfig=require(path.resolve(__dirname,'./appConfig.js'));
var Span=require('./span');
var helper = require('./helper.js');
var request = require('sync-request');

function Cat(option) {
	this._initConfig();
}

Cat.prototype={
	_sizeTable : [1 << 6,  1 << 7,  1 << 8,  1 << 9, 
                        1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14, 1 << 15, 1 << 16, 1 << 17, 1 << 18, 1 << 19, 
                        1 << 20, 1 << 21, 1 << 22, 1 << 23, 1 << 24, 1 << 25, 1 << 26, 1 << 27, 1 << 28],

    _valueTable : ["64",   "128",   "256",   "512", 
                          "1K",    "2K",    "4K",    "8K",   "16K",   "32K",   "64K",  "128K",  "256K",  "512K", 
                          "1M",    "2M",    "4M",    "8M",   "16M",   "32M",   "64M", "128M", "256M"],

	_initConfig : function(){
		
		var _domain = appConfig['AppDomain'] || "catjs";
		addon.glue_set_domain(_domain);

		var _level = appConfig['log_level'] || 1;
		addon.glue_set_log_level(_level);

		var _enable = appConfig['enable'] || 1;
		addon.glue_disable(_enable);

		var obj = addon.glue_get_config_url();
		var url = obj.config;

		var res = request('GET', url);
		var body = res.getBody();
		var fbResponse = JSON.parse(body);

	    for (var i = 0; i < fbResponse.length; i++) {
	    	if(fbResponse[i].Name=="CAT_SERVER"){
	    		console.log(fbResponse[i].Value);

	    		var router = fbResponse[i].Value;
	    		var resInner = request('GET', router);
	    		var innerResponse = resInner.getBody();
		        var servers=[];
				var arr = innerResponse.toString().split(";");
				for(var j=0;j<arr.length;j++){
					var temp = arr[j].split(":");
					console.log(temp[0]);
					servers.push(temp[0]);
				}

				var _server = appConfig['CatServer'] || servers;
				addon.glue_set_server(_server);
	    	}
	    }

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


	 _GetSizeScale : function (size) {
		var sizeTable = this._sizeTable;
		var valueTable = this._valueTable;

        if (size < sizeTable[0]) return "0~" + valueTable[0];

        for (var i = 1; i < sizeTable.length; i++)
        {
            if (size < sizeTable[i])
                return valueTable[i - 1] + "~" + valueTable[i];
        }

        return ">=" + valueTable[valueTable.length - 1];
	 },

	/**
	 * @api {function call} sizeEvent(name,size) sizeEvent
	 * @apiName sizeEvent
	 * @apiGroup Cat
	 * @apiDescription log size event
	 * @apiParam {String} name
	 * @apiParam {Long} size
	 * @apiExample {curl} Example usage:
	 * cat.sizeEvent("aa",133);
	 */
	 sizeEvent : function(name,size){
	 	var scale = this._GetSizeScale(size);
	 	this.event(name, scale, "0", "size=" + size);
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

