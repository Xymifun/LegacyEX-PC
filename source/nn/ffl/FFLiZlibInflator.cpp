#include <nn/ffl/FFLiZlibInflator.h>

#include <nn/ffl/detail/FFLiAllocatorInterface.h>
#include <nn/ffl/detail/FFLiBufferAllocator.h>

#include <cstring>

namespace
{

class InflateAllocator : public FFLiAllocatorInterface
{
public:
    InflateAllocator(void* pBuffer, u32 size) { m_BufferAllocator.Init(pBuffer, size); }

    virtual ~InflateAllocator() {}

    virtual void* Allocate(u32 size);
    virtual void* Allocate(u32 size, u32 alignment);
    virtual void Free(void* ptr);

private:
    FFLiBufferAllocator m_BufferAllocator;
};

void* InflateAllocator::Allocate(u32 size)
{
    return Allocate(size, 4);
}

void* InflateAllocator::Allocate(u32 size, u32 alignment)
{
    void* ptr = NULL;
    if (m_BufferAllocator.CanAllocate(size, alignment))
        ptr = m_BufferAllocator.Allocate(size, alignment);
    return ptr;
}

void InflateAllocator::Free(void* ptr)
{
}

}

FFLiZlibInflator::FFLiZlibInflator(FFLiAllocatorInterface& allocatorInterface, s32 windowBits)
    : m_ZlibAllocator(allocatorInterface),
      m_IsStreamEnd(false)
{
    std::memset(&m_Stream, 0, sizeof(z_stream));
    m_Stream.opaque = &m_ZlibAllocator;
    m_Stream.zalloc = FFLiZlibAllocator::Allocate;
    m_Stream.zfree = FFLiZlibAllocator::Free;
    inflateInit2(&m_Stream, windowBits);
}

FFLiZlibInflator::~FFLiZlibInflator()
{
    inflateEnd(&m_Stream);
}

s32 FFLiZlibInflator::Process(void** ppDst, u32* pDstSize, const void** ppSrc, u32* pSrcSize, s32 flush)
{
    m_Stream.next_out = (Bytef*)*ppDst;
    m_Stream.avail_out = *pDstSize;
    m_Stream.next_in = (Bytef*)*ppSrc;
    m_Stream.avail_in = *pSrcSize;

    s32 ret = inflate(&m_Stream, flush);
    if (ret == Z_STREAM_END)
        m_IsStreamEnd = true;

    *ppDst = m_Stream.next_out;
    *pDstSize = m_Stream.avail_out;
    *ppSrc = m_Stream.next_in;
    *pSrcSize = m_Stream.avail_in;

    return ret;
}

u32 FFLiZlibInflator::TotalInputSize() const
{
    return m_Stream.total_in;
}

u32 FFLiZlibInflator::TotalOutputSize() const
{
    return m_Stream.total_out;
}
