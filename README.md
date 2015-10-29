# cat.js
Javascript client of CAT

This is nodejs addon module which will do node-gpy compile when you install modules

### support platform
 Mac/Linux/Windows

### support node version
 node 0.12+

### config file

create app.config.js with content

minimun:
```javascript
module.exports={
	'AppDomain':'nodejs'
};
```	
full:
```javascript
module.exports={
	'AppDomain':'123',
	'CatServer':["10.2.25.213"],
	'timeout': 10,
	'log_level': 4
};
```	

## Installation

npm install cat-nodejs --registry=http://192.168.19.59:8001

## API Document

http://svn.ui.sh.ctripcorp.com:8081/catjsdoc/

### Usage

Normal:
```javascript
var cat = require("cat-nodejs");

var fileName='file';
var t = cat.span('SubTransaction', fileName); /* Create Transaction t, which is a root transaction */
var subSpan = t.span('ReadFile', fileName); /* Create Sub Transaction of t */
fs.readFile(fileName, function(err, data) {
	t.event("ReadFile", fileName, "0"); /* Create and event of transaction t */
	if(err){
		t.error(err); /* Create and error of transaction t */
	}

	fs.readFile("not exist", function(err, data) {
		if(err){
			subSpan.error(err); /* Create and error of transaction subSpan */
		}
		subSpan.end(); /* complete transaction */
	});	
	
});	
t.end();
```	
For http module:
```javascript
var http = require('http');
var cat = require("cat-nodejs");

var server = http.createServer(function (request, response) {
  
  setTimeout(function(){
  	response.writeHead(200, {'Content-Type': 'text/plain'});
  	response.end('Hello World\n');
  },1000);
  
}).listen(8888);

cat.http(server);

```	