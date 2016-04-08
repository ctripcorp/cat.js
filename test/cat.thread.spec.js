/*

var Threads = require('threads_a_gogo');//加载tagg包
var express = require('express');

xdescribe("Process & Thread",function(){
	it("Multi Thread",function(){
		
		function fibo(n) {//定义斐波那契数组计算函数
			return n > 1 ? fibo(n - 1) + fibo(n - 2) : 1;
		}
		var t = Threads.create().eval(fibo);
		t.eval('fibo(35)', function(err, result) {//将fibo(35)丢入子线程运行
		    if (err) throw err; //线程创建失败
		    console.log('fibo(35)=' + result);//打印fibo执行35次的结果
		});
		console.log('not block');//打印信息了，表示没有阻塞
	});

	it("Process",function(){
		
		var fork = require('child_process').fork;
		var app = express();
		app.get('/', function(req, res){
		  var worker = fork('./work_fibo.js') //创建一个工作进程
		  worker.on('message', function(m) {//接收工作进程计算结果
		          if('object' === typeof m && m.type === 'fibo'){
		                   worker.kill();//发送杀死进程的信号
		                   res.send(m.result.toString());//将结果返回客户端
		          }
		  });
		  worker.send({type:'fibo',num:~~req.query.n || 1});
		  //发送给工作进程计算fibo的数量
		});
		app.listen(8124);
	});

});
*/