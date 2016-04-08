var path=require('path');

var appConfigHidden=process.argv.indexOf('__appconfig_hidden')!=-1;

var appConfig={};
var lastConfigDir='';
var dirname=path.resolve(".");
while (dirname!==lastConfigDir){
	try {
		var config=path.resolve(dirname,'./app.config.js');
		appConfig=require(config);
		if (!appConfigHidden){
			console.log('[AppConfig] Load config from '+config);
		}
		break;
	}catch (e){
		if (!appConfigHidden){
			console.log('[AppConfig] No avaliable app.config.js found in '+dirname);
		}
		lastConfigDir=dirname;
		dirname=path.dirname(lastConfigDir);
	}
}

if (!appConfig['AppID']){
	if (appConfig['AppDomain']){
		appConfig['AppID']=appConfig['AppDomain'];
	}else{
		throw('[cUtil]','Missing AppID');
	}
}

if (!appConfig['LogLevel']){
	appConfig['LogLevel']=appConfig['log_level']||1;
}

if (!appConfig['CatServer.Config.Url']){
	appConfig['CatServer.Config.Url']='';
}

if (!appConfig['CatServer']){
	appConfig['CatServer']=[];
}

if (!appConfig['CatServer.Config.Url'] && !appConfig['CatServer'].length){
	throw('Invalid CatServer.Config.Url or CatServer');
}

if (!appConfigHidden){
//	console.log('[AppConfig]',appConfig);
}

module.exports = appConfig;
