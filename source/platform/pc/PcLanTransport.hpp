#ifndef PLATFORM_PC_PCLANTRANSPORT_HPP
#define PLATFORM_PC_PCLANTRANSPORT_HPP

#include "platform/pc/PcVerifiedIdentities.hpp"
#include "platform/pc/PcOs.hpp"

#include "game/core/CriticalSection.hpp"

#include <deque>
#include <vector>

class C4JThreadImpl;
class C4JEventImpl;

const unsigned short kPcLanGamePort = 25565;
const unsigned short kPcLanDiscoveryPort = 25566;
const unsigned int kPcLanHelloMagic = 0x4350434D;
const unsigned short kPcLanProtocolVersion = 0x886;

class PcLanTransport
{
public:
    enum EventType
    {
        eEvent_Connected,
        eEvent_Disconnected,
        eEvent_ConnectFailed
    };

    struct Event
    {
        EventType type;
        unsigned long long stationId;
    };

    PcLanTransport();
    ~PcLanTransport();

    bool Host(unsigned long long localStationId, unsigned short port);
    void Connect(unsigned long long localStationId, unsigned int address, unsigned short port);
    void Adopt(unsigned long long localStationId, PcOsSocket socket, unsigned long long remoteStationId,
               unsigned int address, unsigned short port, const std::vector<unsigned char>& hostHelloData);
    void ConnectRelay(unsigned long long localStationId, unsigned int address, unsigned short port,
                      unsigned int worldId, unsigned long long secret);
    bool AcceptRelayed(PcOsSocket socket, unsigned int address, const PcVerifiedIdentities* verified);
    bool GetVerifiedIdentities(unsigned long long stationId, PcVerifiedIdentities* verified);
    bool GetPeerHelloData(unsigned long long stationId, std::vector<unsigned char>* data);
    void Disconnect(unsigned long long stationId);
    bool GetHelloData(std::vector<unsigned char>* data);
    bool IsHosting() const { return m_hosting && !m_closing; }
    void SetHelloData(const void* data, unsigned int size);
    void SetWakeEvent(C4JEventImpl* event);
    void Close();

    bool Send(unsigned long long stationId, const void* data, unsigned int size);
    bool Receive(unsigned long long* stationId, std::vector<unsigned char>* message);
    bool PopEvent(Event* event);
    bool IsConnected(unsigned long long stationId);
    void ReapClosedConnections();

private:
    struct Connection
    {
        PcLanTransport* owner;
        PcOsSocket socket;
        unsigned long long stationId;
        unsigned int address;
        unsigned short port;
        bool outgoing;
        bool handshaken;
        unsigned int relayWorld;
        unsigned long long relaySecret;
        bool verified;
        PcVerifiedIdentities verifiedIdentities;
        volatile bool connected;
        volatile bool finished;
        volatile bool aborted;
        OSFastMutex sendLock;
        C4JThreadImpl* thread;
        unsigned long long slowSendLogged;
        std::vector<unsigned char> peerHelloData;
    };

    struct Message
    {
        unsigned long long stationId;
        std::vector<unsigned char> data;
    };

    static int AcceptThreadProc(void* argument);
    static int ConnectionThreadProc(void* argument);
    void RunConnection(Connection* connection);
    bool SendFrame(Connection* connection, const void* data, unsigned int size);
    bool ReceiveFrame(Connection* connection, std::vector<unsigned char>* frame);
    bool ReceiveFromConnection(Connection* connection, unsigned char* buffer, unsigned int size);
    bool StartRelay(Connection* connection);
    Connection* StartConnection(PcOsSocket socket, unsigned int address, unsigned short port, bool outgoing,
                                unsigned long long stationId, unsigned int relayWorld = 0,
                                unsigned long long relaySecret = 0, const PcVerifiedIdentities* verified = 0);
    void PushEvent(EventType type, unsigned long long stationId);

    unsigned long long m_localStationId;
    PcOsSocket m_listener;
    bool m_hosting;
    C4JThreadImpl* m_acceptThread;
    volatile bool m_closing;
    std::vector<Connection*> m_connections;
    std::deque<Message> m_messages;
    std::deque<Event> m_events;
    std::vector<unsigned char> m_helloData;
    OSFastMutex m_connectionLock;
    OSFastMutex m_queueLock;
    C4JEventImpl* m_wakeEvent;
};

class PcLanDiscovery
{
public:
    struct Session
    {
        unsigned int sessionId;
        unsigned int address;
        unsigned short port;
        std::vector<unsigned char> data;
        unsigned long long lastSeen;
        bool manual;
    };

    enum ProbeStatus
    {
        eProbeStatus_None,
        eProbeStatus_Running,
        eProbeStatus_Found,
        eProbeStatus_Failed,
        eProbeStatus_Lobby
    };

    PcLanDiscovery();
    ~PcLanDiscovery();

    void StartAdvertising(unsigned int sessionId, unsigned short gamePort);
    void SetAdvertisedData(const void* data, unsigned int size);
    void StopAdvertising();
    void StartBrowsing();
    void StopBrowsing();
    void AddManualSession(unsigned int address, unsigned short port, const void* data, unsigned int size);
    void SetManualSession(unsigned int address, unsigned short port, const void* data, unsigned int size);
    static unsigned int ManualSessionId(unsigned int address, unsigned short port);
    void ProbeServerList(const char* path, unsigned long long localStationId);
    void ProbeTextServerList(const char* path, unsigned long long localStationId);
    void ProbeAddress(unsigned int address, unsigned short port, unsigned long long localStationId);
    bool TakeProbedConnection(unsigned int address, unsigned short port, PcOsSocket* socket,
                              unsigned long long* stationId, std::vector<unsigned char>* hostHelloData);
    static ProbeStatus GetProbeStatus(unsigned int address, unsigned short port);
    static void DropClosedProbes();
    static void SetLanDisabled(bool disabled);
    std::vector<Session> GetSessions();
    unsigned int FindSessionId(unsigned int address, unsigned short port);
    unsigned int GetRevision();

private:
    static int AdvertiseThreadProc(void* argument);
    static int BrowseThreadProc(void* argument);
    void RunAdvertise();
    void RunBrowse();
    void ExpireSessions();
    void RemoveManualSession(unsigned int address, unsigned short port);

    unsigned int m_sessionId;
    unsigned short m_gamePort;
    std::vector<unsigned char> m_advertisedData;
    volatile bool m_advertising;
    volatile bool m_browsing;
    C4JThreadImpl* m_advertiseThread;
    C4JThreadImpl* m_browseThread;
    std::vector<Session> m_sessions;
    unsigned int m_revision;
    OSFastMutex m_lock;
};

#endif
