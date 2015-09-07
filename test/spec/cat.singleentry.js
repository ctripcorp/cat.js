var http = require('http');
var fs=require("fs"); 
var cat = require("../../src/cat");

describe("sigle suite",function(){
	var fileName='../../src/ccat.cc';
	var t = cat.span('ReadFile',fileName);
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