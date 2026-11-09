#include "nn/nex/PRUDPMessageV1.hpp"

#include <string.h>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/ByteOrder.hpp"
#include "nn/nex/ByteStream.hpp"
#include "nn/nex/CompressionAlgorithm.hpp"
#include "nn/nex/EncryptionAlgorithm.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/KeyedChecksumAlgorithm.hpp"
#include "nn/nex/LiteBuffer.hpp"
#include "nn/nex/LiteByteStream.hpp"
#include "nn/nex/PacketIn.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/Platform.hpp"
#include "nn/nex/SignatureBytes.hpp"
#include "nn/nex/StreamSettings.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int APPEND_AT_END = 0xFFFFFFFFu;
const unsigned int SIGNATURE_LENGTH = 16;
const unsigned int MAGIC_LENGTH = 2;
const unsigned int HEADER_IMAGE_LENGTH = 12;
const unsigned int SIGNED_HEADER_LENGTH = 8;
const unsigned int MAX_OPTION_LENGTH = 0x34;
const unsigned int COMPRESSION_HEADROOM = 0x40;
const unsigned int RANDOM_LIMIT = 0xFF;
const int MAX_STREAM_TYPE = 0xB;
const int RESULT_DECOMPRESSION_FAILED = static_cast<int>(0x8005000Cu);
const unsigned char MAGIC_FIRST = 0xEA;
const unsigned char MAGIC_SECOND = 0xD0;
const unsigned char VERSION = 1;
const unsigned char DEAD_SIGNATURE_BYTE = 0xAB;
const unsigned char UNSIGNED_SIGNATURE_BYTE = 0xEF;
const unsigned int ENDPOINT_FLAG = 0x100;

const unsigned char OPTION_SUPPORTED_FUNCTIONS = 0;
const unsigned char OPTION_CONNECTION_SIGNATURE = 1;
const unsigned char OPTION_FRAGMENT_ID = 2;
const unsigned char OPTION_INITIAL_SEQUENCE_ID = 3;
const unsigned char OPTION_MAX_SUBSTREAM_ID = 4;

inline unsigned short SwapShort(unsigned short uiValue)
{
    return static_cast<unsigned short>(static_cast<unsigned short>((uiValue & 0xFF) << 8)
                                       | static_cast<unsigned short>((uiValue >> 8) & 0xFF));
}

}

PRUDPMessageV1::PRUDPMessageV1() : m_randomNumberGenerator(), m_criticalSection(1)
{
    m_randomNumberGenerator.SetRandomNumberSeed(Platform::GetRandomSeed());
}

PRUDPMessageV1::~PRUDPMessageV1()
{
}

void PRUDPMessageV1::SubHeaderBuilder(PacketOut* pPacket)
{
    pPacket->m_headerImage.m_uiSource = pPacket->m_uiSource;
    pPacket->m_headerImage.m_uiDestination = pPacket->m_uiDestination;
    pPacket->m_headerImage.m_unknown6 = HostOrderToNexNetworkOrder<unsigned short>(pPacket->m_uiTypeAndFlags);
    pPacket->m_headerImage.m_uiSessionID = pPacket->m_uiSessionID;
    pPacket->m_headerImage.m_uiSubStreamID = pPacket->m_uiSubStreamID;
    pPacket->m_headerImage.m_unknown10 = HostOrderToNexNetworkOrder<unsigned short>(pPacket->m_sequenceID.GetValue());
}

