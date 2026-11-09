#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PRUDPMESSAGEV1_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PRUDPMESSAGEV1_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/PRUDPMessageInterface.hpp"
#include "nn/nex/RandomNumberGenerator.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class PRUDPMessageV1 : public PRUDPMessageInterface, public RootObject
{
public:
    PRUDPMessageV1();
    virtual ~PRUDPMessageV1();

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
    void SubHeaderBuilder(PacketOut* pPacket);
    bool OptionBuilder(PacketOut* pPacket);
    bool OptionSubHeaderBuilder(PacketOut* pPacket);
    bool HeaderParse(ByteStream* pStream, PacketIn* pPacket, unsigned int& rOptionLength, unsigned int& rPayloadLength);
    bool OptionUnpack(ByteStream* pStream, PacketIn* pPacket, unsigned int uiOptionLength);
    void CalcSignatureHelper(const Packet* pPacket, SignatureMethod method, const Key* pKey, Stream::Type type,
                             const SignatureBytes* pSignature, SignatureBytes& rResult);

    RandomNumberGenerator m_randomNumberGenerator;
    CriticalSection m_criticalSection;
};

}
}

#endif
