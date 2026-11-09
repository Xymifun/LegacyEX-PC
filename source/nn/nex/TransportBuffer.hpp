#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTBUFFER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTBUFFER_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qReservedQueue.hpp"

namespace nn
{
namespace nex
{

class Buffer;

const unsigned int INHERIT_THREAD_AFFINITY = 0x80000000u;

extern unsigned int g_transportBufferMode;
extern unsigned int g_transportBufferCount;
extern unsigned int g_transportBufferThreadSleep;
extern unsigned int g_transportBufferThreadPriority;
extern unsigned int g_transportBufferSendThreadPriority;
extern unsigned int g_transportRecvCountPerCycle;
extern unsigned int g_transportSendCountPerCycle;
extern unsigned int g_transportBufferThreadAffinity;
extern bool g_transportBufferPoolEnabled;

class TransportBuffer : public RootObject
{
public:
    TransportBuffer(unsigned int uiCount, unsigned int uiBufferSize);
    ~TransportBuffer();

    void ReserveBuffer(unsigned int uiBufferSize, unsigned int uiCount);
    Buffer* AllocateBuffer(unsigned int uiSize);
    void FreeBuffer(Buffer* pBuffer);

    unsigned int GetEmptyDropCount() const { return m_uiEmptyDropCount; }

private:
    qReservedQueue<Buffer*>* m_pBuffers;
    ForcedCriticalSection m_criticalSection;
    unsigned int m_uiEmptyDropCount;
};

}
}

#endif
