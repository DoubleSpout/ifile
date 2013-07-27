#include <node.h>
#include <string>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "ifile_class.h"
#include "ifile_handler.h"
#include "request.h"
#include "mime.h"
#include "gzip.h"
#include "uv.h"
#include "zlib/zlib.h"


using namespace v8;

namespace ifile{
  
  static int count = 0;

  static int isAdd = 0;
	
  static ifile_handler **ifile_handler_p;           //指向get的handler实例指针数组的指针
  static int ifile_handler_p_len;

  static Mime **mime_p;//mime文件类型数组指针
  static int mime_p_len;//mime数组长度
  

  static Persistent<Object> default_callback;     //默认回调函数，表示未匹配到控制器

  static uv_loop_t *loop = uv_default_loop();

  static uv_rwlock_t handler_lock;                 //规则线程锁
  static uv_rwlock_t mime_lock;					   //mime线程锁
  static uv_rwlock_t gzip_lock;					   //gzip线程锁

  static Persistent<Object> default_obj = Persistent<Object>::New(Object::New()); //默认404函数执行的对象


  static Persistent<Object> ifile_obj;

  static std::string cache_controller;
  static int pipe_size;
  static int is_config_gzip;
  static int gzip_min_size;
  static int gzip_level = Z_DEFAULT_COMPRESSION;
  static char **gzip_file;
  static int gzip_file_len;

}
//add param
/*
static_uri_array
static_dir_array
not_found_callback
*/
Handle<Value> ifile_class::add(const Arguments& args){
	HandleScope scope;

	if(ifile::isAdd){ //判断是否已经执行过add
		return ThrowException(Exception::TypeError(String::New("ifile::add can only execute once")));
	}	


	Local<Object> uri_array = args[0]->ToObject(); //获取uri数组
	Local<Object> dir_array = args[1]->ToObject(); //获取静态目录数组
	Local<Object> suffix_array = args[2]->ToObject(); //获取后缀名数组


	ifile::default_callback = Persistent<Object>::New(args[3]->ToObject());//获取未匹配到回调函数
	Local<Object> options = args[4]->ToObject(); //获取后缀名数组

	ifile::ifile_handler_p_len = Local<Array>::Cast(args[0])->Length();//获取静态handler数组长度
	ifile::ifile_handler_p = new ifile_handler*[ifile::ifile_handler_p_len]; //创建ifile_handler_p_len类的动态指针数组

//申请内存，保存静态文件handler规则
	loop_add(uri_array, dir_array, suffix_array, ifile::ifile_handler_p, ifile::ifile_handler_p_len); 


//保存options的属性
	ifile::ifile_obj = Persistent<Object>::New(options->Get(String::New("_ifile_obj"))->ToObject());
	ifile::pipe_size = options->Get(String::New("pipe_szie"))->Int32Value();
	ifile::cache_controller = toCString(options->Get(String::New("expired")));
	ifile::is_config_gzip = options->Get(String::New("gzip"))->Int32Value();
	ifile::gzip_min_size = options->Get(String::New("gzip_min_size"))->Int32Value();
	ifile::gzip_level = options->Get(String::New("gzip_level"))->Int32Value();
	ifile::gzip_file_len = Local<Array>::Cast(options->Get(String::New("gzip_file")))->Length();

	Local<Object> file_array = options->Get(String::New("gzip_file"))->ToObject();
	ifile::gzip_file = new char*[ifile::gzip_file_len];

	for(int i=0;i<ifile::gzip_file_len;i++){

		String::Utf8Value suffix_val(file_array->Get(i)->ToString());//依次获取后缀名
		ifile::gzip_file[i] = new char[16];
		memset(ifile::gzip_file[i],'\0',16);
		strcpy(ifile::gzip_file[i], *suffix_val); //循环录入后缀名

		//std::cout<<ifile::gzip_file[i]<<std::endl;
	}

	ifile::isAdd = 1;

	//调试用
	//打印插入的静态文件handler规则
	//show_handler_p(ifile::ifile_handler_p, ifile::ifile_handler_p_len);


	return scope.Close(Undefined()); 
};


