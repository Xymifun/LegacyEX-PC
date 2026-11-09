#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_ZLIBCOMPRESSOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_ZLIBCOMPRESSOR_HPP

#include "nn/pia/Common.hpp"
#include <zlib.h>

namespace nn
{
namespace pia
{
namespace common
{

class ZlibCompressor
{
public:
    ZlibCompressor();
    ~ZlibCompressor();

    bool IsInitialized() const;
    Result Initialize(void* buffer, unsigned int size);
    void Finalize();

    static void* myAlloc(void* opaque, unsigned int items, unsigned int size);
    static void myFree(void* opaque, void* address);

    Result Startup(unsigned char* output, unsigned int outputSize, int level, int windowBits, int memLevel);
    void Cleanup();
    Result Deflate(const unsigned char* input, unsigned int inputSize);
    Result FinishDeflate(unsigned int* outputSize);

    static void compressImpl(unsigned char*, unsigned int*, const unsigned char*, unsigned long, int);
    static void Compress(unsigned char*, unsigned int*, const unsigned char*, unsigned long, int);
    void Trace(unsigned long long) const;
    unsigned int GetAvailableOutputSize() const;
    unsigned int GetDeflatableSize() const;
    unsigned int GetMemoryUsage() const;

private:
    unsigned char* m_workMemory;
    unsigned int m_workMemorySize;
    int m_allocationCount;
    unsigned int m_allocationOffset;
    unsigned int m_deflatableSize;
    z_stream m_stream;
};


}
}
}

#endif
