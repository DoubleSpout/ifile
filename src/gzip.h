#ifndef GZIP_H
#define GZIP_H
#include <node.h>
#include <iostream>
#include <string>
#include "zlib/zlib.h"

using namespace v8;

class Gzip {

 public:
 	static int gzip_uncompress(char *bufin, int lenin, char *bufout, int lenout);
 	static int gzip_compress(char *bufin, int lenin, char *bufout, int lenout);
	Gzip(){};
    ~Gzip(){};
  
};

#endif