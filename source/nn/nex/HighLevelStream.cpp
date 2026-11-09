#include "nn/nex/HighLevelStream.hpp"

#include <string.h>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/LiteBuffer.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/PRUDPMessageSelector.hpp"
#include "nn/nex/Packet.hpp"
#include "nn/nex/PacketBufferPacketOut.hpp"
#include "nn/nex/PacketEncDec.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/Platform.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/SignatureBytes.hpp"
#include "nn/nex/StreamSettings.hpp"
#include "nn/nex/TransportStreamManager.hpp"
#include "nn/nex/VirtualPort.hpp"

namespace nn
{
namespace nex
{

HighLevelStream::HighLevelStream(Stream::Type type, RootTransport* pTransport)
    : Stream(type),
      m_uiVersion(0),
      m_bSignaturesEnabled(false),
      m_bForceSignatureMethod(false),
      m_uiStreamID(INVALID_STREAM_ID),
      m_pPacketEncDec(0),
      m_sequenceID()
{
    m_pTransport = pTransport;
    if (m_pTransport == 0)
        m_pTransport = RootTransport::GetInstance();

    m_pMessageSelector = &Network::GetInstance()->GetMessageSelector();
    m_uiSessionID = static_cast<unsigned char>(Time::GetTime()) + Platform::GetRandomNumber(SESSION_ID_RANDOM_RANGE);
    m_sequenceID = static_cast<unsigned short>(Time::GetTime()) + Platform::GetRandomNumber(SEQUENCE_ID_RANDOM_RANGE);
}

HighLevelStream::~HighLevelStream()
{
    m_pTransport->GetStreamManager()->UnregisterStream(m_Type, m_uiStreamID);
    if (m_pPacketEncDec != 0)
    {
        delete m_pPacketEncDec;
        m_pPacketEncDec = 0;
    }
}

bool HighLevelStream::IsListened()
{
    return m_uiStreamID != INVALID_STREAM_ID;
}

bool HighLevelStream::Listen(unsigned char uiStreamID)
{
    if (IsListened())
        return false;

    if (m_Type == Stream::Unknown9 && g_streamSettings[m_Type].Get().IsEncryptionRequired())
    {
        LiteBuffer<StreamSettings::KEY_BUFFER_SIZE> keyBuffer = g_streamSettings[m_Type].Get().GetEncryptionKey();
        Key key(keyBuffer.GetData(), keyBuffer.GetLength());
        if (key.GetLength() == 0)
            return false;

        m_pPacketEncDec = new PacketEncDec();
        m_pPacketEncDec->SetUnreliableEncryptionKey(key);
    }

    if (!m_pTransport->GetStreamManager()->RegisterStream(this, uiStreamID, 0))
        return false;

    m_uiStreamID = uiStreamID;
    return true;
}

bool HighLevelStream::ExpectedSignature(Packet* pPacket, SignatureBytes* pSignature)
{
    PRUDPMessageInterface::SignatureMethod method = PRUDPMessageInterface::SIGNATURE_METHOD_7;
    if (!m_bForceSignatureMethod)
    {
        method = m_pMessageSelector->DecideSignatureMethod(pPacket, m_Type, false);
        if (method == PRUDPMessageInterface::SIGNATURE_METHOD_9 || method == PRUDPMessageInterface::SIGNATURE_METHOD_1)
        {
            return false;
        }
    }

    m_pMessageSelector->CalcSignature(pPacket, method, 0, m_Type, 0, 0, *pSignature);
    return true;
}

bool HighLevelStream::ReceiveIncomingPacket(unsigned short uiPort, unsigned char, Packet* pPacket)
{
    if (m_bSignaturesEnabled)
    {
        SignatureBytes signature;
        if (!ExpectedSignature(pPacket, &signature) || signature.m_uiLength != pPacket->m_uiSignatureSize
            || memcmp(signature.m_bytes, pPacket->m_aSignature, pPacket->m_uiSignatureSize) != 0)
        {
            return false;
        }
    }

    if (m_pPacketEncDec != 0)
        m_pPacketEncDec->Decode(pPacket);

    VirtualPort source(pPacket->m_uiSource);
    ReceiveIncomingMsg(pPacket->m_address, source.GetType(), source.GetStreamID(), pPacket->m_pPayload,
                       Time(pPacket->m_unknownF8), uiPort);
    pPacket->ReleaseRef();
    return true;
}

void HighLevelStream::CalcSignature(PacketOut* pPacket)
{
    if (!m_bSignaturesEnabled)
        return;

    PRUDPMessageInterface::SignatureMethod method = PRUDPMessageInterface::SIGNATURE_METHOD_7;
    if (!m_bForceSignatureMethod)
    {
        method = m_pMessageSelector->DecideSignatureMethod(pPacket, m_Type, false);
        if (method == PRUDPMessageInterface::SIGNATURE_METHOD_1)
            return;
    }

    SignatureBytes signature;
    if (m_pMessageSelector->CalcSignature(pPacket, method, 0, m_Type, 0, 0, signature))
    {
        unsigned int uiLength = signature.m_uiLength;
        memcpy(pPacket->m_aSignature, signature.m_bytes, uiLength);
        pPacket->m_uiSignatureSize = uiLength;
    }
}

qResult HighLevelStream::Send(const InetAddress& rAddress, Stream::Type type, unsigned char uiStreamID,
                              Buffer* pPayload, unsigned char uiTTL, unsigned short uiPort)
{
    if (uiPort == 0)
        uiPort = m_pTransport->GetPortNumber();

    PacketOut* pPacket;
    if (pPayload != 0 && pPayload->IsPooled())
    {
        pPacket = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
            PacketBufferPacketOut(0, PACKET_TYPE_USER, 0, pPayload, 0);
        if (pPacket == 0 || !pPacket->IsValid())
        {
            delete pPacket;
            return qResult(SEND_RESULT_PACKET_ALLOCATION_FAILED);
        }
    }
    else
    {
        pPacket = new PacketOut(0, PACKET_TYPE_USER, 0, pPayload, 0);
    }

    pPacket->m_address = rAddress;
    pPacket->m_sequenceID = m_sequenceID;
    pPacket->m_uiSource = VirtualPort(m_Type, m_uiStreamID).m_uiValue;
    pPacket->m_uiDestination = VirtualPort(type, uiStreamID).m_uiValue;
    pPacket->m_uiTTL = uiTTL;
    pPacket->m_uiVersion = m_uiVersion;
    ++m_sequenceID;
    pPacket->m_uiSessionID = m_uiSessionID;
    if (m_pPacketEncDec != 0)
        m_pPacketEncDec->Encode(pPacket, m_Type, false);

    CalcSignature(pPacket);
    bool bSent = m_pTransport->Send(uiPort, m_Type, m_uiStreamID, uiStreamID, pPacket, false);
    pPacket->ReleaseRef();
    if (!bSent)
        return qResult(SEND_RESULT_FAILED);

    return qResult(SEND_RESULT_SUCCESS);
}

void HighLevelStream::DoWork()
{
}

void HighLevelStream::ReceiveIncomingMsg(const InetAddress&, Stream::Type, unsigned char, Buffer*, Time, unsigned short)
{
}

}
}
