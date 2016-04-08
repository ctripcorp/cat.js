var cat=require('../');

var span=cat.span('[type]','[name]');
setTimeout(function(){
	span.event('[type]','[name]','[data]');
	span.error('[message]','[stack]');
	setTimeout(function(){
		span.end();
	},1500);
},1000);

setInterval(function(){},1000);
