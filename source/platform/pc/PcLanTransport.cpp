#include "platform/pc/PcLanTransport.hpp"

#include "platform/pc/PcAccount.hpp"
#include "platform/pc/PcContent.hpp"
#include "platform/pc/PcEd25519.hpp"
#include "platform/pc/PcKnownIdentities.hpp"
#include "platform/pc/PcLobbyClient.hpp"
#include "platform/pc/PcLobbyProtocol.hpp"

#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

#include <algorithm>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>

namespace
{
const unsigned int kHelloMagic = kPcLanHelloMagic;
const unsigned int ADVERT_MAGIC = 0x4C50434D;
const unsigned short kProtocolVersion = kPcLanProtocolVersion;
const unsigned int RELAY_WAIT_MS = 15000;
const unsigned int kHelloSize = 16;
const unsigned int ADVERT_HEADER_SIZE = 16;
const unsigned int kMaxFrameSize = 4 * 1024 * 1024;
const unsigned int MAX_ADVERT_DATA = 1024;
const unsigned int ADVERT_INTERVAL_MS = 1000;
const unsigned int BROWSE_POLL_MS = 250;
const unsigned long long SESSION_EXPIRY_NS = 4000ULL * 1000000ULL;
const size_t MAX_BROADCASTS = 16;
const unsigned long long kProbeRetryNanoseconds = 5000ULL * 1000000ULL;
const unsigned int PROBE_JOIN_WAIT_MS = 5000;
const unsigned int PROBE_JOIN_POLL_MS = 10;
const unsigned int RECEIVE_POLL_MS = 250;
const unsigned int kSendTimeoutMilliseconds = 5000;
const unsigned long long SLOW_SEND_NS = 250ULL * 1000000ULL;
const unsigned long long SLOW_SEND_LOG_NS = 5000ULL * 1000000ULL;
const unsigned int HELLO_WAIT_POLL_MS = 50;
const unsigned char JOIN_CHALLENGE = 'C';
const unsigned char JOIN_IDENTITY = 'J';
const unsigned int CHALLENGE_SIZE = 16;
const unsigned int IDENTITY_ENTRY = 16 + 32 + 64;
const int MAX_JOIN_IDENTITIES = 1 + kPcGuestIdentities;
const char JOIN_CONTEXT[] = "LCE join identity v1";

enum ProbeState
{
    eProbe_Running,
    eProbe_Parked,
    eProbe_Failed,
    eProbe_Taken,
    eProbe_Lobby
};

struct Probe
{
    PcLanDiscovery* discovery;
    unsigned int address;
    unsigned short port;
    unsigned long long localStationId;
    PcOsSocket socket;
    unsigned long long stationId;
    std::vector<unsigned char> sessionData;
    volatile int state;
    unsigned long long finishedAt;
    C4JThreadImpl* thread;
};

std::vector<Probe*> s_probes;
bool lanDisabled = false;

struct ServerEntry
{
    std::string host;
    unsigned short port;
};

struct ResolvedHost
{
    ResolvedHost() : address(0), resolvedAt(0) {}
    unsigned int address;
    unsigned long long resolvedAt;
};

const unsigned long long RESOLVE_RETRY_NS = 60000ULL * 1000000ULL;
std::map<std::string, ResolvedHost> resolvedHosts;

bool ReadServerList(const char* path, std::vector<ServerEntry>* entries)
{
    FILE* fp = fopen(path, "rb");
    if (fp == 0)
        return false;
    std::vector<unsigned char> content;
    unsigned char buf[4096];
    size_t bytes;
    while ((bytes = fread(buf, 1, sizeof(buf), fp)) > 0)
        content.insert(content.end(), buf, buf + bytes);
    fclose(fp);
    if (content.size() < 12 || content[0] != 'M' || content[1] != 'C' || content[2] != 'S' || content[3] != 'V')
        return false;
    unsigned int count = content[8] | (content[9] << 8) | (content[10] << 16)
        | (static_cast<unsigned int>(content[11]) << 24);
    size_t pos = 12;
    for (unsigned int i = 0; i < count; ++i)
    {
        if (pos + 2 > content.size())
            break;
        size_t hostLength = content[pos] | (content[pos + 1] << 8);
        pos += 2;
        if (pos + hostLength + 4 > content.size())
            break;
        ServerEntry entry;
        entry.host.assign(reinterpret_cast<const char*>(&content[pos]), hostLength);
        pos += hostLength;
        entry.port = static_cast<unsigned short>(content[pos] | (content[pos + 1] << 8));
        pos += 2;
        size_t nameLength = content[pos] | (content[pos + 1] << 8);
        pos += 2 + nameLength;
        if (pos > content.size())
            break;
        entries->push_back(entry);
    }
    return true;
}

void WriteU16(unsigned char* out, unsigned int v)
{
    out[0] = (unsigned char)v;
    out[1] = (unsigned char)(v >> 8);
}

void WriteU32(unsigned char* out, unsigned int v)
{
    WriteU16(out, v & 0xFFFF);
    WriteU16(out + 2, v >> 16);
}

void WriteU64(unsigned char* out, unsigned long long v)
{
    WriteU32(out, (unsigned int)v);
    WriteU32(out + 4, (unsigned int)(v >> 32));
}

unsigned int ReadU16(const unsigned char* in)
{
    return (unsigned int)in[0] | ((unsigned int)in[1] << 8);
}

unsigned int ReadU32(const unsigned char* in)
{
    return ReadU16(in) | (ReadU16(in + 2) << 16);
}

unsigned long long ReadU64(const unsigned char* in)
{
    return (unsigned long long)ReadU32(in) | ((unsigned long long)ReadU32(in + 4) << 32);
}

bool ReceiveExactly(PcOsSocket sock, unsigned char* buf, unsigned int len)
{
    while (len > 0)
    {
        int got = PcOsSocketReceive(sock, buf, len);
        if (got <= 0)
            return false;
        buf += got;
        len -= (unsigned int)got;
    }
    return true;
}
}

