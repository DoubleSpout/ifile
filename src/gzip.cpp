

#include <iostream>
#include <string>
#include "gzip.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "zlib/zlib.h"

using namespace std;

int Gzip::gzip_uncompress(char *bufin, int lenin, char *bufout, int lenout){

		z_stream d_stream;
        int result;

        d_stream.zalloc = NULL;
        d_stream.zfree  = NULL;
        d_stream.opaque = NULL;

        result = inflateInit2(&d_stream, MAX_WBITS + 16);
        if (result != Z_OK) {
                return -1;
        }
        d_stream.next_in   = reinterpret_cast<Bytef *>(bufin);
        d_stream.avail_in  = lenin;
        d_stream.next_out  = reinterpret_cast<Bytef *>(bufout);
        d_stream.avail_out = lenout;

        inflate(&d_stream, Z_SYNC_FLUSH);
        inflateEnd(&d_stream);
        return 0;

};
int Gzip::gzip_compress(char *bufin, int lenin, char *bufout, int lenout, int level){

	z_stream d_stream;
        int result;

        d_stream.zalloc = NULL;
        d_stream.zfree  = NULL;
        d_stream.opaque = NULL;

        result = deflateInit2(&d_stream, level,
                                Z_DEFLATED, MAX_WBITS + 16,
                                9, Z_DEFAULT_STRATEGY);
        if (result != Z_OK) {
                return -1;
        }
        d_stream.next_in   = reinterpret_cast<Bytef *>(bufin);
        d_stream.avail_in  = lenin; 
        d_stream.next_out  = reinterpret_cast<Bytef *>(bufout);
        d_stream.avail_out = lenout;

        deflate(&d_stream, Z_SYNC_FLUSH);
        deflateEnd(&d_stream);

        return d_stream.total_out;

};
