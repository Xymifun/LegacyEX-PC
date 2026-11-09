#include "nn/nex/SecureStream.hpp"

#include "nn/nex/AuthenticationClient.hpp"
#include "nn/nex/BitStream.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/KerberosAuthentication.hpp"
#include "nn/nex/SecureConnectionClient.hpp"
#include "nn/nex/SecureEndPoint.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/Ticket.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum
{
    STREAM_RESULT_ERROR = static_cast<int>(0x80050001u),
    ASSOCIATED_URL_COUNT = 1
};

}

SecureStream::SecureStream()
    : ConnectionOrientedStream(Stream::RendezVousSecure, 0),
      m_pAuthenticationClient(new AuthenticationClient),
      m_pSecureConnectionClient(new SecureConnectionClient)
{
    m_pAssociatedStream = 0;
    m_pCredentials = 0;
}

void SecureStream::SetAssociatedStream(ConnectionOrientedStream* pAssociatedStream)
{
    m_pAssociatedStream = pAssociatedStream;
}

SecureStream::~SecureStream()
{
    delete m_pSecureConnectionClient;
    delete m_pAuthenticationClient;
}

bool SecureStream::FilterIncomingConnection(Buffer* pRequest, Buffer* pResponse, EndPoint* pEndPoint)
{
    if (pRequest == 0)
        return false;

    BitStream requestStream(pRequest);
    BitStream responseStream(pResponse);
    unsigned int uiConnectionID = 0;
    Ticket* pTicket = 0;
    if (KerberosAuthentication::ValidateConnectionRequest(&requestStream, &responseStream, m_pAuthenticationClient,
                                                          &uiConnectionID, &pTicket))
    {
        if (!pEndPoint->PeerIsConnected() && !pEndPoint->IsAnyPacketProcessed())
        {
            pEndPoint->SetPrincipalID(pTicket->GetPrincipalID());
            pEndPoint->SetConnectionID(uiConnectionID);
            pEndPoint->SetEncryptionKey(pTicket->GetKey());
        }

        pTicket->ReleaseRef();
        return true;
    }

    return false;
}

bool SecureStream::ResponsibleForURL(const StationURL* pStationURL)
{
    return pStationURL->GetURLType() == StationURL::Prudps;
}

EndPoint* SecureStream::OpenEndPoint(const StationURL* pStationURL)
{
    return CreateEndPoint(pStationURL);
}

EndPoint* SecureStream::OpenEndPoint(unsigned int)
{
    return 0;
}

EndPoint* SecureStream::FindEndPointByPID(unsigned int uiPrincipalID)
{
    return m_pAssociatedStream->FindEndPointByPID(uiPrincipalID);
}

EndPoint* SecureStream::FindEndPointByCID(unsigned int uiConnectionID)
{
    return m_pAssociatedStream->FindEndPointByCID(uiConnectionID);
}

qVector<unsigned int> SecureStream::GetAllConnectionIDs() const
{
    return m_pAssociatedStream->GetAllConnectionIDs();
}

unsigned int SecureStream::GetEndPointNumber()
{
    return m_pAssociatedStream->GetEndPointNumber();
}

EndPoint* SecureStream::CreateEndPoint(const StationURL* pStationURL)
{
    return new SecureEndPoint(this, pStationURL);
}

void SecureStream::CloseEndPoint(EndPoint* pEndPoint)
{
    SecureEndPoint* pSecureEndPoint = static_cast<SecureEndPoint*>(pEndPoint);
    if (pSecureEndPoint->GetAssociatedEndPoint() != 0)
        pSecureEndPoint->CompleteDisconnect();

    delete pEndPoint;
}

bool SecureStream::ReceiveIncomingPacket(unsigned short, unsigned char, Packet*)
{
    return false;
}

void SecureStream::DoWork()
{
}

void SecureStream::LockEndPointMap()
{
    m_pAssociatedStream->LockEndPointMap();
}

void SecureStream::UnlockEndPointMap()
{
    m_pAssociatedStream->UnlockEndPointMap();
}

unsigned int SecureStream::GetNbLocalURL()
{
    if (m_pCredentials != 0 && m_pCredentials->m_state != 0 && m_pCredentials->m_value != 0)
        return ASSOCIATED_URL_COUNT;

    return 0;
}

StationURL* SecureStream::GetLocalURL(unsigned int)
{
    if (m_pCredentials != 0)
        return BuildURL(m_pCredentials->m_state, m_pCredentials->m_value);

    return BuildURL(0, 0);
}

void SecureStream::ReleaseURL(StationURL* pStationURL)
{
    delete pStationURL;
}

StationURL* SecureStream::BuildURL(unsigned int uiConnectionID, unsigned int uiPrincipalID)
{
    StationURL* pStationURL = new StationURL();
    pStationURL->SetURLType(StationURL::Prudps);
    pStationURL->SetConnectionID(uiConnectionID);
    pStationURL->SetPrincipalID(uiPrincipalID);
    return pStationURL;
}

void SecureStream::SetCredentials(Credentials* pCredentials)
{
    m_pCredentials = pCredentials;
    m_pAuthenticationClient->SetDefaultCredentials(m_pCredentials);
    m_pSecureConnectionClient->SetDefaultCredentials(m_pCredentials);
}

const wchar_t* SecureStream::GetURLType()
{
    return L"prudps";
}

bool SecureStream::OpenEndPoint(EndPoint* pEndPoint)
{
    return ConnectionOrientedStream::OpenEndPoint(pEndPoint);
}

qResult SecureStream::Send(StationURL*, Buffer*)
{
    return qResult(STREAM_RESULT_ERROR);
}

qResult SecureStream::Send(StationURL*, Buffer*, SendCallback, const UserContext&)
{
    return qResult(STREAM_RESULT_ERROR);
}

bool SecureStream::IsCapable(unsigned int)
{
    return false;
}

qResult SecureStream::SetOption(unsigned int, const UserContext&)
{
    return qResult(STREAM_RESULT_ERROR);
}

qResult SecureStream::GetOption(unsigned int, UserContext&)
{
    return qResult(STREAM_RESULT_ERROR);
}

bool SecureStreamConnectionFilter(const UserContext& rUserContext, Buffer* pRequest, Buffer* pResponse,
                                  EndPoint* pEndPoint)
{
    return static_cast<SecureStream*>(rUserContext.GetPointer())
        ->FilterIncomingConnection(pRequest, pResponse, pEndPoint);
}

}
}
