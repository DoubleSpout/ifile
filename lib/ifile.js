var ifile_cc = require('../build/Release/ifile.node');
var mime = require('mime');
var zlib = require('zlib');
var fs = require('fs');

var path = require('path');
var ifile_obj = {};

var ADD_ERROR = 'static_array format error, ifile.add(static_array [,not_found_callback]), static_array must like [["/staic",__dirname [,suffix_name_array or 0], ...]';
var ADD_ERROR2 = 'not_found_callback format error, ifile.add(static_array [,not_found_callback]), not_found_callback must be a function';
var ADD_ERROR3 = 'suffix_name_array error,suffix_name_array must like ["js","css","png", ...]';


var default_callback = function(){};
var static_uri_array = [];
var static_dir_array = [];
var static_suffix_array =[];



ifile_obj.default_options = {
	pipe_szie : 1024*1024*20, //超过20MB的静态文件使用pipe传输不读入内存
	expired : 86400*1000,     //cache-control : max-age=86400
	gzip : true,              //是否开启gzip
	gzip_min_size : 1024,       //开启gzip的文件必须大于等于1024byte
	gzip_file : ['js','css','less','html','xhtml','htm','xml','json','txt'], //gzip压缩的文件后缀名
	gzip_level : 0, //0表示使用默认值

}

ifile_obj.pipe = function(req,res,state){ //将静态文件pipe出去

	var size = state.size;
	var gzip_size = state.gzip_min_size;
	var gzip_level = state.gzip_level;
	var filepath = state.file_path;
	var is_gzip = state.is_config_gzip && req.headers["accept-encoding"] && req.headers["accept-encoding"].indexOf('gzip') !== -1 && size>=gzip_size? 1 : 0;

	var raw = fs.createReadStream(filepath);

	res.setHeader("Transfer-Encoding", "chunked");
	res.removeHeader("Cotent-Length");
	if(is_gzip){
		res.setHeader("Content-Encoding", "gzip")
		raw.pipe(zlib.createGzip()).pipe(res);
	}
	else raw.pipe(res);

}


ifile_obj.add = function(static_array, not_found_callback){
	var that = this;

	static_array.forEach(function(v,i){

			if('object' !== typeof v || v.length <2 || v.length >3 ){
				throw(ADD_ERROR);
			}
			if('string' !== typeof v[0] || 'string' !== typeof v[1] || v[0].length<1 || v[1].length<1){
				throw(ADD_ERROR2);
			}

			v[2] = 'undefined' === typeof v[2] ? 0 : v[2];

			if(0 !== v[2] && 'object' !== typeof v[2]){
				throw(ADD_ERROR)
			}

			var s_uri = v[0];
			var s_dir = v[1];
			var array_suffix = v[2];
			s_uri = s_uri.toLowerCase();
			s_dir = s_dir.toLowerCase();


			
			if(s_uri[0] !== '/') s_uri = '/'+s_uri;
			var last_po = s_uri.length-1;
			if(s_uri[last_po] !== '/') s_uri += '/';

			if(s_dir[0] !== '/' && !/^[a-zA-Z][:]+/.test(s_dir)){
				
				var file_array = module.parent.parent.filename.split(path.sep);
				var dirname = file_array.slice(0, file_array.length-1).join(path.sep);

				s_dir = dirname + path.sep + s_dir;
			} 
			var last_po_2 = s_dir.length-1;
			if(s_dir[last_po_2] === path.sep) s_dir = s_dir.slice(0, last_po_2-1);


			if(array_suffix === 0 || array_suffix.length === 0){
				array_suffix = 0;
			}
			else{

				array_suffix.map(function(v){

					if(!/^[a-zA-z0-9]+/.test(v)) throw(ADD_ERROR3)
					return v.toLowerCase();
				})
			}

			static_uri_array.push(s_uri);
			static_dir_array.push(s_dir);
			static_suffix_array.push(array_suffix)
	});


	default_callback = not_found_callback || default_callback;
	if('function' !== typeof default_callback){
		throw(ADD_ERROR2);
	}

	if(static_uri_array.length !== static_dir_array.length ||  static_dir_array.length !== static_suffix_array.length){

			console.log(static_uri_array)
			console.log(static_dir_array)
			console.log(static_suffix_array)
			throw('static_uri_array static_dir_array static_suffix_array length error!')
	}


	if(!ifile_obj.options) ifile_obj.options = {}


	ifile_obj.options.pipe_szie = ifile_obj.options.pipe_szie || ifile_obj.default_options.pipe_szie;

	ifile_obj.options.expired = ifile_obj.options.expired || ifile_obj.default_options.expired;

	ifile_obj.options.gzip = ifile_obj.options.gzip || ifile_obj.default_options.gzip;

	ifile_obj.options.gzip_min_size = ifile_obj.options.gzip_min_size || ifile_obj.default_options.gzip_min_size;

	ifile_obj.options.gzip_file = ifile_obj.options.gzip_file || ifile_obj.default_options.gzip_file;

	ifile_obj.options.gzip_level = ifile_obj.options.gzip_level || ifile_obj.default_options.gzip_level;

	ifile_obj.options._ifile_obj = ifile_obj;


	ifile_obj.options.expired = "max-age="+(ifile_obj.options.expired/1000);

	ifile_obj.options.gzip = ifile_obj.options.gzip ? 1 : 0;

	

	ifile_cc.add(static_uri_array, static_dir_array, static_suffix_array, default_callback, ifile_obj.options);


}


ifile_obj.route = ifile_cc.match;



var add_mime = function(){

	var len = Object.keys(mime.types).length;
	var types = mime.types;
	var types_keys = Object.keys(mime.types);
	var mime_name_array = [];
	var mime_type_array = [];
	for(var i=0;i<len;i++){
		var mkey = types_keys[i];
		var mtype =  types[mkey];

		mime_name_array.push(mkey);
		mime_type_array.push(mtype);

	}

	if(mime_name_array.length !== len || mime_type_array.length !== len){
		console.log(mime_name_array)
		console.log(mime_type_array)
		throw('mime type array length error!')
	}

	ifile_cc.add_mime(mime_name_array, mime_type_array, len)

}()

ifile_obj.options = {}

ifile_obj.connect = function(static_array, not_found_callback){

	if('object' !== typeof static_array){
		var static_array = [
			["/static", path.dirname(module.parent.parent.filename)]
		]
	}
	var not_found_callback = not_found_callback || function(req,res,is_static){

		      if(is_static){
		         res.send(404);
		      }
		      else{
		         req._ifile_next();
		      }

		  }

	ifile_obj.add(static_array, not_found_callback);

	return function(req,res,next){
		req._ifile_next = next;
     	ifile_obj.route(req,res)
	}


}

module.exports =ifile_obj