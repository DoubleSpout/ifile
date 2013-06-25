# ifile(fast and simple nodejs http/https static file module)[![Build Status](https://travis-ci.org/DoubleSpout/ifile.png?branch=master)](https://travis-ci.org/DoubleSpout/ifile)

ifile is a simple static http/https handler module, build with libuv and c++.

the module are also be used in express and flat.js framework.

## Installing the module

With [npm](http://npmjs.org/):

ifile module is supported windows, linux, mac.

Make sure, node-gyp has installed.

     npm install ifile

From source:

     git clone https://github.com/DoubleSpout/ifile.git
     cd ifile
     node-gyp rebuild

To include the module in your project:

     var ifile = require('ifile');

##benchmark
send the same js file, 6KB size.

      nginx 
      ab -c 100 -n 20000 http://192.168.28.5:8124/js/test.js
      Requests per second:    2634.31 [#/sec] (mean)

      ab -c 500 -n 20000 http://192.168.28.5:8124/js/test.js
      Requests per second:    1886.92 [#/sec] (mean)

      ab -c 800 -n 20000 http://192.168.28.5:8124/js/test.js
      Requests per second:    2033.45 [#/sec] (mean)

      ab -c 500 -n 20000 -H "Accept-Encoding: gzip" http://192.168.28.5:8124/js/test.js
      Requests per second:    2029.59 [#/sec] (mean)


      ifile
      ab -c 100 -n 20000 http://192.168.28.5:8125/js/test.js
      Requests per second:    2077.29 [#/sec] (mean)

      ab -c 500 -n 20000 http://192.168.28.5:8125/js/test.js
      Requests per second:    1880.00 [#/sec] (mean)

      ab -c 800 -n 20000 http://192.168.28.5:8125/js/test.js
      Requests per second:    1791.16 [#/sec] (mean)

      ab -c 500 -n 20000 -H "Accept-Encoding: gzip" http://192.168.28.5:8125/js/test.js
      Requests per second:    1858.01 [#/sec] (mean)


      express
      ab -c 100 -n 20000 http://192.168.28.5:8126/js/test.js
      Requests per second:    915.21 [#/sec] (mean)

      ab -c 500 -n 20000 http://192.168.28.5:8126/js/test.js
      Requests per second:    858.89 [#/sec] (mean)

      ab -c 800 -n 20000 http://192.168.28.5:8126/js/test.js
      Requests per second:    668.99 [#/sec] (mean)

      ab -c 500 -n 20000 -H "Accept-Encoding: gzip" http://192.168.28.5:8126/js/test.js
      Requests per second:    677.11 [#/sec] (mean)



      express+ifile
      ab -c 100 -n 20000 http://192.168.28.5:8127/js/test.js
      Requests per second:    1684.85 [#/sec] (mean)

      ab -c 500 -n 20000 http://192.168.28.5:8127/js/test.js
      Requests per second:    1717.32 [#/sec] (mean)

      ab -c 800 -n 20000 http://192.168.28.5:8127/js/test.js
      Requests per second:    1399.09 [#/sec] (mean)

      ab -c 500 -n 20000 -H "Accept-Encoding: gzip" http://192.168.28.5:8127/js/test.js
      Requests per second:    1468.06 [#/sec] (mean)


## example

    var ifile = require('ifile')

    var http = require('http')

    ifile.add([

     ["/static",__dirname,['js','css','jpg']],

    ],function(req,res,is_static){

    	res.statusCode = 404;

        res.end('404')

    })

    var http = require('http');

    http.createServer(function (req, res) {

      ifile.route(req,res);

    }).listen(8124);

then request the 127.0.0.1:8124/static/xxx.js

if you have a file in __dirname/static/xxx.js then you will see it.

##API

ifile.add(routearray,not_match_callback);

ifile.route(req,res); 
route the request

routearray:

  [
    [request_url_prefix, static_folder [,support_extensions_array]],
    ...
  ]

  example:

      [
      	["/static",__dirname,['js','css','jpg']],
      	["/static2",__dirname],
      	["/skin","static",['js','css','jpg']]   //static folder will be __dirname+'/'+static
      ]

not_match_function:
if ifile not match the request,the not_match_function will be called.It has three parameters,req, res and is_static.
for example if add ["/static2",__dirname]
1,request "/user/aaa" will call the not_match_function,and is_static param will be 0;
2,request "/static/not_exist_file" will call the not_match_function,and is_static param will be 1;

##options
set ifile.options property to change the default options,for example:
      
      var ifile = require('ifile');
      ifile.options = {
        expired:86400*100*30,
        gzip:false
      }

default options is that:

      default_options = {
        pipe_szie : 1024*1024*20, //超过20MB的静态文件使用pipe传输不读入内存
        expired : 86400*1000,     //cache-control : max-age=86400
        gzip : true,              //是否开启gzip
        gzip_min_size : 1024,       //开启gzip的文件必须大于等于1024byte
        gzip_file : ['js','css','less','html','xhtml','htm','xml','json','txt'], //gzip压缩的文件后缀名
        gzip_level : 9, //-1表示使用默认值
      }

more example see /test/main.js

##expressjs example

      var express = require('express');
      var app = express();
      var ifile = require("ifile");
      app.use(ifile.connect()); 
      //default is [['/static',__dirname]];

      app.listen(3000);

