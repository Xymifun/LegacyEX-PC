#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALSTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALSTREAM_HPP

#include "nn/nex/HighLevelStream.hpp"
#include "nn/nex/Stream.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class InetAddress;
class NATTraversalEngine;
class RootTransport;
class StationURL;

class NATTraversalStream : public HighLevelStream
{
public:
    NATTraversalStream(NATTraversalEngine* pEngine, RootTransport* pTransport);
    virtual ~NATTraversalStream();

    virtual void DoWork();
    virtual void ReceiveIncomingMsg(const InetAddress& rAddress, Stream::Type type, unsigned char uiStreamID,
                                    Buffer* pPayload, Time receptionTime, unsigned short uiPort);

    void SendMsg(const StationURL& rStationURL, Buffer* pPayload, unsigned char uiTTL);

private:
    enum
    {
        NAT_STREAM_ID = 1
    };

    unsigned int m_uiReceivedCount;
    NATTraversalEngine* m_pEngine;
};

}
}

#endif
