#include "nn/nex/TransportBuffer.hpp"

#include "nn/nex/Buffer.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int CRITICAL_SECTION_TRACE_MASK = 0x200000;

}

unsigned int g_transportBufferMode = 1;
unsigned int g_transportBufferCount = 64;
unsigned int g_transportBufferThreadSleep = 2;
unsigned int g_transportBufferThreadPriority = 10;
unsigned int g_transportBufferSendThreadPriority = 10;
unsigned int g_transportRecvCountPerCycle = 32;
unsigned int g_transportSendCountPerCycle = 32;
unsigned int g_transportBufferThreadAffinity = INHERIT_THREAD_AFFINITY;
bool g_transportBufferPoolEnabled = true;

TransportBuffer::TransportBuffer(unsigned int uiCount, unsigned int uiBufferSize)
    : m_criticalSection(CRITICAL_SECTION_TRACE_MASK),
      m_uiEmptyDropCount(0)
{
    m_pBuffers = new qReservedQueue<Buffer*>(uiCount);
    ReserveBuffer(uiBufferSize, uiCount);
}

TransportBuffer::~TransportBuffer()
{
    ScopedCS lock(&m_criticalSection);
    while (!m_pBuffers->empty())
    {
        delete m_pBuffers->front();
        m_pBuffers->pop();
    }
    delete m_pBuffers;
    m_pBuffers = 0;
}

void TransportBuffer::ReserveBuffer(unsigned int uiBufferSize, unsigned int uiCount)
{
    unsigned int i = 0;
    while (i < uiCount)
    {
        m_pBuffers->push(new Buffer(uiBufferSize, 0));
        ++i;
    }
}

Buffer* TransportBuffer::AllocateBuffer(unsigned int uiSize)
{
    Buffer* pBuffer = 0;
    ScopedCS lock(&m_criticalSection);
    if (!m_pBuffers->empty())
    {
        pBuffer = m_pBuffers->front();
        m_pBuffers->pop();
        pBuffer->Clear();
    }
    else
    {
        ++m_uiEmptyDropCount;
    }
    return pBuffer;
}

void TransportBuffer::FreeBuffer(Buffer* pBuffer)
{
    ScopedCS lock(&m_criticalSection);
    m_pBuffers->push(pBuffer);
}

}
}
