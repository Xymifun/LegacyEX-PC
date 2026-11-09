#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CLIENTSTREAMMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CLIENTSTREAMMANAGER_HPP

#include "nn/nex/StreamManager.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class Credentials;
class EndPoint;

class ClientStreamManager : public StreamManager
{
public:
    explicit ClientStreamManager(Stream::Type type = Stream::RendezVousSecure);
    virtual ~ClientStreamManager();

    virtual void Receive(EndPoint* pEndPoint, Buffer* pBuffer, unsigned char ucParameter);
    virtual void FaultDetection(EndPoint* pEndPoint, unsigned int uiError);
    virtual void PeerDisconnected(EndPoint* pEndPoint);
    virtual void SetCredentials(Credentials* pCredentials);

protected:
    friend class GlobalNotificationEventManager;

    Credentials* m_pCredentials;
};

}
}

#endif
