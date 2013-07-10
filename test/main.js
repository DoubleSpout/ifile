var ifile = require('../index.js');
var assert = require('assert');
var zlib = require('zlib');

ifile.options = {
	pipe_szie:2048,
	expired:86400*1000*30,
	gzip:true,
	gzip_min_size:512,
	gzip_file:['js','xml'],
	gzip_level:0
}


ifile.add([
	["/static",__dirname],
	["/static_aaa",__dirname],
	["static2","static2"],
	["static3/static3/","static2",['js','css','xml']]
],function(req, res, isStatic){

	//console.log(isStatic)
	if(isStatic) res.statusCode = 404;
	res.end('isStatic: '+isStatic);

})

var http = require('http');
http.createServer(function (request, response) {
  	ifile.route(request,response);
}).listen(8124);

console.log('Server running at http://127.0.0.1:8124/');


var fs = require('fs');
var path = require('path');

var hs1 = fs.readFileSync(path.join(__dirname,'static','mount_huangshan.jpg'))
var hs1_stat = fs.statSync(path.join(__dirname,'static','mount_huangshan.jpg'))

var hs2 = fs.readFileSync(path.join(__dirname,'static','mount_huangshan2.jpg'))
var hs2_stat = fs.statSync(path.join(__dirname,'static','mount_huangshan2.jpg'))

var testcss1 = fs.readFileSync(path.join(__dirname,'static2','static2','test.css'))
var testcss1_stat = fs.statSync(path.join(__dirname,'static2','static2','test.css'))

var testjs1 = fs.readFileSync(path.join(__dirname,'static2','static2','test.js'))
var testjs1_stat = fs.statSync(path.join(__dirname,'static2','static2','test.js'))

var testcss2 = fs.readFileSync(path.join(__dirname,'static2','static3','static3','test.css'))
var testcss2_stat = fs.statSync(path.join(__dirname,'static2','static3','static3','test.css'))

var testjs2 = fs.readFileSync(path.join(__dirname,'static2','static3','static3','test.xml'))
var testjs2_stat = fs.statSync(path.join(__dirname,'static2','static3','static3','test.xml'))

var testxml3 = fs.readFileSync(path.join(__dirname,'static2','static3','static3','test3.xml'))
var testxml3_stat = fs.statSync(path.join(__dirname,'static2','static3','static3','test3.xml'))


var n = 19;
var back = function(m){
	console.log(m)
	if(!--n){
		process.exit(0);
	}
}

