#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RESENDING_MESSAGE_MANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RESENDING_MESSAGE_MANAGER_HPP

#include "nn/pia.h"

namespace nn
{
namespace pia
{
namespace transport
{

class PacketHandler;

class ResendingMessageManager
{
public:
    enum
    {
        cWorkBufferSize = 0x584
    };

    ResendingMessageManager();
    ~ResendingMessageManager();
    static Result CreateInstance();
    static void DestroyInstance();
    Result Initialize(unsigned int entryNum);
    void Finalize();
    Result Startup(PacketHandler* packetHandler, bool isRelay);
    void Cleanup();
    Result Dispatch();
    Result SetSendMessageCommon(unsigned int* ackId, const unsigned char* message, unsigned int messageSize,
                                ProtocolId protocolId, bool isRelay, long long timeout, unsigned int* index);
    Result SetSendMessageByStationAddress(unsigned int* ackId, const unsigned char* message, unsigned int messageSize,
                                          const common::StationAddress& stationAddress, ProtocolId protocolId,
                                          long long timeout);
    Result SetSendMessageByStationIndex(unsigned int* ackId, const unsigned char* message, unsigned int messageSize,
                                        StationIndex stationIndex, ProtocolId protocolId, bool isRelay,
                                        long long timeout);
    bool CheckNowResending(unsigned int ackId) const;
    unsigned int ExtractAckIdFromMessage(const unsigned char* message, unsigned int messageSize) const;
    bool StopResending(unsigned int ackId);
    void Trace(unsigned long long value) const;

    long long* m_nextSendTimes;
    long long* m_timeouts;
    unsigned int* m_ackIds;
    unsigned char** m_messages;
    unsigned int* m_messageSizes;
    StationIndex* m_stationIndices;
    common::StationAddress* m_stationAddresses;
    ProtocolId* m_protocolIds;
    bool* m_isRelays;
    PacketHandler* m_packetHandler;
    bool m_isRelay;
    unsigned int m_nextAckId;
    unsigned char m_workBuffer[cWorkBufferSize];
    unsigned int m_entryNum;
};

extern ResendingMessageManager* g_piaResendingMessageManager;

}
}
}

#endif
