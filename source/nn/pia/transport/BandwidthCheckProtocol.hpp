#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_BANDWIDTH_CHECK_PROTOCOL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_BANDWIDTH_CHECK_PROTOCOL_HPP

#include "nn/pia.h"
#include "nn/pia/common/ISerializable.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace inet
{
class InetJoinMeshJob;
}
}
}

namespace nn
{
namespace pia
{
namespace transport
{

class BandwidthCheckProtocol : public Protocol
{
public:
    class Header : public common::ISerializable
    {
    public:
        unsigned int m_type;
        unsigned int m_sequence;
        unsigned int m_offset;
        unsigned int m_flags;
        virtual ~Header();
        virtual Result Serialize(unsigned char* buffer, unsigned int* size) const;
        virtual Result Deserialize(const unsigned char* buffer);
        virtual unsigned int GetSerializedSize() const;
    };

    enum
    {
        cProtocolType = 0x54
    };

    enum SenderState
    {
        SenderState_None = 0,
        SenderState_WaitingForAck = 1,
        SenderState_Measuring = 2,
        SenderState_WaitingForResponse = 3,
        SenderState_Completed = 4,
        SenderState_Canceled = 5
    };
    enum ReceiverState
    {
        ReceiverState_None = 0,
        ReceiverState_WaitingForBulk = 1,
        ReceiverState_Measuring = 2,
        ReceiverState_SendingResponse = 3
    };

    BandwidthCheckProtocol();
    virtual ~BandwidthCheckProtocol();
    void initSender();
    void initReceiver();
    Result Initialize();
    void Finalize();
    virtual Result Startup();
    virtual void Cleanup();
    int GetMaxIpPacketSize() const;
    int GetMinIpPacketSize() const;
    unsigned int IsSetupParamValid(int, unsigned int, int) const;
    Result Activate(int, unsigned int, bool, int);
    void Inactivate();
    bool IsReady() const;
    Result StartSend(StationIndex);
    void CancelSend();
    bool IsCompleted() const;
    Result GetUplinkBitRate(int*) const;
    void SetBandwidthChecked(StationIndex);
    int getBulkPacketSize() const;
    bool isExist(StationIndex) const;
    Result send(StationIndex, const unsigned char*, unsigned int);
    Result sendHeaderOnlyPacket(StationIndex, unsigned int, int, int, int);
    Result sendSynPacket();
    int getPayloadSize(unsigned int) const;
    Result sendBulkPacket();
    Result sendAckPacket();
    Result sendResponse();
    void processSend();
    int getIpPacketSize(unsigned int) const;
    bool IsBandwidthChecked(StationIndex) const;
    int getSendPacketNumInMeasuringTime() const;
    bool isStartSendCalled() const;
    void processCancel();
    void senderStateTransition();
    bool isBulkDataArrived() const;
    void receiverStateTransition();
    void processReceive();
    virtual Result Dispatch();
    virtual Result UpdateProtocolEvent(const ProtocolEvent&);
    virtual void Trace(unsigned long long) const;
    virtual unsigned short GetProtocolType() const;
    static const char* getSenderStateString(SenderState);
    static const char* getReceiverStateString(ReceiverState);

private:
    friend class inet::InetJoinMeshJob;
    bool m_initialized;
    int m_stationNum;
    bool m_activated;
    bool m_isServer;
    unsigned char* m_bulkBuffer;
    unsigned int m_activeStationBitmap;
    unsigned int m_bandwidthCheckedBitmap;
    int m_measurementTime;
    unsigned int m_bulkPacketSize;
    int m_completionTimeout;
    int m_measurementCount;
    SenderState m_senderState;
    common::Time m_senderStartTime;
    bool m_ackReceived;
    common::Time m_bulkStartTime;
    int m_uplinkBitRate;
    StationIndex m_senderStation;
    int m_bulkPacketSequence;
    int m_measurementResult;
    bool m_cancelRequested;
    ReceiverState m_receiverState;
    common::Time m_receiverStartTime;
    common::Time m_bulkArrivalTime;
    common::Time m_responseSentTime;
    unsigned int m_receivedBytes;
    StationIndex m_receiverStation;
    common::StationAddress m_stationAddress;
    unsigned int m_receivedBulkPacketCount;
    int m_firstBulkPacketSequence;
    int m_lastBulkPacketSequence;
    common::Time m_ackSentTime;
    bool m_receiverActive;
};

}
}
}

#endif
