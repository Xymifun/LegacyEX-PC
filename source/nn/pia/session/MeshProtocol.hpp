#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_MESH_PROTOCOL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_MESH_PROTOCOL_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class ProtocolEvent;
class ReceivedMessageAccessor;
class ReliableSlidingWindow;
class Station;

}
}
}

namespace nn
{
namespace pia
{
namespace session
{

class ProcessJoinRequestJob;

class MeshProtocol : public transport::Protocol
{
    friend class ProcessJoinRequestJob;
    friend class Mesh;

public:
    enum
    {
        cProtocolType = 0x18
    };

    enum MessageType
    {
        cMessageTypeJoinRequest = 0x01,
        cMessageTypeJoinResponse = 0x02,
        cMessageTypeLeaveRequest = 0x04,
        cMessageTypeLeaveResponse = 0x08,
        cMessageTypeDestroyMesh = 0x10,
        cMessageTypeDestroyResponse = 0x11,
        cMessageTypeUpdateMesh = 0x20,
        cMessageTypeKickoutNotice = 0x21,
        cMessageTypeDummyMessage = 0x22,
        cMessageTypeDummyAck = 0x23,
        cMessageTypeConnectionFailureNotice = 0x24,
        cMessageTypeGreeting = 0x40,
        cMessageTypeMigrationFinish = 0x41,
        cMessageTypeGreetingResponse = 0x42,
        cMessageTypeMigrationStart = 0x44,
        cMessageTypeMigrationResponse = 0x48,
        cMessageTypeMultiMigrationStart = 0x49,
        cMessageTypeMultiMigrationRankDecision = 0x4a,
        cMessageTypeConnectionReport = 0x80,
        cMessageTypeRelayRouteDirections = 0x81
    };

    MeshProtocol();
    virtual ~MeshProtocol();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned short GetProtocolType() const;
    virtual Result StartupWithStationAddress(const common::StationAddress& address);
    virtual Result Dispatch();
    virtual Result UpdateProtocolEvent(const transport::ProtocolEvent& event);

    Result StartupNative();
    Result Initialize();
    void Finalize();

    common::StepSequenceJob* GetWaitingJoinRequestJob();
    common::StepSequenceJob* GetWaitingJoinResponseJob();
    common::StepSequenceJob* GetWaitingLeaveRequestJob() { return m_waitingLeaveRequestJob; }
    common::StepSequenceJob* GetWaitingLeaveResponseJob();
    void SetWaitingJoinRequestJob(common::StepSequenceJob* job);
    void SetWaitingJoinResponseJob(common::StepSequenceJob* job);
    void SetWaitingLeaveRequestJob(common::StepSequenceJob* job) { m_waitingLeaveRequestJob = job; }
    void SetWaitingLeaveResponseJob(common::StepSequenceJob* job);

    unsigned int GetUnknown24() const { return m_unknown24; }
    unsigned int GetJoinRequestDataSize() const;
    void MakeJoinRequestData(unsigned char* buffer);
    unsigned char GetSystemMessagePartitionNum(unsigned char messageType, unsigned char stationCount);
    unsigned char MakeJoinResponseData(StationIndex stationIndex, unsigned char** buffers, unsigned int* sizes);

    void ParseJoinRequest(const transport::ReceivedMessageAccessor& message);
    void ParseJoinResponse(const transport::ReceivedMessageAccessor& message);
    void ParseLeaveRequest(const transport::ReceivedMessageAccessor& message);
    void ParseLeaveResponse(const transport::ReceivedMessageAccessor& message);
    void ParseDestroyMesh(const transport::ReceivedMessageAccessor& message);
    void ParseDestroyResponse(const transport::ReceivedMessageAccessor& message);
    void ParseUpdateMesh(const transport::ReceivedMessageAccessor& message);
    void ParseKickoutNotice(const transport::ReceivedMessageAccessor& message);
    void ParseDummyMessage(const transport::ReceivedMessageAccessor& message);
    void ParseDummyAck(const transport::ReceivedMessageAccessor& message) {}
    void ParseConnectionFailureNotice(const transport::ReceivedMessageAccessor& message);
    void ParseGreeting(const transport::ReceivedMessageAccessor& message);
    void ParseMigrationFinish(const transport::ReceivedMessageAccessor& message);
    void ParseGreetingResponse(const transport::ReceivedMessageAccessor& message);
    void ParseMigrationStart(const transport::ReceivedMessageAccessor& message);
    void ParseMigrationResponse(const transport::ReceivedMessageAccessor& message);
    void ParseMultiMigrationStart(const transport::ReceivedMessageAccessor& message);
    void ParseMultiMigrationRankDecision(const transport::ReceivedMessageAccessor& message);
    void ParseConnectionReport(const transport::ReceivedMessageAccessor& message);
    void ParseRelayRouteDirections(const transport::ReceivedMessageAccessor& message);
    void ParseHelper(const transport::ReceivedMessageAccessor& message);

    void SendLeaveRequest();
    void SendLeaveResponse(transport::Station* station);
    void SendUpdateMesh(bool force, unsigned int versionIncrement);
    void SendDenyingJoinResponse(const common::StationAddress& address, unsigned char reason);
    bool SendDestroyMesh(StationIndex stationIndex);
    bool SendDestroyResponse(StationIndex stationIndex);
    bool SendGreeting(StationIndex stationIndex);
    bool SendGreetingResponse(StationIndex stationIndex);
    bool SendMigrationFinish(bool succeeded);
    bool SendMigrationStart(StationIndex stationIndex, StationIndex newHostIndex);
    bool SendMigrationResponse(StationIndex stationIndex);
    bool SendConnectionReport(StationIndex stationIndex, unsigned int versionHigh, unsigned int versionLow,
                              unsigned int* stationBitmap);
    bool SendRelayRouteDirections();
    bool SendMultiMigrationStart(bool* sent);
    bool SendMultiMigrationRankDecision(StationIndex stationIndex, long long timeout, unsigned int* ackId);
    bool SendKickoutNotice(StationIndex stationIndex, unsigned char reason);
    bool SendDummyMessage(StationIndex stationIndex);
    bool SendDummyAck(StationIndex stationIndex);
    void SendConnectionFailureNotice(StationIndex destination, StationIndex reporter, StationIndex failed,
                                     unsigned char reason, unsigned int version);

private:
    common::StepSequenceJob* m_waitingJoinRequestJob;
    common::StepSequenceJob* m_waitingJoinResponseJob;
    common::StepSequenceJob* m_waitingLeaveRequestJob;
    common::StepSequenceJob* m_waitingLeaveResponseJob;
    unsigned int m_unknown24;
    unsigned int m_unknown28;
    common::Time m_unknown30;
    unsigned char m_unknown38;
    unsigned short m_windowNum;
    transport::ReliableSlidingWindow* m_windows;
    unsigned int m_unknown40;
    unsigned int m_bufferSize;
    unsigned char* m_buffer;
    unsigned int m_updateMeshBufferSize;
    unsigned char* m_updateMeshBuffer;
    unsigned char m_updateMeshBufferValid;
    unsigned short m_nextTimeNum;
    common::Time* m_nextTimes;
    unsigned char m_broadcastSupported;
};

}
}
}

#endif
