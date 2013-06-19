	var times = 1000
	var count = times;

	var ifile = require("../index.js");

	ifile.add([
		["/static/",__dirname],
	],function(req,res){
		throw(404)
		//console.log('404')
	})

	var req= {
		url:'/static/jquery.1.7.1.js',
		method:'GET',
		headers:{}
	}

	var res = {
		end:function(buf){
			var jquery = buf.toString();
			//console.log(jquery)
			if(!--count){
				console.timeEnd('ifile');
			}
		},
		statusCode:200,
		setHeader:function(name,val){}
	}
	console.time('ifile');
	for(var i=0;i<times;i++){
		ifile.route(req,res);
	}
