#ifndef _MD5_H
#define _MD5_H

// This optimized MD5 implementation conforms to RFC 1321.
// Source: http://www.cr0.net:8040/code/crypto/md5/
// See copyright information in md5.c

typedef unsigned char uint8;
typedef unsigned long uint32;

struct md5_context
{
    uint32 total[2];
    uint32 state[4];
    uint8 buffer[64];
};

extern "C" {
	void md5_starts( md5_context *ctx );
	void md5_update( md5_context *ctx, uint8 *input, uint32 length );
	void md5_finish( md5_context *ctx, uint8 digest[16] );
}

#endif /* md5.h */