void ifile_class::loop_add(Handle<Object> uri_array, Handle<Object> dir_array, Handle<Object> suffix_array, ifile_handler **ifile_handler_p, int len){
	HandleScope scope;

	for(int i=0;i<len;i++){

		ifile_handler_p[i] = new ifile_handler();

		String::Utf8Value uri_val(uri_array->Get(i)->ToString());
		String::Utf8Value dir_val(dir_array->Get(i)->ToString());

		ifile_handler_p[i]->static_uri = new char[strlen(*uri_val)+1];
		strcpy(ifile_handler_p[i]->static_uri , *uri_val); //录入uri匹配规则
		ifile_handler_p[i]->static_uri[strlen(*uri_val)] = '\0';
		ifile_handler_p[i]->static_uri_len = strlen(*uri_val);

		ifile_handler_p[i]->static_dir = new char[strlen(*dir_val)+1]; 
		strcpy(ifile_handler_p[i]->static_dir, *dir_val);  //录入静态文件，文件路径
		ifile_handler_p[i]->static_dir[strlen(*dir_val)] = '\0';
		ifile_handler_p[i]->static_dir_len = strlen(*dir_val);

		ifile_handler_p[i]->is_file_type = suffix_array->Get(i)->IsNumber() ? 0 : 1; //此静态文件路由规则是否需要进行后缀名匹配

		if(ifile_handler_p[i]->is_file_type && suffix_array->Get(i)->IsArray()){//如果需要匹配，则需要循环录入静态文件后缀名

			ifile_handler_p[i]->file_type_len = Local<Array>::Cast(suffix_array->Get(i))->Length(); //录入后缀名数组长度
			ifile_handler_p[i]->file_type = new char*[ifile_handler_p[i]->file_type_len]; //创建后缀名数值


			for(int j=0;j<ifile_handler_p[i]->file_type_len;j++){

				String::Utf8Value suffix_val(suffix_array->Get(i)->ToObject()->Get(j)->ToString());//依次获取后缀名
				ifile_handler_p[i]->file_type[j] = new char[strlen(*suffix_val)+1];
				strcpy(ifile_handler_p[i]->file_type[j], *suffix_val); //循环录入后缀名
				ifile_handler_p[i]->file_type[j][strlen(*suffix_val)] = '\0';

			}//end for j

		}
		else{ //如果传递上来的不是数组也不是数字，则不匹配
			ifile_handler_p[i]->is_file_type = 0;
		}


	}//end for i

};

//add mime type array
Handle<Value> ifile_class::add_mime(const Arguments& args){

	HandleScope scope;

	ifile::mime_p_len = args[2]->Int32Value();
	ifile::mime_p = new Mime*[ifile::mime_p_len];

	Local<Object> m_name_array = args[0]->ToObject(); //mime name 数组
	Local<Object> m_type_array = args[1]->ToObject(); //mime type 数组

	for(int i=0;i<ifile::mime_p_len;i++){

		ifile::mime_p[i] = new Mime();

		String::Utf8Value m_name_val(m_name_array->Get(i)->ToString());
		String::Utf8Value m_type_val(m_type_array->Get(i)->ToString());

		ifile::mime_p[i]->mime_name = new char[strlen(*m_name_val)+1];
		ifile::mime_p[i]->mime_type = new char[strlen(*m_type_val)+1];

		ifile::mime_p[i]->mime_name_len = strlen(*m_name_val);
		ifile::mime_p[i]->mime_type_len = strlen(*m_type_val);	

		strcpy(ifile::mime_p[i]->mime_name,*m_name_val);
		ifile::mime_p[i]->mime_name[strlen(*m_name_val)] = '\0';

		strcpy(ifile::mime_p[i]->mime_type,*m_type_val);
		ifile::mime_p[i]->mime_type[strlen(*m_type_val)] = '\0';

		//std::cout<<ifile::mime_p[i]->mime_name<<std::endl;
		//std::cout<<ifile::mime_p[i]->mime_type<<std::endl;
	}

	return scope.Close(Undefined()); 
}






