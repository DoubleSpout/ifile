var ifile = require('../index.js');
var assert = require('assert');
var zlib = require('zlib');

ifile.options = {

	pipe_szie:1024
	expired:86400*1000*30,
	gzip:true,
	gzip_min_size:512,
	gzip_file:['js','xml'],
	gzip_level:0
}


ifile.add([
	["static3/static3/","static2",['js','css','xml']]
],function(req, res, isStatic){

	//console.log(isStatic)
	if(isStatic) res.statusCode = 404;
	res.end('isStatic: '+isStatic);

})

var http = require('http');
http.createServer(function (request, response) {
  	ifile.route(request,response);
}).listen(8125);

console.log('Server running at http://127.0.0.1:8124/');


var fs = require('fs');
var path = require('path');


var testjs2 = fs.readFileSync(path.join(__dirname,'static2','static3','static3','test.xml'))
var testjs2_stat = fs.statSync(path.join(__dirname,'static2','static3','static3','test.xml'))




var n = 1;
var back = function(m){
	console.log(m)
	if(!--n){
		process.exit(0);
	}
}

var request = function(path,head,cb,method){


	var options = {
	  hostname: '127.0.0.1',
	  port: 8125,
	  path: path,
	  headers: head,
	  method:method||'GET'
	};

	var req = http.request(options, function(res) {

	  //console.log('STATUS: ' + res.statusCode);
	 //console.log('HEADERS: ' + JSON.stringify(res.headers));
	  var buf_list = [];
	  var len=0;


	  res.on('data', function (chunk) {

	  	buf_list.push(chunk);
		len += chunk.length;
	  });
	  res.on('end',function(){
	  	var buf = Buffer.concat(buf_list, len);
	  		  	//console.log(buf.toString())
	  	cb(null, res, buf);
	  })


	});

	req.on('error', function(e) {
	  console.log('problem with request: ' + e.message);
	  cb(e)
	});
	req.end();


}

 
http.globalAgent.maxSockets = 40;

setTimeout(function(){








},500)