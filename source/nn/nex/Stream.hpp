#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STREAM_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Packet;

class Stream : public RootObject
{
public:
    enum Type
    {
        Unknown1 = 1,
        RendezVous = 2,
        Unknown3 = 3,
        NAT = 5,
        Routing = 8,
        Unknown9 = 9,
        RendezVousSecure = 10,
        Relay = 11
    };

    explicit Stream(Type type);
    virtual ~Stream();

    virtual bool ReceiveIncomingPacket(unsigned short uiPort, unsigned char uiStreamID, Packet* pPacket) = 0;
    virtual void DoWork() = 0;
    virtual bool IsDuplicateReorderingPacket(Packet* pPacket);

    Type GetType() const { return m_Type; }

protected:
    Type m_Type;
};

}
}

#endif
