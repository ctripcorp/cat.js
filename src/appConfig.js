var path=require('path');

var appConfig     = {};
var lastConfigDir = '';
var dirname       = path.resolve(".");

while (dirname !== lastConfigDir){

    try {
        var config = path.resolve(dirname, './app.config.js');
        appConfig = require(config);

        console.log('[AppConfig] Load config from ' + config);
        break;
    } catch (e){
        console.log('[AppConfig] No avaliable app.config.js found in %s .', dirname);

        lastConfigDir = dirname;
        dirname = path.dirname(lastConfigDir);
    }

}

module.exports = appConfig;
