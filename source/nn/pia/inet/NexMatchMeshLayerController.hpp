#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHMESHLAYERCONTROLLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHMESHLAYERCONTROLLER_HPP

#include "nn/nex/NotificationEventHandler.hpp"
#include "nn/pia/Common.hpp"
#include "nn/pia/common/Job.hpp"
#include "nn/pia/inet/SessionParticipantInfo.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn
{
namespace nex
{
class NgsBridgeInterface;
class NotificationEvent;
}
}
namespace nn
{
namespace pia
{
namespace session
{
class IMatchmakeSession;
}
}
}

namespace nn
{
namespace pia
{
namespace inet
{

class NexMatchMeshLayerController : public session::MeshLayerController
{
public:
    enum
    {
        cSessionIdArraySize = 4
    };

    class NexNotificationEventHandler : public nex::NotificationEventHandler
    {
    public:
        NexNotificationEventHandler();
        virtual ~NexNotificationEventHandler();
        virtual void ProcessNotificationEvent(const nex::NotificationEvent& event);

        void Cleanup();
        void AllClearSessionId();
        void AllClearIgnoredSessionId();
        bool IsUnregisterGathering() const;
        Result SaveSessionId(unsigned int sessionId);
        bool CheckSessionId(unsigned int sessionId) const;
        bool IsIgnoredSessionId(unsigned int sessionId) const;
        void AddIgnoredSessionId(unsigned int sessionId);
        void RemoveIgnoredSessionId(unsigned int sessionId);
        void ClearSessionId(unsigned int sessionId);
        unsigned int GetSessionId(unsigned int index) const;
        unsigned int GetSessionIdArraySize() const;
        SessionParticipantInfo* GetSessionParticipantInfo(unsigned int sessionId);
        const SessionParticipantInfo* GetSessionParticipantInfo(unsigned int sessionId) const;
        unsigned short GetMatchmakeSessionTotalStations(unsigned int sessionId) const;
        unsigned short GetMatchmakeSessionTotalParticipants(unsigned int sessionId) const;
        bool IsParticipated(unsigned int sessionId, unsigned long long principalId) const;
        void AddParticipantPrincipalId(unsigned int sessionId, unsigned long long principalId,
                                       unsigned long long stationId, unsigned char participantType);
        void RemoveParticipantPrincipalId(unsigned int sessionId, unsigned long long principalId,
                                          short participationEvent);
        void SetLeaving(unsigned int sessionId, unsigned long long principalId);
        bool IsLeaving(unsigned int sessionId, unsigned long long principalId) const;

    private:
        enum
        {
            cIgnoredSessionIdNum = 8
        };

        class IgnoredSessionIdBuffer
        {
        public:
            IgnoredSessionIdBuffer() : m_num(0), m_head(0) {}

            unsigned int m_num;
            unsigned int m_head;
            unsigned int m_sessionId[cIgnoredSessionIdNum];
        };

        bool m_unregisterGathering;
        SessionParticipantInfo m_sessionParticipantInfo[cSessionIdArraySize];
        IgnoredSessionIdBuffer m_ignoredSessionId;
    };

    NexMatchMeshLayerController();
    virtual ~NexMatchMeshLayerController();
    virtual Result Startup(const session::Session::StartupSetting& setting);
    virtual void Cleanup();
    virtual Result MeshStartup(session::IMatchmakeSession* matchmakeSession, bool skipBandwidthCheck);
    virtual void MeshCleanup();
    virtual Result CheckConnectedStatus();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned int GetMatchmakeSessionTotalStations() const;
    virtual unsigned short GetMatchmakeSessionTotalParticipants() const;
    virtual bool IsParticipated(unsigned int sessionId, unsigned long long stationId) const;
    virtual bool IsJoinedUnknownSession(unsigned int sessionId, unsigned int bufferSessionId);
    virtual void SetLeaveFlag();
    virtual bool IsAllStationJoined(unsigned int sessionId);

    Result StartNatSession();
    common::Job::ExecuteKind WaitStartNatSession(Result* result);
    Result CancelStartNatSession();
    Result CheckConnectedStatusCore(nex::NgsBridgeInterface* ngsInterface);
    bool CheckSessionId(unsigned int sessionId);
    Result SaveSessionId(unsigned int sessionId);
    void ClearSessionId(unsigned int sessionId);
    unsigned int GetSessionId(unsigned int index) const;
    unsigned int GetSessionIdArraySize() const;
    const SessionParticipantInfo* GetSessionParticipantInfo(unsigned int sessionId) const;
    unsigned int GetMatchmakeSessionTotalStations(unsigned int sessionId) const;
    unsigned int GetMatchmakeSessionTotalParticipants(unsigned int sessionId) const;
    void SetLeaving(unsigned int sessionId, unsigned long long principalId);
    bool IsLeaving(unsigned int sessionId, unsigned long long principalId) const;
    void CleanupEventHandler();
    nex::NgsBridgeInterface* GetNgsInterface(unsigned char index);
    unsigned char GetNgsInterfaceIndexBoundNexFacade() const;

private:
    nex::NgsBridgeInterface* m_nexFacadeNgsInterface;
    nex::NgsBridgeInterface* m_ngsInterface;
    unsigned char m_ngsInterfaceIndexBoundNexFacade;
    NexNotificationEventHandler* m_notificationEventHandler;
};

}
}
}

#endif
