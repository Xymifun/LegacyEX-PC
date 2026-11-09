#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STREAMMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STREAMMANAGER_HPP

#include "nn/nex/ConnectionManager.hpp"
#include "nn/nex/EndPointEventHandler.hpp"
#include "nn/nex/Stream.hpp"

namespace nn
{
namespace nex
{

class ConnectionOrientedStream;
class Credentials;
class PRUDPStream;
class ProtocolRequestBrokerInterface;
class RootTransport;
class String;

class StreamManager : public EndPointEventHandler
{
public:
    enum
    {
        SandboxAccessKeyCapacity = 128
    };

    StreamManager(Stream::Type type, RootTransport* pTransport);
    virtual ~StreamManager();

    virtual void SetCredentials(Credentials*) {}

    bool Initialize(unsigned short uiPort, unsigned char uiStreamID, unsigned int uiFlags);
    void Terminate();
    bool IsTerminated() const;
    void AssociateSecureStream(ConnectionOrientedStream* pSecureStream, ConnectionManager::ConnectionFilter pFilter);
    void SetMaxSilenceTimeout(unsigned int uiTimeout);
    void SetKeepAliveTimeout(unsigned int uiTimeout);
    bool SimulateFault(unsigned int uiPrincipalID);

    void ClearAssociatedSecureStream() { m_pSecureStream = 0; }

    void EmulateSymmetricNAT() {}

    static void SetSandboxAccessKey(const String& rKey);

    static const wchar_t* GetSandboxAccessKey() { return s_aSandboxAccessKey; }

    PRUDPStream* GetStream() const { return m_pStream; }

    ConnectionOrientedStream* GetSecureStream() const { return m_pSecureStream; }

    ProtocolRequestBrokerInterface* GetProtocolRequestBroker() const { return m_pProtocolRequestBroker; }

    ConnectionManager* GetConnectionManager() const { return m_pConnectionManager; }

protected:
    PRUDPStream* m_pStream;
    ConnectionOrientedStream* m_pSecureStream;
    ProtocolRequestBrokerInterface* m_pProtocolRequestBroker;
    ConnectionManager* m_pConnectionManager;
    bool m_bInitialized;
    RootTransport* m_pTransport;
    Stream::Type m_type;
    unsigned short m_uiPort;

private:
    enum
    {
        INITIALIZE_FLAG_1 = 1,
        INITIALIZE_FLAG_2 = 2
    };

    void SetStreamKey();

    static wchar_t s_aSandboxAccessKey[SandboxAccessKeyCapacity];
    static int s_iKeySuffix;
    static float s_fPacketDropProbability;
};

}
}

#endif
