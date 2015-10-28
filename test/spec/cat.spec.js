var http = require('http');
var fs=require("fs"); 
var cat = require("../../src/cat");

xdescribe("cat client suite",function(){
	jasmine.DEFAULT_TIMEOUT_INTERVAL = 60000;

	it("Transaction", function(done){
		var fileName = '../../src/ccat.cc';
		var t = cat.span('Transaction', fileName);
		fs.readFile(fileName, function(err, data) {
			if(err){
				t.error(err);
			}
			t.end();
			done();
		});	
	});

	it("Sub Transaction",function(done){
		var fileName='../../src/ccat.cc';
		var t = cat.span('SubTransaction', fileName);
		var subSpan = t.span('ReadFile', fileName);
		fs.readFile(fileName, function(err, data) {
			t.event("ReadFile", fileName, "0");
			if(err){
				t.error(err);
			}

			fs.readFile("not exist", function(err, data) {
				if(err){
					subSpan.error(err);
				}
				subSpan.end();
				done();
			});	
			
		});	
		t.end();
	});

	it("timeout",function(done){
		var fileName='../../src/ccat.cc';
		var t=cat.span('Timeout',fileName);
		fs.readFile(fileName, function(err, data) {
			if(err){
			}
			setTimeout(function() {
				t.end(); 
				done();
			}, 20000);
		});	
	});

	it("sub trans timeout",function(done){
		var fileName='../../src/ccat.cc';
		var t=cat.span('ReadFile',fileName);
		var subSpan=t.span('ReadFile',fileName);
		fs.readFile(fileName, function(err, data) {
			t.event("ReadFile", fileName, "0", "filedata");
			if(err){
				t.error(err);
			}
			fs.readFile(fileName, function(err, data) {
				if(err){
					t.error(err);
				}
				//force root timeout before sub finish
				setTimeout(function() {
					subSpan.end();
					done();
				}, 8000);
			});	
		});	
		t.timeout(5);
		t.end();
	});

});

describe("standalone suite",function(){
	it("sub trans timeout",function(done){
		var fileName = '../../src/ccat.cc';
		var t = cat.span('Transaction', fileName);
		fs.readFile(fileName, function(err, data) {
			if(err){
				t.error(err);
			}
			t.end();
			done();
		});	
	});
});
