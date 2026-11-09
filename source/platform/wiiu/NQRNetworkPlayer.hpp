#ifndef PLATFORM_WIIU_NQRNETWORKPLAYER_HPP
#define PLATFORM_WIIU_NQRNETWORKPLAYER_HPP

#include <deque>

#include "game/core/CriticalSection.hpp"
#include "game/world/generation/PlayerUID.hpp"

class NQRNetworkManager;

#if __SIZEOF_POINTER__ == 8
typedef uintptr_t NQRCustomData;
#else
typedef unsigned int NQRCustomData;
#endif

class NQRNetworkPlayer
{
public:
    enum eNQRNetworkPlayerType
    {
        eNQRNetworkPlayerType_Host,
        eNQRNetworkPlayerType_Local,
        eNQRNetworkPlayerType_Remote
    };

    enum AckFlags
    {
        Ack_None = 0,
        Ack_Request = 1,
        Ack_Return = 2,
        Ack_Response = 3
    };

    struct QueuedSendBlock
    {
        void* data;
        unsigned int size;
        unsigned int capacity;
        unsigned int flags;
    };

    NQRNetworkPlayer(NQRNetworkManager* manager, eNQRNetworkPlayerType type, bool local, unsigned long long stationId,
                     int localPlayerIndex, PlayerUID* uid, const char* networkName, const wchar_t* miiName);
    ~NQRNetworkPlayer();

    unsigned char GetSmallId();
    wchar_t* GetMiiName();
    char* GetNetworkName();
    wchar_t* GetNetworkNameW();
    bool IsLocal();
    unsigned char IsRemote();
    bool IsHost();
    unsigned int GetLocalPlayerIndex();
    eNQRNetworkPlayerType GetType() { return m_type; }
    unsigned int GetLocalIndex() { return m_localPlayerIndex; }
    unsigned long long GetStationId() { return m_stationId; }
    bool IsSameSystem(NQRNetworkPlayer* player);
    NQRCustomData GetCustomDataValue();
    void SetCustomDataValue(NQRCustomData value);
    unsigned int HasVoice();
    unsigned int IsTalking();
    unsigned int IsMutedByLocalUser(int userIndex);
    PlayerUID GetUID();
    void SetUID(PlayerUID uid);
    bool IsReady();
    unsigned int IsConnectionComplete();
    void ConnectionComplete();
    void SmallIdAllocated(unsigned char smallId);
    void SendInternal(NQRNetworkPlayer* player, const void* data, unsigned int size, AckFlags flags);
    void SendData(NQRNetworkPlayer* player, const void* data, unsigned int size, bool reliable);
    int GetPacketDataSize();
    void WriteDataPacket(const void* data, int size, AckFlags flags);
    void WriteAck(NQRNetworkPlayer* player);
    void ReadAckReturn(AckFlags flags);
    void ReadAckRequest(AckFlags flags, NQRNetworkPlayer* player);
    int GetOutstandingAckCount();
    int GetSendQueueSizeBytes();
    int GetSendQueueSizeMessages();
    int GetTotalOutstandingAckCount();
    unsigned int GetTotalSendQueueBytes();
    unsigned int GetTotalSendQueueMessages();
    int GetSessionIndex();

private:
    eNQRNetworkPlayerType m_type;
    bool m_local;
    unsigned int m_connectionFlags;
    unsigned int m_localPlayerIndex;
    unsigned long long m_stationId;
    unsigned char m_smallId;
    PlayerUID m_uid;
    NQRNetworkManager* m_manager;
    wchar_t m_miiName[11];
    char m_networkName[21];
    wchar_t m_networkNameW[21];
    NQRCustomData m_customData;
    OSFastMutex m_voiceMutex;
    OSFastMutex m_sendMutex;
    std::deque<QueuedSendBlock> m_sendQueue;
    unsigned int m_totalSendQueueBytes;
    unsigned int m_outstandingAckCount;
    unsigned int m_sessionIndex;
    int m_talkingTicks;
};

#endif
