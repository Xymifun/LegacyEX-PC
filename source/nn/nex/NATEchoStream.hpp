#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATECHOSTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATECHOSTREAM_HPP

#include "nn/nex/HighLevelStream.hpp"
#include "nn/nex/Stream.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class InetAddress;
class RootTransport;

class NATEchoStream : public HighLevelStream
{
public:
    explicit NATEchoStream(RootTransport* pTransport);
    virtual ~NATEchoStream();

    virtual void DoWork() {}

    virtual void ReceiveIncomingMsg(const InetAddress& rAddress, Stream::Type type, unsigned char uiStreamID,
                                    Buffer* pPayload, Time receptionTime, unsigned short uiPort);

    static void PrepareMessage(Buffer* pRequest, Buffer* pResponse);

private:
    enum
    {
        ECHO_STREAM_ID = 1
    };

    static const Stream::Type ECHO_STREAM_TYPE = static_cast<Stream::Type>(7);
};

}
}

#endif
