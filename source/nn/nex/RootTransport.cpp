#include "nn/nex/RootTransport.hpp"

#include <algorithm>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/PacketRecognizer.hpp"
#include "nn/nex/Platform.hpp"
#include "nn/nex/PseudoSingleton.hpp"
#include "nn/nex/Relay.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/ThreadVariable.hpp"
#include "nn/nex/VirtualFilterDevice.hpp"

namespace nn
{
namespace nex
{

namespace
{

const int RESULT_SUCCESS = 0x10001;
const int RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u);

const unsigned int DEFAULT_BUFFER_SIZE = 1364;
const unsigned int NAT_CHECK_SERVER_COUNT = 4;
const unsigned int INVALID_ADDRESS = 0xFFFFFFFFu;
const unsigned int ADDRESS_TEXT_LENGTH = 128;
const unsigned short NAT_CHECK_SERVER_PORT = 10025;
const unsigned short NAT_CHECK_SERVER_REPLY_PORT = 10125;
const unsigned short RELAY_SERVER_PORT = 12525;
const unsigned short DEFAULT_WELL_KNOWN_PORT = 9203;
const unsigned int DYNAMIC_PORT_BASE = 0xC000;
const unsigned int DYNAMIC_PORT_COUNT = 0x3FFF;

const wchar_t* s_pPrimaryNATCheckServer = L"nncs1.app.nintendowifi.net";
const wchar_t* s_pSecondaryNATCheckServer = L"nncs2.app.nintendowifi.net";

}

PseudoGlobalVariable<wchar_t*> RootTransport::s_interfaceAddress(L"");
PseudoGlobalVariable<unsigned short> RootTransport::s_wellKnownPortNumber(DEFAULT_WELL_KNOWN_PORT);

qVector<unsigned int> g_blockedAddresses;

RootTransport::RootTransport()
    : m_uiBufferSize(DEFAULT_BUFFER_SIZE),
      m_reserved0(false),
      m_pVirtualNATDevice(0),
      m_uiPortNumber(0),
      m_bNATCheckEnabled(true),
      m_bUPnPSupported(false),
      m_reserved2(false),
      m_reserved3(false),
      m_bNATCheckServerAddressesSet(false),
      m_bRelayAddressSet(false),
      m_reserved4(false)
{
    m_streamManager.m_pTransport = this;

    m_natCheckServerAddresses.resize(NAT_CHECK_SERVER_COUNT);
    for (unsigned int i = 0; i < NAT_CHECK_SERVER_COUNT; ++i)
        m_natCheckServerAddresses[i] = new StringStream();

    m_pInetAddressSet = new VirtualFilterDevice();
}

RootTransport::~RootTransport()
{
    for (int i = 0; i < static_cast<int>(NAT_CHECK_SERVER_COUNT); ++i)
        delete m_natCheckServerAddresses[i];

    delete m_pInetAddressSet;
}

RootTransport* RootTransport::GetInstance()
{
    Network* pNetwork = Network::GetInstance();
    if (pNetwork == 0)
        return 0;

    return pNetwork->GetTransport();
}

qResult RootTransport::Initialize()
{
    const int code = RESULT_SUCCESS;
    return qResult(code);
}

qResult RootTransport::Send(StationURL* pURL, Buffer* pBuffer)
{
    const int code = RESULT_NOT_IMPLEMENTED;
    return qResult(code);
}

void RootTransport::StopTransportThread()
{
    StopTransportThreadImpl();
}

void RootTransport::SetWellKnownPortNumber(unsigned short uiPort)
{
    s_wellKnownPortNumber.Get() = uiPort;
}

unsigned short RootTransport::GetWellKnownPortNumber()
{
    return s_wellKnownPortNumber.Get();
}

unsigned short RootTransport::GetNextPortNumber()
{
    return static_cast<unsigned short>(DYNAMIC_PORT_BASE + Platform::GetRandomNumber(DYNAMIC_PORT_COUNT));
}

void RootTransport::SetInterfaceAddress(wchar_t* pAddress)
{
    s_interfaceAddress.Get() = pAddress;
}

void RootTransport::SetBlockedAddresses(const qList<InetAddress>& rAddresses)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    for (qList<InetAddress>::const_iterator address = rAddresses.begin(); address != rAddresses.end(); ++address)
    {
        g_blockedAddresses.push_back(address->GetAddress());
    }
}

void RootTransport::AttachVNATDevice(VirtualNATDevice* pDevice)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_pVirtualNATDevice = pDevice;
}

void RootTransport::DetachVNATDevice(VirtualNATDevice* pDevice)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_pVirtualNATDevice = 0;
}

bool RootTransport::RegisterPacketRecognizer(PacketRecognizer* pRecognizer)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (std::find(m_packetRecognizers.begin(), m_packetRecognizers.end(), pRecognizer) != m_packetRecognizers.end())
    {
        return false;
    }

    m_packetRecognizers.push_back(pRecognizer);
    return true;
}

bool RootTransport::UnregisterPacketRecognizer(PacketRecognizer* pRecognizer)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    std::list<PacketRecognizer*, MemAllocator<PacketRecognizer*> >::iterator recognizer = std::find(
        m_packetRecognizers.begin(), m_packetRecognizers.end(), pRecognizer);
    if (recognizer == m_packetRecognizers.end())
        return false;

    m_packetRecognizers.erase(recognizer);
    return true;
}

