#include "nn/nex/PRUDPStream.hpp"

#include <algorithm>
#include <string.h>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/PRUDPEndPoint.hpp"
#include "nn/nex/PRUDPMessageSelector.hpp"
#include "nn/nex/Packet.hpp"
#include "nn/nex/PacketIn.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/Platform.hpp"
#include "nn/nex/PseudoSingleton.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/SignatureBytes.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/StreamSettings.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/TransportAdapter.hpp"
#include "nn/nex/TransportEventHandler.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qResult.hpp"

static void PRUDPStreamCoreTerminateImmediatelyCallback(void* pContext)
{
    static_cast<nn::nex::PRUDPStream*>(pContext)->OnSetTerminateImmediately();
}

namespace nn
{
namespace nex
{

namespace
{

enum ResultCode
{
    RESULT_SUCCESS = 0x00010001,
    RESULT_INVALID_BUFFER = static_cast<int>(0x8001000Au),
    RESULT_INVALID_URL = static_cast<int>(0x80050003u),
    RESULT_CONNECTION_FAILED = static_cast<int>(0x80050007u)
};

const int CHECK_ACCEPTED = 1;
const int CHECK_REJECTED = 2;
const int CHECK_DROPPED = 3;

const unsigned int CRITICAL_SECTION_TRACE_MASK = 0x4000;
const unsigned int DO_WORK_INTERVAL = 25;
const unsigned int INFINITE_SILENCE_TIME = 0xFFFFFFFFu;
const unsigned int RANDOM_SEQUENCE_LIMIT = 0xFFFF;
const unsigned char NEGOTIATION_SIGNATURE_VERSION = 1;
const unsigned int FLAGS_SHIFT = 8;
const unsigned int FLAGS_WITHOUT_SECURE_MODE = 0xFFFFFE;
const Stream::Type STREAM_TYPE_4 = static_cast<Stream::Type>(4);
const Stream::Type STREAM_TYPE_6 = static_cast<Stream::Type>(6);
const wchar_t* const URL_TYPE_NAME = L"prudp";
const wchar_t* const BROADCAST_ADDRESS = L"255.255.255.255";
const wchar_t* const LOOPBACK_ADDRESS = L"127.0.0.1";

}

int g_terminateImmediatelyTimeout = 2000;
unsigned int g_negotiationFlagsMask = 1;
PseudoGlobalVariable<unsigned char> g_negotiationVersion(4);

bool PRUDPStream::s_bSendFlag = false;
unsigned char PRUDPStream::s_uiDefaultVersion = 1;

PRUDPStream::PRUDPStream(Stream::Type type, RootTransport* pTransport)
    : ConnectionOrientedStream(type, pTransport),
      m_timeoutManager(),
      m_endPoints(),
      m_deletedEndPoints(),
      m_lastWorkTime(),
      m_signatureGenerator(),
      m_clock(),
      m_pidEndPoints(),
      m_cidEndPoints(),
      m_connectionIDs(),
      m_endPointMapLock(CRITICAL_SECTION_TRACE_MASK)
{
    m_lastWorkTime = 0;
    m_bDestroying = false;
    unsigned int uiContextId = g_usingInstantiationContext;
    if (g_usingInstantiationContext)
        uiContextId = g_instantiationContextVariable->GetRef();

    m_uiPreferredPort = 0;
    m_uiContextId = uiContextId;
    m_pTransport = pTransport;
    m_clock = static_cast<unsigned long long>(Time::GetTime()) + Platform::GetRandomNumber(RANDOM_SEQUENCE_LIMIT);
    Core::GetInstance()->SetTerminateImmediatelyCallback(PRUDPStreamCoreTerminateImmediatelyCallback, this);
    m_pMessageSelector = &Network::GetInstance()->GetMessageSelector();
    switch (type)
    {
    case Stream::Unknown1:
    case Stream::RendezVous:
    case STREAM_TYPE_4:
    case STREAM_TYPE_6:
    case Stream::Unknown9:
    case Stream::RendezVousSecure:
        m_uiVersion = s_uiDefaultVersion;
        break;
    default:
        m_uiVersion = 0;
        break;
    }
}

PRUDPStream::~PRUDPStream()
{
    Core::GetInstance()->ResetTerminateImmediatelyCallback();
    if (m_uiStreamID != 0)
        StopListen();

    m_bDestroying = true;
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    LockEndPointMap();
    EndPointMap::iterator endPoint;
    for (endPoint = m_endPoints.begin(); endPoint != m_endPoints.end(); ++endPoint)
    {
        delete endPoint->second;
    }

    UnlockEndPointMap();
    PurgeDeletedEndPoints(true);
}

void PRUDPStream::OnSetTerminateImmediately()
{
    m_timeoutManager.ForceSetPacketTimeouts(Core::GetInstance()->GetTerminateImmediatelyTime()
                                            + g_terminateImmediatelyTimeout);
}

bool PRUDPStream::StopListen()
{
    ConnectionOrientedStream::StopListen();
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    LockEndPointMap();
    EndPointMap::iterator endPoint;
    for (endPoint = m_endPoints.begin(); endPoint != m_endPoints.end();)
    {
        (endPoint++)->second->SetConnectionState(EndPoint::Faulty);
    }

    UnlockEndPointMap();
    return true;
}

void PRUDPStream::LockEndPointMap()
{
    m_endPointMapLock.Enter();
}

void PRUDPStream::UnlockEndPointMap()
{
    m_endPointMapLock.Leave();
}

void PRUDPStream::PurgeDeletedEndPoints(bool bForce)
{
    if (g_threadSafetyEnabled)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        PurgeDeletedEndPointsImpl(bForce);
    }
    else
    {
        PurgeDeletedEndPointsImpl(bForce);
    }
}

void PRUDPStream::PurgeDeletedEndPointsImpl(bool bForce)
{
    EndPointList::iterator endPoint = m_deletedEndPoints.begin();
    while (endPoint != m_deletedEndPoints.end())
    {
        PRUDPEndPoint* pEndPoint = *endPoint;
        if (!bForce && pEndPoint->m_uiCtorFlag != 0)
        {
            pEndPoint->CompleteIO(qResult(RESULT_CONNECTION_FAILED));
            ++endPoint;
        }
        else
        {
            delete pEndPoint;
            endPoint = m_deletedEndPoints.erase(endPoint);
        }
    }
}

qResult PRUDPStream::Initialize()
{
    qResult result = ConnectionOrientedStream::Initialize();
    if (result == true && m_pTransport == 0)
        m_pTransport = RootTransport::GetInstance();

    return result;
}

qResult PRUDPStream::Teardown()
{
    return ConnectionOrientedStream::Teardown();
}

bool PRUDPStream::ResponsibleForURL(const StationURL* pStationURL)
{
    return pStationURL->GetURLType() == StationURL::Prudp;
}

PRUDPEndPoint* PRUDPStream::CreateEndPoint(const StationURL* pStationURL, unsigned short uiPortNumber, bool)
{
    PRUDPEndPoint* pEndPoint = 0;
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    InetAddress address;
    if (pStationURL->IsValid())
    {
        unsigned char uiStreamID = pStationURL->GetStreamID();
        EndPointMap::iterator endPoint = m_endPoints.find(EndPointUniqueID(*pStationURL->GetInetAddress(), uiStreamID));
        if (endPoint == m_endPoints.end() || (pEndPoint = endPoint->second) == 0)
        {
            pEndPoint = new PRUDPEndPoint(this, pStationURL, &m_clock, m_uiVersion);
            pEndPoint->m_uiPort = uiPortNumber;
            unsigned char uiEndPointStreamID = pStationURL->GetStreamID();
            m_endPoints.insert(std::pair<const EndPointUniqueID, PRUDPEndPoint*>(
                EndPointUniqueID(*pEndPoint->m_stationURL.GetInetAddress(), uiEndPointStreamID), pEndPoint));
        }
    }

    return pEndPoint;
}

unsigned short PRUDPStream::GetPreferredPortNumber()
{
    unsigned short uiPort = m_uiPreferredPort;
    if (uiPort == 0)
        uiPort = m_pTransport->GetPortNumber();

    return uiPort;
}

EndPoint* PRUDPStream::OpenEndPoint(const StationURL* pStationURL)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    PRUDPEndPoint* pEndPoint = CreateEndPoint(pStationURL, GetPreferredPortNumber(), true);
    if (pEndPoint != 0)
        pEndPoint->_Open();

