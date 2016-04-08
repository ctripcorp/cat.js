var path=require('path');
var cUtil=require(path.resolve(__dirname,'./cutil.js'));

// Async define
function Async(callback){
	this._enabled=false;
	this._data={};
	this._callback=callback;
	this._errors=[];
}

Async.prototype.add=function(fn){
	var _this=this;
	var args=Array.prototype.slice.call(arguments,1);
	var guid=cUtil.uid();
	this._data[guid]={
		fn:fn,
		args:args,
		done:function(err){
			if (err){
				_this._errors.push(err);
			}
			delete _this._data[guid];
			_this._check();
		}
	};
};

Async.prototype.start=function(){
	this._enabled=true;
	for (var guid in this._data){
		if (this._data.hasOwnProperty(guid)){
			this._data[guid].fn.apply(null,[this._data[guid].done].concat(this._data[guid].args));
		}
	}
	this._check();
};

Async.prototype._check=function(){
	if (this._enabled){
		var finish=true;
		for (var guid in this._data){
			if (this._data.hasOwnProperty(guid)){
				finish=false;
				break;
			}
		}
		if (finish){
			this._callback && this._callback(this._errors.length?this._errors:null);
			this._enabled=false;
		}
	}
};

module.exports=Async;