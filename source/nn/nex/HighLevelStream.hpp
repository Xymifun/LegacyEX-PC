#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HIGHLEVELSTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HIGHLEVELSTREAM_HPP

#include "nn/nex/LogicalClockTmpl.hpp"
#include "nn/nex/Stream.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class InetAddress;
class Packet;
class PacketEncDec;
class PacketOut;
class PRUDPMessageSelector;
class RootTransport;
class SignatureBytes;

class HighLevelStream : public Stream
{
public:
    HighLevelStream(Stream::Type type, RootTransport* pTransport);
    virtual ~HighLevelStream();

    virtual bool ReceiveIncomingPacket(unsigned short uiPort, unsigned char uiStreamID, Packet* pPacket);
    virtual void DoWork();
    virtual void ReceiveIncomingMsg(const InetAddress& rAddress, Stream::Type type, unsigned char uiStreamID,
                                    Buffer* pPayload, Time receptionTime, unsigned short uiPort);
    virtual qResult Send(const InetAddress& rAddress, Stream::Type type, unsigned char uiStreamID, Buffer* pPayload,
                         unsigned char uiTTL, unsigned short uiPort);

    bool Listen(unsigned char uiStreamID);
    bool IsListened();

protected:
    void CalcSignature(PacketOut* pPacket);
    bool ExpectedSignature(Packet* pPacket, SignatureBytes* pSignature);

    unsigned char m_uiVersion;
    bool m_bSignaturesEnabled;
    bool m_bForceSignatureMethod;
    RootTransport* m_pTransport;
    unsigned char m_uiStreamID;
    PRUDPMessageSelector* m_pMessageSelector;
    PacketEncDec* m_pPacketEncDec;
    unsigned char m_uiSessionID;
    LogicalClockTmpl<unsigned short, short> m_sequenceID;

private:
    enum
    {
        INVALID_STREAM_ID = 0xFF,
        SESSION_ID_RANDOM_RANGE = 0xFF,
        SEQUENCE_ID_RANDOM_RANGE = 0xFFFF
    };

    enum SendResult
    {
        SEND_RESULT_SUCCESS = 0x00010001,
        SEND_RESULT_FAILED = static_cast<int>(0x80010001u),
        SEND_RESULT_PACKET_ALLOCATION_FAILED = static_cast<int>(0x8005001Fu)
    };
};

}
}

#endif
