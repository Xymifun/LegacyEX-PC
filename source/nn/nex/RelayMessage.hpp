#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RELAYMESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RELAYMESSAGE_HPP

#include <stdint.h>

#include "nn/nex/LiteBuffer.hpp"
#include "nn/nex/PRUDPMessageInterface.hpp"
#include "nn/nex/RelayType.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/Stream.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class ByteStream;
class Packet;
class PacketIn;
class PacketOut;
class SignatureBytes;

class RelayMessage : public PRUDPMessageInterface, public RootObject
{
public:
    RelayMessage();
    virtual ~RelayMessage();

    virtual bool CalcSignature(PacketOut* pPacket, SignatureMethod method, const Key* pKey, Stream::Type type,
                               TransportSignatureGenerator* pGenerator, const SignatureBytes* pSignature,
                               SignatureBytes& rResult);
    virtual void CalcSignature(Packet* pPacket, SignatureMethod method, const Key* pKey, Stream::Type type,
                               TransportSignatureGenerator* pGenerator, const SignatureBytes* pSignature,
                               SignatureBytes& rResult);
    virtual SignatureMethod DecideSignatureMethod(const Packet* pPacket, Stream::Type type, bool bFlag);
    virtual int CalcExpectedSignature(const Packet* pPacket, SignatureMethod method, const Key* pKey,
                                      TransportSignatureGenerator* pGenerator, const SignatureBytes* pSignature,
                                      SignatureBytes& rResult);
    virtual bool Unpack(ByteStream* pStream, PacketIn* pPacket, unsigned int* pConsumed);
    virtual bool FastUnpack(ByteStream* pStream, PacketIn* pPacket, unsigned int* pConsumed);
    virtual bool Pack(PacketOut* pPacket, ByteStream* pStream);

    void SetRelayDataPacketSignatureKey(const unsigned char* pKey, unsigned int uiLength);

    void ClearRelayDataPacketSignatureKey() { m_signatureKey.Clear(); }

    void SetRelayDataPacketSignatureKeyFromP2PDataPacketSessionSignatureKey(const qVector<unsigned char>& rKey);

    bool Pack(PacketOut* pPacket, ByteStream* pStream, const unsigned char* pKey, unsigned int uiKeyLength);
    bool PackPing(PacketOut* pPacket);

    bool Unpack(ByteStream* pStream, PacketIn* pPacket, const unsigned char* pKey, unsigned int uiKeyLength,
                unsigned int* pConsumed);
    bool UnpackWithoutValidation(ByteStream* pStream, PacketIn* pPacket, unsigned int* pConsumed, bool bUnpackPayload);
    bool UnpackPayload(const ByteStream* pStream, PacketIn* pPacket);
    bool UnpackPing(PacketIn* pPacket);

    bool CheckSignature(ByteStream* pStream, PacketIn* pPacket, const unsigned char* pKey, unsigned int uiKeyLength);
    bool CheckTimeStamp(PacketIn* pPacket);
    int GetCurrentTimeStamp();

    static void CalcSignature(SignatureBytes* pSignature, RelayType type, const unsigned char* pKey,
                              unsigned int uiKeyLength, const unsigned char* pHeader, const unsigned char* pPayload,
                              unsigned int uiPayloadLength);

private:
    friend class JobResolveRelayInfo;

    class TimeStampOrigin : public RootObject
    {
    public:
        TimeStampOrigin()
        {
            m_uiBaseTime = 0;
            m_uiStartTime = 0;
        }

        unsigned int m_unknown0;
        unsigned int m_unknown4;
        uint64_t m_uiStartTime;
        uint64_t m_uiBaseTime;
    };

    LiteBuffer<16> m_signatureKey;
    TimeStampOrigin m_timeStampOrigin;
};

}
}

#endif
