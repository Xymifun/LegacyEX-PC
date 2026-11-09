#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTOREUPDATEOBJECT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTOREUPDATEOBJECT_HPP

#include "nn/nex/DataStorePrepareUpdateParam.hpp"
#include "nn/nex/DataStoreReqUpdateInfo.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class DataStoreFileServerClient;
class DataStoreLogicServerClientInterface;
class DataStorePostHttpEventListener;
class DataStorePostObjectEventListener;

class JobDataStoreUpdateObject : public StepSequenceJob
{
public:
    JobDataStoreUpdateObject();
    virtual ~JobDataStoreUpdateObject();

    virtual qResult Initialize(unsigned int uiCallId, DataStoreLogicServerClientInterface* pClient,
                               const DataStorePrepareUpdateParam& rParam, unsigned int uiThreadPriority, bool bUseHttps,
                               unsigned int uiTransferBufferSize, unsigned int uiMinimumTransferRate,
                               int iMinimumTimeout, DataStorePostObjectEventListener* pListener,
                               unsigned int uiBufferSize, const unsigned char* pData, bool bCopyData);

    void CompleteJob(const qResult& rResult);
    void StepLogicServerPrepareUpdateObject();
    void StepFileServerPostObject();
    void StepLogicServerCompleteUpdateObject();
    void StepWaitingLogicServerCompleteUpdateObject();

private:
    enum Result
    {
        RESULT_SUCCESS = 0x10001,
        RESULT_CORE_OPERATION_ABORTED = static_cast<int>(0x80010004u),
        RESULT_CORE_CALL_FAILED = static_cast<int>(0x8001000Du)
    };

    enum
    {
        MILLISECONDS_PER_SECOND = 1000,
        TIMEOUT_INFINITE = 0x7FFFFFFF
    };

    unsigned int m_uiCallId;
    DataStoreLogicServerClientInterface* m_pClient;
    DataStoreFileServerClient* m_pFileServerClient;
    bool m_bUseHttps;
    unsigned int m_uiTransferBufferSize;
    unsigned int m_uiThreadPriority;
    unsigned int m_uiMinimumTransferRate;
    int m_iMinimumTimeout;
    DataStorePostObjectEventListener* m_pListener;
    unsigned int m_uiBufferSize;
    ProtocolCallContext m_context;
    DataStorePrepareUpdateParam m_prepareParam;
    DataStoreReqUpdateInfo m_reqUpdateInfo;
    unsigned char* m_pCopiedData;
    const unsigned char* m_pData;
    qResult m_postResult;
    DataStorePostHttpEventListener* m_pHttpListener;
};

}
}

#endif
