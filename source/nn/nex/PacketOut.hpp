#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PACKETOUT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PACKETOUT_HPP

#include "nn/nex/Packet.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class PRUDPEndPoint;
class Timeout;

class PacketOut : public Packet
{
public:
    PacketOut(PRUDPEndPoint* pEndPoint, PacketType type, unsigned short uiFlags, Buffer* pPayload, unsigned int uiSize);
    virtual ~PacketOut();

    void Initialize(PRUDPEndPoint* pEndPoint, PacketType type, unsigned short uiFlags, Buffer* pPayload,
                    unsigned int uiSize, Timeout* pTimeout);
    bool IsValid();
    bool IsBundlable() const;
    bool IsBundlableWithPortTypeCheck(PacketOut* pPacket);
    bool ReadyToSend();
    bool SetInternalPayload(Buffer* pPayload);

    Timeout* m_pTimeout;
    PRUDPEndPoint* m_pEndPoint;
    unsigned short m_unknown118;
    unsigned short m_unknown11a;
    unsigned int m_unknown11c;
    unsigned long long m_unknown120;
    bool m_unknown128;
    bool m_unknown129;
    unsigned int m_unknown12c;
    unsigned int m_unknown130;
    unsigned int m_unknown134;
    unsigned long long m_unknown138;
    unsigned int m_unknown140;
    unsigned int m_unknown144;

protected:
    PacketOut() : Packet(), m_unknown120(0), m_unknown138(0) {}
};

}
}

#endif