    return pEndPoint;
}

EndPoint* PRUDPStream::OpenEndPoint(unsigned int uiConnectionID)
{
    EndPoint* pEndPoint = FindEndPointByCID(uiConnectionID);
    if (pEndPoint != 0)
        pEndPoint->_Open();

    return pEndPoint;
}

bool PRUDPStream::OpenEndPoint(EndPoint* pEndPoint)
{
    if (g_threadSafetyEnabled)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        pEndPoint->_Open();
        return true;
    }

    pEndPoint->_Open();
    return true;
}

void PRUDPStream::CloseEndPoint(EndPoint* pEndPoint)
{
    PRUDPEndPoint* pPRUDPEndPoint = static_cast<PRUDPEndPoint*>(pEndPoint);
    if (g_threadSafetyEnabled)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        if (pPRUDPEndPoint->m_iOpenCount != 0)
            pPRUDPEndPoint->_Close();

        ReleaseEndPoint(pPRUDPEndPoint);
        return;
    }

    if (pPRUDPEndPoint->m_iOpenCount != 0)
        pPRUDPEndPoint->_Close();

    ReleaseEndPoint(pPRUDPEndPoint);
}

void PRUDPStream::ForceCompleteIO()
{
    {
        EndPointList endPoints;
        LockEndPointMap();
        EndPointMap::iterator endPoint;
        for (endPoint = m_endPoints.begin(); endPoint != m_endPoints.end(); ++endPoint)
        {
            PRUDPEndPoint* pEndPoint = endPoint->second;
            endPoints.push_back(pEndPoint);
        }

        UnlockEndPointMap();
        EndPointList::iterator listedEndPoint;
        for (listedEndPoint = endPoints.begin(); listedEndPoint != endPoints.end(); ++listedEndPoint)
        {
            PRUDPEndPoint* pEndPoint = *listedEndPoint;
            pEndPoint->CompleteIO(qResult(RESULT_CONNECTION_FAILED));
        }
    }

    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    for (EndPointList::iterator endPoint = m_deletedEndPoints.begin(); endPoint != m_deletedEndPoints.end(); ++endPoint)
    {
        PRUDPEndPoint* pEndPoint = *endPoint;
        pEndPoint->CompleteIO(qResult(RESULT_CONNECTION_FAILED));
    }
}