bool PRUDPMessageV1::OptionBuilder(PacketOut* pPacket)
{
    LiteByteStream<MAX_OPTION_LENGTH>& rStream = pPacket->m_stream;
    rStream.Clear();
    if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_SYN
        || (pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_CONNECT)
    {
        if (!rStream.AppendPrimitive(OPTION_SUPPORTED_FUNCTIONS))
            return false;

        if (!rStream.AppendPrimitive(static_cast<unsigned char>(sizeof(pPacket->m_unknown90))))
            return false;

        if (!rStream.AppendPrimitive(pPacket->m_unknown90))
            return false;

        if (!rStream.AppendPrimitive(OPTION_CONNECTION_SIGNATURE))
            return false;

        if (!rStream.AppendPrimitive(static_cast<unsigned char>(SIGNATURE_LENGTH)))
            return false;

        if (pPacket->m_uiConnectionSignatureSize != 0)
        {
            if (!rStream.GetBuffer().AppendData(pPacket->m_aConnectionSignature, pPacket->m_uiConnectionSignatureSize,
                                                APPEND_AT_END))
            {
                return false;
            }
        }
        else
        {
            for (unsigned int i = 0; i < SIGNATURE_LENGTH; ++i)
            {
                unsigned char uiZero = 0;
                if (!rStream.AppendPrimitive(uiZero))
                    return false;
            }
        }

        bool bHasInitialSequenceID = (pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_CONNECT
                                     && pPacket->m_unknown4c != LogicalClockTmpl<unsigned short, short>(0);

        if (bHasInitialSequenceID)
        {
            if (!rStream.AppendPrimitive(OPTION_INITIAL_SEQUENCE_ID))
                return false;

            if (!rStream.AppendPrimitive(static_cast<unsigned char>(sizeof(unsigned short))))
                return false;

            if (!rStream.AppendPrimitive(pPacket->m_unknown4c.GetValue()))
                return false;
        }

        if (!rStream.AppendPrimitive(OPTION_MAX_SUBSTREAM_ID))
            return false;

        if (!rStream.AppendPrimitive(static_cast<unsigned char>(sizeof(pPacket->m_unknown6b))))
            return false;

        if (!rStream.AppendPrimitive(pPacket->m_unknown6b))
            return false;
    }

    if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) != PACKET_TYPE_DATA)
        return true;

    if (!rStream.AppendPrimitive(OPTION_FRAGMENT_ID))
        return false;

    if (!rStream.AppendPrimitive(static_cast<unsigned char>(sizeof(pPacket->m_uiFragmentID))))
        return false;

    if (!rStream.AppendPrimitive(pPacket->m_uiFragmentID))
        return false;

    return true;
}

bool PRUDPMessageV1::OptionSubHeaderBuilder(PacketOut* pPacket)
{
    if (pPacket->m_bHeaderParsed == 0)
    {
        pPacket->m_bHeaderParsed = 1;
        SubHeaderBuilder(pPacket);
        return OptionBuilder(pPacket);
    }

    return true;
}

PRUDPMessageInterface::SignatureMethod PRUDPMessageV1::DecideSignatureMethod(const Packet* pPacket, Stream::Type type,
                                                                             bool bFlag)
{
    if (g_streamSettings[type].Get().GetChecksumAlgorithm() == 0)
        return SIGNATURE_METHOD_0;

    unsigned short uiPacketType = pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK;
    if (uiPacketType != PACKET_TYPE_SYN && uiPacketType != PACKET_TYPE_CONNECT
        && (type == Stream::RendezVousSecure || type == Stream::RendezVous))
    {
        if (bFlag)
            return SIGNATURE_METHOD_6;

        if (uiPacketType == PACKET_TYPE_DISCONNECT && (pPacket->m_uiTypeAndFlags & PACKET_FLAG_ACK) != 0)
        {
            return SIGNATURE_METHOD_9;
        }
    }

    return SIGNATURE_METHOD_5;
}

