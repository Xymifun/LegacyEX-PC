#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PACKET_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PACKET_HPP

#include "nn/nex/InetAddress.hpp"
#include "nn/nex/LiteByteStream.hpp"
#include "nn/nex/LogicalClockTmpl.hpp"
#include "nn/nex/RefCountedObject.hpp"
#include "nn/nex/RelayPingProtocolHeader.hpp"
#include "nn/nex/RelayProtocolHeader.hpp"
#include "nn/nex/Stream.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/VirtualPort.hpp"
#include "nn/nex/qChain.hpp"

namespace nn
{
namespace nex
{

class Buffer;

enum PacketType
{
    PACKET_TYPE_SYN = 0,
    PACKET_TYPE_CONNECT = 1,
    PACKET_TYPE_DATA = 2,
    PACKET_TYPE_DISCONNECT = 3,
    PACKET_TYPE_PING = 4,
    PACKET_TYPE_USER = 5,
    PACKET_TYPE_RAW = 7
};

enum PacketFlag
{
    PACKET_TYPE_MASK = 0x0F,
    PACKET_FLAG_ACK = 0x10,
    PACKET_FLAG_RELIABLE = 0x20,
    PACKET_FLAG_NEED_ACK = 0x40,
    PACKET_FLAG_HAS_SIZE = 0x80,
    PACKET_FLAG_TRAILER = 0x100,
    PACKET_FLAG_ALTERNATE_KEY_A = 0x200,
    PACKET_FLAG_ALTERNATE_KEY_B = 0x400,
    PACKET_FLAG_HISTORY_BUNDLE = 0x800
};

extern bool g_historyBundlingEnabled;
extern bool g_historyBundleWithoutFlag;
extern unsigned short g_historyPacketLimit;
extern unsigned short g_bundlePacketLimit;
extern unsigned short g_bundleSendLimit;
extern int g_historyBundleDelay;
extern int g_historyBundleAge;
extern unsigned int g_bundleBufferSize;

class Packet : public RefCountedObject
{
public:
    struct HeaderImage
    {
        unsigned char m_unknown0;
        unsigned char m_unknown1;
        unsigned short m_unknown2;
        unsigned char m_uiSource;
        unsigned char m_uiDestination;
        unsigned short m_unknown6;
        unsigned char m_uiSessionID;
        unsigned char m_uiSubStreamID;
        unsigned short m_unknown10;
    };

    Packet();
    virtual ~Packet();

    bool SetPayload(Buffer* pPayload);
    bool CanHistoryBundle() const;

    static unsigned int GetOverhead();

    Stream::Type GetStreamType() const { return VirtualPort(m_uiDestination).GetType(); }

    RelayProtocolHeader& GetRelayHeader()
    {
        return *reinterpret_cast<RelayProtocolHeader*>(m_stream.GetBuffer().GetData());
    }

    RelayPingProtocolHeader& GetRelayPingHeader()
    {
        return *reinterpret_cast<RelayPingProtocolHeader*>(&GetRelayHeader() + 1);
    }

    qChainLink<Packet*> m_link;
    qChainLink<Packet*> m_tempLink;
    qChainLink<Packet*> m_historyLink;
    unsigned char m_uiVersion;
    unsigned char m_uiSource;
    unsigned char m_uiDestination;
    unsigned char m_unknown2B;
    unsigned char m_aSignature[16];
    unsigned int m_uiSignatureSize;
    unsigned short m_uiTypeAndFlags;
    LogicalClockTmpl<unsigned short, short> m_sequenceID;
    LogicalClockTmpl<unsigned short, short> m_unknown4c;
    unsigned char m_aConnectionSignature[16];
    unsigned int m_uiConnectionSignatureSize;
    unsigned char m_uiFragmentID;
    unsigned char m_uiSessionID;
    unsigned char m_uiTTL;
    unsigned char m_unknown6b;
    HeaderImage m_headerImage;
    unsigned char m_unknown78[16];
    unsigned char m_uiSubStreamID;
    unsigned short m_uiPort;
    bool m_bSkipEncryption;
    unsigned char m_bHeaderParsed;
    unsigned int m_unknown90;
    LiteByteStream<52> m_stream;
    Buffer* m_pPayload;
    InetAddress m_address;
    Time m_unknownF8;
    Time m_queueTime;
    unsigned long long m_historyTime;
};

}
}

#endif