bool PRUDPStream::IsCapable(unsigned int uiCapability)
{
    switch (uiCapability)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 6:
    case 7:
        return true;
    default:
        return false;
    }
}

EndPoint* PRUDPStream::FindEndPointByPID(unsigned int uiPrincipalID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    EndPointAssociationMap::iterator association = m_pidEndPoints.find(uiPrincipalID);
    if (association != m_pidEndPoints.end())
        return association->second;

    return 0;
}

unsigned int PRUDPStream::GetEndPointNumber()
{
    return m_connectionIDs.size();
}

EndPoint* PRUDPStream::FindEndPointByCID(unsigned int uiConnectionID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    EndPointAssociationMap::iterator association = m_cidEndPoints.find(uiConnectionID);
    if (association != m_cidEndPoints.end())
        return association->second;

    return 0;
}

qVector<unsigned int> PRUDPStream::GetAllConnectionIDs() const
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    return qVector<unsigned int>(m_connectionIDs);
}

qResult PRUDPStream::SendBroadcast(StationURL* pStationURL, Buffer* pBuffer)
{
    StationURL stationURL;
    if (pStationURL != 0)
        stationURL = *pStationURL;

    qList<StationURL> localURLs;
    g_transportAdapter->GetLocalURLs(&localURLs);
    stationURL.SetAddress(localURLs.empty() ? LOOPBACK_ADDRESS : BROADCAST_ADDRESS);
    stationURL.SetPortNumber(RootTransport::GetWellKnownPortNumber());
    return Send(&stationURL, pBuffer);
}

