#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTBUFFERSET_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTBUFFERSET_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/TransportBufferInfo.hpp"
#include "nn/nex/qReservedQueue.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class TransportBuffer;

class TransportBufferSet
{
public:
    TransportBufferSet(TransportBuffer* pTransportBuffer, int iCount);
    ~TransportBufferSet();

    TransportBufferInfo* PopNewSetInfo();
    TransportBufferInfo* PopNewUseInfo();
    void FreeOneBuffer(TransportBufferInfo* pInfo);
    void ClearList_(qReservedQueue<TransportBufferInfo*>& rQueue);

    unsigned int GetEmptyDropCount() const { return m_uiEmptyDropCount; }

private:
    friend class QueuingSocketTransportBuffer;

    TransportBufferInfo* CreateInfo_(Buffer* pBuffer);
    void DeleteOneBuffer_(TransportBufferInfo* pInfo);

    bool m_bHasUse;
    bool m_bFull;
    qReservedQueue<TransportBufferInfo*> m_useInfos;
    qReservedQueue<TransportBufferInfo*> m_freeInfos;
    unsigned int m_unknown4c;
    unsigned int m_uiBufferSize;
    ForcedCriticalSection m_criticalSection;
    unsigned int m_uiEmptyDropCount;
    TransportBuffer* m_pTransportBuffer;
};

}
}

#endif
