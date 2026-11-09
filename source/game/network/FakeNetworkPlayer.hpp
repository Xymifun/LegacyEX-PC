#ifndef GAME_NETWORK_FAKENETWORKPLAYER_HPP
#define GAME_NETWORK_FAKENETWORKPLAYER_HPP

#include <deque>
#include <stdint.h>
#include <string>

#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/PlayerUID.hpp"

class FakeNetworkPlayer : public INetworkPlayer
{
public:
    virtual ~FakeNetworkPlayer();

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

    PlayerUID GetSUID();
    void SetName(std::wstring name);
    void SetSmallId(unsigned char smallId);
    void SetSessionIndex(int sessionIndex);
    void SetUserIndex(int userIndex);
    void SentBRUP(unsigned int size);
    void UpdateBRUPQueue();

private:
    Socket* mSocket;
    uint64_t mLastChunkPacketTime;
    PlayerUID mUID;
    PlayerUID mSUID;
    std::wstring mName;
    unsigned char mSmallId;
    int mUserIndex;
    int mSessionIndex;
    std::deque<unsigned int> mBRUPQueue;
    uint64_t mLastBRUPTime;
};

#endif