qResult PRUDPStream::Send(StationURL* pStationURL, Buffer* pBuffer)
{
    if (!pStationURL->IsValid())
        return qResult(RESULT_INVALID_URL);

    if (pBuffer != 0)
    {
        PacketOut* pPacket = new PacketOut(0, PACKET_TYPE_USER, 0, pBuffer, 0);
        pPacket->m_address = *pStationURL->GetInetAddress();
        bool bSent = Send(GetPreferredPortNumber(), m_uiStreamID, pPacket, 0);
        pPacket->ReleaseRef();
        if (!bSent)
            return qResult(RESULT_CONNECTION_FAILED);
    }
    else
    {
        return qResult(RESULT_INVALID_BUFFER);
    }

    return qResult(RESULT_SUCCESS);
}

void PRUDPStream::SetMaxSilenceTimeAll(unsigned int uiSilenceTime)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    LockEndPointMap();
    EndPointMap::iterator endPoint;
    for (endPoint = m_endPoints.begin(); endPoint != m_endPoints.end(); ++endPoint)
    {
        PRUDPEndPoint* pEndPoint = endPoint->second;
        if (pEndPoint->GetMaxSilenceTime() != INFINITE_SILENCE_TIME)
            pEndPoint->SetMaxSilenceTime(uiSilenceTime);
    }

    UnlockEndPointMap();
}

void PRUDPStream::AddPIDEndPointAssociation(unsigned int uiPrincipalID, EndPoint* pEndPoint)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_pidEndPoints.insert(std::pair<const unsigned int, EndPoint*>(uiPrincipalID, pEndPoint));
}

bool PRUDPStream::RemovePIDEndPointAssociation(unsigned int uiPrincipalID, EndPoint* pEndPoint)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    EndPointAssociationMap::iterator association = m_pidEndPoints.find(uiPrincipalID);
    while (association != m_pidEndPoints.end() && association->first == uiPrincipalID)
    {
        if (association->second == pEndPoint)
        {
            m_pidEndPoints.erase(association);
            return true;
        }

        ++association;
    }

    return false;
}

void PRUDPStream::AddCIDEndPointAssociation(unsigned int uiConnectionID, EndPoint* pEndPoint)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_cidEndPoints.insert(std::pair<const unsigned int, EndPoint*>(uiConnectionID, pEndPoint));
    m_connectionIDs.push_back(uiConnectionID);
}

bool PRUDPStream::RemoveCIDEndPointAssociation(unsigned int uiConnectionID, EndPoint* pEndPoint)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    EndPointAssociationMap::iterator association = m_cidEndPoints.find(uiConnectionID);
    while (association != m_cidEndPoints.end() && association->first == uiConnectionID)
    {
        if (association->second == pEndPoint)
        {
            m_cidEndPoints.erase(association);
            m_connectionIDs.erase(std::find(m_connectionIDs.begin(), m_connectionIDs.end(), uiConnectionID));
            return true;
        }

        ++association;
    }

    return false;
}

