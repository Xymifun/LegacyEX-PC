#include "nn/nex/PacketOut.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/PacketBuffer.hpp"
#include "nn/nex/StreamSettings.hpp"
#include "nn/nex/Timeout.hpp"
#include "nn/nex/TransportAdapter.hpp"
#include "nn/nex/VirtualPort.hpp"

namespace nn
{
namespace nex
{

PacketOut::PacketOut(PRUDPEndPoint* pEndPoint, PacketType type, unsigned short uiFlags, Buffer* pPayload,
                     unsigned int uiSize)
    : Packet(),
      m_unknown120(0),
      m_unknown138(0)
{
    Initialize(pEndPoint, type, uiFlags, pPayload, uiSize, (uiFlags & PACKET_FLAG_NEED_ACK) ? new Timeout() : 0);
}

PacketOut::~PacketOut()
{
    if (m_pTimeout != 0)
    {
        delete m_pTimeout;
        m_pTimeout = 0;
    }
}

void PacketOut::Initialize(PRUDPEndPoint* pEndPoint, PacketType type, unsigned short uiFlags, Buffer* pPayload,
                           unsigned int uiSize, Timeout* pTimeout)
{
    m_unknown11a = 0;
    m_unknown120 = 0;
    m_uiTypeAndFlags = m_uiTypeAndFlags | (type & PACKET_TYPE_MASK)
                       | (uiFlags & static_cast<unsigned short>(~PACKET_TYPE_MASK));
    m_pEndPoint = pEndPoint;
    m_unknown140 = uiSize;
    m_unknown118 = 0;
    m_unknown138 = 0;
    if (pPayload != 0)
    {
        pPayload->AddRef();
        SetPayload(pPayload);
    }
    m_unknown130 = 0;
    m_pTimeout = pTimeout;
    m_unknown12c = 0;
    m_unknown129 = false;
    m_unknown128 = false;
}

bool PacketOut::IsValid()
{
    if ((m_uiTypeAndFlags & PACKET_FLAG_NEED_ACK) && m_pTimeout == 0)
        return false;

    return true;
}

bool PacketOut::ReadyToSend()
{
    if ((m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_RAW)
        return true;

    Stream::Type streamType = VirtualPort(m_uiSource).GetType();
    if (m_unknown128)
        return true;

    if (!g_streamSettings[streamType].Get().GetBundling().IsEnabled())
        return true;

    unsigned long long uiQueueTime = m_queueTime;
    return uiQueueTime <= g_streamSettings[streamType].Get().GetBundling().GetLastFlushTime();
}

bool PacketOut::IsBundlable() const
{
    bool bBundlable = false;
    unsigned short uiType = m_uiTypeAndFlags & PACKET_TYPE_MASK;
    if (uiType != PACKET_TYPE_RAW && uiType != PACKET_TYPE_DISCONNECT)
    {
        Stream::Type streamType = VirtualPort(m_uiSource).GetType();
        if (g_streamSettings[streamType].Get().GetBundling().IsEnabled())
            bBundlable = true;
    }

    return bBundlable;
}

bool PacketOut::IsBundlableWithPortTypeCheck(PacketOut* pPacket)
{
    if (m_uiTTL != pPacket->m_uiTTL || m_uiPort != pPacket->m_uiPort)
        return false;

    if (g_transportAdapter->GetTransportMode() == TransportAdapter::TRANSPORT_MODE_1
        && (m_uiTypeAndFlags & PACKET_FLAG_ALTERNATE_KEY_A)
        && (pPacket->m_uiTypeAndFlags & PACKET_FLAG_ALTERNATE_KEY_A))
        return true;

    return m_address == pPacket->m_address;
}

bool PacketOut::SetInternalPayload(Buffer* pPayload)
{
    Buffer* pBuffer;
    if (pPayload->IsPooled())
    {
        pBuffer = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
            PacketBuffer(pPayload->GetLength() + pPayload->GetHeadShiftSize(), 4);
        if (pBuffer == 0 || !pBuffer->IsValid())
        {
            delete pBuffer;
            return false;
        }
    }
    else
    {
        pBuffer = new Buffer(pPayload->GetRealSize(), 0);
    }

    *pBuffer = *pPayload;
    m_pPayload->ReleaseRef();
    m_pPayload = pBuffer;
    return true;
}

}
}