void PRUDPMessageV1::CalcSignatureHelper(const Packet* pPacket, SignatureMethod method, const Key* pKey,
                                         Stream::Type type, const SignatureBytes* pSignature, SignatureBytes& rResult)
{
    rResult.m_uiLength = SIGNATURE_LENGTH;
    switch (method)
    {
    case SIGNATURE_METHOD_0:
        memset(rResult.m_bytes, DEAD_SIGNATURE_BYTE, SIGNATURE_LENGTH);
        return;
    case SIGNATURE_METHOD_1:
    {
        KeyedChecksumAlgorithm* pChecksum = g_streamSettings[type].Get().GetChecksumAlgorithm();
        if (pChecksum != 0)
        {
            LiteByteStream<6> stream;
            LiteBuffer<6>& buffer = stream.GetBuffer();
            stream.AppendPrimitive(pPacket->m_address.GetAddress());
            stream.AppendPrimitive(pPacket->m_address.GetPortNumber());
            const unsigned char* pData = buffer.GetData();
            unsigned int uiLength = buffer.GetLength();
            pChecksum->ComputeChecksum(&pData, &uiLength, 1, rResult);
            rResult.m_uiLength = SIGNATURE_LENGTH;
            return;
        }

        memset(rResult.m_bytes, UNSIGNED_SIGNATURE_BYTE, SIGNATURE_LENGTH);
        return;
    }
    case SIGNATURE_METHOD_4:
    case SIGNATURE_METHOD_5:
    case SIGNATURE_METHOD_6:
    case SIGNATURE_METHOD_7:
    {
        KeyedChecksumAlgorithm* pChecksum = g_streamSettings[type].Get().GetChecksumAlgorithm();
        if (pChecksum == 0)
        {
            memset(rResult.m_bytes, UNSIGNED_SIGNATURE_BYTE, SIGNATURE_LENGTH);
            return;
        }

        const unsigned char* aData[5];
        unsigned int aLength[5];
        aData[0] = &pPacket->m_headerImage.m_uiSource;
        aLength[0] = SIGNED_HEADER_LENGTH;
        aData[1] = 0;
        aLength[1] = 0;
        if (method == SIGNATURE_METHOD_6 && pKey != 0)
        {
            aData[1] = pKey->GetContentPtr();
            aLength[1] = pKey->GetLength();
        }

        LiteByteStream<20> stream;
        LiteBuffer<20>& rBuffer = stream.GetBuffer();
        unsigned short uiPacketType = pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK;
        if (uiPacketType != PACKET_TYPE_SYN || (pPacket->m_uiTypeAndFlags & PACKET_FLAG_ACK) == 0
            || (pPacket->m_unknown90 & ENDPOINT_FLAG) == 0)
        {
            unsigned int uiKeySum = g_streamSettings[type].Get().GetKeySum();
            stream.AppendPrimitive(uiKeySum);
        }

        if (pSignature != 0)
        {
            unsigned short uiTypeAndFlags = pPacket->m_uiTypeAndFlags;
            if (uiPacketType != PACKET_TYPE_SYN && (method != SIGNATURE_METHOD_4 || uiPacketType != PACKET_TYPE_CONNECT)
                && (uiTypeAndFlags & PACKET_FLAG_ALTERNATE_KEY_A) == 0
                && (uiTypeAndFlags & PACKET_FLAG_ALTERNATE_KEY_B) == 0 && uiPacketType != PACKET_TYPE_USER)
            {
                rBuffer.AppendData(pSignature->m_bytes, pSignature->m_uiLength, APPEND_AT_END);
            }
        }

        if (method == SIGNATURE_METHOD_7)
        {
            aData[2] = 0;
            aLength[2] = 0;
        }
        else
        {
            aData[2] = rBuffer.GetData();
            aLength[2] = rBuffer.GetLength();
        }

        const LiteBuffer<MAX_OPTION_LENGTH>* pOptions = &pPacket->m_stream.GetBuffer();
        if (pOptions != 0 && pOptions->GetLength() != 0)
        {
            aData[3] = pOptions->GetData();
            aLength[3] = pOptions->GetLength();
        }
        else
        {
            aData[3] = 0;
            aLength[3] = 0;
        }

        const Buffer* pPayload = pPacket->m_pPayload;
        if (pPayload != 0 && pPayload->GetLength() != 0)
        {
            aData[4] = pPayload->GetData();
            aLength[4] = pPayload->GetLength();
        }
        else
        {
            aData[4] = 0;
            aLength[4] = 0;
        }

        pChecksum->ComputeChecksum(aData, aLength, 5, rResult);
        rResult.m_uiLength = SIGNATURE_LENGTH;
        return;
    }
    case SIGNATURE_METHOD_8:
    {
        ScopedCS lock(&m_criticalSection);
        for (unsigned int i = 0; i < SIGNATURE_LENGTH; ++i)
        {
            rResult.m_bytes[i] = static_cast<unsigned char>(m_randomNumberGenerator.GetRandomNumber(RANDOM_LIMIT));
        }
        return;
    }
    default:
        return;
    }
}

