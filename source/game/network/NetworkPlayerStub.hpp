#ifndef GAME_NETWORK_NETWORKPLAYERSTUB_HPP
#define GAME_NETWORK_NETWORKPLAYERSTUB_HPP

#include <stdint.h>
#include <string>

#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/IQNetPlayer.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/core/Socket.hpp"

class CPlatformNetworkManager;

class NetworkPlayerStub : public INetworkPlayer
{
public:
    NetworkPlayerStub(IQNetPlayer* qNetPlayer, CPlatformNetworkManager* platformNetworkManager);
    virtual ~NetworkPlayerStub();

    virtual unsigned char GetSmallId();
    virtual void SendData(INetworkPlayer* pTo, const void* pData, int iSize, bool bReliable, bool bRequiresAck);
    virtual bool IsSameSystem(INetworkPlayer* pOther);
    virtual int GetOutstandingAckCount();
    virtual int GetSinglePlayerOutstandingAckCount();
    virtual void DecrementAcksOutstanding();
    virtual int GetSendQueueSizeBytes(INetworkPlayer* pTo, bool bReliable);
    virtual int GetSendQueueSizeMessages(INetworkPlayer* pTo, bool bReliable);
    virtual int GetCurrentRtt();
    virtual bool IsHost();
    virtual bool IsGuest();
    virtual bool IsLocal();
    virtual int GetSessionIndex();
    virtual bool IsTalking();
    virtual bool IsMutedByLocalUser(int iUserIndex);
    virtual bool HasVoice();
    virtual bool HasCamera();
    virtual int GetUserIndex();
    virtual void SetSocket(Socket* pSocket);
    virtual Socket* GetSocket();
    virtual const wchar_t* GetOnlineName();
    virtual std::wstring GetDisplayName();
    virtual PlayerUID GetUID();
    virtual void SetUID(PlayerUID uid);
    virtual void SentChunkPacket();
    virtual int GetTimeSinceLastChunkPacket_ms();
    virtual bool isFake();
    virtual IQNetPlayer* GetQNetPlayer();

private:
    IQNetPlayer* m_qNetPlayer;
    Socket* m_socket;
    int m_acksOutstanding;
    CPlatformNetworkManager* m_platformNetworkManager;
    uint64_t m_lastChunkPacketTime;
};

#endif
