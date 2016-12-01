var net=require('net');
var http=require('http');
var url=require('url');
var path=require('path');
var os=require('os');
var cluster=require('cluster');
var appConfig=require(path.resolve(__dirname,'./appConfig.js'));
var cUtil=require(path.resolve(__dirname,'./cutil.js'));
var Async=require(path.resolve(__dirname,'./async.js'));

var defaultPort=2280;
var useRemoteConfig=!appConfig['CatServer'].length;
var appStartTime=+new Date();

var cat={};
var avalible=false;
var dropMessageCount=0;
var waitQueue=[];
waitQueue.push=function(){
	var args=Array.prototype.slice.call(arguments,0);
	Array.prototype.push.apply(waitQueue,args);
};
var sendQueue=[];
sendQueue.push=function(){
	var args=Array.prototype.slice.call(arguments,0);
	Array.prototype.push.apply(sendQueue,args);

	if (sendQueue.length>150){
		var overflowQueue=sendQueue.splice(0,30);
		countMessage(overflowQueue,function(count,total){
			console.log('[cUtil] Drop Cat '+count+' Messages ('+total+')');
			dropMessageCount+=count;
			var error=new CatError(null,'Drop Messages','Total: '+total);
			error.__overCount=count;
		});
	}else{
		setImmediate(sendPackage);
	}
};

function countMessage(items,callback){
	var count=0;
	var total=0;
	items.forEach(function(item){
		switch (item.__type){
			case 'span':
				count++;
				total++;
				countMessage(item.__children,function(subCount,subTotal){
					count+=subCount;
					total+=subTotal;
				});
				break;
			case 'event':
				count++;
				total++;
				break;
			case 'error':
				count+=item.__overCount?0:1;
				total+=item.__overCount||1;
				break;
			default:
				break;
		}
	});
	callback(count,total);
	return count;
}

cat.span=function(type,name,data){
	var span=new CatSpan(null,type,name,data);
	return span;
};

cat.event=function(type,name,data){
	var event=new CatEvent(null,type,name,data);
	return cat;
};

cat.error=function(message,stack){
	var err=createError(message,stack);
	var error=new CatError(null,err.message,err.stack);
	return cat;
};