PcLanTransport::PcLanTransport()
    : m_localStationId(0), m_listener(kPcOsInvalidSocket), m_hosting(false), m_acceptThread(0), m_closing(false),
      m_wakeEvent(0)
{
    InitializeCriticalSection(&m_connectionLock);
    InitializeCriticalSection(&m_queueLock);
}

PcLanTransport::~PcLanTransport()
{
    Close();
}

void PcLanTransport::SetWakeEvent(C4JEventImpl* ev)
{
    m_wakeEvent = ev;
}

bool PcLanTransport::GetHelloData(std::vector<unsigned char>* data)
{
    EnterCriticalSection(&m_queueLock);
    *data = m_helloData;
    LeaveCriticalSection(&m_queueLock);
    return !data->empty();
}

void PcLanTransport::ConnectRelay(unsigned long long localId, unsigned int addr, unsigned short port,
                                  unsigned int world, unsigned long long secret)
{
    Close();
    m_localStationId = localId;
    m_closing = false;
    if (!PcOsNetInitialise())
    {
        PushEvent(eEvent_ConnectFailed, 0);
        return;
    }
    StartConnection(kPcOsInvalidSocket, addr, port, true, 0, world, secret);
}

bool PcLanTransport::AcceptRelayed(PcOsSocket socket, unsigned int address, const PcVerifiedIdentities* verified)
{
    EnterCriticalSection(&m_connectionLock);
    const bool hosting = m_hosting && !m_closing;
    LeaveCriticalSection(&m_connectionLock);
    if (!hosting)
        return false;
    StartConnection(socket, address, 0, false, 0, 0, 0, verified);
    return true;
}

bool PcLanTransport::GetVerifiedIdentities(unsigned long long stationId, PcVerifiedIdentities* verified)
{
    bool found = false;
    EnterCriticalSection(&m_connectionLock);
    for (unsigned int index = 0; index < m_connections.size() && !found; ++index)
    {
        Connection* connection = m_connections[index];
        if (connection->verified && connection->stationId == stationId && connection->connected)
        {
            *verified = connection->verifiedIdentities;
            found = true;
        }
    }
    LeaveCriticalSection(&m_connectionLock);
    return found;
}

bool PcLanTransport::GetPeerHelloData(unsigned long long station, std::vector<unsigned char>* data)
{
    bool found = false;
    data->clear();
    EnterCriticalSection(&m_connectionLock);
    for (unsigned int i = 0; i < m_connections.size() && !found; ++i)
    {
        if (m_connections[i]->stationId == station && !m_connections[i]->finished)
        {
            *data = m_connections[i]->peerHelloData;
            found = true;
        }
    }
    LeaveCriticalSection(&m_connectionLock);
    return found;
}

void PcLanTransport::Disconnect(unsigned long long station)
{
    EnterCriticalSection(&m_connectionLock);
    for (unsigned int i = 0; i < m_connections.size(); ++i)
    {
        Connection* conn = m_connections[i];
        if (conn->stationId == station && !conn->finished)
        {
            conn->aborted = true;
            if (conn->socket != kPcOsInvalidSocket)
                PcOsSocketShutdown(conn->socket);
        }
    }
    LeaveCriticalSection(&m_connectionLock);
}

void PcLanTransport::SetHelloData(const void* data, unsigned int len)
{
    EnterCriticalSection(&m_queueLock);
    m_helloData.assign(static_cast<const unsigned char*>(data), static_cast<const unsigned char*>(data) + len);
    LeaveCriticalSection(&m_queueLock);
}

void PcLanTransport::Adopt(unsigned long long localId, PcOsSocket sock, unsigned long long remoteId,
                           unsigned int addr, unsigned short port, const std::vector<unsigned char>& hello)
{
    Close();
    m_localStationId = localId;
    m_closing = false;
    Connection* conn = StartConnection(sock, addr, port, true, remoteId);
    EnterCriticalSection(&m_connectionLock);
    if (conn != 0)
        conn->peerHelloData = hello;
    LeaveCriticalSection(&m_connectionLock);
}

bool PcLanTransport::Host(unsigned long long localId, unsigned short port)
{
    Close();
    if (!PcOsNetInitialise())
        return false;
    m_localStationId = localId;
    m_closing = false;
    EnterCriticalSection(&m_queueLock);
    m_helloData.clear();
    LeaveCriticalSection(&m_queueLock);
    m_hosting = true;
    if (port == 0)
        return true;
    m_listener = PcOsTcpListen(port);
    if (m_listener == kPcOsInvalidSocket)
        return false;
    m_acceptThread = new C4JThreadImpl(AcceptThreadProc, this, "LanAccept", 0);
    m_acceptThread->Run();
    return true;
}

void PcLanTransport::Connect(unsigned long long localId, unsigned int addr, unsigned short port)
{
    Close();
    m_localStationId = localId;
    m_closing = false;
    if (!PcOsNetInitialise())
    {
        PushEvent(eEvent_ConnectFailed, 0);
        return;
    }
    StartConnection(kPcOsInvalidSocket, addr, port, true, 0);
}

void PcLanTransport::Close()
{
    m_closing = true;
    m_hosting = false;
    if (m_listener != kPcOsInvalidSocket)
    {
        PcOsSocketShutdown(m_listener);
        PcOsSocketClose(m_listener);
        m_listener = kPcOsInvalidSocket;
    }
    if (m_acceptThread != 0)
    {
        m_acceptThread->WaitForCompletion(-1);
        delete m_acceptThread;
        m_acceptThread = 0;
    }

    EnterCriticalSection(&m_connectionLock);
    std::vector<Connection*> connections;
    connections.swap(m_connections);
    for (unsigned int i = 0; i < connections.size(); ++i)
    {
        connections[i]->aborted = true;
        if (connections[i]->socket != kPcOsInvalidSocket)
            PcOsSocketShutdown(connections[i]->socket);
    }
    LeaveCriticalSection(&m_connectionLock);

    for (unsigned int i = 0; i < connections.size(); ++i)
    {
        Connection* conn = connections[i];
        conn->thread->WaitForCompletion(-1);
        delete conn->thread;
        if (conn->socket != kPcOsInvalidSocket)
            PcOsSocketClose(conn->socket);
        DeleteCriticalSection(&conn->sendLock);
        delete conn;
    }

    EnterCriticalSection(&m_queueLock);
    m_messages.clear();
    m_events.clear();
    LeaveCriticalSection(&m_queueLock);
}