PRUDPEndPoint* PRUDPStream::ServiceConnectionRequest(InetAddress* pAddress, Buffer* pBuffer, unsigned short uiPort,
                                                     unsigned char uiStreamID)
{
    StationURL stationURL;
    stationURL.SetInetAddress(pAddress);
    stationURL.SetStreamID(uiStreamID);
    PRUDPEndPoint* pEndPoint = CreateEndPoint(&stationURL, uiPort, false);
    if (m_pEventHandler != 0)
    {
        bool bAccepted;
        if (pBuffer->GetLength() != 0)
            bAccepted = m_pEventHandler->ConnectionRequest(this, &pEndPoint->m_stationURL, pBuffer, pEndPoint);
        else
            bAccepted = m_pEventHandler->ConnectionRequest(this, &pEndPoint->m_stationURL, 0, pEndPoint);

        if (!bAccepted)
        {
            ReleaseEndPoint(pEndPoint);
            pEndPoint = 0;
        }
    }

    return pEndPoint;
}

void PRUDPStream::ServiceUserMsg(InetAddress* pAddress, Buffer* pBuffer)
{
    if (m_pEventHandler != 0)
    {
        StationURL stationURL;
        stationURL.SetInetAddress(pAddress);
        if (pBuffer->GetLength() != 0)
            m_pEventHandler->Receive(this, pBuffer, &stationURL);
        else
            m_pEventHandler->Receive(this, 0, &stationURL);
    }
}

void PRUDPStream::ReleaseEndPoint(PRUDPEndPoint* pEndPoint)
{
    if (g_threadSafetyEnabled)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        ReleaseEndPointImpl(pEndPoint);
    }
    else
    {
        ReleaseEndPointImpl(pEndPoint);
    }
}

void PRUDPStream::ReleaseEndPointImpl(PRUDPEndPoint* pEndPoint)
{
    if (pEndPoint->m_iOpenCount != 0 || pEndPoint->PeerIsConnected())
        return;

    pEndPoint->SetConnectionID(0);
    pEndPoint->SetPrincipalID(0);
    EndPointMap::iterator endPoint = m_endPoints.find(
        EndPointUniqueID(*pEndPoint->m_stationURL.GetInetAddress(), pEndPoint->m_stationURL.GetStreamID()));
    if (endPoint == m_endPoints.end())
        return;

    if (endPoint->second != 0)
    {
        endPoint = m_endPoints.find(
            EndPointUniqueID(*pEndPoint->m_stationURL.GetInetAddress(), pEndPoint->m_stationURL.GetStreamID()));
        if (endPoint != m_endPoints.end())
            m_endPoints.erase(endPoint);

        m_deletedEndPoints.push_back(pEndPoint);
    }
}

void PRUDPStream::DoWork()
{
    Time currentTime(g_lastTime);
    if (m_lastWorkTime == 0)
    {
        m_lastWorkTime = currentTime;
        PurgeDeletedEndPoints(false);
        return;
    }

    if (static_cast<long long>(currentTime - m_lastWorkTime) > DO_WORK_INTERVAL)
    {
        m_lastWorkTime = currentTime;
        m_timeoutManager.ServicePacketTimeouts();
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        LockEndPointMap();
        EndPointMap::iterator endPoint;
        for (endPoint = m_endPoints.begin(); endPoint != m_endPoints.end(); ++endPoint)
        {
            endPoint->second->SendAggregateACK();
        }

        UnlockEndPointMap();
    }

    PurgeDeletedEndPoints(false);
}

int PRUDPStream::CheckSignature(InetAddress* pAddress, unsigned short uiPort, unsigned char uiStreamID, Packet* pPacket,
                                Key* pKey, SignatureBytes* pSignature, PRUDPMessageInterface::SignatureMethod method)
{
    SignatureBytes expectedSignature;
    int iResult = m_pMessageSelector->CalcExpectedSignature(pPacket, method, pKey, &m_signatureGenerator, pSignature,
                                                            expectedSignature);
    if (iResult != CHECK_DROPPED)
    {
        bool bValid = false;
        if (expectedSignature.m_uiLength == pPacket->m_uiSignatureSize)
            bValid = memcmp(expectedSignature.m_bytes, pPacket->m_aSignature, pPacket->m_uiSignatureSize) == 0;

        if (!bValid && (pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) != PACKET_TYPE_USER)
        {
            return CHECK_REJECTED;
        }

        iResult = CHECK_ACCEPTED;
    }

    return iResult;
}

