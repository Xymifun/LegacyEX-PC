#include "nn/pia/common/SocketStreamBase.hpp"

namespace nn
{
namespace pia
{
namespace common
{

SocketStreamBase::SocketStreamBase() : m_started(false), m_socket(0)
{
}

SocketStreamBase::~SocketStreamBase()
{
    m_socket = 0;
}

Result SocketStreamBase::Startup()
{
    m_started = true;
    return 0;
}

void SocketStreamBase::Cleanup()
{
    m_started = false;
}

Result SocketStreamBase::ImportSocket(Socket* socket)
{
    m_socket = socket;
    return 0;
}

}
}
}
