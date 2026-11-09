#include "nn/nex/DataStoreLogicServerHttpClient.hpp"

#include "nn/nex/DataStorePreparePostParam.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"

namespace nn
{
namespace nex
{

DataStoreLogicServerHttpClient::~DataStoreLogicServerHttpClient()
{
}

bool DataStoreLogicServerHttpClient::PrepareGetObject(ProtocolCallContext* pContext,
                                                      const DataStorePrepareGetParam& rParam,
                                                      DataStoreReqGetInfo* pReqGetInfo)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::PrepareGetObject_PrepareRequest(pContext, &message, rParam, pReqGetInfo);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::PrepareGetObject(ProtocolCallContext* pContext,
                                                      const DataStorePrepareGetParamV1& rParam,
                                                      DataStoreReqGetInfoV1* pReqGetInfo)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::PrepareGetObjectV1_PrepareRequest(pContext, &message, rParam, pReqGetInfo);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::PrepareGetObjectOrMetaBinary(ProtocolCallContext* pContext,
                                                                  const DataStorePrepareGetParam& rParam,
                                                                  DataStoreReqGetInfo* pReqGetInfo,
                                                                  DataStoreReqGetAdditionalMeta* pAdditionalMeta)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::PrepareGetObjectOrMetaBinary_PrepareRequest(pContext, &message, rParam, pReqGetInfo,
                                                                         pAdditionalMeta);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::PrepareGetObjectOrMetaBinary(ProtocolCallContext* pContext,
                                                                  const DataStorePrepareGetParamV1& rParam,
                                                                  DataStoreReqGetInfoV1* pReqGetInfo,
                                                                  DataStoreReqGetAdditionalMeta* pAdditionalMeta)
{
    return false;
}

bool DataStoreLogicServerHttpClient::PreparePostObject(ProtocolCallContext* pContext,
                                                       const DataStorePreparePostParam& rParam,
                                                       DataStoreReqPostInfo* pReqPostInfo)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::PreparePostObject_PrepareRequest(pContext, &message, rParam, pReqPostInfo);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::PreparePostObject(ProtocolCallContext* pContext,
                                                       const DataStorePreparePostParamV1& rParam,
                                                       DataStoreReqPostInfoV1* pReqPostInfo)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::PreparePostObjectV1_PrepareRequest(pContext, &message, rParam, pReqPostInfo);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::CompletePostObject(ProtocolCallContext* pContext,
                                                        const DataStoreCompletePostParam& rParam)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::CompletePostObject_PrepareRequest(pContext, &message, rParam);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::CompletePostObject(ProtocolCallContext* pContext,
                                                        const DataStoreCompletePostParamV1& rParam)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::CompletePostObjectV1_PrepareRequest(pContext, &message, rParam);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::CompleteSuspendedPostObject(ProtocolCallContext* pContext,
                                                                 const qVector<uint64_t>& rDataIds)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::CompletePostObjects_PrepareRequest(pContext, &message, rDataIds);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::PrepareUpdateObject(ProtocolCallContext* pContext,
                                                         const DataStorePrepareUpdateParam& rParam,
                                                         DataStoreReqUpdateInfo* pReqUpdateInfo)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::PrepareUpdateObject_PrepareRequest(pContext, &message, rParam, pReqUpdateInfo);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::CompleteUpdateObject(ProtocolCallContext* pContext,
                                                          const DataStoreCompleteUpdateParam& rParam)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::CompleteUpdateObject_PrepareRequest(pContext, &message, rParam);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::DeleteObject(ProtocolCallContext* pContext, const DataStoreDeleteParam& rParam)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::DeleteObject_PrepareRequest(pContext, &message, rParam);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::ChangeMeta(ProtocolCallContext* pContext, const DataStoreChangeMetaParam& rParam)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::ChangeMeta_PrepareRequest(pContext, &message, rParam);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetMeta(ProtocolCallContext* pContext, const DataStoreGetMetaParam& rParam,
                                             DataStoreMetaInfo* pMetaInfo)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetMeta_PrepareRequest(pContext, &message, rParam, pMetaInfo);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetMeta(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                             const DataStoreGetMetaParam& rParam,
                                             qVector<DataStoreMetaInfo>* pMetaInfos, qVector<qResult>* pResults)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetMetas_PrepareRequest(pContext, &message, rDataIds, rParam, pMetaInfos, pResults);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetMeta(ProtocolCallContext* pContext,
                                             const qVector<DataStoreGetMetaParam>& rParams,
                                             qVector<DataStoreMetaInfo>* pMetaInfos, qVector<qResult>* pResults)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetMetasMultipleParam_PrepareRequest(pContext, &message, rParams, pMetaInfos, pResults);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::SearchObject(ProtocolCallContext* pContext, const DataStoreSearchParam& rParam,
                                                  DataStoreSearchResult* pSearchResult)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::SearchObject_PrepareRequest(pContext, &message, rParam, pSearchResult);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::SearchObjectLight(ProtocolCallContext* pContext,
                                                       const DataStoreSearchParam& rParam,
                                                       DataStoreSearchResult* pSearchResult)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::SearchObjectLight_PrepareRequest(pContext, &message, rParam, pSearchResult);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetNotificationUrl(ProtocolCallContext* pContext,
                                                        const DataStoreGetNotificationUrlParam& rParam,
                                                        DataStoreReqGetNotificationUrlInfo* pInfo)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetNotificationUrl_PrepareRequest(pContext, &message, rParam, pInfo);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetNewArrivedNotifications(ProtocolCallContext* pContext,
                                                                const DataStoreGetNewArrivedNotificationsParam& rParam,
                                                                qVector<DataStoreNotification>* pNotifications,
                                                                bool* pbHasNext)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetNewArrivedNotifications_PrepareRequest(pContext, &message, rParam, pNotifications,
                                                                       pbHasNext);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetNewArrivedNotifications(ProtocolCallContext* pContext,
                                                                const DataStoreGetNewArrivedNotificationsParam& rParam,
                                                                qVector<DataStoreNotificationV1>* pNotifications,
                                                                bool* pbHasNext)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetNewArrivedNotificationsV1_PrepareRequest(pContext, &message, rParam, pNotifications,
                                                                         pbHasNext);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::RateObject(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                                                const DataStoreRateObjectParam& rParam,
                                                DataStoreRatingInfo* pRatingInfo)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::RateObject_PrepareRequest(pContext, &message, rTarget, rParam, pRatingInfo != 0,
                                                       pRatingInfo);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetRating(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                                               DataStoreRatingInfo* pRatingInfo, uint64_t uiAccessPassword)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetRating_PrepareRequest(pContext, &message, rTarget, pRatingInfo, uiAccessPassword);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetRating(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                               qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfos,
                                               qVector<qResult>* pResults, uint64_t uiAccessPassword)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetRatings_PrepareRequest(pContext, &message, rDataIds, pRatingInfos, pResults,
                                                       uiAccessPassword);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::ResetRating(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                                                 uint64_t uiUpdatePassword)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::ResetRating_PrepareRequest(pContext, &message, rTarget, uiUpdatePassword);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::ResetRating(ProtocolCallContext* pContext, const uint64_t& rDataId)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    qVector<uint64_t> dataIds;
    dataIds.push_back(rDataId);
    DataStoreProtocolClient::ResetRatings_PrepareRequest(pContext, &message, dataIds, true, 0);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::ResetRating(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                                 bool bTransactional, qVector<qResult>* pResults)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::ResetRatings_PrepareRequest(pContext, &message, rDataIds, bTransactional, pResults);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetSpecificMeta(ProtocolCallContext* pContext,
                                                     const DataStoreGetSpecificMetaParam& rParam,
                                                     qVector<DataStoreSpecificMetaInfo>* pMetaInfos)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetSpecificMeta_PrepareRequest(pContext, &message, rParam, pMetaInfos);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetSpecificMeta(ProtocolCallContext* pContext,
                                                     const DataStoreGetSpecificMetaParamV1& rParam,
                                                     qVector<DataStoreSpecificMetaInfoV1>* pMetaInfos)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetSpecificMetaV1_PrepareRequest(pContext, &message, rParam, pMetaInfos);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::PostMetaBinary(ProtocolCallContext* pContext,
                                                    const DataStorePreparePostParam& rParam, uint64_t* pDataId)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::PostMetaBinary_PrepareRequest(pContext, &message, rParam, pDataId);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::PostMetaBinaryWithDataId(ProtocolCallContext* pContext, uint64_t uiDataId,
                                                              const DataStorePreparePostParam& rParam)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::PostMetaBinaryWithDataId_PrepareRequest(pContext, &message, uiDataId, rParam);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::PostMetaBinaryWithDataIds(ProtocolCallContext* pContext,
                                                               const qVector<uint64_t>& rDataIds,
                                                               const DataStorePreparePostParam& rParam,
                                                               bool bTransactional, qVector<qResult>* pResults)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    qVector<DataStorePreparePostParam> params;
    params.push_back(rParam);
    DataStoreProtocolClient::PostMetaBinariesWithDataId_PrepareRequest(pContext, &message, rDataIds, params,
                                                                       bTransactional, pResults);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::PostMetaBinariesWithDataId(
    ProtocolCallContext* pContext, const qVector<std::pair<uint64_t, DataStorePreparePostParam> >& rParams,
    bool bTransactional, qVector<qResult>* pResults)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    qVector<uint64_t> dataIds;
    qVector<DataStorePreparePostParam> postParams;
    for (qVector<std::pair<uint64_t, DataStorePreparePostParam> >::const_iterator it = rParams.begin();
         it != rParams.end(); ++it)
    {
        dataIds.push_back(it->first);
        postParams.push_back(it->second);
    }
    DataStoreProtocolClient::PostMetaBinariesWithDataId_PrepareRequest(pContext, &message, dataIds, postParams,
                                                                       bTransactional, pResults);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::TouchObject(ProtocolCallContext* pContext, const DataStoreTouchObjectParam& rParam)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::TouchObject_PrepareRequest(pContext, &message, rParam);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetRatingWithLog(ProtocolCallContext* pContext,
                                                      const DataStoreRatingTarget& rTarget,
                                                      DataStoreRatingInfo* pRatingInfo, DataStoreRatingLog* pRatingLog,
                                                      uint64_t uiAccessPassword)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetRatingWithLog_PrepareRequest(pContext, &message, rTarget, pRatingInfo, pRatingLog,
                                                             uiAccessPassword);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetPersistenceInfo(ProtocolCallContext* pContext, uint32_t uiOwnerId,
                                                        uint16_t uiPersistenceSlotId, DataStorePersistenceInfo* pInfo)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetPersistenceInfo_PrepareRequest(pContext, &message, uiOwnerId, uiPersistenceSlotId,
                                                               pInfo);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetPersistenceInfo(ProtocolCallContext* pContext, uint32_t uiOwnerId,
                                                        qVector<uint16_t> persistenceSlotIds,
                                                        qVector<DataStorePersistenceInfo>* pInfos,
                                                        qVector<qResult>* pResults)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetPersistenceInfos_PrepareRequest(pContext, &message, uiOwnerId, persistenceSlotIds,
                                                                pInfos, pResults);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::PerpetuateObject(ProtocolCallContext* pContext, uint16_t uiPersistenceSlotId,
                                                      uint64_t uiDataId, bool bDeleteLastObject)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::PerpetuateObject_PrepareRequest(pContext, &message, uiPersistenceSlotId, uiDataId,
                                                             bDeleteLastObject);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::UnperpetuateObject(ProtocolCallContext* pContext, uint16_t uiPersistenceSlotId,
                                                        bool bDeleteLastObject)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::UnperpetuateObject_PrepareRequest(pContext, &message, uiPersistenceSlotId,
                                                               bDeleteLastObject);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetPasswordInfo(ProtocolCallContext* pContext, uint64_t uiDataId,
                                                     DataStorePasswordInfo* pPasswordInfo)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetPasswordInfo_PrepareRequest(pContext, &message, uiDataId, pPasswordInfo);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetPasswordInfo(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                                     qVector<DataStorePasswordInfo>* pPasswordInfos,
                                                     qVector<qResult>* pResults)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetPasswordInfos_PrepareRequest(pContext, &message, rDataIds, pPasswordInfos, pResults);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

bool DataStoreLogicServerHttpClient::GetObjectInfo(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                                   qVector<DataStoreReqGetInfo>* pReqGetInfos,
                                                   qVector<qResult>* pResults)
{
    if (m_uiPrincipalId == 0)
        return false;
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, m_protocolClient.GetProtocolID(), Protocol::COMMAND_REQUEST);
    DataStoreProtocolClient::GetObjectInfos_PrepareRequest(pContext, &message, rDataIds, pReqGetInfos, pResults);
    return m_hppClient.Request(pContext, &m_protocolClient, *message.GetBuffer(), m_uiUnknown18, m_uiPrincipalId,
                               m_password, m_accessKey, m_hppUrl, m_iTimeout, m_priorityType);
}

}
}
