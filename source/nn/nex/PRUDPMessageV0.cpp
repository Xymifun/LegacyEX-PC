#include "nn/nex/PRUDPMessageV0.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/ByteOrder.hpp"
#include "nn/nex/ByteStream.hpp"
#include "nn/nex/CompressionAlgorithm.hpp"
#include "nn/nex/EncryptionAlgorithm.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/KeyedChecksumAlgorithm.hpp"
#include "nn/nex/LiteBuffer.hpp"
#include "nn/nex/PacketIn.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/SignatureBytes.hpp"
#include "nn/nex/StreamSettings.hpp"
#include "nn/nex/TransportSignatureGenerator.hpp"
#include "nn/nex/qResult.hpp"

namespace __N_19_OnlineCore_Blob_cpp_9428d484
{
unsigned int CalcDataPacketSignature(const nn::nex::Packet* pPacket, nn::nex::Stream::Type type,
                                     const unsigned char* pData, int iLength, bool bIgnorePayload);
}

namespace nn
{
namespace nex
{

namespace
{

const unsigned int APPEND_AT_END = 0xFFFFFFFFu;
const unsigned int SIGNATURE_LENGTH = 4;
const unsigned int PORT_FIELD_LENGTH = 2;
const unsigned int COMPRESSION_HEADROOM = 0x40;
const int MAX_STREAM_TYPE = 0xB;
enum
{
    RESULT_DECOMPRESSION_FAILED = static_cast<int>(0x8005000Cu)
};

inline void StoreSignature(SignatureBytes& rResult, unsigned int uiSignature)
{
    rResult.m_bytes[0] = static_cast<unsigned char>(uiSignature);
    rResult.m_bytes[1] = static_cast<unsigned char>((uiSignature >> 8) & 0xFF);
    rResult.m_bytes[2] = static_cast<unsigned char>((uiSignature >> 16) & 0xFF);
    rResult.m_bytes[3] = static_cast<unsigned char>(uiSignature >> 24);
}

}

PRUDPMessageV0::PRUDPMessageV0()
{
}

PRUDPMessageV0::~PRUDPMessageV0()
{
}

void PRUDPMessageV0::CalcSignatureHelper(const Packet* pPacket, SignatureMethod method, const Key* pKey,
                                         Stream::Type type, TransportSignatureGenerator* pGenerator,
                                         SignatureBytes& rResult)
{
    unsigned int uiSignature = 0;
    rResult.m_uiLength = SIGNATURE_LENGTH;
    switch (method)
    {
    case SIGNATURE_METHOD_0:
        break;
    case SIGNATURE_METHOD_1:
        if (pGenerator != 0)
        {
            uiSignature = pGenerator->ComputeSourceSignature(pPacket->m_address.GetAddress(),
                                                             pPacket->m_address.GetPortNumber());
        }
        break;
    case SIGNATURE_METHOD_2:
        uiSignature = __N_19_OnlineCore_Blob_cpp_9428d484::CalcDataPacketSignature(pPacket, type, 0, 0, false);
        break;
    case SIGNATURE_METHOD_3:
    {
        unsigned short uiSequenceID = HostOrderToNexNetworkOrder<unsigned short>(pPacket->m_sequenceID.GetValue());
        unsigned char uiFragmentID = pPacket->m_uiFragmentID;
        Buffer buffer(pKey->GetLength() + 3, 0);
        buffer.AppendData(pKey->GetContentPtr(), pKey->GetLength(), APPEND_AT_END);
        buffer.AppendData(&uiSequenceID, sizeof(uiSequenceID), APPEND_AT_END);
        buffer.AppendData(&uiFragmentID, sizeof(uiFragmentID), APPEND_AT_END);
        uiSignature = __N_19_OnlineCore_Blob_cpp_9428d484::CalcDataPacketSignature(pPacket, type, buffer.GetData(),
                                                                                   buffer.GetLength(), false);
        break;
    }
    case SIGNATURE_METHOD_7:
    {
        unsigned short uiSequenceID = HostOrderToNexNetworkOrder<unsigned short>(pPacket->m_sequenceID.GetValue());
        unsigned char uiDestination = pPacket->m_uiDestination;
        unsigned char uiSource = pPacket->m_uiSource;
        unsigned char uiSessionID = pPacket->m_uiSessionID;
        unsigned short uiPayloadLength = 0;
        if (pPacket->m_pPayload != 0)
        {
            uiPayloadLength = HostOrderToNexNetworkOrder<unsigned int>(pPacket->m_pPayload->GetLength());
        }

        LiteBuffer<8> buffer;
        buffer.AppendData(&uiSequenceID, sizeof(uiSequenceID), APPEND_AT_END);
        buffer.AppendData(&uiSessionID, sizeof(uiSessionID), APPEND_AT_END);
        buffer.AppendData(&uiSource, sizeof(uiSource), APPEND_AT_END);
        buffer.AppendData(&uiDestination, sizeof(uiDestination), APPEND_AT_END);
        buffer.AppendData(&uiPayloadLength, sizeof(uiPayloadLength), APPEND_AT_END);
        uiSignature = __N_19_OnlineCore_Blob_cpp_9428d484::CalcDataPacketSignature(pPacket, type, buffer.GetData(),
                                                                                   buffer.GetLength(), true);
        break;
    }
    default:
        break;
    }

    StoreSignature(rResult, uiSignature);
}

bool PRUDPMessageV0::CalcSignature(PacketOut* pPacket, SignatureMethod method, const Key* pKey, Stream::Type type,
                                   TransportSignatureGenerator* pGenerator, const SignatureBytes* pSignature,
                                   SignatureBytes& rResult)
{
    CalcSignatureHelper(pPacket, method, pKey, type, pGenerator, rResult);
    return true;
}

void PRUDPMessageV0::CalcSignature(Packet* pPacket, SignatureMethod method, const Key* pKey, Stream::Type type,
                                   TransportSignatureGenerator* pGenerator, const SignatureBytes* pSignature,
                                   SignatureBytes& rResult)
{
    CalcSignatureHelper(pPacket, method, pKey, type, pGenerator, rResult);
}

PRUDPMessageInterface::SignatureMethod PRUDPMessageV0::DecideSignatureMethod(const Packet* pPacket, Stream::Type type,
                                                                             bool bFlag)
{
    if (g_streamSettings[type].Get().GetChecksumAlgorithm() != 0)
    {
        if (!pPacket->m_bSkipEncryption)
        {
            if (type == Stream::RendezVousSecure || type == Stream::RendezVous)
            {
                unsigned short uiPacketType = pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK;
                if (bFlag)
                {
                    if (uiPacketType == PACKET_TYPE_DATA || uiPacketType == PACKET_TYPE_DISCONNECT)
                        return SIGNATURE_METHOD_3;
                }
                else
                {
                    if (uiPacketType == PACKET_TYPE_DISCONNECT && (pPacket->m_uiTypeAndFlags & PACKET_FLAG_ACK) != 0)
                    {
                        return SIGNATURE_METHOD_9;
                    }
                }
            }
            else
            {
                unsigned short uiPacketType = pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK;
                if (uiPacketType == PACKET_TYPE_DATA || uiPacketType == PACKET_TYPE_USER)
                    return SIGNATURE_METHOD_2;
            }
        }
    }

    return SIGNATURE_METHOD_1;
}

int PRUDPMessageV0::CalcExpectedSignature(const Packet* pPacket, SignatureMethod method, const Key* pKey,
                                          TransportSignatureGenerator* pGenerator, const SignatureBytes* pSignature,
                                          SignatureBytes& rResult)
{
    if (method == SIGNATURE_METHOD_9)
        return 3;

    CalcSignatureHelper(pPacket, method, pKey, VirtualPort(pPacket->m_uiSource).GetType(), pGenerator, rResult);
    return 0;
}

bool PRUDPMessageV0::Unpack(ByteStream* pStream, PacketIn* pPacket, unsigned int* pConsumed)
{
    bool bUnpacked = false;
    if (pStream->GetBuffer()->GetLength() == 0)
        return false;

    if (!pStream->ExtractPrimitive(&pPacket->m_uiSource))
        return false;

    if (!pStream->ExtractPrimitive(&pPacket->m_uiDestination))
        return false;

    if (!pStream->ExtractPrimitive(&pPacket->m_uiTypeAndFlags))
        return false;

    if (!pStream->ExtractPrimitive(&pPacket->m_uiSessionID))
        return false;

    if (!pStream->Extract(pPacket->m_aSignature, 1, SIGNATURE_LENGTH))
        return false;

    unsigned short uiSequenceID = 0;
    pPacket->m_uiSignatureSize = SIGNATURE_LENGTH;
    if (!pStream->ExtractPrimitive(&uiSequenceID))
        return false;

    pPacket->m_sequenceID = uiSequenceID;
    unsigned short uiPacketType = pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK;
    if (uiPacketType == PACKET_TYPE_SYN || uiPacketType == PACKET_TYPE_CONNECT)
    {
        if (!pStream->Extract(pPacket->m_aConnectionSignature, 1, SIGNATURE_LENGTH))
            return false;

        pPacket->m_uiConnectionSignatureSize = SIGNATURE_LENGTH;
    }

    if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_DATA
        && !pStream->ExtractPrimitive(&pPacket->m_uiFragmentID))
    {
        return false;
    }