PcLanTransport::Connection* PcLanTransport::StartConnection(PcOsSocket socket, unsigned int address,
                                                            unsigned short port, bool outgoing,
                                                            unsigned long long stationId, unsigned int relayWorld,
                                                            unsigned long long relaySecret,
                                                            const PcVerifiedIdentities* verified)
{
    Connection* connection = new Connection;
    connection->relayWorld = relayWorld;
    connection->relaySecret = relaySecret;
    connection->verified = verified != 0 && verified->count > 0;
    connection->verifiedIdentities = connection->verified ? *verified : PcVerifiedIdentities();
    connection->owner = this;
    connection->socket = socket;
    connection->stationId = stationId;
    connection->handshaken = stationId != 0;
    connection->address = address;
    connection->port = port;
    connection->outgoing = outgoing;
    connection->slowSendLogged = 0;
    connection->connected = false;
    connection->finished = false;
    connection->aborted = false;
    if (socket != kPcOsInvalidSocket)
        PcOsSocketSetSendTimeout(socket, kSendTimeoutMilliseconds);
    InitializeCriticalSection(&connection->sendLock);
    connection->thread = new C4JThreadImpl(ConnectionThreadProc, connection, "LanConnection", 0);
    EnterCriticalSection(&m_connectionLock);
    m_connections.push_back(connection);
    LeaveCriticalSection(&m_connectionLock);
    connection->thread->Run();
    return connection;
}

bool PcLanTransport::StartRelay(Connection* conn)
{
    unsigned char req[kRelayFrameSize];
    WriteU32(req, kRelayMagic);
    WriteU16(req + 4, kProtocolVersion);
    req[6] = kRelayJoin;
    req[7] = 0;
    WriteU32(req + 8, conn->relayWorld);
    WriteU64(req + 12, conn->relaySecret);
    if (!SendFrame(conn, req, sizeof(req)))
        return false;
    for (unsigned int ms = 0; ms < RELAY_WAIT_MS; ms += RECEIVE_POLL_MS)
    {
        if (m_closing || conn->aborted)
            return false;
        int ready = PcOsSocketWaitReadable(conn->socket, RECEIVE_POLL_MS);
        if (ready < 0)
            return false;
        if (ready == 0)
            continue;
        std::vector<unsigned char> reply;
        return ReceiveFrame(conn, &reply) && reply.size() >= kRelayFrameSize && ReadU32(&reply[0]) == kRelayMagic
               && reply[6] == kRelayReady;
    }
    return false;
}

int PcLanTransport::AcceptThreadProc(void* arg)
{
    PcLanTransport* self = static_cast<PcLanTransport*>(arg);
    while (!self->m_closing)
    {
        unsigned int addr = 0;
        PcOsSocket sock = PcOsTcpAccept(self->m_listener, &addr);
        if (sock == kPcOsInvalidSocket)
            break;
        if (self->m_closing)
        {
            PcOsSocketClose(sock);
            break;
        }
        self->StartConnection(sock, addr, 0, false, 0);
    }
    return 0;
}

int PcLanTransport::ConnectionThreadProc(void* arg)
{
    Connection* conn = static_cast<Connection*>(arg);
    conn->owner->RunConnection(conn);
    return 0;
}

static std::vector<unsigned char> joinMessage(const unsigned char* challenge, unsigned long long hostStation,
                                              const unsigned char* uuid)
{
    std::vector<unsigned char> msg(JOIN_CONTEXT, JOIN_CONTEXT + sizeof(JOIN_CONTEXT) - 1);
    msg.insert(msg.end(), challenge, challenge + CHALLENGE_SIZE);
    for (int i = 0; i < 8; i++)
        msg.push_back((unsigned char)(hostStation >> (8 * i)));
    msg.insert(msg.end(), uuid, uuid + 16);
    return msg;
}

static void addJoinEntry(std::vector<unsigned char>* frame, const unsigned char* uuid, const unsigned char* key,
                         const unsigned char* sig)
{
    frame->insert(frame->end(), uuid, uuid + 16);
    frame->insert(frame->end(), key, key + 32);
    frame->insert(frame->end(), sig, sig + 64);
}

static std::string uuidText(const unsigned char* uuid)
{
    char buf[40];
    for (int i = 0; i < 16; i++)
        sprintf(buf + 2 * i, "%02x", uuid[i]);
    return buf;
}

bool PcLanTransport::SendJoinIdentity(Connection* conn, const unsigned char* challenge)
{
    std::vector<unsigned char> frame;
    frame.push_back(JOIN_IDENTITY);
    frame.push_back((unsigned char)MAX_JOIN_IDENTITIES);
    unsigned char uuid[16], key[32], sig[64];
    PcAccountUuid(uuid);
    std::vector<unsigned char> msg = joinMessage(challenge, conn->stationId, uuid);
    if (!PcAccountSign(&msg[0], msg.size(), key, sig))
    {
        memset(key, 0, sizeof(key));
        memset(sig, 0, sizeof(sig));
    }
    addJoinEntry(&frame, uuid, key, sig);
    for (int guest = 1; guest <= kPcGuestIdentities; guest++)
    {
        PcGuestAccountUuid(guest, uuid);
        msg = joinMessage(challenge, conn->stationId, uuid);
        if (!PcGuestAccountSign(guest, &msg[0], msg.size(), key, sig))
        {
            memset(key, 0, sizeof(key));
            memset(sig, 0, sizeof(sig));
        }
        addJoinEntry(&frame, uuid, key, sig);
    }
    return SendFrame(conn, &frame[0], (unsigned int)frame.size());
}

