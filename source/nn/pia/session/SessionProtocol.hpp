#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_PROTOCOL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_PROTOCOL_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/StationAddress.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class ReceivedMessageAccessor;
class ReliableSlidingWindow;

}
}
}

namespace nn
{
namespace pia
{
namespace session
{

class SessionProtocol : public transport::Protocol
{
public:
    enum
    {
        cProtocolType = 0x94
    };

    enum MessageType
    {
        cMessageTypeNextSessionId = 3,
        cMessageTypePreparedForMigrateSession = 4,
        cMessageTypeInvitationAsCompanion = 6,
        cMessageTypeAnswerToInvitation = 7,
        cMessageTypeDestroyInvitation = 8,
        cMessageTypeAnswerToDestroyInvitation = 9,
        cMessageTypeLeaveMeshInvitation = 10,
        cMessageTypeAnswerToLeaveMeshInvitation = 11,
        cMessageTypeCompletionForMigrateSession = 0x13,
        cMessageTypeCompletionInvitation = 0x14,
        cMessageTypeAnswerToCompletionInvitation = 0x15,
        cMessageTypeCompletionFailureResponse = 0x16
    };

    SessionProtocol();
    virtual ~SessionProtocol();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned short GetProtocolType() const;
    virtual Result StartupWithStationAddress(const common::StationAddress& address);
    virtual void CleanupWithStationAddress();
    virtual Result Dispatch();
    virtual Result UpdateProtocolEvent(const transport::ProtocolEvent& event);

    Result Initialize(unsigned short stationNum);
    void Finalize();
    unsigned long long DeserializeStationId(const unsigned char* buffer);
    unsigned int SerializeStationId(unsigned long long stationId, unsigned char* buffer, unsigned int bufferSize);
    Result SendPreparedForMigrateSession(unsigned char processType, unsigned long long stationId,
                                         unsigned int sessionId, bool value);
    Result SendNextSessionId(unsigned char processType, unsigned int sessionId, unsigned int nextSessionId,
                             unsigned long long matchmakeOwnerUniqueId, const unsigned char* applicationData,
                             unsigned int applicationDataSize, unsigned long long* stationIds,
                             unsigned short stationIdNum);
    Result SendAnswerToInvitation(unsigned char processType, unsigned long long stationId, bool result);
    Result SendAnswerToDestroyInvitation(unsigned char processType, unsigned long long stationId, bool result);
    Result SendAnswerToLeaveMeshInvitation(unsigned char processType, unsigned long long stationId, bool result);
    Result SendAnswerToCompletionInvitation(unsigned char processType, unsigned long long stationId, bool result);
    Result SendInvitationAsCompanion(unsigned char processType, unsigned long long* stationIds,
                                     unsigned short stationIdNum, unsigned long long stationId,
                                     unsigned long long* companionStationIds, unsigned short companionStationIdNum);
    Result SendDestroyInvitation(unsigned char processType, unsigned long long* stationIds, unsigned short stationIdNum,
                                 unsigned long long stationId, unsigned long long* companionStationIds,
                                 unsigned short companionStationIdNum);
    Result SendLeaveMeshInvitation(unsigned char processType, unsigned long long* stationIds,
                                   unsigned short stationIdNum, unsigned long long stationId,
                                   unsigned long long* companionStationIds, unsigned short companionStationIdNum);
    Result SendCompletionInvitation(unsigned char processType, unsigned long long* stationIds,
                                    unsigned short stationIdNum, unsigned long long stationId,
                                    unsigned long long* companionStationIds, unsigned short companionStationIdNum);
    Result SendCompletionFailureResponse(unsigned char processType, unsigned long long stationId);
    Result SendCompletionForMigrateSession(unsigned char processType, unsigned long long stationId, bool result);
    Result CheckIsAbleToSendReliableMessage(StationIndex stationIndex, unsigned int size);
    void ParseNextSessionId(const transport::ReceivedMessageAccessor& message);
    void ParsePreparedForMigrateSession(const transport::ReceivedMessageAccessor& message);
    void ParseInvitationAsCompanion(const transport::ReceivedMessageAccessor& message);
    void ParseDestroyInvitation(const transport::ReceivedMessageAccessor& message);
    void ParseLeaveMeshInvitation(const transport::ReceivedMessageAccessor& message);
    void ParseAnswerToInvitation(const transport::ReceivedMessageAccessor& message);
    void ParseCompletionForMigrateSession(const transport::ReceivedMessageAccessor& message);
    void ParseCompletionFailureResponse(const transport::ReceivedMessageAccessor& message);
    void ParseCompletionInvitation(const transport::ReceivedMessageAccessor& message);
    void ParseHelper(const transport::ReceivedMessageAccessor& message);

private:
    Result SendAnswerToInvitationCore(MessageType messageType, unsigned char processType, unsigned long long stationId,
                                      bool result);
    Result SendInvitationCore(MessageType messageType, unsigned char processType, unsigned long long* stationIds,
                              unsigned short stationIdNum, unsigned long long stationId,
                              unsigned long long* companionStationIds, unsigned short companionStationIdNum);

    int m_stationIndex;
    unsigned long long m_stationId;
    unsigned short m_windowNum;
    transport::ReliableSlidingWindow* m_windows;
    unsigned short m_dispatchWindowIndex;
    unsigned char m_messageBuffer[256];
};

}
}
}

#endif
