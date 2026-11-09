#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBCONNECTENDPOINT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBCONNECTENDPOINT_HPP

#include <stdint.h>
#include <list>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/SystemComponent.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class ConnectionManager;
class EndPoint;
class StationURL;

class JobConnectEndPoint : public StepSequenceJob
{
public:
    JobConnectEndPoint(Job::JobType jobType, ConnectionManager* pConnectionManager, unsigned int uiContextId,
                       Buffer* pRequestBuffer, Buffer* pResponseBuffer, const qList<StationURL>& rURLs,
                       EndPoint** ppEndPoint, unsigned int uiTimeout, bool bFlag);
    virtual ~JobConnectEndPoint();

    virtual void CancelJob();
    virtual void Trace(uint64_t uiFilter);
    virtual void ProcessInTermination();
    bool ConnectCancelled();
    virtual void TestSuspendedJobState();
    virtual void CheckExceptions();

    void SortURLs();
    void SelectConnectionTechnique();
    void TryConnectViaRouting();
    void TryDirectConnect();
    bool IsRoutedURL(StationURL& rURL);
    void TestCurrentURL();
    void PrepareNATTraversal();
    void RestoreOriginalPort();
    void WaitForNATTraversal();
    bool CheckForUpdatedURLImpl();
    void ResolveCurrentURL();
    void WaitForURLResolution();
    void ReportConnectResult(qResult result);
    void TryCurrentURL();
    void ProcessConnectionResult();
    void ProcessConnectionFailure();
    void CheckForUpdatedURL();
    void SetFinalState(CallContext::State state, qResult result);
    void ReportNATTraversalResult(bool bResult);
    void ConnectionFailed();
    void ConnectionCancelled();
    void ConnectionSucceeded();

private:
    static bool s_bNATTraversalEnabled;
    friend class JobConnectSecureEndPoint;
    SystemComponent::Use m_componentUse;
    EndPoint* m_pEndPoint;
    qResult m_result;
    ConnectionManager* m_pConnectionManager;
    uint64_t m_unresolved98;
    uint32_t m_timeout;
    qList<StepSequenceJob::Step> m_steps;
    uint32_t m_unresolvedB0;
    CallContext m_callContext;
    CallContext m_natTraversalContext;
    uint32_t m_uiCallContextId;
    Buffer* m_pRequestBuffer;
    Buffer* m_pResponseBuffer;
    EndPoint** m_ppEndPointOutput;
    uint64_t m_natTraversalDeadline;
    qList<StationURL> m_urls;
    bool m_isRouting;
    unsigned short m_originalPort;
    qList<StationURL>::iterator m_currentURL;
    bool m_flag194;
    bool m_cancelRequested;
    Job* m_pCancelTarget;
    uint32_t m_uiPingTime;
    bool m_flag1A0;
};

}
}

#endif
