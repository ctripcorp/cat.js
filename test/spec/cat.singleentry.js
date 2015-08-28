var http = require('http');
var fs=require("fs"); 
var cat = require("../../src/cat");

function pi()
{
	var c = 100000000;
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
	it("add data",function(){
		var t = cat.span("trans","root");
		//t.event("type","name","0","stur=boy");

		t.end();
	});
});