#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PACKETBUFFERMULTIUNITHEAP_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PACKETBUFFERMULTIUNITHEAP_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/MutexPrimitive.hpp"
#include "nn/nex/PacketBufferHeap.hpp"
#include "nn/nex/PacketBufferManager.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

extern bool g_threadSafetyEnabled;

class PacketBufferMultiUnitHeap : public RootObject
{
public:
    PacketBufferMultiUnitHeap()
        : m_pUnitHeaps(0),
          m_uiUnitHeapNum(0),
          m_uiMaxUnitSize(0),
          m_bInitialized(false),
          m_bUnitExhausted(false),
          m_pMutex(0)
    {
    }

    ~PacketBufferMultiUnitHeap()
    {
        if (m_bInitialized)
            Finalize();
    }

    void Initialize(void* pMemory, unsigned int uiMemorySize, qVector<PacketBufferManager::UnitHeapParam> aParams);
    void Finalize();

    void* Allocate(unsigned int uiSize, MemoryManager::Deallocator pDeallocator);
    void Free(void* pBlock);

    void GetDebugInfo(qVector<PacketBufferManager::UnitHeapDebugInfo>& rDebugInfo) const;

private:
    friend class PacketBufferManager;

    void Lock() const
    {
        if (m_pMutex != 0 && !g_criticalSectionDisabled)
            m_pMutex->Lock();
    }

    void Unlock() const
    {
        if (m_pMutex != 0 && !g_criticalSectionDisabled)
            m_pMutex->Unlock();
    }

    PacketBufferUnitHeap* m_pUnitHeaps;
    unsigned int m_uiUnitHeapNum;
    unsigned int m_uiMaxUnitSize;
    bool m_bInitialized;
    bool m_bUnitExhausted;
    MutexPrimitive* m_pMutex;
};

}
}

#endif
