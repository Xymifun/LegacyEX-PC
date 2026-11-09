#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CREDENTIALS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CREDENTIALS_HPP

#include <stdint.h>

#include "nn/nex/RefCountedObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class EndPoint;
class StreamManager;

class Credentials : public RefCountedObject
{
public:
    Credentials();
    Credentials(uint32_t value, StreamManager* streamManager);
    ~Credentials();

    void Logout();
    StreamManager* MarkInvalid();
    void SetStreamManager(StreamManager* streamManager);
    EndPoint* GetConnection(unsigned short connectionType) const;
    void SetAuthenticationConnection(EndPoint* connection);
    void SetSecureConnection(EndPoint* connection);

    uint32_t GetPrincipalID() const { return m_value; }

private:
    friend class BackEndServices;
    friend class JobBackEndServicesLogin;
    friend class JobBackEndServicesLoginWithData;
    friend class JobBackEndServicesLogout;
    friend class JobConnectSecureEndPoint;
    friend class NATTraversalRelayClient;
    friend class SecureConnectionClient;
    friend class SecureStream;
    friend class ServiceClient;
    friend class KerberosAuthentication;
    friend class GlobalNotificationEventManager;
    friend class JobWaitForNotification;
    uint32_t m_value;
    uint32_t m_state;
    String m_name;
    String m_description;
    StreamManager* m_streamManager;
    EndPoint* m_authenticationConnection;
    EndPoint* m_secureConnection;
};

}
}

#endif