bool PcLanTransport::CheckJoinIdentity(Connection* conn)
{
    unsigned char challenge[1 + CHALLENGE_SIZE];
    challenge[0] = JOIN_CHALLENGE;
    if (!PcOsRandomBytes(challenge + 1, CHALLENGE_SIZE))
        return false;
    std::vector<unsigned char> frame;
    if (!SendFrame(conn, challenge, sizeof(challenge)) || !ReceiveFrame(conn, &frame))
        return false;
    if (frame.size() < 2 || frame[0] != JOIN_IDENTITY || frame[1] < 1 || frame[1] > MAX_JOIN_IDENTITIES
        || frame.size() != 2 + frame[1] * IDENTITY_ENTRY)
    {
        printf("[pc] lan: station %016llx sent no identity; refused\n", conn->stationId);
        fflush(stdout);
        return false;
    }
    PcVerifiedIdentities verified;
    for (int i = 0; i < frame[1]; i++)
    {
        const unsigned char* entry = &frame[2 + i * IDENTITY_ENTRY];
        const unsigned char* uuid = entry;
        const unsigned char* key = entry + 16;
        const unsigned char* sig = entry + 48;
        std::vector<unsigned char> msg = joinMessage(challenge + 1, m_localStationId, uuid);
        bool good = PcEd25519Verify(key, &msg[0], msg.size(), sig);
        bool known = good && PcKnownIdentityCheck(uuid, key);
        if (i == 0 && !known)
        {
            printf("[pc] lan: station %016llx could not prove identity %s (%s); refused\n", conn->stationId,
                   uuidText(uuid).c_str(), good ? "another key than it first joined with" : "bad signature");
            fflush(stdout);
            return false;
        }
        if (known && !verified.Contains(uuid))
            memcpy(verified.uuids[verified.count++], uuid, 16);
    }
    EnterCriticalSection(&m_connectionLock);
    conn->verified = true;
    conn->verifiedIdentities = verified;
    LeaveCriticalSection(&m_connectionLock);
    return true;
}

void PcLanTransport::RunConnection(Connection* conn)
{
    std::vector<unsigned char> frame;
    if (!conn->handshaken && conn->outgoing)
    {
        PcOsSocket sock = PcOsTcpConnect(conn->address, conn->port);
        if (sock != kPcOsInvalidSocket)
            PcOsSocketSetSendTimeout(sock, kSendTimeoutMilliseconds);
        EnterCriticalSection(&m_connectionLock);
        conn->socket = sock;
        LeaveCriticalSection(&m_connectionLock);
        if (sock == kPcOsInvalidSocket || m_closing)
        {
            PushEvent(eEvent_ConnectFailed, 0);
            conn->finished = true;
            return;
        }
        if (conn->relayWorld != 0 && !StartRelay(conn))
        {
            printf("[pc] lan: the lobby could not reach that world's host\n");
            fflush(stdout);
            PushEvent(eEvent_ConnectFailed, 0);
            conn->finished = true;
            return;
        }
    }

    if (!conn->handshaken)
    {
        std::vector<unsigned char> hello(kHelloSize);
        WriteU32(&hello[0], kHelloMagic);
        WriteU16(&hello[4], kProtocolVersion);
        WriteU16(&hello[6], 0);
        WriteU64(&hello[8], m_localStationId);
        if (conn->outgoing)
        {
            PcContentAppendSummary(&hello);
            PcContentAppendDetail(&hello);
        }
        else
        {
            for (;;)
            {
                EnterCriticalSection(&m_queueLock);
                const bool ready = !m_helloData.empty();
                if (ready)
                    hello.insert(hello.end(), m_helloData.begin(), m_helloData.end());
                LeaveCriticalSection(&m_queueLock);
                if (ready)
                {
                    PcContentAppendDetail(&hello);
                    break;
                }
                if (m_closing || conn->aborted)
                {
                    conn->finished = true;
                    return;
                }
                PcOsSleepMilliseconds(HELLO_WAIT_POLL_MS);
            }
        }
        if (!SendFrame(conn, &hello[0], (unsigned int)hello.size())
            || !ReceiveFrame(conn, &frame) || frame.size() < kHelloSize || ReadU32(&frame[0]) != kHelloMagic
            || ReadU16(&frame[4]) != kProtocolVersion)
        {
            if (conn->outgoing)
                PushEvent(eEvent_ConnectFailed, 0);
            conn->finished = true;
            return;
        }
        EnterCriticalSection(&m_connectionLock);
        conn->stationId = ReadU64(&frame[8]);
        conn->peerHelloData.assign(frame.begin() + kHelloSize, frame.end());
        LeaveCriticalSection(&m_connectionLock);
    }

    if (conn->outgoing)
    {
        if (!ReceiveFrame(conn, &frame) || frame.size() != 1 + CHALLENGE_SIZE || frame[0] != JOIN_CHALLENGE
            || !SendJoinIdentity(conn, &frame[1]))
        {
            PushEvent(eEvent_ConnectFailed, 0);
            conn->finished = true;
            return;
        }
    }
    else if (!CheckJoinIdentity(conn))
    {
        conn->finished = true;
        return;
    }

    conn->connected = true;
    PushEvent(eEvent_Connected, conn->stationId);

    while (!m_closing && ReceiveFrame(conn, &frame))
    {
        EnterCriticalSection(&m_queueLock);
        m_messages.push_back(Message());
        m_messages.back().stationId = conn->stationId;
        m_messages.back().data.swap(frame);
        LeaveCriticalSection(&m_queueLock);
        if (m_wakeEvent != 0)
            m_wakeEvent->Set();
    }

    conn->connected = false;
    if (!m_closing)
        PushEvent(eEvent_Disconnected, conn->stationId);
    conn->finished = true;
}

bool PcLanTransport::SendFrame(Connection* conn, const void* data, unsigned int len)
{
    std::vector<unsigned char> frame(len + 4);
    WriteU32(&frame[0], len);
    if (len != 0)
        memcpy(&frame[4], data, len);
    EnterCriticalSection(&conn->sendLock);
    const unsigned long long start = PcOsMonotonicNanoseconds();
    bool ok = PcOsSocketSendAll(conn->socket, &frame[0], frame.size());
    const unsigned long long now = PcOsMonotonicNanoseconds();
    if (ok && now - start > SLOW_SEND_NS && now - conn->slowSendLogged > SLOW_SEND_LOG_NS)
    {
        conn->slowSendLogged = now;
        printf("[pc] lan: sending %u bytes to station %016llx waited %llu ms for the network\n", len,
               conn->stationId, (now - start) / 1000000ULL);
        fflush(stdout);
    }
    LeaveCriticalSection(&conn->sendLock);
    return ok;
}

