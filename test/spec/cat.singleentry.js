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
	it("many transaction",function(){
		var t = cat.span("trans","root");
		for(var i=0;i<3;i++){
			var sub = t.span("trans","subT1");
			pi();
			for(var j=0;j<3;j++){
				var sub1 = sub.span("trans","subT2");
				pi();
				sub1.end();

				var t1 = cat.span("trans","root");
				t1.end();
			}
			sub.end();
		}

		t.end();
	});
});