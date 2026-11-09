#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PRUDPMESSAGEV0_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PRUDPMESSAGEV0_HPP

#include "nn/nex/PRUDPMessageInterface.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class PRUDPMessageV0 : public PRUDPMessageInterface, public RootObject
{
public:
    PRUDPMessageV0();
    virtual ~PRUDPMessageV0();

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

private:
    void CalcSignatureHelper(const Packet* pPacket, SignatureMethod method, const Key* pKey, Stream::Type type,
                             TransportSignatureGenerator* pGenerator, SignatureBytes& rResult);
};

}
}

#endif
