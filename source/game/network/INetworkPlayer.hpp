#ifndef GAME_NETWORK_INETWORKPLAYER_HPP
#define GAME_NETWORK_INETWORKPLAYER_HPP

#include <string>

class IQNetPlayer;
struct PlayerUID;
class Socket;

class INetworkPlayer
{
public:
    virtual ~INetworkPlayer() {}

    virtual unsigned char GetSmallId() = 0;
    virtual void SendData(INetworkPlayer* pTo, const void* pData, int iSize, bool bReliable, bool bRequiresAck) = 0;
    virtual bool IsSameSystem(INetworkPlayer* pOther) = 0;
    virtual int GetOutstandingAckCount() = 0;
    virtual int GetSinglePlayerOutstandingAckCount() = 0;
    virtual void DecrementAcksOutstanding() = 0;
    virtual int GetSendQueueSizeBytes(INetworkPlayer* pTo, bool bReliable) = 0;
    virtual int GetSendQueueSizeMessages(INetworkPlayer* pTo, bool bReliable) = 0;
    virtual int GetCurrentRtt() = 0;
    virtual bool IsHost() = 0;
    virtual bool IsGuest() = 0;
    virtual bool IsLocal() = 0;
    virtual int GetSessionIndex() = 0;
    virtual bool IsTalking() = 0;
    virtual bool IsMutedByLocalUser(int iUserIndex) = 0;
    virtual bool HasVoice() = 0;
    virtual bool HasCamera() = 0;
    virtual int GetUserIndex() = 0;
    virtual void SetSocket(Socket* pSocket) = 0;
    virtual Socket* GetSocket() = 0;
    virtual const wchar_t* GetOnlineName() = 0;
    virtual std::wstring GetDisplayName() = 0;
    virtual PlayerUID GetUID() = 0;
    virtual void SetUID(PlayerUID uid) = 0;
    virtual void SentChunkPacket() = 0;
    virtual int GetTimeSinceLastChunkPacket_ms() = 0;
    virtual bool isFake() = 0;
    virtual IQNetPlayer* GetQNetPlayer() = 0;
};

#endif