PRUDPMessageInterface::SignatureMethod PRUDPStream::DecideSignatureMethod(const Packet* pPacket, Stream::Type type,
                                                                          bool bFlag)
{
    if ((pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK) == PACKET_TYPE_CONNECT
        && pPacket->m_uiVersion == NEGOTIATION_SIGNATURE_VERSION
        && !PRUDPEndPoint::IsNegotiationConnectionSignatureVersion(pPacket))
    {
        return PRUDPMessageInterface::SIGNATURE_METHOD_4;
    }

    return m_pMessageSelector->DecideSignatureMethod(pPacket, type, bFlag);
}

int PRUDPStream::CheckSignature(InetAddress* pAddress, unsigned short uiPort, unsigned char uiStreamID, Packet* pPacket)
{
    PRUDPEndPoint* pEndPoint = 0;
    LockEndPointMap();
    EndPointMap::iterator endPoint = m_endPoints.find(EndPointUniqueID(*pAddress, uiStreamID));
    if (endPoint != m_endPoints.end())
        pEndPoint = endPoint->second;

    UnlockEndPointMap();
    Key* pKey = 0;
    SignatureBytes* pSignature = 0;
    if (pEndPoint != 0)
    {
        pKey = &pEndPoint->GetEncryptionKey();
        pSignature = &pEndPoint->m_signature;
    }

    return CheckSignature(pAddress, uiPort, uiStreamID, pPacket, pKey, pSignature,
                          DecideSignatureMethod(pPacket, m_Type, pKey != 0));
}

void PRUDPStream::VersionNegotiation(Packet* pRequest, Packet* pResponse)
{
    unsigned char uiVersion = pRequest->m_uiVersion != m_uiVersion ? pRequest->m_uiVersion : m_uiVersion;
    pResponse->m_uiVersion = uiVersion;
    unsigned int uiNegotiatedFlags = 0;
    if (uiVersion != 0)
    {
        unsigned char uiRequestedVersion = static_cast<unsigned char>(pRequest->m_unknown90);
        const unsigned char& uiNegotiatedVersion = std::min(uiRequestedVersion, g_negotiationVersion.Get());
        unsigned int uiRequestedFlags = (pRequest->m_unknown90 >> FLAGS_SHIFT) & g_negotiationFlagsMask;
        uiNegotiatedFlags = uiNegotiatedVersion | (uiRequestedFlags << FLAGS_SHIFT);
        if ((m_Type != Stream::RendezVousSecure && m_Type != Stream::RendezVous)
            || g_streamSettings[m_Type].Get().GetSecureMode() != 1)
        {
            uiNegotiatedFlags = (((uiNegotiatedFlags >> FLAGS_SHIFT) & FLAGS_WITHOUT_SECURE_MODE) << FLAGS_SHIFT)
                                | uiNegotiatedVersion;
        }
    }

    unsigned char uiRequestedSubStreamID = pRequest->m_unknown6b;
    pResponse->m_unknown90 = uiNegotiatedFlags;
    pResponse->m_unknown6b = std::min(uiRequestedSubStreamID, g_streamSettings[m_Type].Get().GetMaxSubStreamID());
}

