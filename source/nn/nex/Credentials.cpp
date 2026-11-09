#include "nn/nex/Credentials.hpp"

namespace nn
{
namespace nex
{

Credentials::Credentials()
    : RefCountedObject(),
      m_value(0),
      m_state(0),
      m_name(),
      m_description(""),
      m_streamManager(0),
      m_authenticationConnection(0),
      m_secureConnection(0)
{
}

Credentials::Credentials(uint32_t value, StreamManager* streamManager)
    : RefCountedObject(),
      m_value(value),
      m_state(0),
      m_name(),
      m_description(""),
      m_streamManager(streamManager),
      m_authenticationConnection(0),
      m_secureConnection(0)
{
}

void Credentials::Logout()
{
    if (m_streamManager != 0)
        m_streamManager = 0;
    m_value = 0;
    m_state = 0;
    m_authenticationConnection = 0;
    m_secureConnection = 0;
    m_name = "";
    m_description = "";
}

Credentials::~Credentials()
{
    Logout();
}

StreamManager* Credentials::MarkInvalid()
{
    StreamManager* streamManager = m_streamManager;
    m_streamManager = 0;
    return streamManager;
}

void Credentials::SetStreamManager(StreamManager* streamManager)
{
    m_streamManager = streamManager;
}

EndPoint* Credentials::GetConnection(unsigned short connectionType) const
{
    switch (connectionType)
    {
    case 10:
        return m_authenticationConnection;
    case 0:
        return 0;
    default:
        return m_secureConnection;
    }
}

void Credentials::SetAuthenticationConnection(EndPoint* connection)
{
    m_authenticationConnection = connection;
}

void Credentials::SetSecureConnection(EndPoint* connection)
{
    m_secureConnection = connection;
}

}
}
