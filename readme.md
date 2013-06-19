# ifile(fast and simple nodejs http/https static file module)

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

to run 1000 times send jquery.1.7.1.min.js:

      nodejs: 166ms (with out any route handler and headers)
      ifile : 159ms

gzip it and send

      nodejs_gzip: 3555ms
      ifile_gzip : 2588ms

## example

    var ifile = require('ifile')

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

more example see /test/main.js

##expressjs example

      var express = require('express');
      var app = express();


      var ifile = require("ifile");
      
   	 ifile.add([
  		   ["/static",__dirname,['js','css','jpg']],
   	 ],function(req,res,is_static){
   	 	if(is_static){
	    	res.statusCode = 404;
	        res.end('404')
        }
        else{
        	req._ifile_next();
        }
    })

      app.use(function(req, res, next){
      	req._ifile_next = next;
        ifile.route(req,res)
      });

      app.listen(3000);

