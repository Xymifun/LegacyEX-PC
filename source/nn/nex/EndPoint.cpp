#include "nn/nex/EndPoint.hpp"

#include "nn/nex/ConnectionOrientedStream.hpp"
#include "nn/nex/TransportAdapter.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum
{
    ENDPOINT_RESULT_SUCCESS = 0x00010001,
    ENDPOINT_RESULT_ALREADY_REGISTERED = static_cast<int>(0x8001000Eu)
};

const unsigned int KEEP_ALIVE_DISABLED = 0xFFFFFFFF;

}

EndPointInfoInterface::~EndPointInfoInterface()
{
}

EndPoint::EndPoint(ConnectionOrientedStream* pStream, const StationURL* pStationURL) : m_stationURL(*pStationURL)
{
    m_pStream = pStream;
    m_uiCtorFlag = 0;
    m_pSecureEndPoint = 0;
    m_pEventHandler = 0;
    m_uiPrincipalID = 0;
    m_uiConnectionID = 0;
    m_iOpenCount = 0;
    m_BroadcastState = BroadcastDisabled;
}

EndPoint::~EndPoint()
{
    SetConnectionID(0);
    SetPrincipalID(0);
    g_transportAdapter->ReleaseAddress(m_stationURL);
}

void EndPoint::DisableKeepAlive()
{
    SetKeepAliveTimeout(KEEP_ALIVE_DISABLED);
}

qResult EndPoint::RegisterEventHandler(EndPointEventHandler* pEventHandler)
{
    if (m_pEventHandler == 0)
    {
        m_pEventHandler = pEventHandler;
        return qResult(ENDPOINT_RESULT_SUCCESS);
    }

    return qResult(ENDPOINT_RESULT_ALREADY_REGISTERED);
}

void EndPoint::SetErrorHolder(FirstErrorHolder* pErrorHolder)
{
    m_pErrorHolder = pErrorHolder;
}

void EndPoint::UnsetErrorHolder()
{
    m_pErrorHolder = 0;
}

void EndPoint::SignalFaultEvent(unsigned int, bool)
{
}

void EndPoint::Open()
{
    m_pStream->OpenEndPoint(this);
}

void EndPoint::Close()
{
    m_pStream->CloseEndPoint(this);
}

void EndPoint::SetConnectionID(unsigned int uiConnectionID)
{
    if (m_pStream != 0)
    {
        if (m_uiConnectionID != 0)
        {
            m_pStream->RemoveCIDEndPointAssociation(m_uiConnectionID, this);
        }

        if (uiConnectionID != 0)
        {
            GetStream()->AddCIDEndPointAssociation(uiConnectionID, this);
        }
    }

    m_uiConnectionID = uiConnectionID;
}

void EndPoint::SetPrincipalID(unsigned int uiPrincipalID)
{
    if (m_pStream != 0)
    {
        if (m_uiPrincipalID != 0)
        {
            m_pStream->RemovePIDEndPointAssociation(m_uiPrincipalID, this);
        }

        if (uiPrincipalID != 0)
        {
            GetStream()->AddPIDEndPointAssociation(uiPrincipalID, this);
        }
    }

    m_uiPrincipalID = uiPrincipalID;
}

void EndPoint::_Open()
{
    m_iOpenCount = m_iOpenCount + 1;
}

void EndPoint::_Close()
{
    m_iOpenCount = m_iOpenCount - 1;
}

void EndPoint::SetBroadcastState(_BroadcastState state)
{
    m_BroadcastState = state;
}

EndPointGroup::EndPointGroup()
{
}

EndPointGroup::~EndPointGroup()
{
}

const wchar_t* EndPoint::GetConnectionStateString() const
{
    switch (GetConnectionState())
    {
    case NotConnected:
        return L"NotConnected";
    case Connecting:
        return L"Connecting";
    case Connected:
        return L"Connected";
    case Disconnecting:
        return L"Disconnecting";
    case Faulty:
        return L"Faulty";
    default:
        return L"Unknown state";
    }
}

}
}
