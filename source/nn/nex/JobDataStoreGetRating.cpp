#include "nn/nex/JobDataStoreGetRating.hpp"

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DataStoreLogicServerClientInterface.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

JobDataStoreGetRating::JobDataStoreGetRating()
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_uiCallId(0),
      m_pClient(0),
      m_dataIds(),
      m_ratingInfos(),
      m_pRatingsList(0),
      m_pRatings(0),
      m_pResults(0),
      m_results(),
      m_bSingleTarget(false),
      m_getRatingContext(),
      m_uiAccessPassword(0)
{
}

JobDataStoreGetRating::~JobDataStoreGetRating()
{
}

qResult JobDataStoreGetRating::Initialize(unsigned int uiCallId, DataStoreLogicServerClientInterface* pClient,
                                          const uint64_t& uiDataId, qMap<signed char, DataStoreRatingInfo>* pRatings,
                                          uint64_t uiAccessPassword)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_uiCallId = uiCallId;
    m_dataIds.push_back(uiDataId);
    m_pRatings = pRatings;
    m_pClient = pClient;
    m_pResults = 0;
    m_bSingleTarget = true;
    m_uiAccessPassword = uiAccessPassword;
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    m_getRatingContext.SetCredentials(static_cast<ProtocolCallContext*>(pCallContext)->GetCredentials());
    SetStep(Step(static_cast<Step::Function>(&JobDataStoreGetRating::StepGetRating), L""));
    return qResult(true);
}

qResult JobDataStoreGetRating::Initialize(unsigned int uiCallId, DataStoreLogicServerClientInterface* pClient,
                                          const qVector<uint64_t>& rDataIds,
                                          qVector<qMap<signed char, DataStoreRatingInfo> >* pRatings,
                                          qVector<qResult>* pResults)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_uiCallId = uiCallId;
    m_dataIds = rDataIds;
    m_pResults = pResults;
    m_pRatingsList = pRatings;
    m_pClient = pClient;
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    m_getRatingContext.SetCredentials(static_cast<ProtocolCallContext*>(pCallContext)->GetCredentials());
    SetStep(Step(static_cast<Step::Function>(&JobDataStoreGetRating::StepGetRating), L""));
    return qResult(true);
}

void JobDataStoreGetRating::CompleteJob(const qResult& rResult)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        m_getRatingContext.Cancel(CallContext::CallCanceled);
        SetToComplete();
        return;
    }

    if (rResult)
    {
        if (m_pResults != 0)
            *m_pResults = m_results;

        pCallContext->SignalSuccess(rResult);
        SetToComplete();
        return;
    }

    pCallContext->SignalFailure(rResult);
    SetToComplete();
}

void JobDataStoreGetRating::StepGetRating()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    m_getRatingContext.Reset();
    if (!m_pClient->GetRating(&m_getRatingContext, m_dataIds, &m_ratingInfos, &m_results, m_uiAccessPassword))
    {
        CompleteJob(qResult(RESULT_CORE_CALL_FAILED));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_getRatingContext.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(&m_getRatingContext,
                           new Step(static_cast<Step::Function>(&JobDataStoreGetRating::StepConvertType), L""));
}

void JobDataStoreGetRating::StepConvertType()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    if (m_getRatingContext.GetState() != CallContext::CallSucceeded)
    {
        CompleteJob(m_getRatingContext.GetResult());
        return;
    }

    if (m_bSingleTarget)
    {
        if (m_results.size() != 1)
        {
            CompleteJob(qResult(RESULT_DATASTORE_UNKNOWN));
            return;
        }

        if (!m_results[0])
        {
            CompleteJob(m_results[0]);
            return;
        }

        if (m_pRatings != 0 && m_ratingInfos.size() != 0)
        {
            m_pRatings->clear();
            for (qVector<DataStoreRatingInfoWithSlot>::const_iterator it = m_ratingInfos[0].begin();
                 it != m_ratingInfos[0].end(); ++it)
            {
                m_pRatings->insert(std::make_pair(it->m_iSlot, it->m_Rating));
            }
        }
    }
    else if (m_pRatingsList != 0)
    {
        m_pRatingsList->clear();
        for (unsigned int i = 0; i < m_ratingInfos.size(); ++i)
        {
            qMap<signed char, DataStoreRatingInfo> ratings;
            for (qVector<DataStoreRatingInfoWithSlot>::const_iterator it = m_ratingInfos[i].begin();
                 it != m_ratingInfos[i].end(); ++it)
            {
                ratings.insert(std::make_pair(it->m_iSlot, it->m_Rating));
            }

            m_pRatingsList->push_back(ratings);
        }
    }

    CompleteJob(qResult(RESULT_SUCCESS));
}

}
}