bool PRUDPStream::ReceiveIncomingPacket(unsigned short uiPort, unsigned char uiStreamID, Packet* pPacket)
{
    if (pPacket == 0)
        return true;

    unsigned short uiTypeAndFlags = pPacket->m_uiTypeAndFlags;
    InetAddress* pAddress = &pPacket->m_address;
    PRUDPEndPoint* pEndPoint;
    switch (uiTypeAndFlags & PACKET_TYPE_MASK)
    {
    case PACKET_TYPE_SYN:
    {
        if ((uiTypeAndFlags & PACKET_FLAG_ACK) == 0)
        {
            SignatureBytes signature;
            signature.m_uiLength = 0;
            m_pMessageSelector->CalcSignature(pPacket, PRUDPMessageInterface::SIGNATURE_METHOD_1, 0,
                                              VirtualPort(pPacket->m_uiSource).GetType(), &m_signatureGenerator, 0,
                                              signature);
            PacketOut* pReply = new PacketOut(0, PACKET_TYPE_SYN, PACKET_FLAG_ACK, 0, 0);
            pReply->m_sequenceID = pPacket->m_sequenceID.GetValue();
            pReply->m_bSkipEncryption = pPacket->m_bSkipEncryption;
            memcpy(pReply->m_aConnectionSignature, signature.m_bytes, signature.m_uiLength);
            pReply->m_uiConnectionSignatureSize = signature.m_uiLength;
            pReply->m_address = pPacket->m_address;
            VersionNegotiation(pPacket, pReply);
            Send(uiPort, uiStreamID, pReply, 0);
            pReply->ReleaseRef();
            pPacket->ReleaseRef();
            return true;
        }

        LockEndPointMap();
        EndPointMap::iterator endPoint = m_endPoints.find(EndPointUniqueID(*pAddress, uiStreamID));
        pEndPoint = 0;
        if (endPoint != m_endPoints.end())
            pEndPoint = endPoint->second;

        UnlockEndPointMap();
        break;
    }
    case PACKET_TYPE_CONNECT:
    {
        if ((uiTypeAndFlags & PACKET_FLAG_ACK) != 0)
        {
            int iResult = CheckSignature(pAddress, uiPort, uiStreamID, pPacket);
            if (iResult == CHECK_DROPPED)
            {
                pPacket->ReleaseRef();
                return true;
            }

            if (iResult == CHECK_REJECTED)
                return false;

            LockEndPointMap();
            EndPointMap::iterator endPoint = m_endPoints.find(EndPointUniqueID(*pAddress, uiStreamID));
            pEndPoint = 0;
            if (endPoint != m_endPoints.end())
                pEndPoint = endPoint->second;

            UnlockEndPointMap();
        }
        else
        {
            SignatureBytes signature;
            signature.m_uiLength = 0;
            m_pMessageSelector->CalcSignature(pPacket, PRUDPMessageInterface::SIGNATURE_METHOD_1, 0,
                                              VirtualPort(pPacket->m_uiSource).GetType(), &m_signatureGenerator, 0,
                                              signature);
            PRUDPMessageInterface::SignatureMethod method = DecideSignatureMethod(pPacket, m_Type, false);
            int iResult = CheckSignature(pAddress, uiPort, uiStreamID, pPacket, 0, &signature, method);
            if (iResult == CHECK_DROPPED)
            {
                pPacket->ReleaseRef();
                return true;
            }

            if (iResult == CHECK_REJECTED)
                return false;

            pEndPoint = ServiceConnectionRequest(pAddress, pPacket->m_pPayload, uiPort, uiStreamID);
            if (pEndPoint == 0)
            {
                pPacket->ReleaseRef();
                return true;
            }

            if (pPacket->m_bSkipEncryption)
                pEndPoint->m_bUnknown148 = true;

            if (!PRUDPEndPoint::IsNegotiationConnectionSignatureVersion(pPacket))
            {
                memcpy(pEndPoint->m_negotiationSignature.m_bytes, signature.m_bytes, signature.m_uiLength);
                pEndPoint->m_negotiationSignature.m_uiLength = signature.m_uiLength;
            }
            else
            {
                memcpy(pEndPoint->m_signature.m_bytes, signature.m_bytes, signature.m_uiLength);
                pEndPoint->m_signature.m_uiLength = signature.m_uiLength;
            }
        }

        break;
    }
    case PACKET_TYPE_USER:
    {
        ServiceUserMsg(pAddress, pPacket->m_pPayload);
        pPacket->ReleaseRef();
        return true;
    }
    default:
    {
        LockEndPointMap();
        EndPointMap::iterator endPoint = m_endPoints.find(EndPointUniqueID(*pAddress, uiStreamID));
        pEndPoint = 0;
        if (endPoint != m_endPoints.end())
            pEndPoint = endPoint->second;

        UnlockEndPointMap();
        int iResult = CHECK_ACCEPTED;
        if (pEndPoint == 0
            || (g_historyBundlingEnabled && pEndPoint->IsDuplicateReorderingPacket(static_cast<PacketIn*>(pPacket)))
            || (iResult = CheckSignature(pAddress, uiPort, uiStreamID, pPacket)) == CHECK_DROPPED)
        {
            pPacket->ReleaseRef();
            return true;
        }

        if (iResult == CHECK_REJECTED)
            return false;

        break;
    }
    }

    if (pEndPoint != 0 && !pEndPoint->IsFaulty())
        pEndPoint->ServiceIncomingPacket(static_cast<PacketIn*>(pPacket));

    pPacket->ReleaseRef();
    return true;
}