bool PRUDPMessageV1::CalcSignature(PacketOut* pPacket, SignatureMethod method, const Key* pKey, Stream::Type type,
                                   TransportSignatureGenerator* pGenerator, const SignatureBytes* pSignature,
                                   SignatureBytes& rResult)
{
    pPacket->m_uiTypeAndFlags |= PACKET_FLAG_HAS_SIZE;
    if (OptionSubHeaderBuilder(pPacket))
    {
        CalcSignatureHelper(pPacket, method, pKey, type, pSignature, rResult);
        return true;
    }

    return false;
}

void PRUDPMessageV1::CalcSignature(Packet* pPacket, SignatureMethod method, const Key* pKey, Stream::Type type,
                                   TransportSignatureGenerator* pGenerator, const SignatureBytes* pSignature,
                                   SignatureBytes& rResult)
{
    CalcSignatureHelper(pPacket, method, pKey, type, pSignature, rResult);
}

int PRUDPMessageV1::CalcExpectedSignature(const Packet* pPacket, SignatureMethod method, const Key* pKey,
                                          TransportSignatureGenerator* pGenerator, const SignatureBytes* pSignature,
                                          SignatureBytes& rResult)
{
    if (method == SIGNATURE_METHOD_9)
        return 3;

    CalcSignatureHelper(pPacket, method, pKey, VirtualPort(pPacket->m_uiSource).GetType(), pSignature, rResult);
    return 0;
}

bool PRUDPMessageV1::HeaderParse(ByteStream* pStream, PacketIn* pPacket, unsigned int& rOptionLength,
                                 unsigned int& rPayloadLength)
{
    unsigned int uiPosition = pStream->GetPosition() + MAGIC_LENGTH;
    if (uiPosition <= pStream->GetBuffer()->GetLength())
    {
        unsigned char uiAlreadyParsed = pPacket->m_bHeaderParsed;
        pStream->SetPosition(uiPosition);
        if (uiAlreadyParsed == 0)
        {
            if (pStream->Extract(reinterpret_cast<unsigned char*>(&pPacket->m_headerImage), 1, HEADER_IMAGE_LENGTH))
            {
                unsigned short uiPayloadLength = pPacket->m_headerImage.m_unknown2;
                unsigned short uiTypeAndFlags = pPacket->m_headerImage.m_unknown6;
                pPacket->m_uiVersion = VERSION;
                pPacket->m_uiSource = pPacket->m_headerImage.m_uiSource;
                unsigned char uiDestination = pPacket->m_headerImage.m_uiDestination;
                rOptionLength = pPacket->m_headerImage.m_unknown1;
                pPacket->m_uiDestination = uiDestination;
                pPacket->m_uiTypeAndFlags = SwapShort(uiTypeAndFlags);
                unsigned short uiSequenceID = pPacket->m_headerImage.m_unknown10;
                pPacket->m_uiSessionID = pPacket->m_headerImage.m_uiSessionID;
                unsigned char uiSubStreamID = pPacket->m_headerImage.m_uiSubStreamID;
                rPayloadLength = SwapShort(uiPayloadLength);
                pPacket->m_uiSubStreamID = uiSubStreamID;
                pPacket->m_sequenceID = SwapShort(uiSequenceID);
                return true;
            }
        }
        else
        {
            unsigned int uiLength = pStream->GetBuffer()->GetLength();
            uiPosition = pStream->GetPosition() + HEADER_IMAGE_LENGTH;
            pPacket->m_bHeaderParsed = 1;
            if (uiPosition <= uiLength)
            {
                unsigned short uiPayloadLength = pPacket->m_headerImage.m_unknown2;
                unsigned char uiOptionLength = pPacket->m_headerImage.m_unknown1;
                pStream->SetPosition(uiPosition);
                rOptionLength = uiOptionLength;
                rPayloadLength = SwapShort(uiPayloadLength);
                return true;
            }
        }
    }

    return false;
}

