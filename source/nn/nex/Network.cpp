#include "nn/nex/Network.hpp"

#include <string.h>

#include "nn/nex/CompressionAlgorithm.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/HMACChecksum.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/KeyedChecksumAlgorithm.hpp"
#include "nn/nex/MD5ChecksumWithKey.hpp"
#include "nn/nex/NATTraversalEngine.hpp"
#include "nn/nex/Relay.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/SocketDriver.hpp"
#include "nn/nex/StreamSettings.hpp"
#include "nn/nex/TransportAdapter.hpp"
#include "nn/nex/UDPTransport.hpp"
#include "nn/nex/ZLibCompression.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int APPEND_AT_END = 0xFFFFFFFFu;
const unsigned int NO_RELAY_GATHERING_ID = 0xFFFFFFFFu;
const unsigned int DEFAULT_SIGNATURE_KEY_LENGTH = 8;

}

Network::ECompAlgo Network::s_eCompAlgo = COMP_ALGO_NONE;
int Network::s_iZLibWindowBits = -1;
int Network::s_iZLibMemLevel = -1;
unsigned int Network::s_uiP2PDataPacketDefaultSignatureKeyLength = 0;
unsigned char Network::s_aP2PDataPacketDefaultSignatureKey[DefaultSignatureKeyCapacity];
unsigned char Network::s_aDefaultChecksumKey[DefaultChecksumKeyLength] = {
    0x26, 0xC3, 0x1F, 0x38, 0x1E, 0x46, 0xD6, 0xEB, 0x38, 0xE1, 0xAF, 0x6A, 0xB7, 0x0D, 0x11, 0x09};
bool Network::s_bEncryptionEnabled = true;
bool Network::s_bHMACChecksumEnabled = true;
bool Network::s_bMD5ChecksumEnabled = true;

Network::Network()
    : m_criticalSection(),
      m_instanceControl(ClassID),
      m_localURLs(),
      m_packetBufferManager(),
      m_connectivityManager(),
      m_messageSelector(),
      m_rc4Encryption(),
      m_p2pSignatureKey(),
      m_unknown3d0(),
      m_unknown3e0(),
      m_unknown42c(),
      m_unknown43c()
{
    m_instanceControl.SetDelegatorInstance(this);
    Core::AcquireInstance();
    RegisterTransportAdapter(&g_defaultTransportAdapter);
    m_pTransport = new UDPTransport();
    m_pTransport->Initialize();
    m_pNATTraversalEngine = new NATTraversalEngine();
    m_pCompression = 0;
    InitCompression(s_eCompAlgo);
    if (s_bEncryptionEnabled)
        SetRC4Encryption();

    m_pHMACChecksum = new HMACChecksum();
    unsigned char aDefaultSignatureKey[DEFAULT_SIGNATURE_KEY_LENGTH] = {1, 2, 3, 4, 5, 6, 7, 8};
    SetP2PDataPacketDefaultSignatureKey(aDefaultSignatureKey, DEFAULT_SIGNATURE_KEY_LENGTH);
    m_pMD5Checksum = new MD5ChecksumWithKey();
    InitChecksumAlgorithm();
    g_streamSettings[Stream::RendezVousSecure].Get().SetIsEncryptionRequired(true);
}

Network::~Network()
{
    UnregisterTransportAdapter();
    m_pNATTraversalEngine->Terminate();
    delete m_pNATTraversalEngine;
    m_pNATTraversalEngine = 0;
    delete m_pTransport;
    m_pTransport = 0;
    Core::ReleaseInstance();
    if (m_pCompression != 0)
    {
        delete m_pCompression;
        m_pCompression = 0;
    }

    if (g_streamSettings[Stream::Unknown9].Get().GetChecksumAlgorithm() == m_pMD5Checksum)
        g_streamSettings[Stream::Unknown9].Get().SetChecksumAlgorithm(0);

    if (g_streamSettings[Stream::Unknown1].Get().GetChecksumAlgorithm() == m_pMD5Checksum)
        g_streamSettings[Stream::Unknown1].Get().SetChecksumAlgorithm(0);

    if (g_streamSettings[Stream::Routing].Get().GetChecksumAlgorithm() == m_pMD5Checksum)
        g_streamSettings[Stream::Routing].Get().SetChecksumAlgorithm(0);

    delete m_pHMACChecksum;
    delete m_pMD5Checksum;
}

