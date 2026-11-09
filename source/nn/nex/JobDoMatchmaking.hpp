#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBDOMATCHMAKING_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBDOMATCHMAKING_HPP

#include "nn/nex/AutoMatchmakeParam.hpp"
#include "nn/nex/MatchmakeSession.hpp"
#include "nn/nex/NotificationEvent.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class MatchmakeExtensionClient;
class NotificationEventFilterInterface;

class JobDoMatchmaking : public StepSequenceJob
{
public:
    JobDoMatchmaking();
    virtual ~JobDoMatchmaking();

    virtual qResult Initialize(unsigned int uiCallId, MatchmakeExtensionClient* pMatchmakeExtensionClient,
                               const AutoMatchmakeParam& rParam, MatchmakeSession* pJoinedSession);

    void CompleteJob(const qResult& rResult);
    void StepRequestMatchmaking();
    void StepWaitingRequestMatchmaking();
    void StepWaitingNotify();
    void StepFindMatchmakeSessionByGatheringIdDetail();
    void StepWaitingFindMatchmakeSessionByGatheringIdDetail();

private:
    unsigned int m_uiCallId;
    MatchmakeExtensionClient* m_pMatchmakeExtensionClient;
    AutoMatchmakeParam m_autoMatchmakeParam;
    MatchmakeSession* m_pJoinedSession;
    ProtocolCallContext m_requestContext;
    ProtocolCallContext m_notificationContext;
    NotificationEventFilterInterface* m_pNotificationFilter;
    NotificationEvent m_notificationEvent;
    unsigned long long m_uiRequestID;
    unsigned int m_uiGatheringID;
    MatchmakeSession m_matchmakeSession;
};

}
}

#endif
