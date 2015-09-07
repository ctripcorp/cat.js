var http = require('http');
var fs=require("fs"); 
var cat = require("../../src/cat");

describe("cat client suite",function(){
	it("Transaction", function(){
		var fileName = '../../src/ccat.cc';
		var t = cat.span('RedFile', fileName);
		fs.readFile(fileName, function(err, data) {
			if(err){
				t.error(err);
			}
			t.end();
		});	
	});

	it("Sub Transaction",function(){
		var fileName='../../src/ccat.cc';
		var t = cat.span('ReadFile', fileName);
		var subSpan = t.span('ReadFile', fileName);
		fs.readFile(fileName, function(err, data) {
			t.event("ReadFile", fileName, "0");
			if(err){
				t.error(err);
			}

			fs.readFile("not exist", function(err, data) {
				if(err){
					subSpan.error(err);
				}
				subSpan.end();
			});	
			
		});	
		t.end();
	});

	it("timeout",function(){
		var fileName='../../src/ccat.cc';
		var t=cat.span('ReadFile',fileName);
		fs.readFile(fileName, function(err, data) {
			if(err){
			}
			setTimeout(function() {
				t.end(); 
			}, 20000);
		});	
	});

	xit("sub trans timeout",function(){
		var fileName='../../src/ccat.cc';
		var t=cat.span('ReadFile',fileName);
		var subSpan=t.span('ReadFile',fileName);
		fs.readFile(fileName, function(err, data) {
			t.event("ReadFile", fileName, "0");
			if(err){
				t.error(err);
			}
			fs.readFile(fileName, function(err, data) {
				if(err){
					t.error(err);
				}
				//setTimeout is a  mock for cost 2min to read file
				setTimeout(function() {
					subSpan.end();
				}, 8000);
			});	
		});	
		t.timeout(5)
		t.end();
	});

	xit("express",function(){

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

	xit("express middleware",function(){
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