Handle<Value> ifile_class::match(const Arguments& args){
	HandleScope scope;

	if(!ifile::isAdd){
		return ThrowException(Exception::TypeError(String::New("please run ifile.add(..) first!")));
	}

	std::string req_method = toCString(args[0]->ToObject()->Get(String::New("method")));



	if(req_method != "GET" && req_method != "HEAD"){ //如果不是get或者head请求，则直接返回错误

		Local<Value> argv[2];
		argv[0] = args[0];
		argv[1] = args[1];

		ifile::default_callback->CallAsFunction(ifile::default_obj, 2, argv);
		return scope.Close(Undefined());
	}

	Request *req = new Request();

	String::Utf8Value url(args[0]->ToObject()->Get(String::New("url"))->ToString());//conver
	char *uri_c = *url;
	uri_c = tolower2(uri_c);
	req->url = new char[strlen(uri_c)+1];
	strcpy(req->url , uri_c); //录入uri匹配规则
	req->url[strlen(uri_c)] = '\0';

	req->method = req_method;
	
	req->work_pool.data = req;

	req->res_js_obj = Persistent<Object>::New(args[1]->ToObject());
	req->req_js_obj = Persistent<Object>::New(args[0]->ToObject());

	Local<Value> ims_header = req->req_js_obj->Get(String::New("headers"))->ToObject()->Get(String::New("if-modified-since"));

	if(ims_header->IsString()){//获取if modified since
		String::Utf8Value ims_val(ims_header->ToString());//conver
		req->if_modified_since = new char[strlen(*ims_val)+1];
		strcpy(req->if_modified_since , *ims_val); 
		req->if_modified_since[strlen(*ims_val)] = '\0';
	}


	Local<Value> etag_header = req->req_js_obj->Get(String::New("headers"))->ToObject()->Get(String::New("if-none-match"));

	if(etag_header->IsString()){
		String::Utf8Value etag_val(etag_header->ToString());//conver
		req->if_none_match = new char[strlen(*etag_val)+1];
		strcpy(req->if_none_match , *etag_val); 
		req->if_none_match[strlen(*etag_val)] = '\0';
	}

	Local<Value> accept_encoding = req->req_js_obj->Get(String::New("headers"))->ToObject()->Get(String::New("accept-encoding"));

	if(accept_encoding->IsString()){
		String::Utf8Value accept_encoding_val(accept_encoding->ToString());//conver
		req->accept_encoding = new char[strlen(*accept_encoding_val)+1];
		strcpy(req->accept_encoding , *accept_encoding_val); 
		req->accept_encoding[strlen(*accept_encoding_val)] = '\0';
	}

	req->pipe_size = ifile::pipe_size;
	req->is_config_gzip = ifile::is_config_gzip;
	req->gzip_min_size = ifile::gzip_min_size;
	req->gzip_level = ifile::gzip_level;



	uv_queue_work(ifile::loop, &req->work_pool, worker_callback, after_worker_callback);
	
	return Undefined();
};
	




