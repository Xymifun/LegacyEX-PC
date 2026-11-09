#include "nn/nex/DataStoreDDLDeclarations.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/DataStoreClientTemplate.hpp"
#include "nn/nex/DataStoreLogicServerHttpClient.hpp"
#include "nn/nex/DataStoreCompletePostParam.hpp"
#include "nn/nex/DataStoreCompletePostParamV1.hpp"
#include "nn/nex/DataStorePrepareGetParam.hpp"
#include "nn/nex/DataStorePreparePostParam.hpp"
#include "nn/nex/DataStorePreparePostParamV1.hpp"
#include "nn/nex/DataStoreReqPostInfo.hpp"
#include "nn/nex/DataStoreReqPostInfoV1.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/JobDataStoreCheckNotification.hpp"
#include "nn/nex/JobDataStoreGetObject.hpp"
#include "nn/nex/JobDataStoreGetRating.hpp"
#include "nn/nex/JobDataStorePostObject.hpp"
#include "nn/nex/JobDataStoreReceiveNewArrivedObjects.hpp"
#include "nn/nex/JobDataStoreUpdateObject.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum
{
    RESULT_SUCCESS = 0x10001,
    RESULT_CORE_UNKNOWN = static_cast<int>(0x80010001u),
    RESULT_CORE_CALL_FAILED = static_cast<int>(0x8001000Du)
};

}

template <>
JobDataStoreReceiveNewArrivedObjects<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V1> >::GetObjectFunction
    JobDataStoreReceiveNewArrivedObjects<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V1> >::s_getObjectFunction =
        &DataStoreClientInterface::GetObjectV1;

template <>
JobDataStoreReceiveNewArrivedObjects<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V2> >::GetObjectFunction
    JobDataStoreReceiveNewArrivedObjects<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V2> >::s_getObjectFunction =
        &DataStoreClientInterface::GetObjectOrMetaBinary;

DataStoreDDLDeclarations::DataStoreDDLDeclarations() : DDLDeclarations(true)
{
}

PseudoGlobalVariable<DataStoreDDLDeclarations> DataStoreDDLDeclarations::s_declarations;

void DataStoreDDLDeclarations::Register()
{
    s_declarations.Get().RegisterIfRequired();
}

template <typename T>
DataStoreClientTemplate<T>::~DataStoreClientTemplate()
{
}

template <typename T>
bool DataStoreClientTemplate<T>::PostObject(ProtocolCallContext* pContext, const DataStorePreparePostParam& rParam,
                                            uint64_t* pDataId)
{
    return T::PostMetaBinary(pContext, rParam, pDataId);
}

template <typename T>
bool DataStoreClientTemplate<T>::PostObject(ProtocolCallContext* pContext, uint64_t uiDataId,
                                            const DataStorePreparePostParam& rParam)
{
    return T::PostMetaBinaryWithDataId(pContext, uiDataId, rParam);
}

template <typename T>
bool DataStoreClientTemplate<T>::GetRating(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                                           DataStoreRatingInfo* pRatingInfo, uint64_t uiAccessPassword)
{
    return T::GetRating(pContext, rTarget, pRatingInfo, uiAccessPassword);
}

template <typename T>
bool DataStoreClientTemplate<T>::PostObject(ProtocolCallContext* pContext,
                                            const qVector<std::pair<uint64_t, DataStorePreparePostParam> >& rParams,
                                            bool bTransactional, qVector<qResult>* pResults)
{
    return T::PostMetaBinariesWithDataId(pContext, rParams, bTransactional, pResults);
}

template <typename T>
bool DataStoreClientTemplate<T>::PostObject(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                            const DataStorePreparePostParam& rParam, bool bTransactional,
                                            qVector<qResult>* pResults)
{
    return T::PostMetaBinaryWithDataIds(pContext, rDataIds, rParam, bTransactional, pResults);
}

template <typename T>
bool DataStoreClientTemplate<T>::GetRating(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                           qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfos,
                                           qVector<qResult>* pResults, uint64_t uiAccessPassword)
{
    return T::GetRating(pContext, rDataIds, pRatingInfos, pResults, uiAccessPassword);
}

template <typename T>
bool DataStoreClientTemplate<T>::GetRating(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                                           DataStoreRatingInfo* pRatingInfo, DataStoreRatingLog* pRatingLog,
                                           uint64_t uiAccessPassword)
{
    return T::GetRatingWithLog(pContext, rTarget, pRatingInfo, pRatingLog, uiAccessPassword);
}

