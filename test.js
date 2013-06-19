var ifile = require('./index.js')
var zlib = require('zlib');
var path = require('path')


ifile.add([
	["/static2",__dirname+path.sep+"test"],

	["/static2/",__dirname+path.sep+"user",0],
	["static","user",[]],
	["static/sss/","user",['js','css']],
	["/static/ss/","user"+path.sep+"fsdf",['js']],
	["/static/saa","user"+path.sep+"fsdf"+path.sep,[]]

],function(req, res, isStatic){
	//console.log(res)
	res.end('isStatic: '+isStatic);

})

var req= {
	url:'/static2/static2/aaa.js',
	method:'GET',
	headers:{
		//"if-none-match":"1370955406_34",
		//"if-modified-since":"Thu, 13 Feb 2014 18:35:31 GMT",
		"accept-encoding":"gzip,deflate,sdch"
	}
}
var res = {
	end:function(buf){

		/*
		zlib.gunzip(buf, function(err,buf2){
			if(err) throw(err)
			console.log(buf2.length);
			console.log(buf2.toString())

		})
		*/
		console.log(buf.length)
		console.log(buf.toString())
	},
	setHeader:function(name,value){
		console.log(name)
		console.log(value)
	}
}

ifile.route(req,res);





var http = require('http');
http.createServer(function (request, response) {
	console.log(request.headers)
  	ifile.route(request,response);
}).listen(8124);

console.log('Server running at http://127.0.0.1:8124/');