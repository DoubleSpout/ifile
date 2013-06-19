#ifndef ROUTE_H
#define ROUTE_H
#include <node.h>
#include <time.h>
#include <string>
#include "request.h"
#include "ifile_handler.h"

using namespace v8;

class ifile_class {

 public:
  static Handle<Value> add(const Arguments& args);
  static Handle<Value> match(const Arguments& args);
  static Handle<Value> add_mime(const Arguments& args);

  static void loop_add(Handle<Object> uri_array, Handle<Object> dir_array, Handle<Object> suffix_array, ifile_handler **ifile_handler_p, int len);
  static void worker_callback(uv_work_t* req);
  static void after_worker_callback(uv_work_t *req, int status);
  static void fs_state_callback(uv_fs_t *req);

  static char* mystrsep(char** stringp, const char* delim);
  static inline char* tolower2(char* s);
  static void show_handler_p(ifile_handler **ifile_handler_p, int len);//测试用打印静态文件handler规则数组
  static std::string toCString(Handle<Value> strp);
  static time_t parseLocalDate(char* date);
  static std::string time_to_utc(time_t *time_p);
  static void create_etag(unsigned long mtime, unsigned long size, std::string &etag_str);
  ifile_class(){};
  ~ifile_class(){};


  
};

#endif