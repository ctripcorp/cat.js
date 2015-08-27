
// describe("scene",function(){

// 	xit("Read File",function(){

// 	});

// 	xit("console time",function(){
// 		/*
// 		var i;
// 		console.time("dbsave");

// 		for(i = 1; i < LIMIT; i++){
// 			db.users.save({id : i, name : "MongoUser [" + i + "]"}, end);
// 		}

// 		end = function(err, saved) {
// 			console.log(( err || !saved )?"Error":"Saved");
// 			if(--i === 1){console.timeEnd("dbsave");}
// 		};
// 		*/
// 	});

// 	xit("talked sample",function(){
// 		/*
// 			module.exports = function(..., callback) {

// 				fs.readFile('a.txt', function() {

// 					var st = readFileTS.newTranscation();

// 			// ...

// 			st.complete();
// 		});

// 				callback();
// 			}
// 			*/
// 		});

// 	xit("http handler",function(){
// 		/*
// 		var http = require("http"),
// 		url = require("url"),
// 		path = require("path"),
// 		fs = require("fs"),
// 		port = process.argv[2] || 8888;

// 		http.createServer(function(request, response) {

// 			var uri = url.parse(request.url).pathname
// 			, filename = path.join(process.cwd(), uri);

// 			fs.exists(filename, function(exists) {
// 				if(!exists) {
// 					response.writeHead(404, {"Content-Type": "text/plain"});
// 					response.write("404 Not Found\n");
// 					response.end();
// 					return;
// 				}

// 				if (fs.statSync(filename).isDirectory()) filename += '/index.html';

// 				fs.readFile(filename, "binary", function(err, file) {
// 					if(err) {        
// 						response.writeHead(500, {"Content-Type": "text/plain"});
// 						response.write(err + "\n");
// 						response.end();
// 						return;
// 					}

// 					response.writeHead(200);
// 					response.write(file, "binary");
// 					response.end();
// 				});
// 			});
// 		}).listen(parseInt(port, 10));

// 		console.log("Static file server running at\n  => http://localhost:" + port + "/\nCTRL + C to shutdown");
// 		*/
// 	});

// xit("fork",function(){
	
// 	var root=cat.newTransaction('type','name');
// 	var readFileTS=root.newTransaction('type','name'); 

// 	var data = {subject: "test"};
// 	data = require('querystring').stringify(data);
// 	var opt = {method: "POST"};

// 	end = function(err, saved) {
// 		console.log(( err || !saved )?"Error":"Saved");
// 	};


// 	var req = http.request(opt, function (serverFeedback) {
// 		if (serverFeedback.statusCode == 200) {
// 			var body = "";
// 			serverFeedback.on('data', function (data) { 
// 				body += data;
// 			}).on('end', function () { 
// 				for (var i = 0; i < 10; i++) {
// 					readFileTS.fork();

// 					fs.readFile(i + '.txt', function(err, data) {
// 						if (random) {
// 							fs.readFile('log.txt', function(err, data) {

// 								var parseTS = readFileTS.newTransaction('type', 'name');
// 								parse(data);
// 								parseTS.complete();

// 								readFileTS.join();
// 							});
// 						}
// 						else {
// 							readFileTS.join();
// 						}
// 					});
// 				}

//				root.join();

// 				var dbTS = root.newTransaction('type', 'name');
// 				db.users.save({id : 1, name : "MongoUser1"}, function(){
// 					end();
// 					dbTS.complete();
// 				});

// 				/*
// 				 * 如果transaction 被fork则taransaction需要等待join执行
// 				 */
// 				root.fork();
// 				db.users.save({id : 2, name : "MongoUser2"}, function(){
// 					end();
// 					root.join();
// 				});
// 			});
// 		}
// 		else {
// 			root.complete();
// 			res.send(500, "error");
// 		}
// 	});
// 	req.write(data + "\n");
// 	req.end();

// 	/*
// 	 *	root 写在这里是因为不知道这段代码最后的结束点，所以root.complete写在哪里都可以.
// 	 */
// 	root.complete(); 	
// 	});

// xit("fork",function(){
// 		/*
// 		var root=cat.newTransaction('type','name');
// 		var readFileTS=root.newTransaction('type','name');

// 		root.join = function(err) {
// 			if (this.joined == 10) {}
// 			readFileTS.complete(err);
// 			}
// 		}


// 		for (var i = 0; i < 10; i++) {

// 			root.fork();
// 			fs.readFile(i + '.txt',function(err,data){
// 				var parseTS=readFileTS.newTransaction('type','name');
// 				....
// 				parseTS.complete();

// 				root.join(err);

// 				readFile002();
// 			});
// 		}
// 		*/
// 	});

// xit("readfile",function(){

// 	var timeout=false;
// 	fs.readFile('001.txt',function(err,data){
// 		if (timeout){
// 			return;
// 		}
// 		clearTimeout(clock);
// 		....
// 		readFile002();
// 	});

// 	var clock=setTimeout(function(){
// 		timeout=true;
// 		.....
// 		readFile002();
// 	},3000);

// 	function readFile002(){
// 		var timeout2=false;

// 		fs.readFile('002.txt',function(){
// 			if (timeout2){
// 				return;
// 			}
// 			clearTimeout(clock2);
// 		})

// 		var clock2=setTimeout(function(){
// 			timeout2=true;
// 			.....
// 		},3000);
// 	}

// });

// xit("timeout",function(){
// 		/*
// 		var root=cat.newTransaction('type','name');
// 		var timeout=false;
// 		var readFileTS=root.newTransaction('type','name');
// 		readFileTS.onComplete = function() {
// 			readFile002();
// 		};
// 		readFileTS.setTimeout(3000);

// 		fs.readFile('001.txt',function(err,data){
// 			// if (timeout){
// 			//	return;
// 			// }
// 			// clearTimeout(clock);
			
// 			if (readFileTS.isCompleted) return;

// 			readFileTS.complete(err);
// 			var parseTS=root.newTransaction('type','name');
// 			....
// 			parseTS.complete();
// 			// readFile002();
// 		});

// 		// var clock=setTimeout(function(){
// 		//	timeout=true;
// 		//	readFileTS.complete("Timeout");
// 		//	.....
// 		//	readFile002();
// 		// },3000);

// 		function readFile002(){
// 			var timeout2=false;

// 			var readFileTS2=root.newTransaction('type','name');
// 			fs.readFile('002.txt',function(){
// 				if (timeout2){
// 					return;
// 				}
// 				clearTimeout(clock2);
// 				readFileTS2.complete();
// 				root.complete();
// 			})

// 			var clock2=setTimeout(function(){
// 				timeout2=true;
// 				readFileTS2.complete('Timeout');
// 				.....
// 				root.complete();
// 			},3000);
// 		}
// 		*/
// 	});
// });