void Network::AcquireInstance()
{
    if (GetInstance() == 0)
        new Network;
    else
        GetInstance()->AddRef();
}

void Network::ReleaseInstance()
{
    GetInstance()->ReleaseRef();
}

bool Network::RegisterTransportAdapter(TransportAdapter* pAdapter)
{
    g_transportAdapter = pAdapter;
    if (pAdapter->GetSocketDriver() != 0)
        g_socketDriver = pAdapter->GetSocketDriver();

    g_transportAdapter->Load();
    return true;
}

bool Network::UnregisterTransportAdapter()
{
    g_socketDriver = g_defaultSocketDriver;
    if (g_transportAdapter != 0)
        g_transportAdapter->Unload();

    return true;
}

NATTraversalEngine* Network::GetNATTraversalEngine()
{
    return m_pNATTraversalEngine;
}

qProtectedList<StationURL>& Network::GetLocalURLs()
{
    return m_localURLs;
}

const qProtectedList<StationURL>& Network::GetLocalURLs() const
{
    return m_localURLs;
}

void Network::GetLocalURLs(qList<StationURL>* pURLs) const
{
    const qProtectedList<StationURL>& rURLs = GetLocalURLs();
    ScopedCS lock(&rURLs.GetCriticalSection());
    *pURLs = rURLs.GetList();
}

void Network::SortAndFilterTargetURLs(qList<StationURL>* pURLs)
{
    ScopedCS lock(&m_localURLs.GetCriticalSection());
    g_transportAdapter->SortAndFilterTargetURLs(m_localURLs, pURLs);
}

void Network::AddLocalURL(const StationURL& rURL)
{
    ScopedCS lock(&m_localURLs.GetCriticalSection());
    qList<StationURL>::iterator url = m_localURLs.GetList().begin();
    while (url != m_localURLs.GetList().end())
    {
        bool bMatch = false;
        if (String::IsEqual(url->GetAddress(), rURL.GetAddress()) && url->GetPortNumber() == rURL.GetPortNumber())
            bMatch = true;

        if (bMatch)
            return;

        url++;
    }

    m_localURLs.push_back(rURL);
}

bool Network::RemoveLocalURL(const StationURL& rURL)
{
    ScopedCS lock(&m_localURLs.GetCriticalSection());
    qList<StationURL>::iterator url = m_localURLs.GetList().begin();
    while (url != m_localURLs.GetList().end())
    {
        if (url->IsMatching(rURL))
        {
            m_localURLs.GetList().erase(url);
            return true;
        }

        url++;
    }

    return false;
}

bool Network::ReplaceLocalURL(const StationURL& rURL)
{
    ScopedCS lock(&m_localURLs.GetCriticalSection());
    qList<StationURL>::iterator url = m_localURLs.GetList().begin();
    while (url != m_localURLs.GetList().end())
    {
        if (String::IsEqual(url->GetAddress(), rURL.GetAddress()))
        {
            m_localURLs.GetList().erase(url);
            m_localURLs.push_back(rURL);
            return true;
        }

        url++;
    }

    return false;
}

bool Network::ReplaceLocalURL(const StationURL& rOldURL, const StationURL& rNewURL)
{
    ScopedCS lock(&m_localURLs.GetCriticalSection());
    qList<StationURL>::iterator url = m_localURLs.GetList().begin();
    while (url != m_localURLs.GetList().end())
    {
        bool bMatch = false;
        if (String::IsEqual(url->GetAddress(), rOldURL.GetAddress()) && url->GetPortNumber() == rOldURL.GetPortNumber())
            bMatch = true;

        if (bMatch)
        {
            *url = rNewURL;
            return true;
        }

        url++;
    }

    return false;
}

void Network::InitCompression(ECompAlgo algorithm)
{
    if (m_pCompression != 0)
    {
        delete m_pCompression;
        m_pCompression = 0;
    }

    if (algorithm == COMP_ALGO_ZLIB)
        m_pCompression = new ZLibCompression(s_iZLibWindowBits, s_iZLibMemLevel);

    g_streamSettings[Stream::Unknown3].Get().SetCompressionAlgorithm(m_pCompression);
    g_streamSettings[Stream::RendezVous].Get().SetCompressionAlgorithm(m_pCompression);
}

