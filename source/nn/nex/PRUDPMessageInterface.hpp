#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PRUDPMESSAGEINTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PRUDPMESSAGEINTERFACE_HPP

#include "nn/nex/Stream.hpp"

namespace nn
{
namespace nex
{

class ByteStream;
class Key;
class Packet;
class PacketIn;
class PacketOut;
class SignatureBytes;
class TransportSignatureGenerator;

class PRUDPMessageInterface
{
public:
    enum SignatureMethod
    {
        SIGNATURE_METHOD_0 = 0,
        SIGNATURE_METHOD_1 = 1,
        SIGNATURE_METHOD_2 = 2,
        SIGNATURE_METHOD_3 = 3,
        SIGNATURE_METHOD_4 = 4,
        SIGNATURE_METHOD_5 = 5,
        SIGNATURE_METHOD_6 = 6,
        SIGNATURE_METHOD_7 = 7,
        SIGNATURE_METHOD_8 = 8,
        SIGNATURE_METHOD_9 = 9
    };

    virtual ~PRUDPMessageInterface() {}

    virtual bool CalcSignature(PacketOut* pPacket, SignatureMethod method, const Key* pKey, Stream::Type type,
                               TransportSignatureGenerator* pGenerator, const SignatureBytes* pSignature,
                               SignatureBytes& rResult) = 0;
    virtual void CalcSignature(Packet* pPacket, SignatureMethod method, const Key* pKey, Stream::Type type,
                               TransportSignatureGenerator* pGenerator, const SignatureBytes* pSignature,
                               SignatureBytes& rResult) = 0;
    virtual SignatureMethod DecideSignatureMethod(const Packet* pPacket, Stream::Type type, bool bFlag) = 0;
    virtual int CalcExpectedSignature(const Packet* pPacket, SignatureMethod method, const Key* pKey,
                                      TransportSignatureGenerator* pGenerator, const SignatureBytes* pSignature,
                                      SignatureBytes& rResult) = 0;
    virtual bool Unpack(ByteStream* pStream, PacketIn* pPacket, unsigned int* pConsumed) = 0;
    virtual bool FastUnpack(ByteStream* pStream, PacketIn* pPacket, unsigned int* pConsumed) = 0;
    virtual bool Pack(PacketOut* pPacket, ByteStream* pStream) = 0;
};

}
}

#endif
