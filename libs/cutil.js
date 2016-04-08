var os=require('os');
var exec=require('child_process').exec;

var cUtil={
	uid:function(){
		return 'uid_'+new Date().getTime()+(Math.random()*1e10).toFixed(0);
	},
	type:function(obj){
		return Object.prototype.toString.call(obj).slice(8,-1).toLowerCase();
	},
	__hostIp:null,
	getHostIp:function(){
		if (!cUtil.__hostIp){
			var ipA='',ipB='';
			var inf=os.networkInterfaces();
			infLoops:
				for (var name in inf){
					if (inf.hasOwnProperty(name)){
						var arr=inf[name];
						for (var i=0;i<arr.length;i++){
							if (arr[i].family=='IPv4' && arr[i].internal==false){
								if (/^(10|172|192\.168)\./.test(arr[i].address)){
									ipA=arr[i].address;
									break infLoops;
								}
								if (!ipB){
									ipB=arr[i].address;
								}
							}
						}
					}
				}
			cUtil.__hostIp=ipA||ipB||'127.0.0.1';
		}
		return cUtil.__hostIp;
	},
	__hostName:null,
	getHostName:function(){
		if (!cUtil.__hostName){
			cUtil.__hostName=os.hostname();
		}
		return cUtil.__hostName;
	},
	getDiskInfo:function(callback){
		if (os.platform().indexOf('win')!=-1){
			// windows
			var wmicDefs={
				'Caption':{
					name:'path',
					type:'string',
					filter:function(path){
						return path.replace(/\\/g,'/').replace(/^([A-Z]):/,'/$1').replace(/\/?$/,'/');
					}
				},
				'FileSystem':{
					name:'fileSystem',
					type:'string',
					filter:function(item){
						return item.toLowerCase();
					}
				},
				'FreeSpace':{
					name:'freeSpace',
					type:'int'
				},
				'Size':{
					name:'size',
					type:'int'
				},
				'VolumeName':{
					name:'name',
					type:'string'
				}
			};
			exec('wmic logicaldisk get Caption,FileSystem,FreeSpace,Size,VolumeName',function(err,stdout,stderr){
				if (err){
					callback(err,null);
				}else{
					var lines=stdout.split('\n');
					var head=lines.shift();
					var cols=[];
					var start=0;
					head.replace(/(\w+)\s*/g,function(a,b){
						cols.push({
							name:b,
							start:start,
							end:start+a.length
						});
						start+=a.length;
					});
					var ret=[];
					lines.forEach(function(line){
						var info={};
						cols.forEach(function(col){
							var def=wmicDefs[col.name];
							var val=line.slice(col.start,col.end).trim();
							if (def.type=='int'){
								val=parseInt(val,10);
							}
							if (def.filter){
								val=def.filter(val);
							}
							info[def.name]=val;
						});
						if (info.path && info.size){
							ret.push(info);
						}
					});
					callback(null,ret);
				}
			});
		}else{
			var dfDefs={
				'Mounted on':{
					name:'path',
					type:'string',
					filter:function(path){
						return path.replace(/\/?$/,'/');
					}
				},
				'Type':{
					name:'fileSystem',
					type:'string',
					filter:function(item){
						return item.toLowerCase();
					}
				},
				'Available':{
					name:'freeSpace',
					type:'int'
				},
				'1-blocks':{
					name:'size',
					type:'int'
				},
				'Filesystem':{
					name:'name',
					type:'string'
				}
			};
			exec('df -B 1 -T -P',function(err,stdout,stderr){
				if (err){
					callback(err,null);
				}else{
					var lines=stdout.split('\n');
					var cols=lines.shift().split(/\s+(?=[A-Z\d]|$)/);
					var ret=[];
					lines.forEach(function(line){
						line=line.trim();
						if (line){
							var info={};
							line.split(/\s+/).forEach(function(val,i){
								var def=dfDefs[cols[i]];
								if (def){
									if (def.type=='int'){
										val=parseInt(val,10);
									}
									if (def.filter){
										val=def.filter(val);
									}
									info[def.name]=val;
								}
							});
							if (info.path && info.size){
								ret.push(info);
							}
						}
					});
					callback(null,ret);
				}
			});
		}
	},
	getMicroSeconds:function(){
		var t=process.hrtime();
		return parseInt((t[0]*1e9+t[1])/1e3,10);
	},
	getNanoSeconds:function(){
		var t=process.hrtime();
		return t[0]*1e9+t[1];
	},
	sleep:function(s){
		var e=new Date().getTime()+(s*1000);
		while (new Date().getTime()<=e){;}
	},
	usleep:function(s){
		var e=new Date().getTime()+(s/1000);
		while (new Date().getTime()<=e){;}
	},
	upperCaseFirstChar:function(str){
		return str.slice(0,1).toUpperCase()+str.slice(1).toLowerCase();
	},
	repeat:function(str,times){
		var arr=[];
		arr[times]='';
		var ret=arr.join(str);
		return ret;
	},
	fillZero:function(num,len){
		return (cUtil.repeat('0',len)+num).slice(-Math.max(len,num.toString().length));
	},
	isInt:function(number){
		var ret=parseInt(number,10)==number;
		return ret;
	},
	extendMap:function(obj){
		var args=[].slice.call(arguments,1);
		for (var i=0;i<args.length;i++){
			for (var key in args[i]){
				if (args[i].hasOwnProperty(key)){
					var val=args[i][key];
					switch (cUtil.type(val)){
						case 'string':
							break;
						case 'number':
							val=val.toString();
							break;
						default:
							try {
								val=JSON.stringify(val);
							}catch (e){
								val='';
							}
							break;
					}
					obj[key]=val;
				}
			}
		}
		return obj;
	}
};

module.exports=cUtil;