bool PcLanTransport::ReceiveFromConnection(Connection* conn, unsigned char* buf, unsigned int len)
{
    while (len > 0)
    {
        if (m_closing || conn->aborted)
            return false;
        int ready = PcOsSocketWaitReadable(conn->socket, RECEIVE_POLL_MS);
        if (ready < 0)
            return false;
        if (ready == 0)
            continue;
        int got = PcOsSocketReceive(conn->socket, buf, len);
        if (got <= 0)
            return false;
        buf += got;
        len -= (unsigned int)got;
    }
    return true;
}

bool PcLanTransport::ReceiveFrame(Connection* conn, std::vector<unsigned char>* frame)
{
    unsigned char header[4];
    if (!ReceiveFromConnection(conn, header, sizeof(header)))
        return false;
    unsigned int len = ReadU32(header);
    if (len > kMaxFrameSize)
        return false;
    frame->resize(len);
    return len == 0 || ReceiveFromConnection(conn, &(*frame)[0], len);
}

bool PcLanTransport::Send(unsigned long long station, const void* data, unsigned int len)
{
    bool sent = false;
    std::vector<Connection*> targets;
    EnterCriticalSection(&m_connectionLock);
    for (unsigned int i = 0; i < m_connections.size(); ++i)
    {
        Connection* conn = m_connections[i];
        if (conn->connected && !conn->aborted && (station == 0 || conn->stationId == station))
            targets.push_back(conn);
    }
    for (unsigned int i = 0; i < targets.size(); ++i)
    {
        if (SendFrame(targets[i], data, len))
            sent = true;
        else
        {
            printf("[pc] lan: sending to station %016llx failed; dropping the connection\n", targets[i]->stationId);
            fflush(stdout);
            targets[i]->aborted = true;
            PcOsSocketShutdown(targets[i]->socket);
        }
    }
    LeaveCriticalSection(&m_connectionLock);
    return sent;
}

bool PcLanTransport::Receive(unsigned long long* station, std::vector<unsigned char>* msg)
{
    EnterCriticalSection(&m_queueLock);
    if (m_messages.empty())
    {
        LeaveCriticalSection(&m_queueLock);
        return false;
    }
    *station = m_messages.front().stationId;
    msg->swap(m_messages.front().data);
    m_messages.pop_front();
    LeaveCriticalSection(&m_queueLock);
    return true;
}

void PcLanTransport::PushEvent(EventType type, unsigned long long station)
{
    Event ev;
    ev.type = type;
    ev.stationId = station;
    EnterCriticalSection(&m_queueLock);
    m_events.push_back(ev);
    LeaveCriticalSection(&m_queueLock);
    if (m_wakeEvent != 0)
        m_wakeEvent->Set();
}

bool PcLanTransport::PopEvent(Event* ev)
{
    EnterCriticalSection(&m_queueLock);
    if (m_events.empty())
    {
        LeaveCriticalSection(&m_queueLock);
        return false;
    }
    *ev = m_events.front();
    m_events.pop_front();
    LeaveCriticalSection(&m_queueLock);
    return true;
}

bool PcLanTransport::IsConnected(unsigned long long stationId)
{
    bool connected = false;
    EnterCriticalSection(&m_connectionLock);
    for (unsigned int i = 0; i < m_connections.size(); ++i)
    {
        if (m_connections[i]->connected && m_connections[i]->stationId == stationId)
            connected = true;
    }
    LeaveCriticalSection(&m_connectionLock);
    return connected;
}

void PcLanTransport::ReapClosedConnections()
{
    std::vector<Connection*> finished;
    EnterCriticalSection(&m_connectionLock);
    for (unsigned int i = 0; i < m_connections.size();)
    {
        if (m_connections[i]->finished)
        {
            finished.push_back(m_connections[i]);
            m_connections[i] = m_connections.back();
            m_connections.pop_back();
        }
        else
            ++i;
    }
    LeaveCriticalSection(&m_connectionLock);
    for (unsigned int i = 0; i < finished.size(); ++i)
    {
        finished[i]->thread->WaitForCompletion(-1);
        delete finished[i]->thread;
        if (finished[i]->socket != kPcOsInvalidSocket)
            PcOsSocketClose(finished[i]->socket);
        DeleteCriticalSection(&finished[i]->sendLock);
        delete finished[i];
    }
}

PcLanDiscovery::PcLanDiscovery()
    : m_sessionId(0), m_gamePort(0), m_advertising(false), m_browsing(false), m_advertiseThread(0),
      m_browseThread(0), m_revision(0)
{
    InitializeCriticalSection(&m_lock);
}

PcLanDiscovery::~PcLanDiscovery()
{
    StopAdvertising();
    StopBrowsing();
}

void PcLanDiscovery::SetLanDisabled(bool disabled)
{
    lanDisabled = disabled;
}

void PcLanDiscovery::StartAdvertising(unsigned int id, unsigned short port)
{
    StopAdvertising();
    if (lanDisabled || !PcOsNetInitialise())
        return;
    m_sessionId = id;
    m_gamePort = port;
    m_advertising = true;
    m_advertiseThread = new C4JThreadImpl(AdvertiseThreadProc, this, "LanAdvertise", 0);
    m_advertiseThread->Run();
}

void PcLanDiscovery::SetAdvertisedData(const void* data, unsigned int len)
{
    if (len > MAX_ADVERT_DATA)
        len = MAX_ADVERT_DATA;
    EnterCriticalSection(&m_lock);
    m_advertisedData.assign(static_cast<const unsigned char*>(data), static_cast<const unsigned char*>(data) + len);
    LeaveCriticalSection(&m_lock);
}

void PcLanDiscovery::StopAdvertising()
{
    m_advertising = false;
    if (m_advertiseThread != 0)
    {
        m_advertiseThread->WaitForCompletion(-1);
        delete m_advertiseThread;
        m_advertiseThread = 0;
    }
}

