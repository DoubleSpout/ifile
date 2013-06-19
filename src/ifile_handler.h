#ifndef IFILE_HANDLER_H
#define IFILE_HANDLER_H
#include <node.h>
#include <string>


using namespace v8;

class ifile_handler {

 public:

    char *static_uri;  //静态文件的url地址
    int static_uri_len;
    char *static_dir;  //静态文件的路径地址
    int static_dir_len; 
    char **file_type;  //静态文件的后缀名数组
    int file_type_len;  //后缀名判断数组长度
    int is_file_type;  //是否设置后缀名判断		
   

    ifile_handler(){};
    ~ifile_handler(){};
  
};

#endif