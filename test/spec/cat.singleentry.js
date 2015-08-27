var http = require('http');
var fs=require("fs"); 
var cat = require("../../src/cat");

//耗时运算，计算pi

function pi()
{
	var c = 10000000000;
	var Pi=0;
	var n=1;
	for (i=0;i<=c;i++)
	{
		Pi=Pi+(4/n)-(4/(n+2));
		n=n+4;
		//console.log(Pi);
	}
	console.log(Pi);
}

describe("sigle suite",function(){
	it("read file",function(){
		var fileName='../../src/ccat.cc';
		var t=cat.span('ReadFile',fileName);
		var subSpan=t.span('ReadFile',fileName);
		fs.readFile(fileName, function(err, data) {
			t.event("ReadFile",fileName);
			if(err){
			}
			fs.readFile(fileName, function(err, data) {
				if(err){
				}
				
				pi();
				subSpan.end();
			});	
			
		});	
		t.end();
	});
});