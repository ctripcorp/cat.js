# cat.js #

- Version: 0.0.1
- Author: cdchu@ctrip.com

## Install ##

npm install cat-nodejs

## Usage ##

1. create app.config.js as nodejs module format

	     module.exports={
	     	'AppID':'921821',
	     	'CatServer.Config.Url':'http://10.0.0.1/catconfig',
			'CatServer':['10.0.0.1:1234','10.0.0.2:1234']
	     };

2. require ctriputil

		var cat=require('cat-nodejs');


## CAT APIs ##

- **Method** *spanInstance* cat.span(type,name,data)
	- type as string
	- name as string
	- data as string or number or array or object
- **Method** *self* cat.event(type,name,data)
	- type as string
	- name as string
	- data as string or number or array or object
- **Method** *self* cat.error(message,stack)
	- message as string or error
	- stack as string
- **Method** *self* CtripUtil.cat.http(server)
	- server as httpServerInstance
- **Class** spanInstance
	- **Method** *spanInstance* spanInstance.span(type,name,data)
		- type as string
		- name as string
		- data as string or number or array or object
	- **Method** *self* spanInstance.event(type,name,data)
		- type as string
		- name as string
		- data as string or number or array or object
	- **Method** *self* spanInstance.error(message,stack)
		- message as string or error
		- stack as string
	- **Method** *parent* spanInstance.end()
		- parent as spanInstance or cat
