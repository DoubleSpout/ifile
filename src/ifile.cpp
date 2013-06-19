#include <node.h>
#include "ifile_class.h"



using namespace v8;

void Init(Handle<Object> target) {

  HandleScope scope;

  target->Set(String::NewSymbol("add"),
           FunctionTemplate::New(ifile_class::add)->GetFunction());

 
  target->Set(String::NewSymbol("match"),
           FunctionTemplate::New(ifile_class::match)->GetFunction());
     
           
  target->Set(String::NewSymbol("add_mime"),
           FunctionTemplate::New(ifile_class::add_mime)->GetFunction());


}

NODE_MODULE(ifile, Init)