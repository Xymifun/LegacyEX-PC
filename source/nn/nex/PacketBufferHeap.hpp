#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PACKETBUFFERHEAP_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PACKETBUFFERHEAP_HPP

#include "nn/nex/PacketBufferManager.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class PacketBufferUnitHeap : public RootObject
{
public:
    struct Unit
    {
        Unit* m_pNext;
        PacketBufferUnitHeap* m_pUnitHeap;
    };

    PacketBufferUnitHeap();

    void ResetDebugInfo();

private:
    friend class PacketBufferMultiUnitHeap;
    friend class PacketBufferManager;

    void* m_pMemory;
    unsigned int m_uiTotalSize;
    unsigned int m_uiUnitSize;
    unsigned int m_uiUsedUnitNum;
    Unit* m_pFreeList;
    PacketBufferManager::UnitHeapDebugInfo m_debugInfo;
};

}
}

#endif
