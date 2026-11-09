#include "platform/pc/PcLobbyClient.hpp"

#include "platform/pc/PcAccount.hpp"
#include "platform/pc/PcDebug.hpp"
#include "platform/pc/PcLanTransport.hpp"
#include "platform/pc/PcLobbyProtocol.hpp"

#include "game/core/CriticalSection.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

#include <algorithm>
#include <deque>
#include <stdio.h>
#include <string.h>

namespace
{
const unsigned int kPollMilliseconds = 250;
const unsigned long long kKeepAliveNanoseconds = 15000ULL * 1000000ULL;
const unsigned long long kSilenceNanoseconds = 45000ULL * 1000000ULL;
const unsigned int kReconnectMilliseconds = 5000;
const unsigned int kRefusedRetryMilliseconds = 30000;
const unsigned int MAX_FRAME = 1024 * 1024;

struct RelayedSocket
{
    PcOsSocket socket;
    unsigned int address;
    PcVerifiedIdentities verified;
};

struct LobbyHello
{
    bool hasChallenge;
    unsigned char challenge[kLobbyChallengeSize];
};

bool ParseLobbyHello(const std::vector<unsigned char>& frame, LobbyHello* hello);

struct Client
{
    unsigned int address;
    unsigned short port;
    unsigned long long stationId;
    PcOsSocket initialSocket;
    LobbyHello initialHello;
    volatile bool stop;
    bool threadless;
};

struct Shared
{
    Shared() { InitializeCriticalSection(&lock); }
    OSFastMutex lock;
};

OSFastMutex& Mutex()
{
    static Shared shared;
    return shared.lock;
}

Client* s_current = 0;
EPcLobbyState lobbyState = ePcLobby_Off;
std::wstring s_refusal;
std::wstring s_linkCode;
std::wstring s_linkText;
std::vector<PcLobbyWorld> s_worlds;
unsigned int wantedAddress = 0;
std::vector<unsigned short> wantedPorts;
unsigned int s_revision = 0;
unsigned long long s_secret = 0;
std::vector<unsigned char> s_hosted;
unsigned char s_hostedVisibility = kVisibilityPublic;
unsigned int s_hostedRevision = 0;
std::deque<RelayedSocket> s_relayed;

void Lock()
{
    EnterCriticalSection(&Mutex());
}

void Unlock()
{
    LeaveCriticalSection(&Mutex());
}

void PutU16(std::vector<unsigned char>* out, unsigned int v)
{
    out->push_back((unsigned char)v);
    out->push_back((unsigned char)(v >> 8));
}

void PutU32(std::vector<unsigned char>* out, unsigned int v)
{
    PutU16(out, v & 0xFFFF);
    PutU16(out, v >> 16);
}

void PutU64(std::vector<unsigned char>* out, unsigned long long v)
{
    PutU32(out, (unsigned int)v);
    PutU32(out, (unsigned int)(v >> 32));
}

unsigned int GetU32(const unsigned char* in)
{
    return (unsigned int)in[0] | ((unsigned int)in[1] << 8)
           | ((unsigned int)in[2] << 16) | ((unsigned int)in[3] << 24);
}

unsigned long long GetU64(const unsigned char* in)
{
    return (unsigned long long)GetU32(in) | ((unsigned long long)GetU32(in + 4) << 32);
}

bool SendFrame(PcOsSocket sock, const std::vector<unsigned char>& frame)
{
    std::vector<unsigned char> buf;
    PutU32(&buf, (unsigned int)frame.size());
    buf.insert(buf.end(), frame.begin(), frame.end());
    return PcOsSocketSendAll(sock, &buf[0], buf.size());
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

bool ReceiveFrame(PcOsSocket sock, std::vector<unsigned char>* frame)
{
    unsigned char header[4];
    if (!ReceiveExactly(sock, header, sizeof(header)))
        return false;
    unsigned int len = GetU32(header);
    if (len > MAX_FRAME)
        return false;
    frame->resize(len);
    return len == 0 || ReceiveExactly(sock, &(*frame)[0], len);
}

std::string FormatAddress(const Client* c)
{
    char ip[32];
    PcOsFormatIPv4(c->address, ip, sizeof(ip));
    char buf[48];
    snprintf(buf, sizeof(buf), "%s:%u", ip, c->port);
    return buf;
}

bool ParseLobbyHello(const std::vector<unsigned char>& frame, LobbyHello* hello)
{
    hello->hasChallenge = false;
    if (frame.size() < kLobbyHelloSize || GetU32(&frame[0]) != kLobbyMagic)
        return false;
    const unsigned int version = frame[6] | (frame[7] << 8);
    if (version >= kLobbySignedVersion && frame.size() >= kLobbyChallengeOffset + kLobbyChallengeSize)
    {
        memcpy(hello->challenge, &frame[kLobbyChallengeOffset], kLobbyChallengeSize);
        hello->hasChallenge = true;
    }
    return true;
}

PcOsSocket ConnectControl(const Client* c, LobbyHello* outHello)
{
    PcOsSocket sock = PcOsTcpConnect(c->address, c->port);
    if (sock == kPcOsInvalidSocket)
        return sock;
    std::vector<unsigned char> hello;
    PutU32(&hello, kPcLanHelloMagic);
    PutU16(&hello, kPcLanProtocolVersion);
    PutU16(&hello, 0);
    PutU64(&hello, c->stationId);
    std::vector<unsigned char> reply;
    if (!SendFrame(sock, hello) || !ReceiveFrame(sock, &reply) || !ParseLobbyHello(reply, outHello))
    {
        PcOsSocketClose(sock);
        return kPcOsInvalidSocket;
    }
    return sock;
}

bool SendIdentity(PcOsSocket socket, const LobbyHello& hello)
{
    std::vector<unsigned char> frame(1, kLobbyIdentify);
    unsigned char uuid[16];
    PcAccountUuid(uuid);
    frame.insert(frame.end(), uuid, uuid + sizeof(uuid));
    std::string name = PcPlayerName();
    if (name.size() > 255)
        name.resize(255);
    frame.push_back(static_cast<unsigned char>(name.size()));
    frame.insert(frame.end(), name.begin(), name.end());
    if (hello.hasChallenge)
    {
        std::vector<unsigned char> message(kIdentityContext, kIdentityContext + sizeof(kIdentityContext) - 1);
        message.insert(message.end(), hello.challenge, hello.challenge + kLobbyChallengeSize);
        message.insert(message.end(), uuid, uuid + sizeof(uuid));
        unsigned char publicKey[32];
        unsigned char signature[64];
        if (PcAccountSign(&message[0], message.size(), publicKey, signature))
        {
            frame.insert(frame.end(), publicKey, publicKey + sizeof(publicKey));
            frame.insert(frame.end(), signature, signature + sizeof(signature));
            frame.push_back(kLobbyShowsLinkCode | kLobbyKnowsFriends);
            frame.push_back(static_cast<unsigned char>(kPcGuestIdentities));
            for (int guest = 1; guest <= kPcGuestIdentities; ++guest)
            {
                unsigned char guestUuid[16];
                PcGuestAccountUuid(guest, guestUuid);
                std::vector<unsigned char> guestMessage(kIdentityContext,
                                                        kIdentityContext + sizeof(kIdentityContext) - 1);
                guestMessage.insert(guestMessage.end(), hello.challenge, hello.challenge + kLobbyChallengeSize);
                guestMessage.insert(guestMessage.end(), guestUuid, guestUuid + sizeof(guestUuid));
                if (!PcGuestAccountSign(guest, &guestMessage[0], guestMessage.size(), publicKey, signature))
                    memset(signature, 0, sizeof(signature));
                frame.insert(frame.end(), guestUuid, guestUuid + sizeof(guestUuid));
                frame.insert(frame.end(), publicKey, publicKey + sizeof(publicKey));
                frame.insert(frame.end(), signature, signature + sizeof(signature));
            }
        }
    }
    return SendFrame(socket, frame);
}

void OpenRelay(const Client* client, unsigned int token, const PcVerifiedIdentities& verified)
{
    PcOsSocket socket = PcOsTcpConnect(client->address, client->port);
    if (socket == kPcOsInvalidSocket)
        return;
    std::vector<unsigned char> frame;
    PutU32(&frame, kRelayMagic);
    PutU16(&frame, kPcLanProtocolVersion);
    frame.push_back(kRelayAccept);
    frame.push_back(0);
    PutU32(&frame, token);
    PutU64(&frame, client->stationId);
    if (!SendFrame(socket, frame))
    {
        PcOsSocketClose(socket);
        return;
    }
    RelayedSocket relayed;
    relayed.socket = socket;
    relayed.address = client->address;
    relayed.verified = verified;
    Lock();
    if (s_current == client)
    {
        s_relayed.push_back(relayed);
        socket = kPcOsInvalidSocket;
    }
    Unlock();
    if (socket != kPcOsInvalidSocket)
        PcOsSocketClose(socket);
}

void HandleList(const Client* client, const std::vector<unsigned char>& frame)
{
    std::vector<PcLobbyWorld> worlds;
    if (frame.size() < 5)
        return;
    const bool flagged = frame[0] == kLobbyFriendList;
    const size_t header = flagged ? 9 : 8;
    unsigned int count = GetU32(&frame[1]);
    size_t position = 5;
    for (unsigned int index = 0; index < count && position + header <= frame.size(); ++index)
    {
        PcLobbyWorld world;
        world.id = GetU32(&frame[position]);
        if (flagged)
            world.flags = frame[position + 4];
        unsigned int size = GetU32(&frame[position + header - 4]);
        position += header;
        if (position + size > frame.size())
            break;
        world.sessionData.assign(frame.begin() + position, frame.begin() + position + size);
        position += size;
        worlds.push_back(world);
    }
    Lock();
    if (s_current == client)
    {
        s_worlds.swap(worlds);
        ++s_revision;
    }
    Unlock();
}

void SetStateLocked(EPcLobbyState state)
{
    lobbyState = state;
    if (state != ePcLobby_Connected)
    {
        if (!s_worlds.empty())
            ++s_revision;
        s_worlds.clear();
    }
}

void SetState(const Client* c, EPcLobbyState state)
{
    Lock();
    if (s_current == c)
        SetStateLocked(state);
    Unlock();
}

std::string FrameText(const std::vector<unsigned char>& frame, size_t start, size_t len)
{
    if (start >= frame.size())
        return std::string();
    if (len > frame.size() - start)
        len = frame.size() - start;
    return std::string(frame.begin() + start, frame.begin() + start + len);
}

std::wstring DecodeUtf8(const std::string& text)
{
    std::wstring out;
    for (size_t i = 0; i < text.size();)
    {
        const unsigned char lead = (unsigned char)text[i];
        unsigned int cp = lead;
        size_t len = 1;
        if (lead >= 0xF0)
            len = 4;
        else if (lead >= 0xE0)
            len = 3, cp = lead & 0x0F;
        else if (lead >= 0xC0)
            len = 2, cp = lead & 0x1F;
        if (i + len > text.size())
            break;
        for (size_t j = 1; j < len && len < 4; ++j)
            cp = (cp << 6) | ((unsigned char)text[i + j] & 0x3F);
        out.push_back(len == 4 || (lead >= 0x80 && lead < 0xC0) ? L'?' : (wchar_t)cp);
        i += len;
    }
    return out;
}

void ShowLinkCode(Client* client, const std::vector<unsigned char>& frame)
{
    if (frame.size() < 2)
        return;
    const std::wstring code = DecodeUtf8(FrameText(frame, 2, frame[1]));
    const std::wstring text = DecodeUtf8(FrameText(frame, 2 + frame[1], frame.size()));
    Lock();
    if (s_current == client)
    {
        s_linkCode = code;
        s_linkText = text;
        SetStateLocked(ePcLobby_Linking);
    }
    Unlock();
    printf("[pc] lobby: link this identity with the code %s\n", FrameText(frame, 2, frame[1]).c_str());
    fflush(stdout);
}

unsigned int RunConnection(Client* client, PcOsSocket socket, const LobbyHello& hello)
{
    if (!SendIdentity(socket, hello))
        return kReconnectMilliseconds;
    unsigned long long lastHeard = PcOsMonotonicNanoseconds();
    unsigned long long lastSent = lastHeard;
    unsigned int sentHosted = 0xFFFFFFFFu;
    bool wasHosting = false;
    bool welcomed = false;
    std::vector<unsigned char> frame;
    while (!client->stop)
    {
        int ready = PcOsSocketWaitReadable(socket, kPollMilliseconds);
        if (ready < 0)
            break;
        unsigned long long now = PcOsMonotonicNanoseconds();
        if (ready > 0)
        {
            if (!ReceiveFrame(socket, &frame) || frame.empty())
                break;
            lastHeard = now;
            switch (frame[0])
            {
            case kLobbyWelcome:
                if (frame.size() >= 9)
                {
                    Lock();
                    if (s_current == client)
                    {
                        s_secret = GetU64(&frame[1]);
                        SetStateLocked(ePcLobby_Connected);
                    }
                    Unlock();
                    welcomed = true;
                    printf("[pc] lobby: joined the session at %s\n", FormatAddress(client).c_str());
                    fflush(stdout);
                }
                break;
            case kLobbyRefused:
            {
                std::wstring reason = DecodeUtf8(FrameText(frame, 1, frame.size()));
                Lock();
                if (s_current == client)
                {
                    s_refusal = reason;
                    SetStateLocked(ePcLobby_Refused);
                }
                Unlock();
                printf("[pc] lobby: %s refused this game\n", FormatAddress(client).c_str());
                fflush(stdout);
                return kRefusedRetryMilliseconds;
            }
            case kLobbyLinkCode:
                if (!welcomed)
                    ShowLinkCode(client, frame);
                break;
            case kLobbyList:
            case kLobbyFriendList:
                HandleList(client, frame);
                break;
            case kLobbyOpen:
                if (frame.size() >= 5)
                {
                    PcVerifiedIdentities verified;
                    if (frame.size() >= 21)
                    {
                        memcpy(verified.uuids[0], &frame[5], 16);
                        verified.count = 1;
                        const unsigned int extra = frame.size() >= 22 ? frame[21] : 0;
                        for (unsigned int index = 0; index < extra && verified.count < 4
                                                     && 22 + (index + 1) * 16 <= frame.size();
                             ++index)
                            memcpy(verified.uuids[verified.count++], &frame[22 + index * 16], 16);
                    }
                    OpenRelay(client, GetU32(&frame[1]), verified);
                }
                break;
            default:
                break;
            }
        }
        if (now - lastHeard > kSilenceNanoseconds)
            break;
        if (!welcomed)
            continue;
        Lock();
        const unsigned int hostedRevision = s_hostedRevision;
        std::vector<unsigned char> hosted = s_hosted;
        const unsigned char visibility = s_hostedVisibility;
        Unlock();
        if (hostedRevision != sentHosted)
        {
            if (!hosted.empty())
            {
                std::vector<unsigned char> visibilityFrame(1, kLobbyVisibility);
                visibilityFrame.push_back(visibility);
                if (!SendFrame(socket, visibilityFrame))
                    break;
            }
            if (!hosted.empty() || wasHosting)
            {
                std::vector<unsigned char> update(1, hosted.empty() ? kLobbyStopHosting : kLobbyHosting);
                update.insert(update.end(), hosted.begin(), hosted.end());
                if (!SendFrame(socket, update))
                    break;
                lastSent = now;
            }
            wasHosting = !hosted.empty();
            sentHosted = hostedRevision;
        }
        if (now - lastSent > kKeepAliveNanoseconds)
        {
            if (!SendFrame(socket, std::vector<unsigned char>(1, kLobbyKeepAlive)))
                break;
            lastSent = now;
        }
    }
    if (!client->stop)
    {
        printf("[pc] lobby: lost the session at %s\n", FormatAddress(client).c_str());
        fflush(stdout);
    }
    return kReconnectMilliseconds;
}

int ThreadProc(void* arg)
{
    Client* c = static_cast<Client*>(arg);
    PcOsSocket sock = c->initialSocket;
    LobbyHello hello = c->initialHello;
    while (!c->stop)
    {
        if (sock == kPcOsInvalidSocket)
            sock = ConnectControl(c, &hello);
        unsigned int delay = kReconnectMilliseconds;
        if (sock != kPcOsInvalidSocket)
        {
            delay = RunConnection(c, sock, hello);
            PcOsSocketClose(sock);
            sock = kPcOsInvalidSocket;
        }
        if (c->stop)
            break;
        Lock();
        if (s_current == c && lobbyState != ePcLobby_Refused)
            SetStateLocked(ePcLobby_Connecting);
        Unlock();
        for (unsigned int ms = 0; ms < delay && !c->stop; ms += kPollMilliseconds)
            PcOsSleepMilliseconds(kPollMilliseconds);
        Lock();
        if (s_current == c && lobbyState == ePcLobby_Refused)
            SetStateLocked(ePcLobby_Connecting);
        Unlock();
    }
    if (sock != kPcOsInvalidSocket)
        PcOsSocketClose(sock);
    delete c;
    return 0;
}
}

void PcLobbyClientStart(unsigned int addr, unsigned short port, PcOsSocket sock, unsigned long long station,
                        const std::vector<unsigned char>& hello)
{
    Client* c = new Client;
    c->address = addr;
    c->port = port;
    c->stationId = station;
    c->initialSocket = sock;
    ParseLobbyHello(hello, &c->initialHello);
    c->stop = false;
    c->threadless = PcDebugMode();
    Lock();
    bool wanted = wantedAddress != 0 && wantedAddress == addr;
    if (wanted)
        wanted = std::find(wantedPorts.begin(), wantedPorts.end(), port) != wantedPorts.end();
    if (s_current != 0 || !wanted)
    {
        Unlock();
        PcOsSocketClose(sock);
        if (!wanted)
        {
            printf("[pc] lobby: not joining the session at %s (not the one connected to)\n",
                   FormatAddress(c).c_str());
            fflush(stdout);
        }
        delete c;
        return;
    }
    s_current = c;
    s_secret = 0;
    s_refusal.clear();
    ++s_hostedRevision;
    if (c->threadless)
    {
        s_refusal = L"debug mode (--debug) joins no online session";
        SetStateLocked(ePcLobby_Refused);
        Unlock();
        PcOsSocketClose(sock);
        printf("[pc] lobby: not joining the session at %s in debug mode\n", FormatAddress(c).c_str());
        fflush(stdout);
        return;
    }
    SetStateLocked(ePcLobby_Connecting);
    Unlock();
    C4JThreadImpl* t = new C4JThreadImpl(ThreadProc, c, "LobbyClient", 0);
    t->SetDeleteOnExit(true);
    t->Run();
}

bool PcLobbyClientRunning(unsigned int addr, unsigned short port)
{
    Lock();
    const bool running = s_current != 0 && s_current->address == addr && s_current->port == port;
    Unlock();
    return running;
}

void PcLobbyClientSetWanted(unsigned int addr, const std::vector<unsigned short>& ports)
{
    Lock();
    wantedAddress = addr;
    wantedPorts = ports;
    Unlock();
}

bool PcLobbyClientWanted(unsigned int addr, unsigned short port)
{
    Lock();
    const bool wanted = wantedAddress != 0 && wantedAddress == addr
                        && std::find(wantedPorts.begin(), wantedPorts.end(), port) != wantedPorts.end();
    Unlock();
    return wanted;
}

void PcLobbyClientStop()
{
    Lock();
    if (s_current != 0 && s_current->threadless)
        delete s_current;
    else if (s_current != 0)
        s_current->stop = true;
    s_current = 0;
    while (!s_relayed.empty())
    {
        PcOsSocketClose(s_relayed.front().socket);
        s_relayed.pop_front();
    }
    SetStateLocked(ePcLobby_Off);
    Unlock();
}

EPcLobbyState PcLobbyClientState()
{
    Lock();
    EPcLobbyState st = lobbyState;
    Unlock();
    return st;
}

void PcLobbyClientLinkCode(std::wstring* code, std::wstring* text)
{
    Lock();
    *code = s_linkCode;
    *text = s_linkText;
    Unlock();
}

std::wstring PcLobbyClientRefusal()
{
    Lock();
    std::wstring r = s_refusal;
    Unlock();
    return r;
}

unsigned int PcLobbyClientRevision()
{
    Lock();
    unsigned int rev = s_revision;
    Unlock();
    return rev;
}

std::vector<PcLobbyWorld> PcLobbyClientWorlds(unsigned int* addr, unsigned short* port, unsigned long long* secret)
{
    Lock();
    std::vector<PcLobbyWorld> worlds = s_worlds;
    *addr = s_current != 0 ? s_current->address : 0;
    *port = s_current != 0 ? s_current->port : 0;
    *secret = s_secret;
    Unlock();
    return worlds;
}

void PcLobbyClientSetHosted(const std::vector<unsigned char>& sessionData, unsigned char visibility)
{
    Lock();
    if (sessionData != s_hosted || visibility != s_hostedVisibility)
    {
        s_hosted = sessionData;
        s_hostedVisibility = visibility;
        ++s_hostedRevision;
    }
    Unlock();
}

bool PcLobbyClientTakeRelayed(PcOsSocket* socket, unsigned int* address, PcVerifiedIdentities* verified)
{
    Lock();
    const bool found = !s_relayed.empty();
    if (found)
    {
        *socket = s_relayed.front().socket;
        *address = s_relayed.front().address;
        *verified = s_relayed.front().verified;
        s_relayed.pop_front();
    }
    Unlock();
    return found;
}
