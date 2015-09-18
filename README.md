# cat.js
Javascript client of CAT

This is nodejs addon module which will do node-gpy compile when you install modules

### support platform
 Mac/Linux/Windows

### support node version
 node 0.12, node 4.0

### config file

create app.config.js with content

```javascript
module.exports={
	'AppDomain':'nodejs',
	'CatServer':["127.0.0.1"]
};
```	

### Usage

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
