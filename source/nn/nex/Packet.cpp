#include "nn/nex/Packet.hpp"

#include "nn/nex/Buffer.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int PACKET_OVERHEAD = 0x40;
const unsigned int HISTORY_BUNDLE_PORT = 1;

bool s_historyBundleReliableEnabled = true;
bool s_historyBundlePlainEnabled = true;

}

bool g_historyBundlingEnabled = false;
bool g_historyBundleWithoutFlag = true;
unsigned short g_historyPacketLimit = 32;
unsigned short g_bundlePacketLimit = 0x7FFF;
unsigned short g_bundleSendLimit = 2;
int g_historyBundleDelay = 150;
int g_historyBundleAge = 182;
unsigned int g_bundleBufferSize = 1364;

Packet::Packet()
    : m_link(),
      m_tempLink(),
      m_historyLink(),
      m_uiVersion(0),
      m_uiSource(0),
      m_uiDestination(0),
      m_uiSignatureSize(0),
      m_uiTypeAndFlags(0),
      m_sequenceID(),
      m_unknown4c(),
      m_uiConnectionSignatureSize(0),
      m_uiFragmentID(0),
      m_uiSessionID(0),
      m_uiTTL(0),
      m_unknown6b(0),
      m_uiSubStreamID(0),
      m_uiPort(0),
      m_bSkipEncryption(false),
      m_bHeaderParsed(0),
      m_unknown90(0),
      m_stream(),
      m_pPayload(0),
      m_address(),
      m_unknownF8(0),
      m_queueTime(0),
      m_historyTime(0)
{
}

Packet::~Packet()
{
    if (m_pPayload != 0)
    {
        m_pPayload->ReleaseRef();
    }
}

bool Packet::SetPayload(Buffer* pPayload)
{
    if (m_pPayload != 0)
    {
        m_pPayload->ReleaseRef();
    }
    m_pPayload = pPayload;
    return true;
}

bool Packet::CanHistoryBundle() const
{
    bool canBundle = false;
    if ((m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_DATA
        && VirtualPort(m_uiSource).GetType() == HISTORY_BUNDLE_PORT)
    {
        if (s_historyBundlePlainEnabled || (m_uiTypeAndFlags & PACKET_FLAG_RELIABLE)
            || (m_uiTypeAndFlags & PACKET_FLAG_ACK))
        {
            if (s_historyBundleReliableEnabled && !(m_uiTypeAndFlags & PACKET_FLAG_ACK))
            {
                canBundle = true;
            }
            else if (!(m_uiTypeAndFlags & PACKET_FLAG_RELIABLE) && !(m_uiTypeAndFlags & PACKET_FLAG_ACK))
            {
                canBundle = true;
            }
        }
    }
    return canBundle;
}

unsigned int Packet::GetOverhead()
{
    return PACKET_OVERHEAD;
}

}
}