template <typename T>
bool DataStoreClientTemplate<T>::GetRating(ProtocolCallContext* pContext, uint64_t uiDataId,
                                           qMap<signed char, DataStoreRatingInfo>* pRatings, uint64_t uiAccessPassword)
{
    return GetRatingImpl(pContext, uiDataId, pRatings, uiAccessPassword);
}

template <typename T>
bool DataStoreClientTemplate<T>::GetRating(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                           qVector<qMap<signed char, DataStoreRatingInfo> >* pRatings,
                                           qVector<qResult>* pResults)
{
    return GetRatingImpl(pContext, rDataIds, pRatings, pResults);
}

template <typename T>
bool DataStoreClientTemplate<T>::GetRatingImpl(ProtocolCallContext* pContext, uint64_t uiDataId,
                                               qMap<signed char, DataStoreRatingInfo>* pRatings,
                                               uint64_t uiAccessPassword)
{
    if (!pContext->InitiateCall())
        return false;
    if (!SetDependentRvConnection(pContext))
    {
        pContext->SignalFailure(qResult(RESULT_CORE_UNKNOWN));
        return false;
    }
    JobDataStoreGetRating* pJob = new JobDataStoreGetRating();
    qResult result = pJob->Initialize(pContext->GetCallID(), this, uiDataId, pRatings, uiAccessPassword);
    if (!result)
    {
        pContext->SignalFailure(result);
        pJob->ReleaseRef();
        return result;
    }
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

template <typename T>
bool DataStoreClientTemplate<T>::GetRatingImpl(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                               qVector<qMap<signed char, DataStoreRatingInfo> >* pRatings,
                                               qVector<qResult>* pResults)
{
    if (!pContext->InitiateCall())
        return false;
    if (!SetDependentRvConnection(pContext))
    {
        pContext->SignalFailure(qResult(RESULT_CORE_UNKNOWN));
        return false;
    }
    JobDataStoreGetRating* pJob = new JobDataStoreGetRating();
    qResult result = pJob->Initialize(pContext->GetCallID(), this, rDataIds, pRatings, pResults);
    if (!result)
    {
        pContext->SignalFailure(result);
        pJob->ReleaseRef();
        return result;
    }
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

template <typename T>
bool DataStoreClientTemplate<T>::GetObject(ProtocolCallContext* pContext, const DataStorePrepareGetParam& rParam,
                                           DataStoreGetObjectEventListenerTemplate<uint64_t>* pListener)
{
    return GetObjectImpl<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V2> >(pContext, rParam, pListener, 0, 0, false,
                                                                               0);
}

template <typename T>
bool DataStoreClientTemplate<T>::GetObject(ProtocolCallContext* pContext, const DataStorePrepareGetParam& rParam,
                                           void* pBuffer, uint32_t uiBufferSize, uint32_t* pReceivedSize)
{
    return GetObjectImpl<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V2> >(
        pContext, rParam, 0, static_cast<unsigned char*>(pBuffer), uiBufferSize, false, pReceivedSize);
}

template <typename T>
bool DataStoreClientTemplate<T>::GetObjectOrMetaBinary(ProtocolCallContext* pContext,
                                                       const DataStorePrepareGetParam& rParam,
                                                       DataStoreGetObjectEventListenerTemplate<uint64_t>* pListener)
{
    return GetObjectImpl<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V2> >(pContext, rParam, pListener, 0, 0, true,
                                                                               0);
}

template <typename T>
bool DataStoreClientTemplate<T>::GetObjectV1(ProtocolCallContext* pContext, const DataStorePrepareGetParamV1& rParam,
                                             DataStoreGetObjectEventListenerTemplate<uint32_t>* pListener)
{
    return GetObjectImpl<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V1> >(pContext, rParam, pListener, 0, 0, false,
                                                                               0);
}

template <typename T>
bool DataStoreClientTemplate<T>::PostObject(ProtocolCallContext* pContext, const DataStorePreparePostParam& rParam,
                                            uint64_t* pDataId, DataStorePostObjectEventListener* pListener)
{
    return PostObjectImpl<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V2> >(pContext, rParam, pDataId, pListener, 0,
                                                                                false);
}

template <typename T>
bool DataStoreClientTemplate<T>::PostObject(ProtocolCallContext* pContext, const DataStorePreparePostParam& rParam,
                                            uint64_t* pDataId, const void* pData, bool bCopyData)
{
    return PostObjectImpl<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V2> >(
        pContext, rParam, pDataId, 0, static_cast<const unsigned char*>(pData), bCopyData);
}

template <typename T>
bool DataStoreClientTemplate<T>::PostObjectV1(ProtocolCallContext* pContext, const DataStorePreparePostParamV1& rParam,
                                              uint32_t* pDataId, DataStorePostObjectEventListener* pListener)
{
    return PostObjectImpl<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V1> >(pContext, rParam, pDataId, pListener, 0,
                                                                                false);
}

template <typename T>
bool DataStoreClientTemplate<T>::UpdateObject(ProtocolCallContext* pContext, const DataStorePrepareUpdateParam& rParam,
                                              DataStorePostObjectEventListener* pListener)
{
    return UpdateObjectImpl(pContext, rParam, pListener, 0, false);
}

template <typename T>
bool DataStoreClientTemplate<T>::UpdateObject(ProtocolCallContext* pContext, const DataStorePrepareUpdateParam& rParam,
                                              const void* pData, bool bCopyData)
{
    return UpdateObjectImpl(pContext, rParam, 0, static_cast<const unsigned char*>(pData), bCopyData);
}

template <typename T>
bool DataStoreClientTemplate<T>::CheckNotification(ProtocolCallContext* pContext, const String& rNotificationUrl,
                                                   String* pNextNotificationUrl, uint64_t uiLastDataId,
                                                   uint64_t* pLastDataId, uint32_t uiSequence, uint32_t* pSequence,
                                                   uint32_t uiKey, bool* pbUrlRequestFailed, bool bForceUrlRequest)
{
    return CheckNotificationImpl(pContext, rNotificationUrl, pNextNotificationUrl, uiLastDataId, pLastDataId,
                                 uiSequence, pSequence, uiKey, pbUrlRequestFailed, bForceUrlRequest);
}

template <typename T>
qResult DataStoreClientTemplate<T>::CheckNewArrival(
    ProtocolCallContext* pContext, const unsigned char* pNotificationBuffer,
    DataStoreReceiveNewArrivedObjectsEventListenerTemplate<uint64_t>* pListener, bool* pbHasNext)
{
    return CheckNewArrivalImpl<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V2> >(pContext, pNotificationBuffer,
                                                                                     pListener, pbHasNext);
}

template <typename T>
qResult DataStoreClientTemplate<T>::ReceiveNewArrivedObjects(
    ProtocolCallContext* pContext, const unsigned char* pNotificationBuffer,
    qVector<DataStoreGetObjectEventListenerTemplate<uint64_t>*> getObjectListeners,
    DataStoreReceiveNewArrivedObjectsEventListenerTemplate<uint64_t>* pListener, bool* pbHasNext)
{
    return ReceiveNewArrivedObjectsImpl<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V2> >(
        pContext, pNotificationBuffer, getObjectListeners, pListener, pbHasNext);
}

template <typename T>
qResult DataStoreClientTemplate<T>::ReceiveNewArrivedObjectsV1(
    ProtocolCallContext* pContext, const unsigned char* pNotificationBuffer,
    qVector<DataStoreGetObjectEventListenerTemplate<uint32_t>*> getObjectListeners,
    DataStoreReceiveNewArrivedObjectsEventListenerTemplate<uint32_t>* pListener, bool* pbHasNext)
{
    return ReceiveNewArrivedObjectsImpl<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V1> >(
        pContext, pNotificationBuffer, getObjectListeners, pListener, pbHasNext);
}

template <typename T>
bool DataStoreClientTemplate<T>::GetObjectByInfo(ProtocolCallContext* pContext, const DataStoreReqGetInfo& rReqGetInfo,
                                                 DataStoreGetObjectEventListenerTemplate<uint64_t>* pListener)
{
    return GetObjectByInfoImpl<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V2> >(pContext, rReqGetInfo, pListener,
                                                                                     0, 0, 0);
}

template <typename T>
bool DataStoreClientTemplate<T>::GetObjectByInfo(ProtocolCallContext* pContext, const DataStoreReqGetInfo& rReqGetInfo,
                                                 void* pBuffer, uint32_t uiBufferSize, uint32_t* pReceivedSize)
{
    return GetObjectByInfoImpl<DataStoreApiVersionTraits<DATASTORE_API_VERSION_V2> >(
        pContext, rReqGetInfo, 0, static_cast<unsigned char*>(pBuffer), uiBufferSize, pReceivedSize);
}

template <typename T>
bool DataStoreClientTemplate<T>::SetDependentRvConnection(ProtocolCallContext* pContext) const
{
    if (!this->IsCredentialsNeeded())
        return true;

    Credentials* pCredentials = pContext->GetCredentials() != 0 ? pContext->GetCredentials() : this->GetCredentials();
    if (pCredentials == 0)
        return false;

    EndPoint* pConnection = pCredentials->GetConnection(DATASTORE_PROTOCOL_ID);
    if (pConnection == 0)
        return false;

    pContext->SetDependentConnection(pConnection, pConnection->GetConnectionID());
    return true;
}

template <typename T>
template <typename TTraits>
bool DataStoreClientTemplate<T>::GetObjectImpl(ProtocolCallContext* pContext,
                                               const typename TTraits::TPrepareGetParam& rParam,
                                               typename TTraits::TGetObjectEventListener* pListener,
                                               unsigned char* pBuffer, uint32_t uiBufferSize, bool bOrMetaBinary,
                                               uint32_t* pReceivedSize)
{
    if (!pContext->InitiateCall())
        return false;
    if (!SetDependentRvConnection(pContext))
    {
        pContext->SignalFailure(qResult(RESULT_CORE_UNKNOWN));
        return false;
    }
    JobDataStoreGetObject<TTraits>* pJob = new JobDataStoreGetObject<TTraits>();
    qResult result = pJob->Initialize(pContext->GetCallID(), this, rParam, m_uiThreadPriority, m_bUseHttps,
                                      m_uiTransferBufferSize, m_uiMinimumTransferRate, m_iMinimumTimeout, pListener,
                                      m_uiBufferSize, pBuffer, uiBufferSize, bOrMetaBinary, pReceivedSize, 0);
    if (!result)
    {
        pJob->ReleaseRef();
        return result;
    }
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

template <typename T>
template <typename TTraits>
bool DataStoreClientTemplate<T>::PostObjectImpl(ProtocolCallContext* pContext,
                                                const typename TTraits::TPreparePostParam& rParam,
                                                typename TTraits::TDataId* pDataId,
                                                DataStorePostObjectEventListener* pListener, const unsigned char* pData,
                                                bool bCopyData)
{
    if (!pContext->InitiateCall())
        return false;
    if (!SetDependentRvConnection(pContext))
    {
        pContext->SignalFailure(qResult(RESULT_CORE_UNKNOWN));
        return false;
    }
    JobDataStorePostObject<TTraits>* pJob = new JobDataStorePostObject<TTraits>();
    qResult result = pJob->Initialize(pContext->GetCallID(), this, rParam, m_uiThreadPriority, m_bUseHttps,
                                      m_uiTransferBufferSize, m_uiMinimumTransferRate, m_iMinimumTimeout, pDataId,
                                      pListener, m_uiBufferSize, pData, bCopyData);
    if (!result)
    {
        pJob->ReleaseRef();
        return result;
    }
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

template <typename T>
bool DataStoreClientTemplate<T>::UpdateObjectImpl(ProtocolCallContext* pContext,
                                                  const DataStorePrepareUpdateParam& rParam,
                                                  DataStorePostObjectEventListener* pListener,
                                                  const unsigned char* pData, bool bCopyData)
{
    if (!pContext->InitiateCall())
        return false;
    if (!SetDependentRvConnection(pContext))
    {
        pContext->SignalFailure(qResult(RESULT_CORE_UNKNOWN));
        return false;
    }
    JobDataStoreUpdateObject* pJob = new JobDataStoreUpdateObject();
    qResult result = pJob->Initialize(pContext->GetCallID(), this, rParam, m_uiThreadPriority, m_bUseHttps,
                                      m_uiTransferBufferSize, m_uiMinimumTransferRate, m_iMinimumTimeout, pListener,
                                      m_uiBufferSize, pData, bCopyData);
    if (!result)
    {
        pJob->ReleaseRef();
        return result;
    }
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

template <typename T>
bool DataStoreClientTemplate<T>::CheckNotificationImpl(ProtocolCallContext* pContext, const String& rNotificationUrl,
                                                       String* pNextNotificationUrl, uint64_t uiLastDataId,
                                                       uint64_t* pLastDataId, uint32_t uiSequence, uint32_t* pSequence,
                                                       uint32_t uiKey, bool* pbUrlRequestFailed, bool bForceUrlRequest)
{
    if (!pContext->InitiateCall())
        return false;
    if (!SetDependentRvConnection(pContext))
    {
        pContext->SignalFailure(qResult(RESULT_CORE_UNKNOWN));
        return false;
    }
    JobDataStoreCheckNotification* pJob = new JobDataStoreCheckNotification();
    qResult result = pJob->Initialize(pContext->GetCallID(), this, m_uiThreadPriority, m_iMinimumTimeout,
                                      rNotificationUrl, pNextNotificationUrl, uiLastDataId, pLastDataId, uiSequence,
                                      pSequence, uiKey, pbUrlRequestFailed, bForceUrlRequest);
    if (!result)
    {
        pJob->ReleaseRef();
        return result;
    }
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

template <typename T>
template <typename TTraits>
qResult DataStoreClientTemplate<T>::CheckNewArrivalImpl(
    ProtocolCallContext* pContext, const unsigned char* pNotificationBuffer,
    DataStoreReceiveNewArrivedObjectsEventListenerTemplate<uint64_t>* pListener, bool* pbHasNext)
{
    if (!pContext->InitiateCall())
        return qResult(RESULT_CORE_CALL_FAILED);
    if (!SetDependentRvConnection(pContext))
    {
        pContext->SignalFailure(qResult(RESULT_CORE_UNKNOWN));
        return qResult(RESULT_CORE_UNKNOWN);
    }
    unsigned int uiPrincipalId = this->GetMyPrincipalId();
    if (pContext->GetCredentials() != 0)
        uiPrincipalId = pContext->GetCredentials()->GetPrincipalID();
    JobDataStoreReceiveNewArrivedObjects<TTraits>* pJob = new JobDataStoreReceiveNewArrivedObjects<TTraits>();
    qResult result = pJob->Initialize(pContext->GetCallID(), this, m_uiThreadPriority, uiPrincipalId,
                                      pNotificationBuffer, pListener, pbHasNext);
    if (!result)
    {
        pContext->SignalFailure(result);
        pJob->ReleaseRef();
        return result;
    }
    Scheduler::GetInstance()->Queue(pJob, false);
    return qResult(RESULT_SUCCESS);
}

template <typename T>
template <typename TTraits>
qResult DataStoreClientTemplate<T>::ReceiveNewArrivedObjectsImpl(
    ProtocolCallContext* pContext, const unsigned char* pNotificationBuffer,
    qVector<typename TTraits::TGetObjectEventListener*> getObjectListeners,
    typename TTraits::TReceiveNewArrivedObjectsEventListener* pListener, bool* pbHasNext)
{
    if (!pContext->InitiateCall())
        return qResult(RESULT_CORE_CALL_FAILED);
    if (!SetDependentRvConnection(pContext))
    {
        pContext->SignalFailure(qResult(RESULT_CORE_UNKNOWN));
        return qResult(RESULT_CORE_UNKNOWN);
    }
    unsigned int uiPrincipalId = this->GetMyPrincipalId();
    if (pContext->GetCredentials() != 0)
        uiPrincipalId = pContext->GetCredentials()->GetPrincipalID();
    JobDataStoreReceiveNewArrivedObjects<TTraits>* pJob = new JobDataStoreReceiveNewArrivedObjects<TTraits>();
    qResult result = pJob->Initialize(pContext->GetCallID(), this, this, m_uiThreadPriority, m_bUseHttps, uiPrincipalId,
                                      pNotificationBuffer, getObjectListeners, pListener, pbHasNext);
    if (!result)
    {
        pContext->SignalFailure(result);
        pJob->ReleaseRef();
        return result;
    }
    Scheduler::GetInstance()->Queue(pJob, false);
    return qResult(RESULT_SUCCESS);
}

template <typename T>
template <typename TTraits>
bool DataStoreClientTemplate<T>::GetObjectByInfoImpl(ProtocolCallContext* pContext,
                                                     const DataStoreReqGetInfo& rReqGetInfo,
                                                     typename TTraits::TGetObjectEventListener* pListener,
                                                     unsigned char* pBuffer, uint32_t uiBufferSize,
                                                     uint32_t* pReceivedSize)
{
    if (!pContext->InitiateCall())
        return false;
    if (!SetDependentRvConnection(pContext))
    {
        pContext->SignalFailure(qResult(RESULT_CORE_UNKNOWN));
        return false;
    }
    JobDataStoreGetObject<TTraits>* pJob = new JobDataStoreGetObject<TTraits>();
    typename TTraits::TPrepareGetParam param;
    qResult result = pJob->Initialize(pContext->GetCallID(), this, param, m_uiThreadPriority, m_bUseHttps,
                                      m_uiTransferBufferSize, m_uiMinimumTransferRate, m_iMinimumTimeout, pListener,
                                      m_uiBufferSize, pBuffer, uiBufferSize, false, pReceivedSize, &rReqGetInfo);
    if (!result)
    {
        pJob->ReleaseRef();
        return result;
    }
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

template class DataStoreClientTemplate<DataStoreLogicServerClient>;
template class DataStoreClientTemplate<DataStoreLogicServerHttpClient>;

}
}
