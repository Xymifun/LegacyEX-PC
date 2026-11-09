#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PRUDPMESSAGESELECTOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PRUDPMESSAGESELECTOR_HPP

#include "nn/nex/PRUDPMessageInterface.hpp"
#include "nn/nex/RelayMessage.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class PRUDPMessageSelector : public PRUDPMessageInterface, public RootObject
{
public:
    PRUDPMessageSelector();
    virtual ~PRUDPMessageSelector();

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

    RelayMessage* GetRelayMessage() const { return m_pRelayMessage; }

private:
    enum
    {
        VersionCount = 2
    };

    qVector<PRUDPMessageInterface*> m_messages;
    RelayMessage* m_pRelayMessage;
};

}
}

#endif