bool PRUDPMessageV1::OptionUnpack(ByteStream* pStream, PacketIn* pPacket, unsigned int uiOptionLength)
{
    if (uiOptionLength < MAX_OPTION_LENGTH)
    {
        unsigned int uiStart = pStream->GetPosition();
        unsigned int uiEnd = uiStart + uiOptionLength;
        if (uiEnd <= pStream->GetBuffer()->GetLength())
        {
            while (pStream->GetPosition() < uiEnd)
            {
                unsigned char uiType = 0;
                unsigned char uiLength = 0;
                if (!pStream->ExtractPrimitive(&uiType))
                    return false;

                if (!pStream->ExtractPrimitive(&uiLength))
                    return false;

                unsigned int uiNext = pStream->GetPosition() + uiLength;
                if (uiEnd < uiNext)
                    return false;

                switch (uiType)
                {
                case OPTION_SUPPORTED_FUNCTIONS:
                {
                    if (uiLength != sizeof(unsigned int))
                        return false;

                    unsigned int uiSupportedFunctions;
                    if (!pStream->ExtractPrimitive(&uiSupportedFunctions))
                        return false;

                    pPacket->m_unknown90 = uiSupportedFunctions;
                    break;
                }
                case OPTION_CONNECTION_SIGNATURE:
                    if (uiLength != SIGNATURE_LENGTH)
                        return false;

                    if (!pStream->Extract(pPacket->m_aConnectionSignature, 1, SIGNATURE_LENGTH))
                        return false;

                    pPacket->m_uiConnectionSignatureSize = SIGNATURE_LENGTH;
                    break;
                case OPTION_FRAGMENT_ID:
                {
                    if (uiLength != sizeof(unsigned char))
                        return false;

                    unsigned char uiFragmentID;
                    if (!pStream->ExtractPrimitive(&uiFragmentID))
                        return false;

                    pPacket->m_uiFragmentID = uiFragmentID;
                    break;
                }
                case OPTION_INITIAL_SEQUENCE_ID:
                {
                    if (uiLength != sizeof(unsigned short))
                        return false;

                    unsigned short uiInitialSequenceID;
                    if (!pStream->ExtractPrimitive(&uiInitialSequenceID))
                        return false;

                    pPacket->m_unknown4c = uiInitialSequenceID;
                    break;
                }
                case OPTION_MAX_SUBSTREAM_ID:
                {
                    if (uiLength != sizeof(unsigned char))
                        return false;

                    unsigned char uiMaxSubStreamID;
                    if (!pStream->ExtractPrimitive(&uiMaxSubStreamID))
                        return false;

                    pPacket->m_unknown6b = uiMaxSubStreamID;
                    break;
                }
                default:
                    if (pStream->GetBuffer()->GetLength() < uiNext)
                        return false;

                    pStream->SetPosition(uiNext);
                    break;
                }
            }

            pPacket->m_stream.Clear();
            return pPacket->m_stream.GetBuffer().AppendData(pStream->GetBuffer()->GetData() + uiStart, uiOptionLength,
                                                            APPEND_AT_END);
        }
    }

    return false;
}