void PcLanDiscovery::StartBrowsing()
{
    if (m_browseThread != 0 || lanDisabled || !PcOsNetInitialise())
        return;
    m_browsing = true;
    m_browseThread = new C4JThreadImpl(BrowseThreadProc, this, "LanBrowse", 0);
    m_browseThread->Run();
}

void PcLanDiscovery::StopBrowsing()
{
    m_browsing = false;
    if (m_browseThread != 0)
    {
        m_browseThread->WaitForCompletion(-1);
        delete m_browseThread;
        m_browseThread = 0;
    }
    EnterCriticalSection(&m_lock);
    for (unsigned int i = 0; i < m_sessions.size();)
    {
        if (!m_sessions[i].manual)
        {
            m_sessions[i] = m_sessions.back();
            m_sessions.pop_back();
            ++m_revision;
        }
        else
            ++i;
    }
    LeaveCriticalSection(&m_lock);
}

void PcLanDiscovery::AddManualSession(unsigned int addr, unsigned short port, const void* data, unsigned int len)
{
    Session s;
    s.sessionId = ManualSessionId(addr, port);
    s.address = addr;
    s.port = port;
    s.data.assign(static_cast<const unsigned char*>(data), static_cast<const unsigned char*>(data) + len);
    s.lastSeen = PcOsMonotonicNanoseconds();
    s.manual = true;
    EnterCriticalSection(&m_lock);
    for (unsigned int i = 0; i < m_sessions.size(); ++i)
    {
        if (m_sessions[i].address == addr && m_sessions[i].port == port)
        {
            LeaveCriticalSection(&m_lock);
            return;
        }
    }
    m_sessions.push_back(s);
    ++m_revision;
    LeaveCriticalSection(&m_lock);
}

unsigned int PcLanDiscovery::ManualSessionId(unsigned int addr, unsigned short port)
{
    return 0x80000000u | ((addr * 2654435761u) ^ port);
}

void PcLanDiscovery::SetManualSession(unsigned int addr, unsigned short port, const void* data, unsigned int len)
{
    RemoveManualSession(addr, port);
    AddManualSession(addr, port, data, len);
}

void PcLanDiscovery::RemoveManualSession(unsigned int addr, unsigned short port)
{
    EnterCriticalSection(&m_lock);
    for (unsigned int i = 0; i < m_sessions.size();)
    {
        if (m_sessions[i].manual && m_sessions[i].address == addr && m_sessions[i].port == port)
        {
            m_sessions[i] = m_sessions.back();
            m_sessions.pop_back();
            ++m_revision;
        }
        else
            ++i;
    }
    LeaveCriticalSection(&m_lock);
}

static int ProbeThreadProc(void* argument)
{
    Probe* probe = static_cast<Probe*>(argument);
    PcOsSocket socket = PcOsTcpConnect(probe->address, probe->port);
    std::vector<unsigned char> hello(4 + kHelloSize);
    WriteU32(&hello[4], kHelloMagic);
    WriteU16(&hello[8], kProtocolVersion);
    WriteU16(&hello[10], 0);
    WriteU64(&hello[12], probe->localStationId);
    PcContentAppendSummary(&hello);
    PcContentAppendDetail(&hello);
    WriteU32(&hello[0], static_cast<unsigned int>(hello.size() - 4));
    unsigned char header[4];
    std::vector<unsigned char> frame;
    bool parked = false;
    if (socket != kPcOsInvalidSocket && PcOsSocketSendAll(socket, &hello[0], static_cast<unsigned int>(hello.size()))
        && ReceiveExactly(socket, header, sizeof(header)))
    {
        unsigned int size = ReadU32(header);
        if (size >= kLobbyHelloSize && size <= kMaxFrameSize)
        {
            frame.resize(size);
            const bool received = ReceiveExactly(socket, &frame[0], size);
            if (received && ReadU32(&frame[0]) == kLobbyMagic)
            {
                char text[32];
                PcOsFormatIPv4(probe->address, text, sizeof(text));
                if (ReadU16(&frame[4]) != kProtocolVersion)
                {
                    printf("[pc] lan: the lobby at %s:%u runs another protocol version (%x)\n", text, probe->port,
                           ReadU16(&frame[4]));
                    fflush(stdout);
                }
                else
                {
                    printf("[pc] lan: found a lobby at %s:%u\n", text, probe->port);
                    fflush(stdout);
                    PcLobbyClientStart(probe->address, probe->port, socket, probe->localStationId, frame);
                    socket = kPcOsInvalidSocket;
                    probe->finishedAt = PcOsMonotonicNanoseconds();
                    probe->state = eProbe_Lobby;
                    return 0;
                }
            }
            else
                parked = received && size > kHelloSize && ReadU32(&frame[0]) == kHelloMagic
                         && ReadU16(&frame[4]) == kProtocolVersion;
        }
    }
    if (!parked)
    {
        if (socket != kPcOsInvalidSocket)
            PcOsSocketClose(socket);
        probe->finishedAt = PcOsMonotonicNanoseconds();
        probe->state = eProbe_Failed;
        return 0;
    }
    probe->socket = socket;
    probe->stationId = ReadU64(&frame[8]);
    probe->sessionData.assign(frame.begin() + kHelloSize, frame.end());
    char text[32];
    PcOsFormatIPv4(probe->address, text, sizeof(text));
    printf("[pc] lan: found a game at %s:%u\n", text, probe->port);
    fflush(stdout);
    probe->discovery->SetManualSession(probe->address, probe->port, &probe->sessionData[0],
                                       static_cast<unsigned int>(probe->sessionData.size()));
    probe->state = eProbe_Parked;
    return 0;
}

void PcLanDiscovery::ProbeServerList(const char* path, unsigned long long localId)
{
    std::vector<ServerEntry> entries;
    if (!ReadServerList(path, &entries) || !PcOsNetInitialise())
        return;
    for (unsigned int i = 0; i < entries.size(); ++i)
    {
        unsigned int addr = 0;
        if (PcOsParseIPv4(entries[i].host.c_str(), &addr) && entries[i].port != 0)
            ProbeAddress(addr, entries[i].port, localId);
    }
}