bool PRUDPStream::IsDuplicateReorderingPacket(Packet* pPacket)
{
    unsigned char uiStreamID = VirtualPort(pPacket->m_uiSource).GetStreamID();
    LockEndPointMap();
    EndPointMap::iterator endPoint = m_endPoints.find(EndPointUniqueID(pPacket->m_address, uiStreamID));
    PRUDPEndPoint* pEndPoint = 0;
    if (endPoint != m_endPoints.end())
        pEndPoint = endPoint->second;

    UnlockEndPointMap();
    if (pEndPoint == 0)
        return false;

    return pEndPoint->IsDuplicateReorderingPacket(static_cast<PacketIn*>(pPacket));
}

const wchar_t* PRUDPStream::GetURLType()
{
    return URL_TYPE_NAME;
}

bool PRUDPStream::Send(unsigned short uiPort, unsigned char uiStreamID, PacketOut* pPacket, Key* pKey)
{
    pPacket->m_uiSource = VirtualPort(m_Type, m_uiStreamID).m_uiValue;
    pPacket->m_uiDestination = VirtualPort(m_Type, uiStreamID).m_uiValue;
    LockEndPointMap();
    EndPointMap::iterator endPoint = m_endPoints.find(EndPointUniqueID(pPacket->m_address, uiStreamID));
    PRUDPEndPoint* pEndPoint = 0;
    if (endPoint != m_endPoints.end())
        pEndPoint = endPoint->second;

    UnlockEndPointMap();
    PRUDPMessageInterface::SignatureMethod method;
    if (pEndPoint != 0)
    {
        pPacket->m_uiVersion = pEndPoint->m_uiVersion;
        method = pEndPoint->DecideSignatureMethod(pPacket, m_Type, pKey != 0);
    }
    else
    {
        method = m_pMessageSelector->DecideSignatureMethod(pPacket, m_Type, pKey != 0);
    }

    if (method != PRUDPMessageInterface::SIGNATURE_METHOD_1)
    {
        SignatureBytes signature;
        SignatureBytes* pSignature = pEndPoint != 0 ? &pEndPoint->m_negotiationSignature : 0;

        if (!m_pMessageSelector->CalcSignature(pPacket, method, pKey, m_Type, &m_signatureGenerator, pSignature,
                                               signature))
        {
            return false;
        }

        memcpy(pPacket->m_aSignature, signature.m_bytes, signature.m_uiLength);
        pPacket->m_uiSignatureSize = signature.m_uiLength;
    }

    return m_pTransport->Send(uiPort, m_Type, m_uiStreamID, uiStreamID, pPacket, s_bSendFlag);
}

}
}
