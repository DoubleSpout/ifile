#ifndef REQUEST_H
#define REQUEST_H
#include <node.h>
#include "uv.h"
#include <iostream>
#include <string>
#include <node_buffer.h>
#include <fcntl.h>

using namespace v8;

class Request {

 public:

    char *url;              //保存url
    char *if_modified_since;  //客户端缓存日期字符串
    char *if_none_match;     //etag对应的请求头,表示静态资源缓存
    char *accept_encoding; //gzip压缩请求头
    int is_gzip;
    std::string method;     //保存请求方法
    char *no_param_uri;     //拆分之后的请求url
    char *char_uri_sep;     //保存用来切分的char*
    char *suffix;           //请求后缀名
    char *file_path;        //文件保存的文件路径
    char *file_dir;         //文件保存的文件夹路径
    std::string file_hole_path;    //读取完整的文件路径
    node::Buffer* buffer_p; //保存nodejs的buffer指针

    uv_statbuf_t *uv_statbuf_p; //uvstatebuf的指针

    char *buf;             //保存文件的buf指针
    unsigned long buf_size;
    char *buf_gzip;       //保存gzip的buf指针
    unsigned long buf_gzip_size;

    uv_work_t work_pool;    //libuv工作线程池
    uv_fs_t  fs_t;           //libuv fs_t

    int is_find_file;       //是否匹配到文件
    int is_static;          //请求是否是static
    Persistent<Object> res_js_obj;   //保存res对象at
    Persistent<Object> req_js_obj;   //保存res对象at

    //res header use
    int status_code;
    char *content_type; 
    int content_type_len;
    unsigned long content_length;
    time_t mtime;              //mtime时间戳
    std::string last_modify;
    std::string etag;


    Request(){
        url = 0;
        if_modified_since=0;
        if_none_match=0;
        accept_encoding=0;
        is_gzip=0;
        no_param_uri = 0;
        char_uri_sep = 0;
        suffix = 0;
        file_path = 0;
        file_dir = 0;
        buf = 0;
        is_static = 0;
        content_length = 0;
        content_type = 0;
        status_code = 200;
        buf_gzip=0;
    };
    ~Request(){
    	
        if(url) delete []url;
        if(if_modified_since) delete[]if_modified_since;
        if(if_none_match) delete[]if_none_match;
        if(accept_encoding)  delete[]accept_encoding;
    	if(no_param_uri) delete []no_param_uri;
    	if(char_uri_sep) delete []char_uri_sep;
    	if(suffix) delete []suffix;
    	if(file_path) delete []file_path;
    	if(file_dir) delete []file_dir;
        if(buf) delete []buf;
        if(content_type) delete[]content_type;
        if(buf_gzip) delete[]buf_gzip;

    	res_js_obj.Dispose();
        req_js_obj.Dispose();
    };
  
};

#endif