void ifile_class::worker_callback(uv_work_t* req){ //线程中执行代码

	 char *sign1 = "?";
	 char *sign2 = ".";
	 char *suffix;
	 uv_loop_t *loop_thread = uv_loop_new();
	 //uv_run(loop_thread, UV_RUN_DEFAULT);

	Request* req_p = (Request *) req->data;
	req_p->file_dir = 0; //初始化文件夹路径指针
	req_p->is_find_file = 0; //初始化是否匹配文件的判断依据



#ifdef WIN32
	req_p->no_param_uri = mystrsep(&req_p->url, sign1);
#else
	req_p->no_param_uri = strsep(&req_p->url, sign1);
#endif
	
	req_p->char_uri_sep =  new char[strlen(req_p->no_param_uri)+1];
	strcpy(req_p->char_uri_sep, req_p->no_param_uri );
	req_p->char_uri_sep[strlen(req_p->no_param_uri)] = '\0';

#ifndef TARGET_OS_MAC
	uv_rwlock_tryrdlock(&ifile::handler_lock); //加锁读取handler数组
#endif
	

	for(int i=0;i<ifile::ifile_handler_p_len;i++){

		int loc = strncmp(req_p->char_uri_sep, ifile::ifile_handler_p[i]->static_uri, ifile::ifile_handler_p[i]->static_uri_len);

		if(loc == 0){//如果uri路径匹配到

				do{
#ifdef WIN32
					suffix = mystrsep(&req_p->char_uri_sep, sign2);
#else
					suffix = strsep(&req_p->char_uri_sep, sign2);
#endif					
				}while(req_p->char_uri_sep);


			req_p->suffix = new char[strlen(suffix)+1];
			strcpy(req_p->suffix, suffix);
			req_p->suffix[strlen(suffix)] = '\0';
			req_p->is_static = 1;//表示匹配到请求路径，此请求为正常静态文件请求，在express中不应该next()下发如果未找到则应响应404页面

			if(ifile::ifile_handler_p[i]->is_file_type){ //如果需要匹配后缀名				

				

				int loc2;
				loc2 = strncmp(req_p->no_param_uri,suffix,strlen(suffix));//如果请求url路径里没有.这个字符，则直接break
				if(loc2 == 0){
					break;
				}

				for(int j=0;j<ifile::ifile_handler_p[i]->file_type_len;j++){

					int loc3;
					//循环匹配后缀名
					loc3 = strncmp(suffix,ifile::ifile_handler_p[i]->file_type[j],strlen(ifile::ifile_handler_p[i]->file_type[j]));
					if(loc3 == 0){
						req_p->file_dir = new char[ifile::ifile_handler_p[i]->static_dir_len+1];
						strcpy(req_p->file_dir, ifile::ifile_handler_p[i]->static_dir);
						req_p->file_dir[ifile::ifile_handler_p[i]->static_dir_len] = '\0';
					}
				}
				
	

			}
			else{

				req_p->file_dir = new char[ifile::ifile_handler_p[i]->static_dir_len+1];
				strcpy(req_p->file_dir, ifile::ifile_handler_p[i]->static_dir);
				req_p->file_dir[ifile::ifile_handler_p[i]->static_dir_len] = '\0';

			}

			break;
		}
	}
#ifndef TARGET_OS_MAC
	uv_rwlock_rdunlock(&ifile::handler_lock); //解锁
#endif

	if(!req_p->file_dir){ //如果未匹配到，则不执行去获取文件的代码
		uv_loop_delete(loop_thread);
		return;
	}


	char *path_sep;
	std::string path_str= "";

#ifdef WIN32
 		std::string os_sep = "\\";
#else
  		std::string os_sep = "/";
#endif
  	 char *url_sep = "/";

	do{ //拼接文件的完整路径
		
#ifdef WIN32	
		path_sep = mystrsep(&req_p->no_param_uri, url_sep);
#else		
		path_sep = strsep(&req_p->no_param_uri, url_sep);
#endif	
		if(path_sep && strlen(path_sep)>1){
			char *temp = new char[strlen(path_sep)+1];
			strcpy(temp, path_sep);	
			temp[strlen(path_sep)] = '\0';
			path_str = path_str + os_sep + std::string(temp);

			delete []temp;
		}
		
	}
	while(path_sep);


	std::string path = req_p->file_dir;
	req_p->file_hole_path = path + path_str;
	const char* file_path_char = req_p->file_hole_path.c_str();


	int r = uv_fs_stat(loop_thread, &req_p->fs_t, file_path_char, NULL); //同步获取文件的状态

	if(r == -1){ //如果r=-1则表示未找到
		uv_loop_delete(loop_thread);
		return;
	}
	

	req_p->uv_statbuf_p = &req_p->fs_t.statbuf; //将文件的状态存入req_p指针
	req_p->buf_size = req_p->uv_statbuf_p->st_size;//获取文件的大小

//#ifdef WIN32    
      req_p->mtime = req_p->uv_statbuf_p->st_mtime;//获取文件mtime
//#else       
      //req_p->mtime = req_p->uv_statbuf_p->st_mtim.tv_sec;//获取文件mtime
//#endif 
	
	req_p->content_length = req_p->buf_size;//设置content-length大小
	uv_fs_req_cleanup(&req_p->fs_t);


//获取完文件状态，开始匹配mime_type,读取主线程mime数组，需要加上线程锁
#ifndef TARGET_OS_MAC
	uv_rwlock_tryrdlock(&ifile::mime_lock); 
#endif

	for(int k=0;k<ifile::mime_p_len;k++){


		int loc4 = strncmp(req_p->suffix, ifile::mime_p[k]->mime_name, ifile::mime_p[k]->mime_name_len);
		
		if(loc4 == 0){
			req_p->content_type_len = ifile::mime_p[k]->mime_type_len;
			req_p->content_type = new char[req_p->content_type_len + 1];
			strcpy(req_p->content_type, ifile::mime_p[k]->mime_type);	
			req_p->content_type[req_p->content_type_len] = '\0';
			
			break;
		}

	}
#ifndef TARGET_OS_MAC
	uv_rwlock_rdunlock(&ifile::mime_lock); //解锁
#endif
//mimetype 匹配完毕




//etag匹配
create_etag(static_cast<unsigned long>(req_p->mtime), req_p->buf_size, req_p->etag);

if(req_p->if_none_match){

	const char *etag_char = req_p->etag.c_str();

	//std::cout<<etag_char<<std::endl;
	//std::cout<<req_p->if_none_match<<std::endl;

	if(strncmp(req_p->if_none_match,etag_char,strlen(req_p->if_none_match)) == 0){ //如果etag匹配，则304

		req_p->status_code = 304;//设置响应的状态码
		req_p->is_find_file = 1;//表示读取缓存找到匹配
		uv_loop_delete(loop_thread);
		return;
	}
	
}



//last modify 匹配
if(req_p->if_modified_since){
	
	

	time_t timer = parseLocalDate(req_p->if_modified_since);

	if(timer){
		unsigned long uiNow = static_cast<unsigned long>(timer);
		unsigned long mtime_ts = static_cast<unsigned long>(req_p->mtime);

	//std::cout<<uiNow<<std::endl;
	//std::cout<<mtime_ts<<std::endl;

		if(mtime_ts <= uiNow){ //缓存过期，重新获取

			req_p->status_code = 304;//设置响应的状态码
			req_p->is_find_file = 1;//表示读取缓存找到匹配
			uv_loop_delete(loop_thread);
			return;

		}
	}

}


if(!req_p->content_type){
	char *def_type = "text/plain";
	req_p->content_type_len = strlen(def_type);
	req_p->content_type = new char[req_p->content_type_len + 1];
	strcpy(req_p->content_type, def_type);	
	req_p->content_type[req_p->content_type_len] = '\0';
}

if(req_p->method == "HEAD"){
	req_p->is_find_file = 1;//表示读取缓存找到匹配
	uv_loop_delete(loop_thread);
	return;
}



if(req_p->buf_size > req_p->pipe_size){ //如果超过了大小，必须使用pipe输出,不将文件读入内存中了
	req_p->is_pipe = 1;
	req_p->is_find_file = 1;
	uv_loop_delete(loop_thread);

	return;
}


	r = uv_fs_open(loop_thread, &req_p->fs_t, file_path_char, O_RDONLY, 0, NULL); //打开文件

	if(r==-1){ //如果打开失败
		uv_fs_req_cleanup(&req_p->fs_t);
		uv_loop_delete(loop_thread);
		return;
	}
	uv_fs_req_cleanup(&req_p->fs_t);
	

	req_p->buf = new char[req_p->buf_size+1]; //new内存地址保存文件
	
	uv_fs_t read_req;
	r = uv_fs_read(loop_thread, &read_req, req_p->fs_t.result, 
                   req_p->buf, req_p->buf_size+1, 0, NULL); //读取文件内容

	if(r != -1){
		req_p->is_find_file = 1;
	}
	
	uv_fs_close(loop_thread, &req_p->fs_t, req_p->fs_t.result, NULL);//关闭文件读取

	uv_fs_req_cleanup(&req_p->fs_t);
	uv_loop_delete(loop_thread);

	//判断是否gzip压缩
if(req_p->accept_encoding && req_p->is_config_gzip  && req_p->buf_size > req_p->gzip_min_size ){//如果request有gzip的accept-encoding头则

		const char *gzip = "gzip";
		char *is_gzip_head = strstr(tolower2(req_p->accept_encoding), gzip);


		if(is_gzip_head){//如果匹配成功需要gzip


		int file_name_gzip = 0;


#ifndef TARGET_OS_MAC
	uv_rwlock_tryrdlock(&ifile::gzip_lock); //加锁读取handler数组
#endif

	for(int n=0; n<ifile::gzip_file_len; n++){


	//	std::cout<<req_p->suffix<<std::endl;
	//	std::cout<<ifile::gzip_file[n]<<std::endl;
	//	std::cout<<strlen(ifile::gzip_file[n])<<std::endl;

		int loc6 = strncmp(req_p->suffix, ifile::gzip_file[n], strlen(ifile::gzip_file[n]));
		
		if(loc6 == 0){
			file_name_gzip = 1;		
			break;
		}

	}

#ifndef TARGET_OS_MAC
	uv_rwlock_rdunlock(&ifile::gzip_lock); //解锁
#endif


		if(file_name_gzip){

				req_p->buf_gzip = new char[req_p->buf_size];

				int gzip_r = -1;
				if(req_p->gzip_level>0 && req_p->gzip_level<10){
					gzip_r = Gzip::gzip_compress(req_p->buf, req_p->buf_size, req_p->buf_gzip, req_p->buf_size, req_p->gzip_level);
				}
				else{
					gzip_r = Gzip::gzip_compress(req_p->buf, req_p->buf_size, req_p->buf_gzip, req_p->buf_size);
				}
				
				//std::cout<<"#########"<<std::endl;
				//std::cout<<gzip_r<<std::endl;
				if(gzip_r != -1){
					req_p->is_gzip = 1;
					req_p->buf_gzip_size = gzip_r;
				}
				
			}

		}// end if(is_gzip_head) 


	}//end if gzip
	
};



 void ifile_class::after_worker_callback(uv_work_t *req, int status){//主线程中执行代码

 		HandleScope scope;

 		Request* req_p = (Request *) req->data;


 		if(req_p->is_find_file){

 			//创建etag头部	
 			Local<Value> argv_5[2];
 			argv_5[0] = String::New("ETag");
 			argv_5[1] = String::New(req_p->etag.c_str());
 			req_p->res_js_obj->Get(String::New("setHeader"))->ToObject()->CallAsFunction(req_p->res_js_obj, 2, argv_5);


 			//开始拼接last-modify头部
 			req_p->last_modify = time_to_utc(&req_p->mtime);
 			Local<Value> argv_4[2];
 			argv_4[0] = String::New("Last-Modified");
 			argv_4[1] = String::New(req_p->last_modify.c_str());
 			req_p->res_js_obj->Get(String::New("setHeader"))->ToObject()->CallAsFunction(req_p->res_js_obj, 2, argv_4);

			//创建maxage
			const char *max_age = ifile::cache_controller.c_str();
 			Local<Value> argv_6[2];
 			argv_6[0] = String::New("Cache-Control");
 			argv_6[1] = String::New(max_age);
 			req_p->res_js_obj->Get(String::New("setHeader"))->ToObject()->CallAsFunction(req_p->res_js_obj, 2, argv_6); 			
 			
			//设置content-type
			Local<Value> argv_1[2];
			argv_1[0] = String::New("Content-Type");
			argv_1[1] = String::New(req_p->content_type, req_p->content_type_len);
			req_p->res_js_obj->Get(String::New("setHeader"))->ToObject()->CallAsFunction(req_p->res_js_obj, 2, argv_1); 

 			if(req_p->status_code == 304 || req_p->method == "HEAD"){//如果响应304

				req_p->res_js_obj->Set(String::New("statusCode"),Number::New(req_p->status_code));//设置响应状态码304

 				Local<Value> argv_3[1];
 				argv_3[0] = String::New("");
 				req_p->res_js_obj->Get(String::New("end"))->ToObject()->CallAsFunction(req_p->res_js_obj, 1, argv_3); //执行res.end函数

 			}
 			else if(req_p->is_pipe == 1){ //如果是pipe输出

 				Local<Object> state = Object::New(); //生成file state 对象，免去ndoejs再去执行一次fs.state函数
 				state->Set(String::New("size"),Number::New(req_p->content_length));
 				state->Set(String::New("mtime"),Number::New(static_cast<unsigned long>(req_p->mtime)));
 				state->Set(String::New("suffix"),String::New(req_p->suffix));
 				state->Set(String::New("file_path"),String::New(req_p->file_hole_path.c_str()));
 				state->Set(String::New("is_config_gzip"),Number::New(req_p->is_config_gzip));
 				state->Set(String::New("gzip_min_size"),Number::New(req_p->gzip_min_size));
				state->Set(String::New("gzip_level"),Number::New(req_p->gzip_level));

 				Persistent<Value> argv_4[3];
				argv_4[0] = req_p->req_js_obj;
				argv_4[1] = req_p->res_js_obj;
				argv_4[2] = Persistent<Object>::New(state);

 				ifile::ifile_obj->Get(String::New("pipe"))->ToObject()->CallAsFunction(ifile::ifile_obj, 3, argv_4); //调用ifile.pipe方法并将req、res和state对象传递出去

 				argv_4[2].Dispose();
 			}
 			else{//如果响应200


 				int content_len = req_p->is_gzip ? req_p->buf_gzip_size : req_p->content_length;
 				//设置content-length
	 			Local<Value> argv_2[2];
	 			argv_2[0] = String::New("Content-Length");
	 			argv_2[1] = Number::New(content_len);
	 			req_p->res_js_obj->Get(String::New("setHeader"))->ToObject()->CallAsFunction(req_p->res_js_obj, 2, argv_2); 


	 			if(req_p->is_gzip){//如果是gzip压缩
	 				Local<Value> argv_7[2];
		 			argv_7[0] = String::New("Content-Encoding");
		 			argv_7[1] = String::New("gzip");
		 			req_p->res_js_obj->Get(String::New("setHeader"))->ToObject()->CallAsFunction(req_p->res_js_obj, 2, argv_7); 

		 			req_p->buffer_p = node::Buffer::New(req_p->buf_gzip, req_p->buf_gzip_size); //创建buffer gzip

	 			}
	 			else{//如果不是gzip
		 			req_p->buffer_p = node::Buffer::New(req_p->buf, req_p->buf_size); //创建buffer
		 		}


			 		Persistent<Value> argv[1];
					argv[0] = req_p->buffer_p->handle_; //创建nodejs的buffer实例
			 		req_p->res_js_obj->Get(String::New("end"))->ToObject()->CallAsFunction(req_p->res_js_obj, 1, argv); //执行res.end函数
			 		argv[0].Dispose();

	 		}

 		}
 		else{

 			Persistent<Value> argv[3];
			argv[0] = req_p->req_js_obj;
			argv[1] = req_p->res_js_obj;
			argv[2] = Persistent<Number>::New(Number::New(req_p->is_static));

	 		ifile::default_callback->CallAsFunction(ifile::default_obj, 3, argv); //执行res.end函数

	 		argv[2].Dispose();
 		}

		delete req_p;
 	
 }


