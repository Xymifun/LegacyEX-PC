#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBCONNECTSECUREENDPOINT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBCONNECTSECUREENDPOINT_HPP

#include <stdint.h>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/ConnectionData.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/Job.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/UserContext.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class BitStream;
class Buffer;
class JobConnectEndPoint;
class SecureEndPoint;
class Ticket;

class JobConnectSecureEndPoint : public Job
{
public:
    JobConnectSecureEndPoint(SecureEndPoint* pSecureEndPoint, const StationURL* pStationURL, Buffer* pRequestData,
                             Buffer* pResponseData, EndPoint::ConnectionCallback pCallback,
                             const UserContext& rUserContext, unsigned int uiTimeout);
    virtual ~JobConnectSecureEndPoint();

    virtual void Execute();
    virtual void CancelJob();

    qResult GetResult() const { return m_result; }

    void ParseURL();
    bool ReadyToPerformConnect();
    void CheckConnectionData();
    void RequestConnectionData();
    void InitializeBufferRequest();
    void BuildListURLs(qList<StationURL>* pURLs);
    void ConnectCompletion(CallContext* pCallContext);
    void PerformConnect();
    void ProcessConnectResult();
    void ExecuteStep();

    static void ConnectCompletionCallback(CallContext* pCallContext, const UserContext* pUserContext);

private:
    enum Step
    {
        STEP_PARSE_URL = 0,
        STEP_REQUEST_CONNECTION_DATA = 1,
        STEP_CHECK_CONNECTION_DATA = 2,
        STEP_PERFORM_CONNECT = 3,
        STEP_PROCESS_CONNECT_RESULT = 4,
        STEP_COMPLETE = 5
    };

    unsigned int m_uiStep;
    ProtocolCallContext m_ticketContext;
    Ticket* m_pTicket;
    BitStream* m_pRequestStream;
    BitStream* m_pResponseStream;
    unsigned int m_uiNonce;
    ProtocolCallContext m_connectionDataContext;
    CallContext m_connectContext;
    SecureEndPoint* m_pSecureEndPoint;
    EndPoint* m_pEndPoint;
    qList<ConnectionData> m_connectionData;
    StationURL m_parsedURL;
    qResult m_result;
    StationURL m_stationURL;
    Buffer* m_pResponseData;
    EndPoint::ConnectionCallback m_pCallback;
    UserContext m_userContext;
    Time m_deadline;
    unsigned int m_uiPrincipalID;
    unsigned int m_uiConnectionID;
    JobConnectEndPoint* m_pConnectJob;
};

}
}

#endif
