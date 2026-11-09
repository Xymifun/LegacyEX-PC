#ifndef PLATFORM_WIIU_NETWORKPLAYERNINTENDO_HPP
#define PLATFORM_WIIU_NETWORKPLAYERNINTENDO_HPP

#include <stdint.h>
#include <string>
#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/PlayerUID.hpp"

class NQRNetworkPlayer;
class Socket;

class NetworkPlayerNintendo : public INetworkPlayer
{
public:
    NetworkPlayerNintendo(NQRNetworkPlayer* player);
    virtual ~NetworkPlayerNintendo();
    virtual unsigned char GetSmallId();
    virtual void SendData(INetworkPlayer* to, const void* data, int size, bool reliable, bool requiresAck);
    virtual bool IsSameSystem(INetworkPlayer* other);
    virtual int GetOutstandingAckCount();
    virtual int GetSinglePlayerOutstandingAckCount();
    virtual void DecrementAcksOutstanding();
    virtual int GetSendQueueSizeBytes(INetworkPlayer* to, bool reliable);
    virtual int GetSendQueueSizeMessages(INetworkPlayer* to, bool reliable);
    virtual int GetCurrentRtt();
    virtual bool IsHost();
    virtual bool IsGuest();
    virtual bool IsLocal();
    virtual int GetSessionIndex();
    virtual bool IsTalking();
    virtual bool IsMutedByLocalUser(int userIndex);
    virtual bool HasVoice();
    virtual bool HasCamera();
    virtual int GetUserIndex();
    virtual void SetSocket(Socket* socket);
    virtual Socket* GetSocket();
    virtual const wchar_t* GetOnlineName();
    virtual std::wstring GetDisplayName();
    virtual PlayerUID GetUID();
    virtual void SetUID(PlayerUID uid);
    virtual void SentChunkPacket();
    virtual int GetTimeSinceLastChunkPacket_ms();
    virtual bool isFake();
    virtual IQNetPlayer* GetQNetPlayer();
    std::wstring GetMiiName();
    const char* GetNetworkName();

private:
    NQRNetworkPlayer* m_player;
    Socket* m_socket;
    uint64_t m_lastChunkPacketTime;
};

#endif
