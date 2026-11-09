#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PRUDPSTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PRUDPSTREAM_HPP

#include <functional>
#include <list>
#include <map>
#include <utility>
#include <vector>

#include "nn/nex/ConnectionOrientedStream.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/EndPointUniqueID.hpp"
#include "nn/nex/LogicalClockTmpl.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/PRUDPMessageInterface.hpp"
#include "nn/nex/PseudoGlobalVariable.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/TimeoutManager.hpp"
#include "nn/nex/TransportSignatureGenerator.hpp"

namespace nn
{
namespace nex
{

class Key;
class Packet;
class PacketOut;
class PRUDPEndPoint;
class PRUDPMessageSelector;
class SignatureBytes;

extern int g_terminateImmediatelyTimeout;
extern unsigned int g_negotiationFlagsMask;
extern PseudoGlobalVariable<unsigned char> g_negotiationVersion;

class PRUDPStream : public ConnectionOrientedStream
{
public:
    PRUDPStream(Stream::Type type, RootTransport* pTransport);
    virtual ~PRUDPStream();

    virtual bool ReceiveIncomingPacket(unsigned short uiPort, unsigned char uiStreamID, Packet* pPacket);
    virtual void DoWork();
    virtual bool IsDuplicateReorderingPacket(Packet* pPacket);
    virtual bool StopListen();
    virtual bool ResponsibleForURL(const StationURL* pStationURL);
    virtual const wchar_t* GetURLType();
    virtual EndPoint* OpenEndPoint(const StationURL* pStationURL);
    virtual EndPoint* OpenEndPoint(unsigned int uiConnectionID);
    virtual bool OpenEndPoint(EndPoint* pEndPoint);
    virtual EndPoint* FindEndPointByPID(unsigned int uiPrincipalID);
    virtual unsigned int GetEndPointNumber();
    virtual EndPoint* FindEndPointByCID(unsigned int uiConnectionID);
    virtual qVector<unsigned int> GetAllConnectionIDs() const;
    virtual void CloseEndPoint(EndPoint* pEndPoint);
    virtual void LockEndPointMap();
    virtual void UnlockEndPointMap();
    virtual qResult Initialize();
    virtual qResult Teardown();
    virtual qResult SendBroadcast(StationURL* pStationURL, Buffer* pBuffer);
    virtual qResult Send(StationURL* pStationURL, Buffer* pBuffer);
    virtual bool IsCapable(unsigned int uiCapability);
    virtual void AddPIDEndPointAssociation(unsigned int uiPrincipalID, EndPoint* pEndPoint);
    virtual bool RemovePIDEndPointAssociation(unsigned int uiPrincipalID, EndPoint* pEndPoint);
    virtual void AddCIDEndPointAssociation(unsigned int uiConnectionID, EndPoint* pEndPoint);
    virtual bool RemoveCIDEndPointAssociation(unsigned int uiConnectionID, EndPoint* pEndPoint);
    virtual void ForceCompleteIO();
    virtual void SetMaxSilenceTimeAll(unsigned int uiSilenceTime);

    TimeoutManager& GetTimeoutManager() { return m_timeoutManager; }

    unsigned short GetPreferredPortNumber();
    bool Send(unsigned short uiPort, unsigned char uiStreamID, PacketOut* pPacket, Key* pKey);
    PRUDPMessageInterface::SignatureMethod DecideSignatureMethod(const Packet* pPacket, Stream::Type type, bool bFlag);
    void ReleaseEndPoint(PRUDPEndPoint* pEndPoint);
    void OnSetTerminateImmediately();

private:
    friend class PRUDPEndPoint;
    friend class StreamManager;

    typedef std::map<EndPointUniqueID, PRUDPEndPoint*, std::less<EndPointUniqueID>,
                     MemAllocator<std::pair<const EndPointUniqueID, PRUDPEndPoint*> > >
        EndPointMap;
    typedef std::list<PRUDPEndPoint*, MemAllocator<PRUDPEndPoint*> > EndPointList;
    typedef std::multimap<unsigned int, EndPoint*, std::less<unsigned int>,
                          MemAllocator<std::pair<const unsigned int, EndPoint*> > >
        EndPointAssociationMap;
    typedef std::vector<unsigned int, MemAllocator<unsigned int> > ConnectionIDVector;

    PRUDPEndPoint* CreateEndPoint(const StationURL* pStationURL, unsigned short uiPortNumber, bool bFlag);
    void ReleaseEndPointImpl(PRUDPEndPoint* pEndPoint);
    void PurgeDeletedEndPoints(bool bForce);
    void PurgeDeletedEndPointsImpl(bool bForce);
    PRUDPEndPoint* ServiceConnectionRequest(InetAddress* pAddress, Buffer* pBuffer, unsigned short uiPort,
                                            unsigned char uiStreamID);
    void ServiceUserMsg(InetAddress* pAddress, Buffer* pBuffer);
    int CheckSignature(InetAddress* pAddress, unsigned short uiPort, unsigned char uiStreamID, Packet* pPacket);
    int CheckSignature(InetAddress* pAddress, unsigned short uiPort, unsigned char uiStreamID, Packet* pPacket,
                       Key* pKey, SignatureBytes* pSignature, PRUDPMessageInterface::SignatureMethod method);
    void VersionNegotiation(Packet* pRequest, Packet* pResponse);

    TimeoutManager m_timeoutManager;
    EndPointMap m_endPoints;
    EndPointList m_deletedEndPoints;
    Time m_lastWorkTime;
    bool m_bDestroying;
    unsigned int m_uiContextId;
    unsigned short m_uiPreferredPort;
    TransportSignatureGenerator m_signatureGenerator;
    LogicalClockTmpl<unsigned short, short> m_clock;
    EndPointAssociationMap m_pidEndPoints;
    EndPointAssociationMap m_cidEndPoints;
    ConnectionIDVector m_connectionIDs;
    PRUDPMessageSelector* m_pMessageSelector;
    unsigned char m_uiVersion;
    CriticalSection m_endPointMapLock;

    static bool s_bSendFlag;
    static unsigned char s_uiDefaultVersion;
};

}
}

#endif