cat.http=function(server){
	server.on('request',function(req,res){
		var tarUrl=req.url.replace(/[\?#].*$/,'')+' ';
		var span=cat.span('URL',tarUrl);

		var isEnd=false;
		function endSpan(){
			if (!isEnd){
				isEnd=true;
				span.end();
			}
		}

		req.on('error',function(err){
			span.error(err,'Request Error');
			endSpan();
		});
		res.on('error',function(err){
			span.error(err,'Response Error');
			endSpan();
		});
		res.on('close',endSpan);
		res.on('finish',endSpan);
	});

	return cat;
};

function CatSpan(parent,type,name,data){
	var _this=this;
	this.__id=cUtil.uid();
	this.__messageId=parent?parent.__messageId:getMessageId();
	this.__type='span';
	this.__close=false;
	this.__parent=parent;
	this.__root=parent?parent.__root:this;
	this.__startTime=+new Date();
	this.__startMicroSeconds=cUtil.getMicroSeconds();
	this.__children=[];
	this.__timeout=setTimeout(function(){
		_this.error('TIMEOUT');
		_this.end();
	},30000);

	this.type=type||'Unknow Type';
	this.name=name||'Unknow Name';
	this.status='0';
	this.data=data||'';

	if (this.__parent){
		if (this.__parent.__close){
			console.log('[cUtil] Cat span is already end');
		}else{
			this.__parent.__children.push(this);
		}
	}else{
		waitQueue.push(this);
	}
}

CatSpan.prototype.end=function(){
	if (this.__close){
		console.log('[cUtil] Cat span is already end');
	}else{
		clearTimeout(this.__timeout);
		this.__children.forEach(function(item){
			if (item.__type=='span' && !item.__close){
				item.end();
			}
		});
		this.__close=true;
		this.__endTime=+new Date();
		this.__endMicroSeconds=cUtil.getMicroSeconds();
		var idx=waitQueue.indexOf(this);
		if (idx!=-1 && !this.__parent){
			waitQueue.splice(idx,1);
			sendQueue.push(this);
		}
		return this.__parent||cat;
	}
};

CatSpan.prototype.span=function(type,name,data){
	var span=new CatSpan(this,type,name,data);
	return span;
};

CatSpan.prototype.event=function(type,name,data){
	var event=new CatEvent(this,type,name,data);
	return this;
};

CatSpan.prototype.error=function(message,stack){
	var err=createError(message,stack);
	var error=new CatError(this,err.message,err.stack);
	return this;
};

var lastCpuStatus=os.cpus();
function getCpuInfo(){
	var cpuStatus=os.cpus();
	var ret={
		count:cpuStatus.length,
		load:os.loadavg()[0],
		user:0,
		nice:0,
		sys:0,
		used:0,
		idle:100,
		irq:0
	};
	cpuStatus.forEach(function(core,i){
		var lastCore=lastCpuStatus[i];
		if (lastCore){
			coreSum=core.times.user+core.times.nice+core.times.sys+core.times.idle+core.times.irq;
			lastCoreSum=lastCore.times.user+lastCore.times.nice+lastCore.times.sys+lastCore.times.idle+lastCore.times.irq;
			diffSum=coreSum-lastCoreSum;
			ret.user=parseInt((core.times.user-lastCore.times.user)/diffSum*1000,10)/1000;
			ret.nice=parseInt((core.times.nice-lastCore.times.nice)/diffSum*1000,10)/1000;
			ret.sys=parseInt((core.times.sys-lastCore.times.sys)/diffSum*1000,10)/1000;
			ret.idle=parseInt((core.times.idle-lastCore.times.idle)/diffSum*1000,10)/1000;
			ret.irq=parseInt((core.times.irq-lastCore.times.irq)/diffSum*1000,10)/1000;
			ret.used=1-ret.idle;
		}
	});
	return ret;
}

function CatHeartbeat(parent,callback){
	var _this=this;

	var now=+new Date();

	this.__id=cUtil.uid();
	this.__messageId=parent?parent.__messageId:getMessageId();
	this.__type='heartbeat';
	this.__close=true;
	this.__parent=parent;
	this.__root=parent?parent.__root:this;
	this.__startTime=this.__endTime=now;
	this.__startMicroSeconds=this.__endMicroSeconds=cUtil.getMicroSeconds();

	this.type='Heartbeat';
	this.name=cUtil.getHostIp();
	this.status='0';

	cUtil.getDiskInfo(function(err,diskInfo){
		var diskInfoString=(err?[]:diskInfo).map(function(item){
			return '<disk-volume id="'+item.path+'" total="'+item.size+'" free="'+item.freeSpace+'" usable="'+(item.size-item.freeSpace)+'" />';
		}).join('\n');

		var extDiskInfoString=(err?[]:diskInfo).map(function(item){
			return '<extensionDetail id="'+item.path+' Free" value="'+item.freeSpace+'" />';
		}).join('\n');

		var memory=process.memoryUsage();
		var freeMemory=os.freemem();
		var cpu=getCpuInfo();
		var version=process.version.slice(1);

		_this.data=[
			'<?xml version="1.0" encoding="utf-8"?>',
			'<status timestamp="'+formatTime(_this.__startTime).replace(',','.')+'">',
			'<runtime start-time="'+appStartTime+'" up-time="'+(now-appStartTime)+'" java-version="'+version+'" user-name="" file-version="" last-flush-mark="'+formatTime(now).slice(0,19)+'">',
			'<user-dir>'+process.cwd()+'</user-dir>',
			'<java-classpath>node-'+version+'.jar</java-classpath>',
			'</runtime>',
			'<os name="'+os.type()+'" arch="'+os.arch()+'" version="'+os.release()+'" available-processors="'+cpu.count+'" system-load-average="'+cpu.load+'" process-time="0" total-physical-memory="'+os.totalmem()+'" free-physical-memory="'+freeMemory+'" committed-virtual-memory="0" total-swap-space="0" free-swap-space="0" />',
			'<disk>',
			diskInfoString,
			'</disk>',
			'<memory max="'+memory.rss+'" total="'+memory.rss+'" free="'+(memory.heapTotal-memory.heapUsed)+'" heap-usage="'+memory.heapUsed+'" non-heap-usage="'+(memory.rss-memory.heapTotal)+'">',
			'</memory>',
			'<thread count="1" total-started-count="1">',
			'<dump>',
			'</dump>',
			'</thread>', 
			'<message produced="'+msgSeq+'" overflowed="'+dropMessageCount+'" bytes="'+sendBytes+'" />',
			'<extension id="System">',
			'<extensionDetail id="LoadAverage" value="'+cpu.load+'" />',
			'<extensionDetail id="FreePhysicalMemory" value="'+freeMemory+'"/>',
//			'<extensionDetail id="FreeSwapSpaceSize" value="8.50157568E9"/>',
//			'<extensionDetail id="User" value="'+cpu.user+'" />',
//			'<extensionDetail id="Nice" value="'+cpu.nice+'" />',
//			'<extensionDetail id="Sys" value="'+cpu.sys+'" />',
//			'<extensionDetail id="Idle" value="'+cpu.idle+'" />',
//			'<extensionDetail id="Irq" value="'+cpu.irq+'" />',
			'</extension>',
			'<extension id="Disk">',
			extDiskInfoString,
			'</extension>',
			'<extension id="CatUsage">',
			'<extensionDetail id="Produced" value="'+msgSeq+'" />',
			'<extensionDetail id="Overflowed" value="'+dropMessageCount+'" />',
			'<extensionDetail id="Bytes" value="'+sendBytes+'" />',
			'</extension>',
			'<extension id="JVMHeap">',
//			'<extensionDetail id="Gen-0-HeapSize" value="'+memory.rss+'" />',
			'<extensionDetail id="Free" value="'+(memory.heapTotal-memory.heapUsed)+'" />',
			'<extensionDetail id="Heap Usage" value="'+memory.heapUsed+'" />',
			'</extension>',
			'<extension id="LocksAndThreads">',
			'</extension>',
			'</status>'
		].join('\n');

		if (_this.__parent){
			if (_this.__parent.__close){
				console.log('[cUtil] Cat span is already end');
			}else{
				_this.__parent.__children.push(_this);
			}
		}else{
			sendQueue.push(_this);
		}

		callback && callback(_this);
	});
}

function CatEvent(parent,type,name,data){
	this.__id=cUtil.uid();
	this.__messageId=parent?parent.__messageId:getMessageId();
	this.__type='event';
	this.__close=true;
	this.__parent=parent;
	this.__root=parent?parent.__root:this;
	this.__startTime=this.__endTime=+new Date();
	this.__startMicroSeconds=this.__endMicroSeconds=cUtil.getMicroSeconds();

	this.type=type||'Unknown Type';
	this.name=name||'Unknown Name';
	this.status='0';
	this.data=data||'';

	if (this.__parent){
		if (this.__parent.__close){
			console.log('[cUtil] Cat span is already end');
		}else{
			this.__parent.__children.push(this);
		}
	}else{
		sendQueue.push(this);
	}
};

function CatError(parent,message,stack){
	this.__id=cUtil.uid();
	this.__messageId=parent?parent.__messageId:getMessageId();
	this.__type='error';
	this.__close=true;
	this.__parent=parent;
	this.__root=parent?parent.__root:this;
	this.__startTime=this.__endTime=+new Date();
	this.__startMicroSeconds=this.__endMicroSeconds=cUtil.getMicroSeconds();

	this.type='Error';
	this.name=message||'Unknown Name';
	this.status='ERROR';
	var t=this;
	while (t=t.__parent){
		t.status='ERROR';
	}
	this.data=stack||'';

	if (this.__parent){
		if (this.__parent.__close){
			console.log('[cUtil] Cat span is already end');
		}else{
			this.__parent.__children.push(this);
		}
	}else{
		sendQueue.push(this);
	}
}

function createError(err,stack){
	var message='Error Unknown';
	stack=stack||'';
	if (cUtil.type(stack)!='string'){
		stack=JSON.stringify(stack);
	}
	if (err===null){
		message='Error Null';
	}else if (err===void(0)){
		message='Error Undefined';
	}else{
		switch (cUtil.type(err)){
			case 'error':
				message=''+err.message;
				stack=(stack?stack+' ||| ':'')+err.stack;
				break;
			case 'string':
			case 'number':
				message=''+err;
				stack=(stack?stack+' ||| ':'')+err;
				break;
			case 'array':
				var messages=['Error Array'];
				var stacks=[];
				err.map(createError).forEach(function(err){
					messages.push(''+err.message);
					stacks.push('['+err.message+'] '+err.stack);
				});
				message=messages.join(' ||| ');
				stack=(stack?stack+' ||| ':'')+stacks.join(' ||| ');
				break;
			case 'object':
				var messages=['Error Object'];
				var stacks=[];
				for (var key in err){
					if (err.hasOwnProperty(key)){
						var subErr=createError(err[key]);
						if (cUtil.type(err[key])=='error'){
							messages.push(''+subErr.message);
						}
						stacks.push('['+subErr.message+'] '+subErr.stack);
					}
				}
				message=messages.join(' ||| ');
				stack=(stack?stack+' ||| ':'')+stacks.join(' ||| ');
				break;
			case 'date':
				message='Error Date';
				stack=(stack?stack+' ||| ':'')+err.toLocaleString();
				break;
			default:
				message=''+err;
				stack=(stack?stack+' ||| ':'')+err;
				break;
		}
	}
	if (message.length>100){
		message=message.slice(0,97)+'...';
	}
	var newErr=new Error(message);
	try{
		newErr.type=message;
		newErr.stack=stack;
	}catch (e){};
	return newErr;
}

var isSending=false;
function sendPackage(){
	if (!avalible || isSending){
		return;
	}
	isSending=true;
	var item=sendQueue.shift();
	if (item){
		var raw=createRaw(item);
		sendRaw(raw,function(err){
			if (err){
				console.log('[cUtil] Send Cat Package Failed:',err);
				sendQueue.unshift(item);
				setTimeout(sendPackage,10000);
				isSending=false;
			}else{
				isSending=false;
				setImmediate(sendPackage);
			}
		});
	}else{
		isSending=false;
	}
}

var msgSeq=0;
var hourSeq=getHourString();
function getHourString(){
	return parseInt((+new Date())/3600000,10).toString();
}

var maxMsgSeq=2147483647;
var procPrefix=0;
if (cluster.isWorker){
	procPrefix=(cluster.worker.id%256)<<23;
	maxMsgSeq=8388607;
}
function getMessageId(){
	var messageId=[];
	messageId.push(appConfig['AppID']);
	messageId.push(cUtil.getHostIp().split('.').map(function(item){
		return cUtil.fillZero(parseInt(item,10).toString(16),2);
	}).join(''));
	var hourStr=getHourString();
	if (hourSeq!=hourStr){
		hourSeq=hourStr;
		msgSeq=0;
	}
	messageId.push(hourStr);
	messageId.push(cUtil.fillZero(msgSeq,10));
	msgSeq++;
	if (msgSeq>=maxMsgSeq){
		msgSeq=0;
	}
	return messageId.join('-');
}

function formatTime(time){
	var d=new Date(time);
	var ret=cUtil.fillZero(d.getFullYear(),4)+'-'
		+cUtil.fillZero(d.getMonth()+1,2)+'-'
		+cUtil.fillZero(d.getDate(),2)+' '
		+cUtil.fillZero(d.getHours(),2)+':'
		+cUtil.fillZero(d.getMinutes(),2)+':'
		+cUtil.fillZero(d.getSeconds(),2)+','
		+cUtil.fillZero(d.getMilliseconds(),3);
	return ret;
}

function createRaw(item){
	var procName=process.execPath.replace(/^.*[\/\\]/,'');
	var header=[
		'PT1',
		appConfig['AppID'],
		cUtil.getHostName(),
		cUtil.getHostIp(),
		procName,
		process.pid,
		procName,
		item.__messageId,
		'null',
		'null',
		'null'
	].join('\t')+'\n';
	var body=createItemRaw(item);
	var buff=[];
	buff[1]=new Buffer(header+body);
	buff[0]=new Buffer(4);
	buff[0].writeUIntBE(buff[1].length,0,4);
	return Buffer.concat(buff);
}

function createItemRaw(item){
	var body='';
	if (item.__type=='event' || item.__type=='heartbeat' || item.__type=='error'){
		body=({
			'event':'E',
			'error':'E',
			'heartbeat':'H'
		})[item.__type]+[
			formatTime(item.__startTime),
			createRawString(item.type),
			createRawString(item.name),
			createRawString(item.status),
			createRawString(item.data)
		].join('\t')+'\t\n';
	}else if (item.__type=='span' && !item.__children.length){
		body='A'+[
			formatTime(item.__startTime),
			createRawString(item.type),
			createRawString(item.name),
			createRawString(item.status),
			(item.__endMicroSeconds-item.__startMicroSeconds).toString()+'us',
			createRawString(item.data)
		].join('\t')+'\t\n';
	}else if (item.__type=='span' && item.__children.length){
		body='t'+[
			formatTime(item.__startTime),
			createRawString(item.type),
			createRawString(item.name)
		].join('\t')+'\t\n';
		body+=item.__children.map(createItemRaw).join('');
		body+='T'+[
			formatTime(item.__endTime),
			createRawString(item.type),
			createRawString(item.name),
			createRawString(item.status),
			(item.__endMicroSeconds-item.__startMicroSeconds).toString()+'us',
			createRawString(item.data)
		].join('\t')+'\t\n';
	}
	return body;
}

function createRawString(data,defaultValue){
	var ret='';
	if (data===null || data===void(0)){
		ret=defaultValue||'';
	}else{
		switch (cUtil.type(data)){
			case 'string':
				ret=data;
				break;
			case 'number':
				ret=''+data;
				break;
			case 'date':
				ret=formatMilliSeconds(data);
				break;
			case 'array':
			case 'object':
				try{
					ret=JSON.stringify(data);
				}catch(e){
					ret='[ERROR: JSON ENCODE FAILED]';
				}
				break;
			default:
				ret=''+data;
				break;
		}
		var h={
			'\t':'\\t',
			'\r':'\\r',
			'\n':'\\n'
		};
		ret=ret.replace(/\\/g,'\\\\').replace(/[\t\r\n]/g,function(a){
			return h[a];
		});
	}
	return ret;
}

var conn=null;
function prepareConn(callback){
	if (!conn){
		var servers=appConfig['CatServer'].slice(0);
		tryConnectServer(servers,callback);
	}else{
		callback(null,conn);
	}
}

function tryConnectServer(servers,callback){
	var server=servers.shift();
	if (server){
		var arr=server.split(':');
		var host=arr[0];
		var port=parseInt(arr[1],10);
		var newConn=net.connect(port,host);
		newConn.on('error',function(){
			try{
				newConn.destroy();
			}catch(e){};
			if (conn==newConn){
				conn=null;
			}
			console.log('[cUtil] Connect To Cat Server Error: '+server);
			tryConnectServer(servers,callback);
		});
		newConn.on('connect',function(){
			conn=newConn;
			console.log('[cUtil] Connect To Cat Server Success: '+server);
			callback(null,conn);
		});
		newConn.on('end',function(){
			conn=null;
		});
	}else{
		callback('None Avalible Server');
	}
}

var sendBytes=0;
function sendRaw(raw,callback){
	prepareConn(function(err,conn){
		if (err){
			callback(err);
		}else{
//			console.log('[cUtil] Sending Cat Message');
//			console.log(raw.toString());
			sendBytes+=raw.length;
			conn.write(raw);
			callback(null);
		}
	});
}

function fetchServerConfig(){
//	console.log('[cUtil] Start Fetch Cat Server Config: '+appConfig['CatServer.Config.Url']);
	var tarUrl=url.parse(appConfig['CatServer.Config.Url']);
	tarUrl.method='GET';
	var req=http.request(tarUrl,function(res){
		res.on('error',errorHandle);
		var buffArr=[];
		res.on('data',function(buff){
			buffArr.push(buff);
		});
		res.on('end',function(){
			var content=Buffer.concat(buffArr).toString();
			var json={};
			try{
				json=JSON.parse(content);
			}catch (e){}
			if (json){
				var arr=[];
				var asyncTask=new Async(function(){
					if (arr.length){
						appConfig['CatServer']=arr;
//						console.log('[cUtil] Fetch Cat Server Config Success, ',appConfig['CatServer']);
						setTimeout(fetchServerConfig,1200000);
						avalible=true;
						setImmediate(sendPackage);
					}else{
						console.log('[cUtil] Fetch Cat Server Config Failed');
						setTimeout(fetchServerConfig,30000);
					}
				});
				cUtil.type(json)=='array' && json.forEach(function(item){
					if (item.Name=="CAT_SERVER" && item.Value){
						asyncTask.add(function(done,router){
							var tarUrlRouter=url.parse(router);
							tarUrlRouter.query.domain=appConfig['AppID'];
							tarUrlRouter=url.parse(url.format(tarUrlRouter));
							tarUrlRouter.method='GET';
							var reqRouter=http.request(tarUrlRouter,function(resRouter){
								resRouter.on('error',errorHandle);
								var buffArr=[];
								resRouter.on('data',function(buff){
									buffArr.push(buff);
								});
								resRouter.on('end',function(){
									var content=Buffer.concat(buffArr).toString();
									content.replace(/(\d{0,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})(:(\d{1,5}))?/g,function(a,b,c,d){
										arr.push(b+':'+(d||defaultPort));
									});
									done();
								});
							});
							reqRouter.on('error',errorHandle);
							reqRouter.end();

							function errorHandle(err){
								console.log('[cUtil]','Fetch Cat Server Router Error, '+router,err);
								done();
							}
						},item.Value);
					}
				});
				asyncTask.start();
			}else{
				errorHandle('Invalid Json Format');
			}
		});
	});
	req.on('error',errorHandle);
	req.end();

	function errorHandle(err){
		console.log('[cUtil]','Fetch Cat Server Config Error, '+appConfig['CatServer.Config.Url'],err);
		setTimeout(fetchServerConfig,30000);
	}
}

function sendHeartbeat(){
	var span=cat.span('System','Status');
	new CatHeartbeat(span,function(){
		span.end();
	});
}

function init(){
	if (useRemoteConfig){
		fetchServerConfig();
	}else{
		var arr=[];
		appConfig['CatServer'].forEach(function(item){
			var m=item.match(/^(\d{0,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})(:(\d{1,5}))?$/);
			if (m){
				arr.push(m[1]+':'+(m[3]||defaultPort));
			}
		});
		if (arr.length){
			appConfig['CatServer']=arr;
			avalible=true;
			setImmediate(sendPackage);
		}else{
			useRemoteConfig=true;
			fetchServerConfig();
		}
	}

	sendHeartbeat();
	setInterval(sendHeartbeat,60000);
}

var pipeAvalible=process.argv.indexOf('__ctriputil_cat_pipe_enabled')!=-1;
var pipeIds={};
if (pipeAvalible){
	process.on('message',function(d){
		if (d && d.__op){
			switch (d.__op){
				case 'span':
					if (d.__parentId){
						var parent=pipeIds[d.__parentId];
						if (parent && !parent.__close){
							var span=pipeIds[d.__id]=parent.span(d.type,d.name,d.data);
							span.__pipeId=d.__id;
						}
					}else{
						var span=pipeIds[d.__id]=cat.span(d.type,d.name,d.data);
						span.__pipeId=d.__id;
					}
					break;
				case 'span.end':
					var span=pipeIds[d.__id];
					if (span){
						span.end();
					}
					break;
				case 'event':
					if (d.__parentId){
						var parent=pipeIds[d.__parentId];
						if (parent && !parent.__close){
							parent.event(d.type,d.name,d.data);
						}
					}else{
						cat.event(d.type,d.name,d.data);
					}
					break;
				case 'error':
					if (d.__parentId){
						var parent=pipeIds[d.__parentId];
						if (parent && !parent.__close){
							parent.error(d.message,d.stack);
						}
					}else{
						cat.error(d.message,d.stack);
					}
					break;
				default:
					break;
			}
		}
	});
}

init();

module.exports=cat;