bool RootTransport::RejectPacket(Buffer* pBuffer, const InetAddress* pAddress)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    for (std::list<PacketRecognizer*, MemAllocator<PacketRecognizer*> >::iterator recognizer =
             m_packetRecognizers.begin();
         recognizer != m_packetRecognizers.end(); ++recognizer)
    {
        StationURL url;
        url.SetInetAddress(pAddress);
        if ((*recognizer)->Recognize(pBuffer, &url))
            return false;
    }

    return true;
}

bool RootTransport::SetPrimaryNATCheckServer(const wchar_t* pAddress, unsigned short uiFirstPort,
                                             unsigned short uiSecondPort)
{
    if (uiFirstPort == 0 || uiSecondPort == 0)
        return false;

    InetAddress address(pAddress, 0);
    if (address.GetAddress() == INVALID_ADDRESS)
        return false;

    wchar_t text[ADDRESS_TEXT_LENGTH];
    text[0] = 0;
    address.GetAddress(text, sizeof(text));
    m_natCheckServerAddresses[0]->Clear();
    (*m_natCheckServerAddresses[0] << L"prudp:/address=" << text << L";port=")
        .StreamNumber(static_cast<int>(uiFirstPort));

    address.SetPortNumber(uiSecondPort);
    text[0] = 0;
    address.GetAddress(text, sizeof(text));
    m_natCheckServerAddresses[1]->Clear();
    (*m_natCheckServerAddresses[1] << L"prudp:/address=" << text << L";port=")
        .StreamNumber(static_cast<int>(uiSecondPort));
    return true;
}

bool RootTransport::SetSecondaryNATCheckServer(const wchar_t* pAddress, unsigned short uiFirstPort,
                                               unsigned short uiSecondPort)
{
    if (uiFirstPort == 0 || uiSecondPort == 0)
        return false;

    InetAddress address(pAddress, 0);
    if (address.GetAddress() == INVALID_ADDRESS)
        return false;

    wchar_t text[ADDRESS_TEXT_LENGTH];
    text[0] = 0;
    address.GetAddress(text, sizeof(text));
    m_natCheckServerAddresses[2]->Clear();
    (*m_natCheckServerAddresses[2] << L"prudp:/address=" << text << L";port=")
        .StreamNumber(static_cast<int>(uiFirstPort));

    address.SetPortNumber(uiSecondPort);
    text[0] = 0;
    address.GetAddress(text, sizeof(text));
    m_natCheckServerAddresses[3]->Clear();
    (*m_natCheckServerAddresses[3] << L"prudp:/address=" << text << L";port=")
        .StreamNumber(static_cast<int>(uiSecondPort));
    return true;
}

const qVector<StringStream*>& RootTransport::GetNATCheckServerAddresses() const
{
    return m_natCheckServerAddresses;
}

void RootTransport::SetIsNATCheckServerAddressesSet()
{
    StringStream** addresses = &m_natCheckServerAddresses[0];
    if (addresses[0]->GetLength() == 0 || addresses[1]->GetLength() == 0 || addresses[2]->GetLength() == 0
        || addresses[3]->GetLength() == 0)
    {
        m_bNATCheckServerAddressesSet = false;
    }
    else
    {
        m_bNATCheckServerAddressesSet = true;
    }
}

bool RootTransport::SetUpDefaultNATCheckServerInternal()
{
    if (m_bNATCheckServerAddressesSet)
        return true;

    if (!SetPrimaryNATCheckServer(s_pPrimaryNATCheckServer, NAT_CHECK_SERVER_PORT, NAT_CHECK_SERVER_REPLY_PORT))
        return false;

    if (!SetSecondaryNATCheckServer(s_pSecondaryNATCheckServer, NAT_CHECK_SERVER_PORT, NAT_CHECK_SERVER_REPLY_PORT))
        return false;

    SetIsNATCheckServerAddressesSet();
    return true;
}

NATProperties RootTransport::GetNATProperties() const
{
    return m_natProperties;
}

void RootTransport::SetNATProperties(const NATProperties& rProperties)
{
    m_natProperties = rProperties;
}

void RootTransport::EnableUPnPSupport(bool bEnable)
{
    m_bUPnPSupported = bEnable;
}

bool RootTransport::GetRelayServerAddress(const wchar_t* pAddress)
{
    m_relayAddress.Init();
    m_relayAddress.SetPortNumber(RELAY_SERVER_PORT);
    return m_relayAddress.SetAddress(pAddress);
}

bool RootTransport::SetUpRelayServer()
{
    if (!GetRelay()->IsEnabled() || m_bRelayAddressSet)
        return true;

    if (m_relayName.GetLength() <= 0 || GetRelayServerAddress(m_relayName))
    {
        m_bRelayAddressSet = m_relayAddress.GetAddress() != 0;
        return true;
    }

    return false;
}

void RootTransport::SetRelayNameAddress(String& rName)
{
    m_relayName = rName;
}

bool RootTransport::ExistsRelayTable()
{
    RootTransport* pTransport = GetInstance();
    bool exists = false;
    if (pTransport != 0 && pTransport->GetRelay()->IsEnabled() && pTransport->GetRelay()->Exists())
        exists = true;

    return exists;
}

}
}
