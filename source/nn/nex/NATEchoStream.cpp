#include "nn/nex/NATEchoStream.hpp"

#include <wchar.h>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/ByteStream.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/VirtualPort.hpp"

namespace nn
{
namespace nex
{

void NATEchoStream::PrepareMessage(Buffer* pRequest, Buffer* pResponse)
{
    PacketOut* pPacket = new PacketOut(0, PACKET_TYPE_USER, 0, pRequest, 0);
    pPacket->m_uiSource = VirtualPort(ECHO_STREAM_TYPE, ECHO_STREAM_ID).m_uiValue;
    pPacket->m_uiDestination = VirtualPort(ECHO_STREAM_TYPE, ECHO_STREAM_ID).m_uiValue;
    pPacket->m_uiVersion = 0;
    ByteStream message;
    Network::GetInstance()->GetMessageSelector().Pack(pPacket, &message);
    *pResponse = *message.GetBuffer();
    pPacket->ReleaseRef();
}

NATEchoStream::~NATEchoStream()
{
}

void NATEchoStream::ReceiveIncomingMsg(const InetAddress& rAddress, Stream::Type type, unsigned char uiStreamID,
                                       Buffer* pPayload, Time receptionTime, unsigned short uiPort)
{
    ByteStream response;
    response.AppendRaw(pPayload->GetData(), pPayload->GetLength());
    StationURL sourceURL;
    sourceURL.SetURLType(StationURL::Udp);
    sourceURL.SetInetAddress(&rAddress);
    response.AppendRaw(reinterpret_cast<const unsigned char*>(sourceURL.GetURL()),
                       wcslen(sourceURL.GetURL()) + sizeof(wchar_t));
    m_pTransport->Send(&sourceURL, response.GetBuffer());
}

}
}
