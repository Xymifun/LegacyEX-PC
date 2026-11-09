#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_QUAZALZLIB_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_QUAZALZLIB_HPP

#include <cstddef>

struct internal_state;

typedef unsigned char Bytef;
typedef unsigned int uInt;
typedef unsigned long uLong;
typedef void* voidpf;
typedef voidpf (*alloc_func)(voidpf opaque, uInt items, uInt size);
typedef void (*free_func)(voidpf opaque, voidpf address);

typedef struct z_stream_s
{
    Bytef* next_in;
    uInt avail_in;
    uLong total_in;
    Bytef* next_out;
    uInt avail_out;
    uLong total_out;
    char* msg;
    struct internal_state* state;
    alloc_func zalloc;
    free_func zfree;
    voidpf opaque;
    int data_type;
    uLong adler;
    uLong reserved;
} z_stream;

typedef z_stream* z_streamp;

enum
{
    Z_NO_FLUSH = 0,
    Z_FINISH = 4,
    Z_OK = 0,
    Z_STREAM_END = 1,
    Z_BUF_ERROR = -5,
    Z_DEFLATED = 8,
    Z_DEFAULT_COMPRESSION = -1,
    Z_DEFAULT_STRATEGY = 0
};

#define ZLIB_VERSION "1.2.5"

extern "C"
{
int quazal_deflate(z_streamp strm, int flush);
int quazal_deflateEnd(z_streamp strm);
int quazal_deflateReset(z_streamp strm);
int quazal_deflateInit2_(z_streamp strm, int level, int method, int windowBits, int memLevel, int strategy,
                         const char* version, int stream_size);
int quazal_inflate(z_streamp strm, int flush);
int quazal_inflateEnd(z_streamp strm);
int quazal_inflateReset(z_streamp strm);
int quazal_inflateInit_(z_streamp strm, const char* version, int stream_size);
voidpf QuazalCZlibAlloc(voidpf opaque, uInt items, uInt size);
void QuazalCZlibFree(voidpf opaque, voidpf address);
}

#endif