bool PRUDPMessageV1::Unpack(ByteStream* pStream, PacketIn* pPacket, unsigned int* pConsumed)
{
    bool bUnpacked = false;
    if (pStream->GetBuffer()->GetLength() == 0)
        return false;

    unsigned int uiOptionLength = 0;
    unsigned int uiPayloadLength = 0;
    if (!HeaderParse(pStream, pPacket, uiOptionLength, uiPayloadLength))
        return false;

    if (!pStream->Extract(pPacket->m_aSignature, 1, SIGNATURE_LENGTH))
        return false;

    pPacket->m_uiSignatureSize = SIGNATURE_LENGTH;
    Buffer* pBuffer = pStream->GetBuffer();
    if (uiOptionLength != 0 && !OptionUnpack(pStream, pPacket, uiOptionLength))
        return false;

    int iStreamType = VirtualPort(pPacket->m_uiSource).GetType();
    if (iStreamType > MAX_STREAM_TYPE)
        return false;

    Buffer* pPayload = pPacket->m_pPayload;
    pPayload->Clear();
    if (uiPayloadLength == 0)
    {
        bUnpacked = true;
    }
    else
    {
        if (pBuffer->GetLength() < pStream->GetPosition() + uiPayloadLength)
            return false;

        if (pPayload->AppendData(pBuffer->GetData() + pStream->GetPosition(), uiPayloadLength, APPEND_AT_END))
        {
            unsigned int uiPosition = pStream->GetPosition() + uiPayloadLength;
            if (uiPosition <= pStream->GetBuffer()->GetLength())
                pStream->SetPosition(uiPosition);

            bUnpacked = true;
            if (pPacket->m_pPayload->GetLength() != 0)
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
    }

    if (pConsumed != 0)
        *pConsumed = pStream->GetPosition();

    return bUnpacked;
}

bool PRUDPMessageV1::FastUnpack(ByteStream* pStream, PacketIn* pPacket, unsigned int* pConsumed)
{
    unsigned int uiStart = pStream->GetPosition();
    if (static_cast<int>(uiStart) >= static_cast<int>(pStream->GetLength()))
        return false;

    unsigned int uiOptionLength = 0;
    unsigned int uiPayloadLength = 0;
    if (!HeaderParse(pStream, pPacket, uiOptionLength, uiPayloadLength))
        return false;

    unsigned int uiPosition = pStream->GetPosition() + SIGNATURE_LENGTH;
    if (uiPosition > pStream->GetLength())
        return false;

    pStream->SetPosition(uiPosition);
    uiPosition += uiOptionLength;
    if (uiPosition > pStream->GetLength())
        return false;

    pStream->SetPosition(uiPosition);
    uiPosition += uiPayloadLength;
    if (uiPosition > pStream->GetLength())
        return false;

    pStream->SetPosition(uiPosition);
    if (pConsumed != 0)
        *pConsumed = uiPosition - uiStart;

    return true;
}

bool PRUDPMessageV1::Pack(PacketOut* pPacket, ByteStream* pStream)
{
    bool bOwnsBuffer = false;
    unsigned short uiPayloadLength = 0;
    int iStreamType = VirtualPort(pPacket->m_uiSource).GetType();
    if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_RAW)
    {
        *pStream << *pPacket->m_pPayload;
        pPacket->m_unknown12c = pStream->GetBuffer()->GetLength();
        return true;
    }

    if (!OptionSubHeaderBuilder(pPacket))
        return false;

    Buffer* pCurrent = pPacket->m_pPayload;
    if (pPacket->m_pPayload != 0)
    {
        CompressionAlgorithm* pCompression = g_streamSettings[iStreamType].Get().GetCompressionAlgorithm();
        if (pCompression != 0)
        {
            Buffer* pCompressed = new Buffer(pPacket->m_pPayload->GetLength() + COMPRESSION_HEADROOM, 0);
            pCompression->Compress(*pPacket->m_pPayload, pCompressed);
            pCurrent = pCompressed;
            bOwnsBuffer = true;
        }

        EncryptionAlgorithm* pEncryption = g_streamSettings[iStreamType].Get().GetEncryptionAlgorithm();
        if (pEncryption != 0)
        {
            Buffer* pEncrypted = new Buffer(pCurrent->GetLength(), 0);
            pEncryption->Encrypt(*pCurrent, pEncrypted);
            if (bOwnsBuffer)
                delete pCurrent;

            pCurrent = pEncrypted;
            bOwnsBuffer = true;
        }

        uiPayloadLength = static_cast<unsigned short>(pCurrent->GetLength());
    }

    pStream->AppendPrimitive(MAGIC_FIRST);
    pStream->AppendPrimitive(MAGIC_SECOND);
    pPacket->m_headerImage.m_unknown0 = VERSION;
    pPacket->m_headerImage.m_unknown1 = static_cast<unsigned char>(pPacket->m_stream.GetBuffer().GetLength());
    pPacket->m_headerImage.m_unknown2 = SwapShort(uiPayloadLength);
    if (!pStream->Append(reinterpret_cast<const unsigned char*>(&pPacket->m_headerImage), 1, HEADER_IMAGE_LENGTH))
    {
        return false;
    }

    if (pPacket->m_uiSignatureSize != 0)
    {
        if (!pStream->AppendRaw(pPacket->m_aSignature, pPacket->m_uiSignatureSize))
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

    if (pPacket->m_stream.GetBuffer().GetLength() != 0
        && !pStream->Append(pPacket->m_stream.GetBuffer().GetData(), 1, pPacket->m_stream.GetBuffer().GetLength()))
    {
        return false;
    }

    if (pPacket->m_pPayload != 0)
        *pStream << *pCurrent;

    pPacket->m_unknown12c = pStream->GetBuffer()->GetLength();
    if (bOwnsBuffer && pCurrent != 0)
        delete pCurrent;

    return true;
}

}
}
