#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ROOTTRANSPORT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ROOTTRANSPORT_HPP

#include <vector>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/InputEmulationDevice.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/NATProperties.hpp"
#include "nn/nex/OutputEmulationDevice.hpp"
#include "nn/nex/PseudoGlobalVariable.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/Stream.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/StringStream.hpp"
#include "nn/nex/TransportStreamManager.hpp"
#include "nn/nex/UPnPProperties.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qSet.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class PacketOut;
class PacketRecognizer;
class Relay;
class Router;
class StationURL;
class VirtualFilterDevice;
class VirtualNATDevice;

extern qVector<unsigned int> g_blockedAddresses;

class RootTransport : public RootObject
{
public:
    RootTransport();
    virtual ~RootTransport();

    virtual qResult Initialize();
    virtual bool StartListen(unsigned short uiPort, unsigned short* pPortOut, bool bFlag, unsigned int uiFirst,
                             bool bSecond) = 0;
    virtual bool StartListenReuse(unsigned short uiPort, unsigned short* pPortOut, bool bFlag, unsigned int uiFirst,
                                  bool bSecond) = 0;
    virtual bool StopListen(unsigned short uiPort) = 0;
    virtual bool StopListen() = 0;
    virtual unsigned int GetNbListeningPorts() = 0;
    virtual bool Send(unsigned short uiPort, Stream::Type type, unsigned char uiSource, unsigned char uiDestination,
                      PacketOut* pPacket, bool bFlag) = 0;
    virtual qResult Send(StationURL* pURL, Buffer* pBuffer);
    virtual bool Receive(unsigned short uiPort, Buffer* pBuffer, const InetAddress* pAddress) = 0;
    virtual Router* GetRouter() { return 0; }

    virtual Relay* GetRelay() { return 0; }

    virtual unsigned int GetPacketQueueSize() const = 0;
    virtual unsigned int GetEstimatedPacketQueueMemoryUsage() const = 0;
    virtual void ClearPacketInPacketBufferPool() {}

    virtual void StopTransportThreadImpl() = 0;

    static RootTransport* GetInstance();
    static bool ExistsRelayTable();

    static void SetWellKnownPortNumber(unsigned short uiPort);
    static unsigned short GetWellKnownPortNumber();
    static unsigned short GetNextPortNumber();
    static void SetInterfaceAddress(wchar_t* pAddress);
    static void SetBlockedAddresses(const qList<InetAddress>& rAddresses);

    void StopTransportThread();

    OutputEmulationDevice* GetOutputEmulationDevice() { return &m_outputEmulation; }

    InputEmulationDevice* GetInputEmulationDevice() { return &m_inputEmulation; }

    TransportStreamManager* GetStreamManager() { return &m_streamManager; }

    void AttachVNATDevice(VirtualNATDevice* pDevice);
    void DetachVNATDevice(VirtualNATDevice* pDevice);
    bool RegisterPacketRecognizer(PacketRecognizer* pRecognizer);
    bool UnregisterPacketRecognizer(PacketRecognizer* pRecognizer);
    bool RejectPacket(Buffer* pBuffer, const InetAddress* pAddress);

    bool SetPrimaryNATCheckServer(const wchar_t* pAddress, unsigned short uiFirstPort, unsigned short uiSecondPort);
    bool SetSecondaryNATCheckServer(const wchar_t* pAddress, unsigned short uiFirstPort, unsigned short uiSecondPort);
    const qVector<StringStream*>& GetNATCheckServerAddresses() const;
    void SetIsNATCheckServerAddressesSet();
    bool SetUpDefaultNATCheckServerInternal();

    unsigned int GetBufferSize() const { return m_uiBufferSize; }

    unsigned short GetPortNumber() const { return m_uiPortNumber; }

    bool IsStopped() const { return m_reserved4; }

    NATProperties GetNATProperties() const;
    void SetNATProperties(const NATProperties& rProperties);

    void EnableUPnPSupport(bool bEnable);

    bool GetRelayServerAddress(const wchar_t* pAddress);
    bool SetUpRelayServer();
    void SetRelayNameAddress(String& rName);

private:
    friend class BackEndServices;
    friend class ConnectionOrientedStream;
    friend class JobCheckCurrentPort;
    friend class JobCheckNATProperties;
    friend class JobNATCheckBase;
    friend class JobPerformNATCheck;
    friend class JobResolveRelayInfo;
    friend class JobStartNATSession;
    friend class Relay;
    friend class Router;
    friend class SocketTransport;
    friend class TransportStreamManager;

    CriticalSection m_criticalSection;
    CriticalSection m_recognizerCriticalSection;
    unsigned int m_uiBufferSize;
    bool m_reserved0;
    OutputEmulationDevice m_outputEmulation;
    InputEmulationDevice m_inputEmulation;
    TransportStreamManager m_streamManager;
    VirtualNATDevice* m_pVirtualNATDevice;
    unsigned short m_uiPortNumber;
    std::list<PacketRecognizer*, MemAllocator<PacketRecognizer*> > m_packetRecognizers;
    NATProperties m_natProperties;
    UPnPProperties m_upnpProperties;
    bool m_bNATCheckEnabled;
    bool m_bUPnPSupported;
    bool m_reserved2;
    bool m_reserved3;
    bool m_bNATCheckServerAddressesSet;
    bool m_bRelayAddressSet;
    bool m_reserved4;
    VirtualFilterDevice* m_pInetAddressSet;
    qVector<StringStream*> m_natCheckServerAddresses;
    InetAddress m_relayAddress;
    String m_relayName;

    static PseudoGlobalVariable<wchar_t*> s_interfaceAddress;
    static PseudoGlobalVariable<unsigned short> s_wellKnownPortNumber;
};

}
}

#endif