void PcLanDiscovery::ProbeTextServerList(const char* path, unsigned long long localId)
{
    FILE* fp = fopen(path, "r");
    if (fp == 0)
        return;
    std::vector<ServerEntry> entries;
    char line[256];
    while (fgets(line, sizeof(line), fp) != 0)
    {
        std::string text(line);
        std::string::size_type hash = text.find('#');
        if (hash != std::string::npos)
            text.erase(hash);
        std::string::size_type start = text.find_first_not_of(" \t\r\n");
        if (start == std::string::npos)
            continue;
        std::string::size_type stop = text.find_first_of(" \t\r\n", start);
        text = text.substr(start, stop == std::string::npos ? std::string::npos : stop - start);
        ServerEntry entry;
        entry.port = kPcLanGamePort;
        std::string::size_type colon = text.rfind(':');
        if (colon != std::string::npos)
        {
            entry.port = (unsigned short)atoi(text.c_str() + colon + 1);
            text.erase(colon);
        }
        entry.host = text;
        if (!entry.host.empty() && entry.port != 0)
            entries.push_back(entry);
    }
    fclose(fp);
    if (entries.empty() || !PcOsNetInitialise())
        return;
    unsigned long long now = PcOsMonotonicNanoseconds();
    for (unsigned int i = 0; i < entries.size(); ++i)
    {
        ResolvedHost& resolved = resolvedHosts[entries[i].host];
        if (resolved.resolvedAt == 0 || (resolved.address == 0 && now - resolved.resolvedAt > RESOLVE_RETRY_NS))
        {
            unsigned int addr = 0;
            resolved.address = PcOsResolveIPv4(entries[i].host.c_str(), &addr) ? addr : 0;
            resolved.resolvedAt = now;
            if (resolved.address == 0)
            {
                printf("[pc] lan: could not resolve %s from %s\n", entries[i].host.c_str(), path);
                fflush(stdout);
            }
        }
        if (resolved.address != 0)
            ProbeAddress(resolved.address, entries[i].port, localId);
    }
}

void PcLanDiscovery::ProbeAddress(unsigned int address, unsigned short port, unsigned long long localStationId)
{
    if (!PcOsNetInitialise())
        return;
    unsigned long long now = PcOsMonotonicNanoseconds();
    Probe* existing = 0;
    for (unsigned int probeIndex = 0; probeIndex < s_probes.size(); ++probeIndex)
    {
        if (s_probes[probeIndex]->address == address && s_probes[probeIndex]->port == port)
            existing = s_probes[probeIndex];
    }
    if (existing != 0)
    {
        if (existing->state == eProbe_Running || existing->state == eProbe_Parked)
            return;
        if (existing->state == eProbe_Lobby
            && (PcLobbyClientRunning(address, port) || !PcLobbyClientWanted(address, port)))
            return;
        if (existing->state == eProbe_Failed && now - existing->finishedAt < kProbeRetryNanoseconds)
            return;
        existing->thread->WaitForCompletion(-1);
        delete existing->thread;
        s_probes.erase(std::find(s_probes.begin(), s_probes.end(), existing));
        delete existing;
    }
    Probe* probe = new Probe;
    probe->discovery = this;
    probe->address = address;
    probe->port = port;
    probe->localStationId = localStationId;
    probe->socket = kPcOsInvalidSocket;
    probe->stationId = 0;
    probe->state = eProbe_Running;
    probe->finishedAt = 0;
    probe->thread = new C4JThreadImpl(ProbeThreadProc, probe, "LanProbe", 0);
    s_probes.push_back(probe);
    probe->thread->Run();
}

bool PcLanDiscovery::TakeProbedConnection(unsigned int addr, unsigned short port, PcOsSocket* sock,
                                          unsigned long long* station, std::vector<unsigned char>* hello)
{
    for (unsigned int i = 0; i < s_probes.size(); ++i)
    {
        Probe* p = s_probes[i];
        if (p->address == addr && p->port == port)
        {
            for (unsigned int ms = 0; p->state == eProbe_Running && ms < PROBE_JOIN_WAIT_MS;
                 ms += PROBE_JOIN_POLL_MS)
                PcOsSleepMilliseconds(PROBE_JOIN_POLL_MS);
        }
        if (p->address == addr && p->port == port && p->state == eProbe_Parked)
        {
            p->state = eProbe_Taken;
            p->finishedAt = PcOsMonotonicNanoseconds();
            *sock = p->socket;
            *station = p->stationId;
            *hello = p->sessionData;
            RemoveManualSession(addr, port);
            return true;
        }
    }
    return false;
}

PcLanDiscovery::ProbeStatus PcLanDiscovery::GetProbeStatus(unsigned int addr, unsigned short port)
{
    for (unsigned int i = 0; i < s_probes.size(); ++i)
    {
        Probe* p = s_probes[i];
        if (p->address != addr || p->port != port)
            continue;
        switch (p->state)
        {
        case eProbe_Running:
            return eProbeStatus_Running;
        case eProbe_Parked:
        case eProbe_Taken:
            return eProbeStatus_Found;
        case eProbe_Lobby:
            return eProbeStatus_Lobby;
        default:
            return eProbeStatus_Failed;
        }
    }
    return eProbeStatus_None;
}

void PcLanDiscovery::DropClosedProbes()
{
    for (unsigned int i = 0; i < s_probes.size(); ++i)
    {
        Probe* p = s_probes[i];
        if (p->state != eProbe_Parked || PcOsSocketWaitReadable(p->socket, 0) <= 0)
            continue;
        unsigned char b;
        if (PcOsSocketPeek(p->socket, &b, 1) > 0)
            continue;
        PcOsSocketClose(p->socket);
        p->socket = kPcOsInvalidSocket;
        p->finishedAt = PcOsMonotonicNanoseconds();
        p->state = eProbe_Failed;
        p->discovery->RemoveManualSession(p->address, p->port);
        char ip[32];
        PcOsFormatIPv4(p->address, ip, sizeof(ip));
        printf("[pc] lan: the game at %s:%u closed\n", ip, p->port);
        fflush(stdout);
    }
}

