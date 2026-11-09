#include "nn/nex/NATTraversalStream.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/NATTraversalEngine.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/StationURL.hpp"

namespace nn
{
namespace nex
{

NATTraversalStream::NATTraversalStream(NATTraversalEngine* pEngine, RootTransport* pTransport)
    : HighLevelStream(NAT, pTransport),
      m_uiReceivedCount(0),
      m_pEngine(pEngine)
{
    Listen(NAT_STREAM_ID);
}

NATTraversalStream::~NATTraversalStream()
{
}

void NATTraversalStream::SendMsg(const StationURL& rStationURL, Buffer* pPayload, unsigned char uiTTL)
{
    Send(*rStationURL.GetInetAddress(), m_Type, m_uiStreamID, pPayload, uiTTL, 0);
}

void NATTraversalStream::DoWork()
{
    if (m_pEngine != 0)
        m_pEngine->Execute();
}

void NATTraversalStream::ReceiveIncomingMsg(const InetAddress& rAddress, Stream::Type type, unsigned char uiStreamID,
                                            Buffer* pPayload, Time receptionTime, unsigned short uiPort)
{
    StationURL stationURL;
    stationURL.SetInetAddress(&rAddress);
    stationURL.SetStreamType(type);
    stationURL.SetStreamID(uiStreamID);
    if (m_pEngine != 0)
        m_pEngine->ReceiveMessage(stationURL, pPayload->GetData(), pPayload->GetLength());
    ++m_uiReceivedCount;
}

}
}
