var fs = require('fs');
var zlib = require('zlib');




	var times =  1000;
	var count = times;
	console.time('node');
	for(var i=0;i<times;i++){
		fs.stat(__dirname+'/static/jquery.1.7.1.js', function(err,stats){
		if(err) throw(err)
		var size = stats.size;
		fs.readFile(__dirname+'/static/jquery.1.7.1.js',function(err,buf){
			//if(err) throw(err)
			zlib.gzip(buf, function(err,buf2){
				if(err) throw(err)
				var jquery = buf2.toString();
				if(!--count){
					console.timeEnd('node');
				}
			})		
		}) 

	})
	}




