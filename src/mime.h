#ifndef Mime_H
#define Mime_H
#include <node.h>
#include <iostream>
#include <string>


using namespace v8;

class Mime {

 public:

    char *mime_name;//文件后缀名,比如.js文件保存为js
    int mime_name_len;
    char *mime_type;//文件响应类型比如,text/plain
    int mime_type_len;

    Mime(){};
    ~Mime(){};
  
};

#endif