    Buffer* pBuffer = pStream->GetBuffer();
    unsigned int uiBufferLength = pBuffer->GetLength();
    unsigned int uiPacketLength;
    if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_HAS_SIZE) != 0)
    {
        unsigned short uiSize = 0;
        if (!pStream->ExtractPrimitive(&uiSize))
            return false;

        uiPacketLength = uiSize + pStream->GetPosition() + 1;
        if (uiBufferLength < uiPacketLength)
            return false;
    }
    else
    {
        uiPacketLength = pStream->GetBuffer()->GetLength();
    }

    int iStreamType = VirtualPort(pPacket->m_uiSource).GetType();
    pBuffer->SetLength(uiPacketLength);
    if (iStreamType > MAX_STREAM_TYPE)
        return false;

    if (pBuffer->IsCheckSumValid(g_streamSettings[iStreamType].Get().GetKeyByteSum()))
    {
        pBuffer->StripCheckSum();
        Buffer* pPayload = pPacket->m_pPayload;
        pPayload->Clear();
        unsigned int uiPosition = pStream->GetPosition();
        if (pBuffer->GetLength() < uiPosition)
            return false;

        if (pPayload->AppendData(pBuffer->GetData() + uiPosition, pBuffer->GetLength() - uiPosition, APPEND_AT_END)
            && (bUnpacked = true, pPacket->m_pPayload->GetLength() != 0))
        {
            EncryptionAlgorithm* pEncryption = g_streamSettings[iStreamType].Get().GetEncryptionAlgorithm();
            if (pEncryption != 0)
                pEncryption->Decrypt(pPacket->m_pPayload);

            CompressionAlgorithm* pCompression = g_streamSettings[iStreamType].Get().GetCompressionAlgorithm();
            if (pCompression != 0)
            {
                bUnpacked = pCompression->Decompress(pPacket->m_pPayload) == 0;
                if (!bUnpacked)
                {
                    qResult error(RESULT_DECOMPRESSION_FAILED);
                    return false;
                }
            }
        }
    }

    pPacket->m_uiVersion = 0;
    pBuffer->SetLength(uiBufferLength);
    if (pConsumed != 0)
        *pConsumed = uiPacketLength;

    return bUnpacked;
}

