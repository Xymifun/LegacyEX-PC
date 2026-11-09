#include "nn/pia/common/ZlibCompressor.hpp"

namespace nn
{
namespace pia
{
namespace common
{

namespace
{

enum
{
    cResultNotSupported = 0xc0b,
    cResultOutOfMemory = 0x10c01,
    cResultBufferTooSmall = 0x10c04
};

}

ZlibCompressor::ZlibCompressor()
    : m_workMemory(0),
      m_workMemorySize(0),
      m_allocationCount(0),
      m_allocationOffset(0),
      m_deflatableSize(0)
{
}

ZlibCompressor::~ZlibCompressor()
{
}

bool ZlibCompressor::IsInitialized() const
{
    return m_workMemory != 0;
}

Result ZlibCompressor::Initialize(void* buffer, unsigned int size)
{
    if (buffer == 0)
    {
        return ResultInvalidArgument();
    }

    if ((reinterpret_cast<uintptr_t>(buffer) & 3u) != 0)
    {
        return ResultInvalidArgument();
    }

    if (size < 0x16c4u)
    {
        return ResultInvalidArgument();
    }

    if (IsInitialized())
    {
        return ResultAlreadyInitialized();
    }

    m_workMemory = static_cast<unsigned char*>(buffer);
    m_workMemorySize = size;
    return 0;
}

void ZlibCompressor::Finalize()
{
    m_workMemory = 0;
    m_workMemorySize = 0;
}

void* ZlibCompressor::myAlloc(void* opaque, unsigned int items, unsigned int size)
{
    ZlibCompressor* compressor = static_cast<ZlibCompressor*>(opaque);
    if (compressor->m_allocationCount >= 5)
    {
        return 0;
    }

    unsigned int oldOffset = compressor->m_allocationOffset;
    unsigned int nextOffset = oldOffset + items * size;
    if (compressor->m_workMemorySize < nextOffset)
    {
        return 0;
    }

    compressor->m_allocationOffset = nextOffset;
    ++compressor->m_allocationCount;
    return compressor->m_workMemory + oldOffset;
}

void ZlibCompressor::myFree(void* opaque, void*)
{
    ZlibCompressor* compressor = static_cast<ZlibCompressor*>(opaque);
    if (compressor->m_allocationCount > 0)
    {
        --compressor->m_allocationCount;
    }
}

Result ZlibCompressor::Startup(unsigned char* output, unsigned int outputSize, int level, int windowBits, int memLevel)
{
    if (output == 0)
    {
        return ResultInvalidArgument();
    }

    if (outputSize == 0)
    {
        return ResultInvalidArgument();
    }

    if (static_cast<unsigned int>(level) >= 10u)
    {
        return ResultInvalidArgument();
    }

    if (windowBits < 8 || windowBits > 15)
    {
        return ResultInvalidArgument();
    }

    if (memLevel < 1 || memLevel > 9)
    {
        return ResultInvalidArgument();
    }

    if (!IsInitialized())
    {
        return ResultInvalidState();
    }

    m_stream.opaque = this;
    m_stream.zalloc = &ZlibCompressor::myAlloc;
    m_stream.zfree = &ZlibCompressor::myFree;

    int initResult = deflateInit2(&m_stream, level, Z_DEFLATED, windowBits, memLevel, Z_DEFAULT_STRATEGY);
    if (initResult == Z_OK)
    {
        const uLong bound = deflateBound(&m_stream, 0x80u);
        m_stream.next_out = output;
        m_stream.avail_out = outputSize;
        m_deflatableSize = static_cast<unsigned int>(bound - 0x80u);
        return 0;
    }

    return initResult == Z_MEM_ERROR ? Result(cResultOutOfMemory) : Result(cResultNotSupported);
}

void ZlibCompressor::Cleanup()
{
    deflateEnd(&m_stream);
    m_allocationOffset = 0;
}

Result ZlibCompressor::Deflate(const unsigned char* input, unsigned int inputSize)
{
    if (input == 0)
    {
        return ResultInvalidArgument();
    }

    if (inputSize == 0)
    {
        return ResultInvalidArgument();
    }

    if (m_stream.avail_out < inputSize + m_deflatableSize + 6u)
    {
        return Result(cResultBufferTooSmall);
    }

    m_stream.next_in = const_cast<Bytef*>(input);
    m_stream.avail_in = inputSize;
    if (deflate(&m_stream, Z_SYNC_FLUSH) == Z_OK)
    {
        if (m_stream.avail_in == 0)
        {
            return 0;
        }

        if (m_stream.avail_out == 0)
        {
            return Result(cResultNotSupported);
        }

        return Result(cResultNotSupported);
    }

    return Result(cResultNotSupported);
}

Result ZlibCompressor::FinishDeflate(unsigned int* outputSize)
{
    if (outputSize == 0)
    {
        return ResultInvalidArgument();
    }

    if (m_stream.total_out == 0)
    {
        return ResultInvalidState();
    }

    if (m_stream.avail_in != 0)
    {
        return ResultInvalidState();
    }

    if (deflate(&m_stream, Z_FINISH) == Z_STREAM_END)
    {
        *outputSize = static_cast<unsigned int>(m_stream.total_out);
        return 0;
    }

    return Result(cResultNotSupported);
}

}
}
}
