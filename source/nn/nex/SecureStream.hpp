#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SECURESTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SECURESTREAM_HPP

#include "nn/nex/ConnectionOrientedStream.hpp"
#include "nn/nex/UserContext.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class AuthenticationClient;
class Buffer;
class Credentials;
class EndPoint;
class Packet;
class SecureConnectionClient;
class StationURL;

class SecureStream : public ConnectionOrientedStream
{
public:
    typedef void (*SendCallback)(ConnectionOrientedStream* pStream, Buffer* pBuffer, unsigned int uiParameter,
                                 UserContext* pUserContext);

    SecureStream();
    virtual ~SecureStream();

    virtual bool ReceiveIncomingPacket(unsigned short uiPort, unsigned char uiStreamID, Packet* pPacket);
    virtual void DoWork();
    virtual bool ResponsibleForURL(const StationURL* pStationURL);
    virtual const wchar_t* GetURLType();
    virtual EndPoint* OpenEndPoint(const StationURL* pStationURL);
    virtual EndPoint* OpenEndPoint(unsigned int uiPrincipalID);
    virtual bool OpenEndPoint(EndPoint* pEndPoint);
    virtual EndPoint* FindEndPointByPID(unsigned int uiPrincipalID);
    virtual unsigned int GetEndPointNumber();
    virtual EndPoint* FindEndPointByCID(unsigned int uiConnectionID);
    virtual qVector<unsigned int> GetAllConnectionIDs() const;
    virtual void CloseEndPoint(EndPoint* pEndPoint);
    virtual void LockEndPointMap();
    virtual void UnlockEndPointMap();
    virtual bool IsCapable(unsigned int uiCapability);
    virtual EndPoint* CreateEndPoint(const StationURL* pStationURL);
    virtual unsigned int GetNbLocalURL();
    virtual StationURL* GetLocalURL(unsigned int uiIndex);
    virtual void ReleaseURL(StationURL* pStationURL);
    virtual StationURL* BuildURL(unsigned int uiConnectionID, unsigned int uiPrincipalID);
    virtual qResult Send(StationURL* pStationURL, Buffer* pBuffer);
    virtual qResult Send(StationURL* pStationURL, Buffer* pBuffer, SendCallback pCallback,
                         const UserContext& rUserContext);
    virtual qResult SetOption(unsigned int uiOption, const UserContext& rUserContext);
    virtual qResult GetOption(unsigned int uiOption, UserContext& rUserContext);

    bool FilterIncomingConnection(Buffer* pRequest, Buffer* pResponse, EndPoint* pEndPoint);
    void SetCredentials(Credentials* pCredentials);

    void SetAssociatedStream(ConnectionOrientedStream* pAssociatedStream);

    ConnectionOrientedStream* GetAssociatedStream() const { return m_pAssociatedStream; }

    AuthenticationClient* GetAuthenticationClient() const { return m_pAuthenticationClient; }

    SecureConnectionClient* GetSecureConnectionClient() const { return m_pSecureConnectionClient; }

protected:
    friend class JobBackEndServicesLogin;
    friend class JobBackEndServicesLoginWithData;
    friend class JobConnectSecureEndPoint;
    ConnectionOrientedStream* m_pAssociatedStream;
    AuthenticationClient* m_pAuthenticationClient;
    SecureConnectionClient* m_pSecureConnectionClient;
    Credentials* m_pCredentials;
};

bool SecureStreamConnectionFilter(const UserContext& rUserContext, Buffer* pRequest, Buffer* pResponse,
                                  EndPoint* pEndPoint);

}
}

#endif