var request = function(path,head,cb,method){


	var options = {
	  hostname: '127.0.0.1',
	  port: 8124,
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


//get test 1
request('/static/Mount_Huangshan.jpg', {}, function(err,res,buf){

	if(err) throw(err);
	assert.equal(res.statusCode, 200)
	var ts = Date.parse(hs1_stat.mtime)+'';
	assert.equal(res.headers["etag"],  ts.slice(0, ts.length-3) +'_'+hs1_stat.size)
	assert.equal(res.headers["last-modified"], new Date(hs1_stat.mtime).toUTCString())
	assert.equal(res.headers["cache-control"], 'max-age=2592000')
	assert.equal(res.headers["transfer-encoding"], "chunked")
	assert.equal(res.headers["content-type"], 'image/jpeg')
	assert.equal(buf.toString(), hs1.toString());

	back('test1')

},'GET')


//get test 2

request('/static/Mount_Huangshan1.jpg?key=1', {}, function(err,res,buf){

	if(err) throw(err);
	assert.equal(res.statusCode, 404)
	assert.equal(buf.toString(), 'isStatic: 1');

	back('test2')

},'GET')


//get test 3
request('/static/Mount_Huangshan2.jpg?key2=111&key3=222', {}, function(err,res,buf){

	if(err) throw(err);
	assert.equal(res.statusCode, 200)
	var ts = Date.parse(hs2_stat.mtime)+'';
	assert.equal(res.headers["etag"],  ts.slice(0, ts.length-3) +'_'+hs2_stat.size)
	assert.equal(res.headers["last-modified"], new Date(hs2_stat.mtime).toUTCString())
	assert.equal(res.headers["cache-control"], 'max-age=2592000')
	assert.equal(res.headers["transfer-encoding"], "chunked")
	assert.equal(res.headers["content-type"], 'image/jpeg')
	assert.equal(buf.toString(), hs2.toString());

	back('test3')

},'GET')


var ts = Date.parse(hs2_stat.mtime)+'';
request('/static/Mount_Huangshan2.jpg', {
	"If-None-Match":ts.slice(0, ts.length-3) +'_'+hs2_stat.size,
}, function(err,res,buf){
	if(err) throw(err);
	assert.equal(res.statusCode, 304)
	var ts = Date.parse(hs2_stat.mtime)+'';
	assert.equal(res.headers["etag"],  ts.slice(0, ts.length-3) +'_'+hs2_stat.size)
	assert.equal(res.headers["last-modified"], new Date(hs2_stat.mtime).toUTCString())
	assert.equal(res.headers["cache-control"], 'max-age=2592000')
	assert.equal(res.headers["content-type"], 'image/jpeg')
	assert.equal(buf.toString(), '');

	back('test4')

},'GET')


request('/static/Mount_Huangshan.jpg', {
	"If-Modified-Since":new Date(hs1_stat.mtime).toUTCString(),
}, function(err,res,buf){
	if(err) throw(err);
	assert.equal(res.statusCode, 304)
	var ts = Date.parse(hs1_stat.mtime)+'';
	assert.equal(res.headers["etag"],  ts.slice(0, ts.length-3) +'_'+hs1_stat.size)
	assert.equal(res.headers["last-modified"], new Date(hs1_stat.mtime).toUTCString())
	assert.equal(res.headers["cache-control"], 'max-age=2592000')
	assert.equal(res.headers["content-type"], 'image/jpeg')
	assert.equal(buf.toString(), '');

	back('test5')

},'GET')



request('/static_aaa/Mount_Huangshan.jpg', {}, function(err,res,buf){

	if(err) throw(err);
	assert.equal(res.statusCode, 404)
	back('test6')

},'GET')




request('/static2/test.css', {}, function(err,res,buf){

	if(err) throw(err);
	assert.equal(res.statusCode, 200)
	var ts = Date.parse(testcss1_stat.mtime)+'';
	assert.equal(res.headers["etag"],  ts.slice(0, ts.length-3) +'_'+testcss1_stat.size)
	assert.equal(res.headers["last-modified"], new Date(testcss1_stat.mtime).toUTCString())
	assert.equal(res.headers["cache-control"], 'max-age=2592000')
	assert.equal(res.headers["content-length"], testcss1_stat.size)
	assert.equal(res.headers["content-type"], 'text/css')
	assert.equal(buf.toString(), testcss1.toString());

	back('test7')

},'GET')


request('/static2/test.js', {}, function(err,res,buf){

	if(err) throw(err);
	assert.equal(res.statusCode, 200)
	var ts = Date.parse(testjs1_stat.mtime)+'';
	assert.equal(res.headers["etag"],  ts.slice(0, ts.length-3) +'_'+testjs1_stat.size)
	assert.equal(res.headers["last-modified"], new Date(testjs1_stat.mtime).toUTCString())
	assert.equal(res.headers["cache-control"], 'max-age=2592000')
	assert.equal(res.headers["content-length"], testjs1_stat.size)
	assert.equal(res.headers["content-type"], 'application/javascript')
	assert.equal(buf.toString(), testjs1.toString());

	back('test8')

},'GET')




var ts = Date.parse(testjs1_stat.mtime)+'';
request('/static2/test.js', {
	"If-None-Match":ts.slice(0, ts.length-3) +'_'+testjs1_stat.size,
}, function(err,res,buf){
	if(err) throw(err);
	assert.equal(res.statusCode, 304)
	var ts = Date.parse(testjs1_stat.mtime)+'';
	assert.equal(res.headers["etag"],  ts.slice(0, ts.length-3) +'_'+testjs1_stat.size)
	assert.equal(res.headers["last-modified"], new Date(testjs1_stat.mtime).toUTCString())
	assert.equal(res.headers["cache-control"], 'max-age=2592000')
	assert.equal(res.headers["content-type"], 'application/javascript')
	assert.equal(buf.toString(), '');

	back('test9')

},'GET')



request('/static2/test.css', {
	"If-Modified-Since":new Date(testcss1_stat.mtime).toUTCString(),
}, function(err,res,buf){
	if(err) throw(err);
	assert.equal(res.statusCode, 304)
	var ts = Date.parse(testcss1_stat.mtime)+'';
	assert.equal(res.headers["etag"],  ts.slice(0, ts.length-3) +'_'+testcss1_stat.size)
	assert.equal(res.headers["last-modified"], new Date(testcss1_stat.mtime).toUTCString())
	assert.equal(res.headers["cache-control"], 'max-age=2592000')
	assert.equal(res.headers["content-type"], 'text/css')
	assert.equal(buf.toString(), '');

	back('test10')

},'GET')




request('/static3/static3/test.css', {}, function(err,res,buf){

	if(err) throw(err);
	assert.equal(res.statusCode, 200)
	var ts = Date.parse(testcss2_stat.mtime)+'';
	assert.equal(res.headers["etag"],  ts.slice(0, ts.length-3) +'_'+testcss2_stat.size)
	assert.equal(res.headers["last-modified"], new Date(testcss2_stat.mtime).toUTCString())
	assert.equal(res.headers["cache-control"], 'max-age=2592000')
	assert.equal(res.headers["content-length"], testcss2_stat.size)
	assert.equal(res.headers["content-type"], 'text/css')
	assert.equal(buf.toString(), testcss2.toString());

	back('test11')

},'GET')


request('/static3/static3/test.xml', {}, function(err,res,buf){

	if(err) throw(err);
	assert.equal(res.statusCode, 200)
	var ts = Date.parse(testjs2_stat.mtime)+'';
	assert.equal(res.headers["etag"],  ts.slice(0, ts.length-3) +'_'+testjs2_stat.size)
	assert.equal(res.headers["last-modified"], new Date(testjs2_stat.mtime).toUTCString())
	assert.equal(res.headers["cache-control"], 'max-age=2592000')
	assert.equal(res.headers["content-length"], testjs2_stat.size)
	assert.equal(res.headers["content-type"], 'application/xml')
	assert.equal(buf.toString(), testjs2.toString());

	back('test12')

},'GET')


request('/static3/static3/test.txt', {}, function(err,res,buf){

	if(err) throw(err);
	assert.equal(res.statusCode, 404)
	assert.equal(buf.toString(), 'isStatic: 1');

	back('test13')

},'GET')



request('/aaa/bbb/test.txt', {}, function(err,res,buf){

	if(err) throw(err);
	assert.equal(buf.toString(), 'isStatic: 0');

	back('test14')

},'GET')


request('/static3/static3/test.xml', {"Accept-Encoding":"gzip,deflate,sdch"}, function(err,res,buf){

	if(err) throw(err);
	assert.equal(res.statusCode, 200)
	var ts = Date.parse(testjs2_stat.mtime)+'';
	assert.equal(res.headers["etag"],  ts.slice(0, ts.length-3) +'_'+testjs2_stat.size)
	assert.equal(res.headers["last-modified"], new Date(testjs2_stat.mtime).toUTCString())
	assert.equal(res.headers["cache-control"], 'max-age=2592000')

	assert.equal(res.headers["content-type"], 'application/xml')
	assert.equal(res.headers["content-encoding"], 'gzip')
	
	//console.log(buf.length)
	zlib.gunzip(buf, function(err,buf2){
			if(err) throw(err)
			//console.log(buf2.length)
			assert.equal(buf2.toString(), testjs2.toString());
			back('test15')
		})


	

},'GET')


request('/static3/static3/test2.xml', {"Accept-Encoding":"gzip,deflate,sdch"}, function(err,res,buf){

	if(err) throw(err);
	assert.equal(res.statusCode, 200)
	var ts = Date.parse(testjs2_stat.mtime)+'';

	assert.equal(res.headers["cache-control"], 'max-age=2592000')

	assert.equal(res.headers["content-type"], 'application/xml')
	assert.equal(!res.headers["content-encoding"], true)
	
	//console.log(buf.length)
back('test16')


	

},'GET')




request('/static3/static3/test2.css', {"Accept-Encoding":"gzip,deflate,sdch"}, function(err,res,buf){

	if(err) throw(err);
	assert.equal(res.statusCode, 200)
	var ts = Date.parse(testjs2_stat.mtime)+'';

	assert.equal(res.headers["cache-control"], 'max-age=2592000')

	assert.equal(res.headers["content-type"], 'text/css')
	assert.equal(!res.headers["content-encoding"], true)
	
	//console.log(buf.length)
back('test17')


	

},'GET')




request('/static3/static3/test3.xml', {"Accept-Encoding":"gzip,deflate,sdch"}, function(err,res,buf){ //pipe response test

	if(err) throw(err);
	assert.equal(res.statusCode, 200)
	var ts = Date.parse(testxml3_stat.mtime)+'';
	assert.equal(res.headers["etag"],  ts.slice(0, ts.length-3) +'_'+testxml3_stat.size)
	assert.equal(res.headers["last-modified"], new Date(testxml3_stat.mtime).toUTCString())
	assert.equal(res.headers["cache-control"], 'max-age=2592000')

	assert.equal(res.headers["content-type"], 'application/xml')
	assert.equal(res.headers["content-encoding"], 'gzip')
	
	//console.log(buf.length)
	zlib.gunzip(buf, function(err,buf2){
			if(err) throw(err)
			//console.log(buf2.length)
			assert.equal(buf2.toString(), testxml3.toString());
			back('test18')
		})

},'GET')


request('/static3/static3/test3.xml', {}, function(err,res,buf){ //pipe response test

	if(err) throw(err);
	assert.equal(res.statusCode, 200)
	var ts = Date.parse(testxml3_stat.mtime)+'';
	assert.equal(res.headers["etag"],  ts.slice(0, ts.length-3) +'_'+testxml3_stat.size)
	assert.equal(res.headers["last-modified"], new Date(testxml3_stat.mtime).toUTCString())
	assert.equal(res.headers["cache-control"], 'max-age=2592000')

	assert.equal(res.headers["content-type"], 'application/xml')
	
	//console.log(buf.length)
	back('test19')

},'GET')


},500)