bool PRUDPMessageV0::FastUnpack(ByteStream* pStream, PacketIn* pPacket, unsigned int* pConsumed)
{
    unsigned int uiStart = pStream->GetPosition();
    if (static_cast<int>(uiStart) < static_cast<int>(pStream->GetBuffer()->GetLength())
        && pStream->ExtractPrimitive(&pPacket->m_uiSource) && pStream->ExtractPrimitive(&pPacket->m_uiDestination)
        && pStream->ExtractPrimitive(&pPacket->m_uiTypeAndFlags))
    {
        unsigned int uiPosition = pStream->GetPosition() + 1;
        if (uiPosition <= pStream->GetBuffer()->GetLength())
        {
            pStream->SetPosition(uiPosition);
            uiPosition += 4;
            if (uiPosition <= pStream->GetBuffer()->GetLength())
            {
                pStream->SetPosition(uiPosition);
                unsigned short uiSequenceID = 0;
                if (pStream->ExtractPrimitive(&uiSequenceID))
                {
                    unsigned short uiTypeAndFlags = pPacket->m_uiTypeAndFlags;
                    pPacket->m_sequenceID = uiSequenceID;
                    unsigned short uiPacketType = uiTypeAndFlags & PACKET_TYPE_MASK;
                    if (uiPacketType == PACKET_TYPE_SYN || uiPacketType == PACKET_TYPE_CONNECT)
                    {
                        uiPosition = pStream->GetPosition() + 4;
                        if (pStream->GetBuffer()->GetLength() < uiPosition)
                            return false;

                        pStream->SetPosition(uiPosition);
                    }

                    if ((uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_DATA)
                    {
                        uiPosition = pStream->GetPosition() + 1;
                        if (pStream->GetBuffer()->GetLength() < uiPosition)
                            return false;

                        pStream->SetPosition(uiPosition);
                    }

                    unsigned short uiSize = 0;
                    unsigned int uiPayloadLength;
                    unsigned char uiSource;
                    unsigned int uiPacketOffset;
                    int iConsumed;
                    if ((uiTypeAndFlags & PACKET_FLAG_HAS_SIZE) == 0)
                    {
                        uiSource = pPacket->m_uiSource;
                        uiPacketOffset = pStream->GetPosition();
                        iConsumed = pStream->GetBuffer()->GetLength();
                        uiPayloadLength = (iConsumed - uiPacketOffset - 1) & 0xFFFF;
                    }
                    else
                    {
                        if (!pStream->ExtractPrimitive(&uiSize))
                            return false;

                        uiPacketOffset = pStream->GetPosition();
                        uiPayloadLength = uiSize;
                        uiSource = pPacket->m_uiSource;
                        iConsumed = ((uiPayloadLength + uiPacketOffset) - uiStart) + 1;
                    }

                    if (VirtualPort(uiSource).GetType() <= MAX_STREAM_TYPE)
                    {
                        uiPosition = uiPacketOffset + uiPayloadLength + 1;
                        if (uiPosition <= pStream->GetBuffer()->GetLength())
                        {
                            pStream->SetPosition(uiPosition);
                            if (pConsumed != 0)
                                *pConsumed = iConsumed;

                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool PRUDPMessageV0::Pack(PacketOut* pPacket, ByteStream* pStream)
{
    if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_RAW)
    {
        *pStream << *pPacket->m_pPayload;
        pPacket->m_unknown12c = pStream->GetBuffer()->GetLength();
        return true;
    }

    if (pPacket->IsBundlable() || (g_historyBundlingEnabled && pPacket->CanHistoryBundle() && !pPacket->m_unknown129))
    {
        pPacket->m_uiTypeAndFlags |= PACKET_FLAG_HAS_SIZE;
    }

    pStream->AppendPrimitive(pPacket->m_uiSource);
    pStream->AppendPrimitive(pPacket->m_uiDestination);
    if (!pStream->Append(reinterpret_cast<const unsigned char*>(&pPacket->m_uiTypeAndFlags), 2, 1)
        || !pStream->Append(&pPacket->m_uiSessionID, 1, 1))
    {
        return false;
    }

    if (pPacket->m_uiSignatureSize != 0)
    {
        if (!pStream->Append(pPacket->m_aSignature, 1, pPacket->m_uiSignatureSize))
            return false;
    }
    else
    {
        for (unsigned int i = 0; i < SIGNATURE_LENGTH; ++i)
        {
            unsigned char uiZero = 0;
            pStream->AppendPrimitive(uiZero);
        }
    }

    pStream->AppendPrimitive(pPacket->m_sequenceID.GetValue());
    unsigned short uiPacketType = pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK;
    if (uiPacketType == PACKET_TYPE_SYN || uiPacketType == PACKET_TYPE_CONNECT)
    {
        if (pPacket->m_uiConnectionSignatureSize != 0)
        {
            if (!pStream->Append(pPacket->m_aConnectionSignature, 1, pPacket->m_uiConnectionSignatureSize))
            {
                return false;
            }
        }
        else
        {
            for (unsigned int i = 0; i < SIGNATURE_LENGTH; ++i)
            {
                unsigned char uiZero = 0;
                pStream->AppendPrimitive(uiZero);
            }
        }
    }

    if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_DATA)
        pStream->AppendPrimitive(pPacket->m_uiFragmentID);

    Buffer* pBuffer = pStream->GetBuffer();
    int iStreamType = VirtualPort(pPacket->m_uiSource).GetType();
    if (pPacket->m_pPayload != 0)
    {
        CompressionAlgorithm* pCompression = g_streamSettings[iStreamType].Get().GetCompressionAlgorithm();
        Buffer compressed(pPacket->m_pPayload->GetLength() + COMPRESSION_HEADROOM, 0);
        const Buffer* pCurrent = pPacket->m_pPayload;
        if (pCompression != 0)
        {
            pCompression->Compress(*pPacket->m_pPayload, &compressed);
            pCurrent = &compressed;
        }

        EncryptionAlgorithm* pEncryption = g_streamSettings[iStreamType].Get().GetEncryptionAlgorithm();
        Buffer encrypted(pCurrent->GetLength(), 0);
        if (pEncryption != 0)
        {
            pEncryption->Encrypt(*pCurrent, &encrypted);
            pCurrent = &encrypted;
        }

        if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_HAS_SIZE) != 0)
            pStream->AppendPrimitive(static_cast<unsigned short>(pCurrent->GetLength()));

        *pStream << *pCurrent;
    }
    else if ((pPacket->m_uiTypeAndFlags & PACKET_FLAG_HAS_SIZE) != 0)
    {
        pStream->AppendPrimitive(static_cast<unsigned short>(0));
    }

    pBuffer->AppendCheckSum(g_streamSettings[iStreamType].Get().GetKeyByteSum());
    pPacket->m_unknown12c = pStream->GetBuffer()->GetLength();
    return true;
}

}
}

namespace __N_19_OnlineCore_Blob_cpp_9428d484
{

namespace
{

const unsigned int SIGNATURE_UNAVAILABLE = 0x9999;
const unsigned int SIGNATURE_EMPTY = 0x12345678;
const unsigned int SIGNATURE_PART_COUNT = 2;

}

unsigned int CalcDataPacketSignature(const nn::nex::Packet* pPacket, nn::nex::Stream::Type type,
                                     const unsigned char* pData, int iLength, bool bIgnorePayload)
{
    nn::nex::KeyedChecksumAlgorithm* pChecksum = nn::nex::g_streamSettings[type].Get().GetChecksumAlgorithm();
    if (pChecksum == 0)
        return SIGNATURE_UNAVAILABLE;

    nn::nex::Buffer* pPayload = pPacket->m_pPayload;
    if (bIgnorePayload)
    {
        pPayload = 0;
        if (iLength == 0)
            return SIGNATURE_EMPTY;
    }
    else if (pPayload == 0 || pPayload->GetLength() == 0)
    {
        if (iLength == 0)
            return SIGNATURE_EMPTY;
    }

    if (pPayload != 0)
    {
        const unsigned char* aData[SIGNATURE_PART_COUNT];
        unsigned int aLength[SIGNATURE_PART_COUNT];
        aData[0] = pData;
        aData[1] = pPayload->GetData();
        aLength[0] = iLength;
        aLength[1] = pPayload->GetLength();
        return pChecksum->ComputeChecksumForTransportArray(aData, aLength, SIGNATURE_PART_COUNT);
    }

    return pChecksum->ComputeChecksumForTransport(pData, iLength);
}

}
