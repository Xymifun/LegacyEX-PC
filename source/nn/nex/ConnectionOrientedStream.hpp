#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CONNECTIONORIENTEDSTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CONNECTIONORIENTEDSTREAM_HPP

#include "nn/nex/Stream.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class EndPoint;
class EndPointGroup;
class RootTransport;
class StationURL;
class TransportEventHandler;

class ConnectionOrientedStream : public Stream
{
public:
    ConnectionOrientedStream(Stream::Type type, RootTransport* pTransport);
    virtual ~ConnectionOrientedStream();

    virtual bool StartListen(unsigned char uiStreamID, unsigned char* pAssignedStreamID);
    virtual bool StopListen();
    virtual bool ResponsibleForURL(const StationURL* pStationURL) = 0;
    virtual const wchar_t* GetURLType() = 0;
    virtual EndPoint* OpenEndPoint(const StationURL* pStationURL) = 0;
    virtual EndPoint* OpenEndPoint(unsigned int uiPrincipalID) = 0;
    virtual bool OpenEndPoint(EndPoint* pEndPoint);
    virtual EndPoint* FindEndPointByPID(unsigned int uiPrincipalID);
    virtual unsigned int GetEndPointNumber();
    virtual EndPoint* FindEndPointByCID(unsigned int uiConnectionID);
    virtual qVector<unsigned int> GetAllConnectionIDs() const;
    virtual void CloseEndPoint(EndPoint* pEndPoint) = 0;
    virtual void LockEndPointMap();
    virtual void UnlockEndPointMap();
    virtual qResult Initialize();
    virtual qResult Teardown();
    virtual qResult RegisterEventHandler(TransportEventHandler* pEventHandler);
    virtual qResult SendBroadcast(StationURL* pStationURL, Buffer* pBuffer);
    virtual qResult Send(StationURL* pStationURL, Buffer* pBuffer);
    virtual qResult AddEndPointToGroup(EndPoint* pEndPoint, EndPointGroup* pEndPointGroup);
    virtual qResult RemoveEndPointFromGroup(EndPoint* pEndPoint, EndPointGroup* pEndPointGroup);
    virtual bool IsCapable(unsigned int uiCapability) = 0;
    virtual void Trace(unsigned long long uiFilter);
    virtual void AddPIDEndPointAssociation(unsigned int uiPrincipalID, EndPoint* pEndPoint);
    virtual bool RemovePIDEndPointAssociation(unsigned int uiPrincipalID, EndPoint* pEndPoint);
    virtual void AddCIDEndPointAssociation(unsigned int uiConnectionID, EndPoint* pEndPoint);
    virtual bool RemoveCIDEndPointAssociation(unsigned int uiConnectionID, EndPoint* pEndPoint);

    unsigned char GetStreamID() const { return m_uiStreamID; }

protected:
    TransportEventHandler* m_pEventHandler;
    RootTransport* m_pTransport;
    unsigned char m_uiStreamID;
};

}
}

#endif
