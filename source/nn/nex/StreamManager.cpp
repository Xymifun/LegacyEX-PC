#include "nn/nex/StreamManager.hpp"

#include <wchar.h>

#include "nn/nex/ConnectionOrientedStream.hpp"
#include "nn/nex/EmulationDevice.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/PRUDPStream.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/StreamSettings.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/SystemSetting.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int MAX_SILENCE_TIMEOUT = 30000;
const unsigned int KEEP_ALIVE_TIMEOUT = 5000;
const unsigned int UNKNOWN_LISTEN_OPTION = 2;
const unsigned int SIMULATED_FAULT_ERROR = 5;

}

wchar_t StreamManager::s_aSandboxAccessKey[SandboxAccessKeyCapacity];
int StreamManager::s_iKeySuffix = 0;
float StreamManager::s_fPacketDropProbability = 0.0f;

SystemSetting g_useDistinctPortForStreamManager(L"UseDistinctPortForStreamManager", 0);

void StreamManager::SetStreamKey()
{
    if (s_iKeySuffix == 0)
    {
        g_streamSettings[m_type].Get().SetKey(String(s_aSandboxAccessKey));
        return;
    }

    String key;
    key.Format(L"%s%d", s_aSandboxAccessKey, s_iKeySuffix);
    g_streamSettings[m_type].Get().SetKey(key);
}

StreamManager::StreamManager(Stream::Type type, RootTransport* pTransport)
{
    m_type = type;
    m_pProtocolRequestBroker = 0;
    m_pTransport = pTransport;
    Network::AcquireInstance();
    SetStreamKey();
    m_pStream = new PRUDPStream(m_type, m_pTransport);
    g_streamSettings[m_type].Get().SetMaxSilenceTime(MAX_SILENCE_TIMEOUT);
    g_streamSettings[m_type].Get().SetKeepAliveTimeout(KEEP_ALIVE_TIMEOUT);
    if (s_fPacketDropProbability > 0.0f)
    {
        m_pTransport->GetInputEmulationDevice()->Enable();
        m_pTransport->GetInputEmulationDevice()->SetPacketDropProbability(s_fPacketDropProbability);
    }

    m_pConnectionManager = new ConnectionManager(this, Job::JOB_TYPE_2, true, false);
    m_pConnectionManager->AttachStream(m_pStream, true);
    m_pSecureStream = 0;
    m_uiPort = 0;
    m_bInitialized = false;
}

StreamManager::~StreamManager()
{
    if (m_pSecureStream != 0)
    {
        delete m_pSecureStream;
        m_pSecureStream = 0;
    }

    if (m_bInitialized)
    {
        m_pStream->StopListen();
        m_pTransport->StopListen(m_uiPort);
        m_bInitialized = false;
    }

    if (m_pStream != 0)
    {
        m_pStream->Teardown();
        delete m_pStream;
        m_pStream = 0;
    }

    delete m_pConnectionManager;
    delete m_pProtocolRequestBroker;
    Network::ReleaseInstance();
}

bool StreamManager::Initialize(unsigned short uiPort, unsigned char uiStreamID, unsigned int uiFlags)
{
    if (m_bInitialized)
        return false;

    m_pConnectionManager->m_bFilterEnabled = (uiFlags & INITIALIZE_FLAG_1) != 0;
    if (m_pStream->Initialize() == false)
        return false;

    if (g_useDistinctPortForStreamManager.m_value != 0)
    {
        bool bListening;
        if ((uiFlags & INITIALIZE_FLAG_2) != 0)
        {
            bListening = m_pTransport->StartListen(uiPort, &m_uiPort, false, UNKNOWN_LISTEN_OPTION, false);
        }
        else
        {
            bListening = m_pTransport->StartListen(uiPort, &m_uiPort, false, UNKNOWN_LISTEN_OPTION, false)
                         || m_pTransport->StartListen(0, &m_uiPort, false, UNKNOWN_LISTEN_OPTION, false);
        }

        if (!bListening)
            return false;

        m_pStream->m_uiPreferredPort = m_uiPort;
    }
    else
    {
        bool bListening;
        if ((uiFlags & INITIALIZE_FLAG_2) != 0)
        {
            bListening = m_pTransport->StartListen(uiPort, &m_uiPort, true, 0, false);
        }
        else
        {
            bListening = m_pTransport->StartListen(uiPort, &m_uiPort, true, 0, false)
                         || m_pTransport->StartListen(0, &m_uiPort, true, 0, false);
        }

        if (!bListening)
            return false;
    }

    bool bStreamListening;
    if ((uiFlags & INITIALIZE_FLAG_2) != 0)
    {
        bStreamListening = m_pStream->StartListen(uiStreamID, 0);
    }
    else
    {
        bStreamListening = m_pStream->StartListen(uiStreamID, 0) || m_pStream->StartListen(0, 0);
    }

    if (!bStreamListening)
        return false;

    m_bInitialized = true;
    return true;
}

void StreamManager::Terminate()
{
    m_pConnectionManager->Terminate();
}

bool StreamManager::IsTerminated() const
{
    return m_pConnectionManager->IsTerminated();
}

void StreamManager::AssociateSecureStream(ConnectionOrientedStream* pSecureStream,
                                          ConnectionManager::ConnectionFilter pFilter)
{
    m_pSecureStream = pSecureStream;
    m_pConnectionManager->AttachStream(m_pSecureStream, false);
    m_pConnectionManager->RegisterConnectionFilter(pFilter, UserContext(pSecureStream));
}

void StreamManager::SetSandboxAccessKey(const String& rKey)
{
    wcsncpy(s_aSandboxAccessKey, rKey, SandboxAccessKeyCapacity - 1);
}

void StreamManager::SetMaxSilenceTimeout(unsigned int uiTimeout)
{
    g_streamSettings[m_type].Get().SetMaxSilenceTime(uiTimeout);
}

void StreamManager::SetKeepAliveTimeout(unsigned int uiTimeout)
{
    g_streamSettings[m_type].Get().SetKeepAliveTimeout(uiTimeout);
}

bool StreamManager::SimulateFault(unsigned int uiPrincipalID)
{
    ConnectionOrientedStream* pStream = m_pSecureStream != 0 ? m_pSecureStream : m_pStream;
    if (pStream == 0)
        return false;

    EndPoint* pEndPoint = pStream->FindEndPointByPID(uiPrincipalID);
    if (pEndPoint == 0)
        return false;

    pEndPoint->SignalFaultEvent(SIMULATED_FAULT_ERROR, true);
    return true;
}

}
}
