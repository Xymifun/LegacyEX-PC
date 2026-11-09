#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SOCKET_STREAM_BASE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SOCKET_STREAM_BASE_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class Socket;

class SocketStreamBase
{
public:
    SocketStreamBase();
    virtual ~SocketStreamBase();
    Result Startup();
    void Cleanup();
    Result ImportSocket(Socket* socket);

protected:
    bool m_started;
    Socket* m_socket;
};

}
}
}

#endif
