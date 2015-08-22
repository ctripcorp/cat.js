
var http = require('http');
var fs=require("fs"); 

var cat = require("../src/cat");

describe("cat client suite",function(){
	it("sub transaction",function(){
		var t=cat.span("Type","Name");
		var ts=t.span("Type","Name");
		// ...
		ts.end();
		t.end();
	});

	it("log",function(){
		//event and error with transaction
		var t=cat.span("Type","Name");
		t.event("Type","Name");
		//t.logError(err);
		t.end();

		//event and error send immediately
		cat.event("type","name");
		cat.event("type","name","status","key1=value1&key2=value2");
		//cat.logError(err);
	});

	it("general",function(){
		var t=cat.span("Type","Name");
		fs.readFile('../src/ccat.cc', function(err, data) {
			if(err){
				t.logError(err);
			}
			t.end(err);
		});
	});

	it("fork&join",function(){
		var t=cat.span("Type","Name");
		for (var i = 0; i < 5; i++) {
			ts=t.span("","");

			fs.readFile('../src/ccat.cc', function(err, data) {
				if(err){
					logError(err);
					ts.end();
					return;
				}
				/*
				 * if all branch have joined, t will complete self
				 */
				 // ts.timeout(2000)
				 ts.end();
				});
				
		}
		/*
		 * t won't send here, it will wait all fork branch finish
		 */
		 t.timeout(1000);
		 t.end();
		});

	it("express",function(){
		var express = require('express');
		var app = express();

		app.get('/', function (req, res) {
			var t=cat.span("Express","Name");
			res.send('Hello World!');
			t.end();
		});


	});

	it("cat express middleware",function(){
		/*
		var express = require('express');
		var app = express();
		
		//自动在请求的结束埋点，
		var rootTS = cat.span('type','name')
		app.use(rootTS('combined'));

		app.get('/', function (req, res) {
			res.send('hello, world!')
		});
		*/
	});

	it("cat express middleware like morgan",function(){
		/*
		var express = require('express');
		var app = express();
		
		//自动在请求的开始和结束埋点
		app.use(cat('combined',"type","name"));

		app.get('/', function (req, res) {
			res.send('hello, world!')
		});

		app.post("/", function(req, res){
	
		});
		*/
	});

	it("express middleware",function(){
		var express = require('express');

		var app = express();
		app.use(function(req, res, next) {
			console.log('%s %s', req.method, req.url);
			next();
		});

		app.get('/', function(req, res, next) {
			console.log("enter");
			res.send('Hello World!');
			console.log("finish");
		});

		app.get('/help', function(req, res, next) {
			res.send('Nope.. nothing to see here');
		});

		var server = app.listen(3000, function () {
			var host = server.address().address;
			var port = server.address().port;

			console.log('Example app listening at http://%s:%s', host, port);
		});
	});
});

describe("comment",function(){
/*
	xit("console time",function(){
		var i;
		console.time("dbsave");

		for(i = 1; i < LIMIT; i++){
			db.users.save({id : i, name : "MongoUser [" + i + "]"}, end);
		}

		end = function(err, saved) {
			console.log(( err || !saved )?"Error":"Saved");
			if(--i === 1){console.timeEnd("dbsave");}
		};
	});


	xit("Send Test",function(){
		cat.NewTransaction("static","tt");
	});

	it("Transaction",function(){
		
	});

	xit("Log Event",function(){
		expect("success").toEqual(cat.LogEvent("aaa","bbb"));
	});

	xit("Init Domain",function(){
		cat.InitWithDomain("stur");
		expect("success").toEqual(cat.NewTransaction("aaa","bbb"));
	});

	xit("HTTP Request",function(){
		var count = 0;
		http.createServer(function (request, response) {
			response.writeHead(200, {'Content-Type': 'text/plain'});
			response.end((++count).toString())
		}).listen(8124);
		console.log('Server running at http://127.0.0.1:8124/');
	});

	xit("Permormance",function(){
		var start = Date.now();//获取当前时间戳
		setTimeout(function () {
			console.log(Date.now() - start);
	    	for (var i = 0; i < 1000000000; i++){//执行长循环
	    	}
	    }, 1000);
		setTimeout(function () {
			console.log(Date.now() - start);
		}, 2000);	
	});
*/

});