char* ifile_class::mystrsep(char** stringp, const char* delim)
{
  char* start = *stringp;
  char* p;

  p = (start != NULL) ? strpbrk(start, delim) : NULL;

  if (p == NULL)
  {
    *stringp = NULL;
  }
  else
  {
    *p = '\0';
    *stringp = p + 1;
  }

  return start;
}

char* ifile_class::tolower2(char* s)
{

	for(int i = 0; i<strlen(s); i++){
         s[i] = tolower(s[i]);
        
	}
  
  return s;
}

std::string ifile_class::toCString(Handle<Value> strp){
      String::Utf8Value value(strp->ToString());//conver to utf8-value
      std::string str = *value;
      return str;
}

time_t ifile_class::parseLocalDate(char *date)
{
    tm tm_;
    int year, day, hour, minute,second;
    char month[3];
    char week[3];
    char gmt[3];

    sscanf(date,"%s %d %s %d %d:%d:%d %s", &week, &day, &month, &year, &hour, &minute, &second, &gmt);
    tm_.tm_year  = year-1900;
    tm_.tm_mday  = day;
    tm_.tm_hour  = hour;
    tm_.tm_min   = minute;
    tm_.tm_sec   = second;
    tm_.tm_isdst = 0;

    char Jan[] = "Jan";
    char Feb[] = "Feb";
    char Mar[] = "Mar";
    char Apr[] = "Apr";
    char May[] = "May";
    char Jun[] = "Jun";
    char Jul[] = "Jul";
    char Aug[] = "Aug";
    char Sep[] = "Sep";
    char Oct[] = "Oct";
    char Nov[] = "Nov";
    char Dec[] = "Dec";


	if(strncmp(month,Jan,3) == 0){
		tm_.tm_mon = 0;
	}
	else if(strncmp(month,Feb,3) == 0){
		tm_.tm_mon = 1;
	}
	else if(strncmp(month,Mar,3) == 0){
		tm_.tm_mon = 2;
	}
	else if(strncmp(month,Apr,3) == 0){
		tm_.tm_mon = 3;
	}
	else if(strncmp(month,May,3) == 0){
		tm_.tm_mon = 4;
	}
	else if(strncmp(month,Jun,3) == 0){
		tm_.tm_mon = 5;
	}
	else if(strncmp(month,Jul,3) == 0){
		tm_.tm_mon = 6;
	}
	else if(strncmp(month,Aug,3) == 0){
		tm_.tm_mon = 7;
	}
	else if(strncmp(month,Sep,3) == 0){
		tm_.tm_mon = 8;
	}
	else if(strncmp(month,Oct,3) == 0){
		tm_.tm_mon = 9;
	}
	else if(strncmp(month,Nov,3) == 0){
		tm_.tm_mon = 10;
	}
	else if(strncmp(month,Dec,3) == 0){
		tm_.tm_mon = 11;
	}
	else{
		return 0;
	}

    time_t t_ = mktime(&tm_); //已经减了8个时区
    return t_; //秒时间
}

  void ifile_class::show_handler_p(ifile_handler **ifile_handler_p, int len){ //测试用打印静态文件handler规则数组
  		using namespace std;
  		
  		for(int i=0;i<len;i++){
  		cout<<"########### one handler begin ###########"<<endl;
  			cout<<ifile_handler_p[i]->static_uri<<endl;
  			cout<<ifile_handler_p[i]->static_dir<<endl;
  			cout<<ifile_handler_p[i]->is_file_type<<endl;
  			
  			if(ifile_handler_p[i]->is_file_type){

  				cout<<ifile_handler_p[i]->file_type_len<<endl;

  				for(int j=0;j<ifile_handler_p[i]->file_type_len;j++){

  						cout<<ifile_handler_p[i]->file_type[j]<<endl;

  				}

  			}
  		cout<<"###########one handler end###########"<<endl;
  		}



  };


  std::string ifile_class::time_to_utc(time_t *time_p){


  		struct tm * ptm;
 		ptm = gmtime(time_p);

 		char space = 32;

		std::string week_day;
 		if(ptm->tm_wday == 0){
 			week_day = "Sun";
 		}
 		else if(ptm->tm_wday == 1){
 			week_day = "Mon";
 		}
 		else if(ptm->tm_wday == 2){
 			week_day = "Tue";
 		}
 		else if(ptm->tm_wday == 3){
 			week_day = "Wed";
 		}
 		else if(ptm->tm_wday == 4){
 			week_day = "Thu";
 		}
 		else if(ptm->tm_wday == 5){
 			week_day = "Fri";
 		}
 		else if(ptm->tm_wday == 6){
 			week_day = "Sat";
 		}

 		std::string month;
 		if(ptm->tm_mon == 0){
 			month = "Jan";
 		}
 		else if(ptm->tm_mon == 1){
 			month = "Feb";
 		}
 		else if(ptm->tm_mon == 2){
 			month = "Mar";
 		}
 		else if(ptm->tm_mon == 3){
 			month = "Apr";
 		}
 		else if(ptm->tm_mon == 4){
 			month = "May";
 		}
 		else if(ptm->tm_mon == 5){
 			month = "Jun";
 		}
 		else if(ptm->tm_mon == 6){
 			month = "Jul";
 		}
 		else if(ptm->tm_mon == 7){
 			month = "Aug";
 		}
 		else if(ptm->tm_mon == 8){
 			month = "Sep";
 		}
 		else if(ptm->tm_mon == 9){
 			month = "Oct";
 		}
 		else if(ptm->tm_mon == 10){
 			month = "Nov";
 		}
 		else if(ptm->tm_mon == 11){
 			month = "Dec";
 		}

 		char char_year[5];

#ifdef WIN32   
		itoa((ptm->tm_year+1900),char_year,10);
#else
		snprintf(char_year, 5, "%d", (ptm->tm_year+1900));
#endif
		std::string year = std::string(char_year);

		char char_day[4];

#ifdef WIN32
		itoa(ptm->tm_mday,char_day,10);
#else
		snprintf(char_day, 4, "%d", ptm->tm_mday);
#endif
		std::string day = std::string(char_day);

		char char_hour[4];
#ifdef WIN32
		itoa(ptm->tm_hour,char_hour,10);
#else
		snprintf(char_hour, 4, "%d", ptm->tm_hour);
#endif
		std::string hour;
		if(ptm->tm_hour<10){
			hour = "0" + std::string(char_hour);
		}
		else{
			hour = std::string(char_hour);
		}

		char char_min[4];
#ifdef WIN32
		itoa(ptm->tm_min,char_min,10);
#else
		snprintf(char_min, 4, "%d", ptm->tm_min);
#endif
		std::string min;
		if(ptm->tm_min<10){
			min = "0" + std::string(char_min);
		}
		else{
			min = std::string(char_min);
		}
		

		char char_sec[4];

#ifdef WIN32
		itoa(ptm->tm_sec,char_sec,10);
#else
		snprintf(char_sec, 4, "%d", ptm->tm_sec);
#endif
		std::string sec;
		if(ptm->tm_sec<10){
			sec = "0" + std::string(char_sec);
		}
		else{
			sec = std::string(char_sec);
		}

 		std::string	utc_str = week_day +","+space + day + space + month + space + year + space + hour +":"+min+":"+sec+" GMT";

 		return utc_str;

  }

void ifile_class::create_etag(unsigned long mtime, unsigned long size, std::string &etag_str){

    	char mtime_char[30];
    	char size_char[30];

#ifdef WIN32
    	itoa(mtime,mtime_char,10);
    	itoa(size,size_char,10);
#else
		snprintf(mtime_char, 30, "%d", mtime);
		snprintf(size_char, 30, "%d", size);
#endif

    	std::string mtime_str = mtime_char;
    	std::string size_str = size_char;

    	etag_str = mtime_str+"_"+size_str;

    	return;
    }