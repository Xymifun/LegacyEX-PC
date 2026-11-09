#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_IOCOMPLETIONCONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_IOCOMPLETIONCONTEXT_HPP

#include "nn/nex/InetAddress.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Socket;
class StringStream;

class IOCompletionContext : public RootObject
{
public:
    enum State
    {
        STATE_PENDING = 1,
        STATE_COMPLETE = 2
    };

    enum Operation
    {
        OPERATION_CONNECT = 1
    };

    IOCompletionContext();
    ~IOCompletionContext();

    void Reset();
    void Trace(unsigned long long uiFilter);

    unsigned int m_unknown0;
    unsigned int m_unknown4;
    unsigned int m_unknown8;
    unsigned int m_unknownC;
    InetAddress m_address;
    unsigned int m_unknown24;
    unsigned char m_uiState;
    unsigned char m_uiOperation;
    unsigned int m_unknown2C;
    Socket* m_pSocket;
};

StringStream& operator<<(StringStream& rStream, const IOCompletionContext& rContext);

}
}

#endif
