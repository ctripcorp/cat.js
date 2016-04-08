var http=require('http');
var path=require('path');
var cat=require('../');

function handler(req,res){
	res.writeHead(200,{
		'Content-Type':'text/html'
	});
	res.end('hello world');
}

var server=http.createServer(handler);
cat.http(server);
server.listen(8567);

console.log('Porcess ID: ',process.pid);