void Network::SetRC4Encryption()
{
    g_streamSettings[Stream::Unknown3].Get().SetEncryptionAlgorithm(&m_rc4Encryption);
    g_streamSettings[Stream::RendezVous].Get().SetEncryptionAlgorithm(&m_rc4Encryption);
}

void Network::SetP2PDataPacketDefaultSignatureKey(const unsigned char* pKey, unsigned int uiLength)
{
    if (uiLength == 0)
    {
        s_uiP2PDataPacketDefaultSignatureKeyLength = uiLength;
        return;
    }

    if (pKey == 0)
        return;

    if (uiLength < SignatureKeyMinLength || uiLength > SignatureKeyMaxLength)
        return;

    s_uiP2PDataPacketDefaultSignatureKeyLength = uiLength;
    memcpy(s_aP2PDataPacketDefaultSignatureKey, pKey, uiLength);
}

void Network::InitP2PChecksumAlgorithmKey()
{
    KeyedChecksumAlgorithm* pChecksum = m_pMD5Checksum;
    if (s_uiP2PDataPacketDefaultSignatureKeyLength == 0)
        pChecksum->SetKey(Key(s_aDefaultChecksumKey, DefaultChecksumKeyLength));
    else
        pChecksum->SetKey(Key(s_aP2PDataPacketDefaultSignatureKey, s_uiP2PDataPacketDefaultSignatureKeyLength));
}

void Network::InitChecksumAlgorithm()
{
    KeyedChecksumAlgorithm* pHMACChecksum = m_pHMACChecksum;
    pHMACChecksum->SetKey(Key(s_aDefaultChecksumKey, DefaultChecksumKeyLength));

    InitP2PChecksumAlgorithmKey();
    if (s_bHMACChecksumEnabled)
        g_streamSettings[Stream::RendezVousSecure].Get().SetChecksumAlgorithm(m_pHMACChecksum);

    if (!s_bMD5ChecksumEnabled)
        return;

    if (g_streamSettings[Stream::Unknown9].Get().GetChecksumAlgorithm() == 0)
        g_streamSettings[Stream::Unknown9].Get().SetChecksumAlgorithm(m_pMD5Checksum);

    if (g_streamSettings[Stream::Unknown1].Get().GetChecksumAlgorithm() == 0)
        g_streamSettings[Stream::Unknown1].Get().SetChecksumAlgorithm(m_pMD5Checksum);

    if (g_streamSettings[Stream::Routing].Get().GetChecksumAlgorithm() == 0)
        g_streamSettings[Stream::Routing].Get().SetChecksumAlgorithm(m_pMD5Checksum);
}

void Network::SetP2PDataPacketSessionSignatureKey(const qVector<unsigned char>& rKey)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (rKey.size() < SignatureKeyMinLength || rKey.size() > SignatureKeyMaxLength)
        return;

    m_p2pSignatureKey.Clear();
    m_p2pSignatureKey.AppendData(&rKey[0], rKey.size(), APPEND_AT_END);
    Key key(m_p2pSignatureKey.GetData(), m_p2pSignatureKey.GetLength());
    m_pMD5Checksum->SetKey(key);
    if (RootTransport::GetInstance()->GetRelay()->IsEnabled())
    {
        GetInstance()
            ->m_messageSelector.GetRelayMessage()
            ->SetRelayDataPacketSignatureKeyFromP2PDataPacketSessionSignatureKey(rKey);
    }
}

void Network::ClearP2PDataPacketSessionSignatureKey()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_p2pSignatureKey.GetLength() != 0)
    {
        m_p2pSignatureKey.Clear();
        InitP2PChecksumAlgorithmKey();
        RootTransport::GetInstance()->GetRelay()->SetRelayGatheringID(NO_RELAY_GATHERING_ID);
    }
}

void Network::SetP2PChecksumAlgorithm(KeyedChecksumAlgorithm* pAlgorithm)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!s_bMD5ChecksumEnabled)
        return;

    g_streamSettings[Stream::Unknown9].Get().SetChecksumAlgorithm(pAlgorithm);
    g_streamSettings[Stream::Unknown1].Get().SetChecksumAlgorithm(pAlgorithm);
    g_streamSettings[Stream::Routing].Get().SetChecksumAlgorithm(pAlgorithm);
}

}
}
