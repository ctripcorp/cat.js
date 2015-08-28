var http = require('http');
var cat = require("../src/cat");



http.createServer(function (request, response) {
  var t = cat.span("root","http");

  response.writeHead(200, {'Content-Type': 'text/plain'});
  
  t.event("response","helloworld","0","");
  response.end('Hello World\n');

  t.end();
}).listen(8888);

console.log('Server running at http://127.0.0.1:8888/');