unsigned int PcLanDiscovery::FindSessionId(unsigned int addr, unsigned short port)
{
    unsigned int id = 0;
    EnterCriticalSection(&m_lock);
    for (unsigned int i = 0; i < m_sessions.size(); ++i)
    {
        if (m_sessions[i].address == addr && m_sessions[i].port == port)
            id = m_sessions[i].sessionId;
    }
    LeaveCriticalSection(&m_lock);
    return id;
}

std::vector<PcLanDiscovery::Session> PcLanDiscovery::GetSessions()
{
    ExpireSessions();
    EnterCriticalSection(&m_lock);
    std::vector<Session> sessions = m_sessions;
    LeaveCriticalSection(&m_lock);
    return sessions;
}

unsigned int PcLanDiscovery::GetRevision()
{
    ExpireSessions();
    EnterCriticalSection(&m_lock);
    unsigned int rev = m_revision;
    LeaveCriticalSection(&m_lock);
    return rev;
}

void PcLanDiscovery::ExpireSessions()
{
    unsigned long long now = PcOsMonotonicNanoseconds();
    EnterCriticalSection(&m_lock);
    for (unsigned int i = 0; i < m_sessions.size();)
    {
        if (!m_sessions[i].manual && now - m_sessions[i].lastSeen > SESSION_EXPIRY_NS)
        {
            m_sessions[i] = m_sessions.back();
            m_sessions.pop_back();
            ++m_revision;
        }
        else
            ++i;
    }
    LeaveCriticalSection(&m_lock);
}

int PcLanDiscovery::AdvertiseThreadProc(void* arg)
{
    static_cast<PcLanDiscovery*>(arg)->RunAdvertise();
    return 0;
}

int PcLanDiscovery::BrowseThreadProc(void* arg)
{
    static_cast<PcLanDiscovery*>(arg)->RunBrowse();
    return 0;
}

void PcLanDiscovery::RunAdvertise()
{
    PcOsSocket sock = PcOsUdpOpen(0);
    if (sock == kPcOsInvalidSocket)
        return;
    std::vector<unsigned char> packet;
    while (m_advertising)
    {
        EnterCriticalSection(&m_lock);
        packet.assign(ADVERT_HEADER_SIZE, 0);
        WriteU32(&packet[0], ADVERT_MAGIC);
        WriteU16(&packet[4], kProtocolVersion);
        WriteU16(&packet[6], m_gamePort);
        WriteU32(&packet[8], m_sessionId);
        WriteU32(&packet[12], (unsigned int)m_advertisedData.size());
        packet.insert(packet.end(), m_advertisedData.begin(), m_advertisedData.end());
        LeaveCriticalSection(&m_lock);
        unsigned int addrs[MAX_BROADCASTS];
        size_t numAddrs = PcOsListIPv4Broadcasts(addrs, MAX_BROADCASTS);
        for (size_t i = 0; i < numAddrs; ++i)
            PcOsUdpSendTo(sock, addrs[i], kPcLanDiscoveryPort, &packet[0], packet.size());
        PcOsUdpSendTo(sock, kPcOsBroadcastAddress, kPcLanDiscoveryPort, &packet[0], packet.size());
        PcOsUdpSendTo(sock, kPcOsLoopbackAddress, kPcLanDiscoveryPort, &packet[0], packet.size());
        for (unsigned int ms = 0; m_advertising && ms < ADVERT_INTERVAL_MS;
             ms += BROWSE_POLL_MS)
            PcOsSleepMilliseconds(BROWSE_POLL_MS);
    }
    PcOsSocketClose(sock);
}

void PcLanDiscovery::RunBrowse()
{
    PcOsSocket sock = PcOsUdpOpen(kPcLanDiscoveryPort);
    if (sock == kPcOsInvalidSocket)
        return;
    std::vector<unsigned char> packet(ADVERT_HEADER_SIZE + MAX_ADVERT_DATA);
    while (m_browsing)
    {
        unsigned int addr = 0;
        int len = PcOsUdpReceiveFrom(sock, &packet[0], packet.size(), &addr, BROWSE_POLL_MS);
        if (len < (int)ADVERT_HEADER_SIZE || ReadU32(&packet[0]) != ADVERT_MAGIC
            || ReadU16(&packet[4]) != kProtocolVersion)
            continue;
        unsigned int dataLen = ReadU32(&packet[12]);
        if (dataLen > (unsigned int)len - ADVERT_HEADER_SIZE)
            continue;

        Session s;
        s.sessionId = ReadU32(&packet[8]);
        s.address = addr;
        s.port = (unsigned short)ReadU16(&packet[6]);
        s.data.assign(packet.begin() + ADVERT_HEADER_SIZE, packet.begin() + ADVERT_HEADER_SIZE + dataLen);
        s.lastSeen = PcOsMonotonicNanoseconds();
        s.manual = false;

        EnterCriticalSection(&m_lock);
        for (unsigned int i = 0; i < m_sessions.size();)
        {
            if (m_sessions[i].manual && m_sessions[i].port == s.port
                && m_sessions[i].address == s.address && s.address != kPcOsLoopbackAddress)
            {
                m_sessions[i] = m_sessions.back();
                m_sessions.pop_back();
                ++m_revision;
            }
            else
                ++i;
        }
        bool found = false;
        for (unsigned int i = 0; i < m_sessions.size(); ++i)
        {
            if (m_sessions[i].sessionId == s.sessionId)
            {
                if (m_sessions[i].data != s.data || m_sessions[i].port != s.port)
                    ++m_revision;
                if (s.address != kPcOsLoopbackAddress || m_sessions[i].address == kPcOsLoopbackAddress)
                    m_sessions[i].address = s.address;
                m_sessions[i].port = s.port;
                m_sessions[i].data.swap(s.data);
                m_sessions[i].lastSeen = s.lastSeen;
                found = true;
                break;
            }
        }
        if (!found)
        {
            m_sessions.push_back(s);
            ++m_revision;
        }
        LeaveCriticalSection(&m_lock);
    }
    PcOsSocketClose(sock);
}
