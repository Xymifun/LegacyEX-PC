#include "nn/nex/ConnectionOrientedStream.hpp"

#include "nn/nex/Network.hpp"
#include "nn/nex/RootTransport.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum StreamResult
{
    STREAM_RESULT_SUCCESS = 0x00010001,
    STREAM_RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u),
    STREAM_RESULT_ALREADY_REGISTERED = static_cast<int>(0x8001000Eu)
};

}

ConnectionOrientedStream::ConnectionOrientedStream(Stream::Type type, RootTransport* pTransport) : Stream(type)
{
    Network::AcquireInstance();
    m_pTransport = pTransport;
    m_uiStreamID = 0;
    m_pEventHandler = 0;
}

ConnectionOrientedStream::~ConnectionOrientedStream()
{
    Network::ReleaseInstance();
}

bool ConnectionOrientedStream::StartListen(unsigned char uiStreamID, unsigned char* pAssignedStreamID)
{
    if (m_pTransport->m_streamManager.RegisterStream(this, uiStreamID, &m_uiStreamID))
    {
        if (pAssignedStreamID != 0)
            *pAssignedStreamID = m_uiStreamID;
        return true;
    }
    return false;
}

bool ConnectionOrientedStream::StopListen()
{
    if (m_pTransport->m_streamManager.UnregisterStream(m_Type, m_uiStreamID))
    {
        m_uiStreamID = 0;
        return true;
    }
    return false;
}

bool ConnectionOrientedStream::OpenEndPoint(EndPoint*)
{
    return false;
}

EndPoint* ConnectionOrientedStream::FindEndPointByPID(unsigned int)
{
    return 0;
}

unsigned int ConnectionOrientedStream::GetEndPointNumber()
{
    return 0;
}

EndPoint* ConnectionOrientedStream::FindEndPointByCID(unsigned int)
{
    return 0;
}

qVector<unsigned int> ConnectionOrientedStream::GetAllConnectionIDs() const
{
    return qVector<unsigned int>();
}

void ConnectionOrientedStream::LockEndPointMap()
{
}

void ConnectionOrientedStream::UnlockEndPointMap()
{
}

qResult ConnectionOrientedStream::Initialize()
{
    return qResult(STREAM_RESULT_SUCCESS);
}

qResult ConnectionOrientedStream::Teardown()
{
    return qResult(STREAM_RESULT_SUCCESS);
}

qResult ConnectionOrientedStream::RegisterEventHandler(TransportEventHandler* pEventHandler)
{
    if (m_pEventHandler != 0)
    {
        return qResult(STREAM_RESULT_ALREADY_REGISTERED);
    }
    m_pEventHandler = pEventHandler;
    return qResult(STREAM_RESULT_SUCCESS);
}

qResult ConnectionOrientedStream::SendBroadcast(StationURL*, Buffer*)
{
    return qResult(STREAM_RESULT_NOT_IMPLEMENTED);
}

qResult ConnectionOrientedStream::Send(StationURL*, Buffer*)
{
    return qResult(STREAM_RESULT_NOT_IMPLEMENTED);
}

qResult ConnectionOrientedStream::AddEndPointToGroup(EndPoint*, EndPointGroup*)
{
    return qResult(STREAM_RESULT_NOT_IMPLEMENTED);
}

qResult ConnectionOrientedStream::RemoveEndPointFromGroup(EndPoint*, EndPointGroup*)
{
    return qResult(STREAM_RESULT_NOT_IMPLEMENTED);
}

void ConnectionOrientedStream::Trace(unsigned long long)
{
}

void ConnectionOrientedStream::AddPIDEndPointAssociation(unsigned int, EndPoint*)
{
}

bool ConnectionOrientedStream::RemovePIDEndPointAssociation(unsigned int, EndPoint*)
{
    return false;
}

void ConnectionOrientedStream::AddCIDEndPointAssociation(unsigned int, EndPoint*)
{
}

bool ConnectionOrientedStream::RemoveCIDEndPointAssociation(unsigned int, EndPoint*)
{
    return false;
}

}
}
