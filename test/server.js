var http = require('http');
var cat = require("../src/cat");

var server = http.createServer(function (request, response) {
  
  setTimeout(function(){
  	response.writeHead(200, {'Content-Type': 'text/plain'});
  	response.end('Hello World\n');
  },1000);
  
}).listen(8888);

cat.http(server);

console.log('Server running at http://127.0.0.1:8888/');

