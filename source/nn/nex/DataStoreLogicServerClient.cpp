#include "nn/nex/DataStoreLogicServerClient.hpp"

#include <cafe/sci.h>

#include "nn/nex/Credentials.hpp"
#include "nn/nex/DataStoreChangeMetaParam.hpp"
#include "nn/nex/DataStoreChangeMetaParamV1.hpp"
#include "nn/nex/DataStoreDeleteParam.hpp"
#include "nn/nex/DataStorePrepareGetParam.hpp"
#include "nn/nex/DataStorePreparePostParam.hpp"
#include "nn/nex/DataStorePrepareUpdateParam.hpp"
#include "nn/nex/DataStoreRateObjectParam.hpp"
#include "nn/nex/DataStoreRatingTarget.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace nex
{

void DataStoreLogicServerClient::CreateExtraData(qVector<String>& rExtraData) const
{
    rExtraData.resize(EXTRA_DATA_COUNT, String(L""));
    rExtraData[EXTRA_DATA_PLATFORM] = L"WUP";

    StringStream stream;
    unsigned int uiRegion;
    if (SCIGetPlatformRegion(&uiRegion) == SCI_STATUS_SUCCESS)
    {
        stream.StreamNumber(static_cast<int>(uiRegion));
        rExtraData[EXTRA_DATA_REGION_ID] = stream.GetBuffer();
        stream.Clear();
        switch (uiRegion)
        {
        case PLATFORM_REGION_EUR:
            stream << L"EUR";
            break;
        case PLATFORM_REGION_AUS:
            stream << L"AUS";
            break;
        case PLATFORM_REGION_CHN:
            stream << L"CHN";
            break;
        case PLATFORM_REGION_KOR:
            stream << L"KOR";
            break;
        case PLATFORM_REGION_TWN:
            stream << L"TWN";
            break;
        case PLATFORM_REGION_INVALID:
            stream << L"Invalid";
            break;
        case PLATFORM_REGION_JPN:
            stream << L"JPN";
            break;
        case PLATFORM_REGION_USA:
            stream << L"USA";
            break;
        default:
            break;
        }
        rExtraData[EXTRA_DATA_REGION_NAME] = stream.GetBuffer();
    }

    unsigned int uiCountry = 0;
    if (SCIGetCafeCountry(&uiCountry) == SCI_STATUS_SUCCESS)
    {
        stream.Clear();
        stream.StreamNumber(uiCountry);
        rExtraData[EXTRA_DATA_COUNTRY_ID] = stream.GetBuffer();
        const char* pCountryCode = SCIGetCountryCodeA2(uiCountry);
        if (pCountryCode != NULL)
            rExtraData[EXTRA_DATA_COUNTRY_CODE] = pCountryCode;
    }
}

bool DataStoreLogicServerClient::PrepareGetObject(ProtocolCallContext* pContext, const DataStorePrepareGetParam& rParam,
                                                  DataStoreReqGetInfo* pReqGetInfo)
{
    CreateExtraData(const_cast<qVector<String>&>(rParam.m_ExtraData));
    return m_protocolClient.CallPrepareGetObject(pContext, rParam, pReqGetInfo);
}

bool DataStoreLogicServerClient::PrepareGetObjectOrMetaBinary(ProtocolCallContext* pContext,
                                                              const DataStorePrepareGetParam& rParam,
                                                              DataStoreReqGetInfo* pReqGetInfo,
                                                              DataStoreReqGetAdditionalMeta* pAdditionalMeta)
{
    CreateExtraData(const_cast<qVector<String>&>(rParam.m_ExtraData));
    return m_protocolClient.CallPrepareGetObjectOrMetaBinary(pContext, rParam, pReqGetInfo, pAdditionalMeta);
}

bool DataStoreLogicServerClient::PreparePostObject(ProtocolCallContext* pContext,
                                                   const DataStorePreparePostParam& rParam,
                                                   DataStoreReqPostInfo* pReqPostInfo)
{
    CreateExtraData(const_cast<qVector<String>&>(rParam.m_ExtraData));
    return m_protocolClient.CallPreparePostObject(pContext, rParam, pReqPostInfo);
}

bool DataStoreLogicServerClient::PrepareUpdateObject(ProtocolCallContext* pContext,
                                                     const DataStorePrepareUpdateParam& rParam,
                                                     DataStoreReqUpdateInfo* pReqUpdateInfo)
{
    CreateExtraData(const_cast<qVector<String>&>(rParam.m_ExtraData));
    return m_protocolClient.CallPrepareUpdateObject(pContext, rParam, pReqUpdateInfo);
}

bool DataStoreLogicServerClient::PostMetaBinary(ProtocolCallContext* pContext, const DataStorePreparePostParam& rParam,
                                                uint64_t* pDataId)
{
    return m_protocolClient.CallPostMetaBinary(pContext, rParam, pDataId);
}

bool DataStoreLogicServerClient::SearchObject(ProtocolCallContext* pContext, const DataStoreSearchParam& rParam,
                                              DataStoreSearchResult* pSearchResult)
{
    return m_protocolClient.CallSearchObject(pContext, rParam, pSearchResult);
}

bool DataStoreLogicServerClient::SearchObjectLight(ProtocolCallContext* pContext, const DataStoreSearchParam& rParam,
                                                   DataStoreSearchResult* pSearchResult)
{
    return m_protocolClient.CallSearchObjectLight(pContext, rParam, pSearchResult);
}

bool DataStoreLogicServerClient::GetNotificationUrl(ProtocolCallContext* pContext,
                                                    const DataStoreGetNotificationUrlParam& rParam,
                                                    DataStoreReqGetNotificationUrlInfo* pInfo)
{
    return m_protocolClient.CallGetNotificationUrl(pContext, rParam, pInfo);
}

bool DataStoreLogicServerClient::GetNewArrivedNotifications(ProtocolCallContext* pContext,
                                                            const DataStoreGetNewArrivedNotificationsParam& rParam,
                                                            qVector<DataStoreNotification>* pNotifications,
                                                            bool* pbHasNext)
{
    return m_protocolClient.CallGetNewArrivedNotifications(pContext, rParam, pNotifications, pbHasNext);
}

bool DataStoreLogicServerClient::GetNewArrivedNotifications(ProtocolCallContext* pContext,
                                                            const DataStoreGetNewArrivedNotificationsParam& rParam,
                                                            qVector<DataStoreNotificationV1>* pNotifications,
                                                            bool* pbHasNext)
{
    return m_protocolClient.CallGetNewArrivedNotificationsV1(pContext, rParam, pNotifications, pbHasNext);
}

bool DataStoreLogicServerClient::GetSpecificMeta(ProtocolCallContext* pContext,
                                                 const DataStoreGetSpecificMetaParam& rParam,
                                                 qVector<DataStoreSpecificMetaInfo>* pMetaInfos)
{
    return m_protocolClient.CallGetSpecificMeta(pContext, rParam, pMetaInfos);
}

bool DataStoreLogicServerClient::GetSpecificMeta(ProtocolCallContext* pContext,
                                                 const DataStoreGetSpecificMetaParamV1& rParam,
                                                 qVector<DataStoreSpecificMetaInfoV1>* pMetaInfos)
{
    return m_protocolClient.CallGetSpecificMetaV1(pContext, rParam, pMetaInfos);
}

bool DataStoreLogicServerClient::TouchObject(ProtocolCallContext* pContext, const DataStoreTouchObjectParam& rParam)
{
    return m_protocolClient.CallTouchObject(pContext, rParam);
}

bool DataStoreLogicServerClient::GetPersistenceInfo(ProtocolCallContext* pContext, uint32_t uiOwnerId,
                                                    qVector<uint16_t> persistenceSlotIds,
                                                    qVector<DataStorePersistenceInfo>* pInfos,
                                                    qVector<qResult>* pResults)
{
    return m_protocolClient.CallGetPersistenceInfos(pContext, uiOwnerId, persistenceSlotIds, pInfos, pResults);
}

bool DataStoreLogicServerClient::PostMetaBinaryWithDataId(ProtocolCallContext* pContext, uint64_t uiDataId,
                                                          const DataStorePreparePostParam& rParam)
{
    return m_protocolClient.CallPostMetaBinaryWithDataId(pContext, uiDataId, rParam);
}

bool DataStoreLogicServerClient::GetRating(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                                           DataStoreRatingInfo* pRatingInfo, uint64_t uiAccessPassword)
{
    return m_protocolClient.CallGetRating(pContext, rTarget, pRatingInfo, uiAccessPassword);
}

bool DataStoreLogicServerClient::GetRating(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                           qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfos,
                                           qVector<qResult>* pResults, uint64_t uiAccessPassword)
{
    return m_protocolClient.CallGetRatings(pContext, rDataIds, pRatingInfos, pResults, uiAccessPassword);
}

bool DataStoreLogicServerClient::GetRatingWithLog(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                                                  DataStoreRatingInfo* pRatingInfo, DataStoreRatingLog* pRatingLog,
                                                  uint64_t uiAccessPassword)
{
    return m_protocolClient.CallGetRatingWithLog(pContext, rTarget, pRatingInfo, pRatingLog, uiAccessPassword);
}

bool DataStoreLogicServerClient::ResetRating(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                                             uint64_t uiUpdatePassword)
{
    return m_protocolClient.CallResetRating(pContext, rTarget, uiUpdatePassword);
}

bool DataStoreLogicServerClient::ResetRating(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                             bool bTransactional, qVector<qResult>* pResults)
{
    return m_protocolClient.CallResetRatings(pContext, rDataIds, bTransactional, pResults);
}

bool DataStoreLogicServerClient::PerpetuateObject(ProtocolCallContext* pContext, uint16_t uiPersistenceSlotId,
                                                  uint64_t uiDataId, bool bDeleteLastObject)
{
    return m_protocolClient.CallPerpetuateObject(pContext, uiPersistenceSlotId, uiDataId, bDeleteLastObject);
}

bool DataStoreLogicServerClient::UnperpetuateObject(ProtocolCallContext* pContext, uint16_t uiPersistenceSlotId,
                                                    bool bDeleteLastObject)
{
    return m_protocolClient.CallUnperpetuateObject(pContext, uiPersistenceSlotId, bDeleteLastObject);
}

bool DataStoreLogicServerClient::GetPersistenceInfo(ProtocolCallContext* pContext, uint32_t uiOwnerId,
                                                    uint16_t uiPersistenceSlotId, DataStorePersistenceInfo* pInfo)
{
    return m_protocolClient.CallGetPersistenceInfo(pContext, uiOwnerId, uiPersistenceSlotId, pInfo);
}

bool DataStoreLogicServerClient::RateObject(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                                            const DataStoreRateObjectParam& rParam, DataStoreRatingInfo* pRatingInfo)
{
    return m_protocolClient.CallRateObject(pContext, rTarget, rParam, pRatingInfo != NULL, pRatingInfo);
}

bool DataStoreLogicServerClient::PrepareGetObject(ProtocolCallContext* pContext,
                                                  const DataStorePrepareGetParamV1& rParam,
                                                  DataStoreReqGetInfoV1* pReqGetInfo)
{
    return m_protocolClient.CallPrepareGetObjectV1(pContext, rParam, pReqGetInfo);
}

bool DataStoreLogicServerClient::PrepareGetObjectOrMetaBinary(ProtocolCallContext* pContext,
                                                              const DataStorePrepareGetParamV1& rParam,
                                                              DataStoreReqGetInfoV1* pReqGetInfo,
                                                              DataStoreReqGetAdditionalMeta* pAdditionalMeta)
{
    return false;
}

bool DataStoreLogicServerClient::PreparePostObject(ProtocolCallContext* pContext,
                                                   const DataStorePreparePostParamV1& rParam,
                                                   DataStoreReqPostInfoV1* pReqPostInfo)
{
    return m_protocolClient.CallPreparePostObjectV1(pContext, rParam, pReqPostInfo);
}

bool DataStoreLogicServerClient::CompletePostObject(ProtocolCallContext* pContext,
                                                    const DataStoreCompletePostParam& rParam)
{
    return m_protocolClient.CallCompletePostObject(pContext, rParam);
}

bool DataStoreLogicServerClient::CompletePostObject(ProtocolCallContext* pContext,
                                                    const DataStoreCompletePostParamV1& rParam)
{
    return m_protocolClient.CallCompletePostObjectV1(pContext, rParam);
}

bool DataStoreLogicServerClient::CompleteSuspendedPostObject(ProtocolCallContext* pContext,
                                                             const qVector<uint64_t>& rDataIds)
{
    return m_protocolClient.CallCompletePostObjects(pContext, rDataIds);
}

bool DataStoreLogicServerClient::CompleteUpdateObject(ProtocolCallContext* pContext,
                                                      const DataStoreCompleteUpdateParam& rParam)
{
    return m_protocolClient.CallCompleteUpdateObject(pContext, rParam);
}

bool DataStoreLogicServerClient::DeleteObject(ProtocolCallContext* pContext, const DataStoreDeleteParam& rParam)
{
    return m_protocolClient.CallDeleteObject(pContext, rParam);
}

bool DataStoreLogicServerClient::ChangeMeta(ProtocolCallContext* pContext, const DataStoreChangeMetaParam& rParam)
{
    return m_protocolClient.CallChangeMeta(pContext, rParam);
}

bool DataStoreLogicServerClient::ChangeMeta(ProtocolCallContext* pContext, const DataStoreChangeMetaParamV1& rParam)
{
    return m_protocolClient.CallChangeMetaV1(pContext, rParam);
}

bool DataStoreLogicServerClient::GetMeta(ProtocolCallContext* pContext, const DataStoreGetMetaParam& rParam,
                                         DataStoreMetaInfo* pMetaInfo)
{
    return m_protocolClient.CallGetMeta(pContext, rParam, pMetaInfo);
}

bool DataStoreLogicServerClient::GetMeta(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                         const DataStoreGetMetaParam& rParam, qVector<DataStoreMetaInfo>* pMetaInfos,
                                         qVector<qResult>* pResults)
{
    return m_protocolClient.CallGetMetas(pContext, rDataIds, rParam, pMetaInfos, pResults);
}

bool DataStoreLogicServerClient::GetMeta(ProtocolCallContext* pContext, const qVector<DataStoreGetMetaParam>& rParams,
                                         qVector<DataStoreMetaInfo>* pMetaInfos, qVector<qResult>* pResults)
{
    return m_protocolClient.CallGetMetasMultipleParam(pContext, rParams, pMetaInfos, pResults);
}

bool DataStoreLogicServerClient::GetPasswordInfo(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                                 qVector<DataStorePasswordInfo>* pPasswordInfos,
                                                 qVector<qResult>* pResults)
{
    return m_protocolClient.CallGetPasswordInfos(pContext, rDataIds, pPasswordInfos, pResults);
}

bool DataStoreLogicServerClient::GetObjectInfo(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                               qVector<DataStoreReqGetInfo>* pReqGetInfos, qVector<qResult>* pResults)
{
    return m_protocolClient.CallGetObjectInfos(pContext, rDataIds, pReqGetInfos, pResults);
}

bool DataStoreLogicServerClient::IsCredentialsNeeded() const
{
    return true;
}

void DataStoreLogicServerClient::UpdateProtocolsDefaultCredentials(Credentials* pCredentials)
{
    m_protocolClient.SetDefaultCredentials(pCredentials);
}

DataStoreLogicServerClient::~DataStoreLogicServerClient()
{
    if (m_bBound)
        Unbind();
}

bool DataStoreLogicServerClient::Bind(Credentials* pCredentials)
{
    if (!ServiceClient::Bind(pCredentials) || !RegisterProtocol(&m_protocolClient))
        return false;

    m_bBound = true;
    return true;
}

bool DataStoreLogicServerClient::Unbind()
{
    if (!ServiceClient::Unbind() || !UnregisterProtocol(&m_protocolClient))
        return false;

    m_bBound = false;
    return true;
}

uint32_t DataStoreLogicServerClient::GetMyPrincipalId() const
{
    Credentials* pCredentials = GetCredentials();
    if (pCredentials == NULL)
        return 0;

    return pCredentials->GetPrincipalID();
}

bool DataStoreLogicServerClient::ResetRating(ProtocolCallContext* pContext, const uint64_t& rDataId)
{
    qVector<uint64_t> dataIds;
    dataIds.push_back(rDataId);
    const bool bTransactional = true;

    return m_protocolClient.CallResetRatings(pContext, dataIds, bTransactional, NULL);
}

bool DataStoreLogicServerClient::DeleteObject(ProtocolCallContext* pContext, const uint64_t& rDataId)
{
    DataStoreDeleteParam param;
    param.m_uiUpdatePassword = 0;
    param.m_uiDataId = rDataId;

    return m_protocolClient.CallDeleteObject(pContext, param);
}

bool DataStoreLogicServerClient::DeleteObject(ProtocolCallContext* pContext,
                                              const qVector<DataStoreDeleteParam>& rParams, bool bTransactional,
                                              qVector<qResult>* pResults)
{
    return m_protocolClient.CallDeleteObjects(pContext, rParams, bTransactional, pResults);
}

bool DataStoreLogicServerClient::DeleteObject(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                              bool bTransactional, qVector<qResult>* pResults)
{
    qVector<DataStoreDeleteParam> params;
    DataStoreDeleteParam param;
    param.m_uiUpdatePassword = 0;
    for (qVector<uint64_t>::const_iterator it = rDataIds.begin(); it != rDataIds.end(); it++)
    {
        param.m_uiDataId = *it;
        params.push_back(param);
    }

    return m_protocolClient.CallDeleteObjects(pContext, params, bTransactional, pResults);
}

bool DataStoreLogicServerClient::ChangeMeta(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                            const DataStoreChangeMetaParam& rParam, bool bTransactional,
                                            qVector<qResult>* pResults)
{
    qVector<DataStoreChangeMetaParam> params;
    params.push_back(rParam);

    return m_protocolClient.CallChangeMetas(pContext, rDataIds, params, bTransactional, pResults);
}

bool DataStoreLogicServerClient::ChangeMeta(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                            const DataStoreChangeMetaParamV1& rParam, bool bTransactional,
                                            qVector<qResult>* pResults)
{
    qVector<DataStoreChangeMetaParamV1> params;
    params.push_back(rParam);

    return m_protocolClient.CallChangeMetasV1(pContext, rDataIds, params, bTransactional, pResults);
}

bool DataStoreLogicServerClient::ChangeMeta(ProtocolCallContext* pContext,
                                            const qVector<DataStoreChangeMetaParam>& rParams, bool bTransactional,
                                            qVector<qResult>* pResults)
{
    qVector<uint64_t> dataIds;

    return m_protocolClient.CallChangeMetas(pContext, dataIds, rParams, bTransactional, pResults);
}

bool DataStoreLogicServerClient::ChangeMeta(ProtocolCallContext* pContext,
                                            const qVector<DataStoreChangeMetaParamV1>& rParams, bool bTransactional,
                                            qVector<qResult>* pResults)
{
    qVector<uint64_t> dataIds;

    return m_protocolClient.CallChangeMetasV1(pContext, dataIds, rParams, bTransactional, pResults);
}

bool DataStoreLogicServerClient::RateObject(ProtocolCallContext* pContext,
                                            const qVector<DataStoreRatingTarget>& rTargets,
                                            const DataStoreRateObjectParam& rParam, bool bTransactional,
                                            qVector<DataStoreRatingInfo>* pRatingInfos, qVector<qResult>* pResults)
{
    qVector<DataStoreRateObjectParam> params;
    params.push_back(rParam);

    return m_protocolClient.CallRateObjects(pContext, rTargets, params, bTransactional, pRatingInfos != NULL,
                                            pRatingInfos, pResults);
}

bool DataStoreLogicServerClient::RateObject(ProtocolCallContext* pContext,
                                            const qVector<DataStoreRatingTarget>& rTargets,
                                            const qVector<DataStoreRateObjectParam>& rParams, bool bTransactional,
                                            qVector<DataStoreRatingInfo>* pRatingInfos, qVector<qResult>* pResults)
{
    return m_protocolClient.CallRateObjects(pContext, rTargets, rParams, bTransactional, pRatingInfos != NULL,
                                            pRatingInfos, pResults);
}

bool DataStoreLogicServerClient::RateObjectsWithPosting(ProtocolCallContext* pContext,
                                                        const qVector<DataStoreRatingTarget>& rTargets,
                                                        const qVector<DataStoreRateObjectParam>& rRateParams,
                                                        const qVector<DataStorePreparePostParam>& rPostParams,
                                                        bool bTransactional, qVector<DataStoreRatingInfo>* pRatingInfos,
                                                        qVector<qResult>* pResults)
{
    for (uint32_t i = 0; i < rTargets.size(); ++i)
    {
        const DataStorePreparePostParam& rPostParam = rPostParams[rRateParams.size() == 1 ? 0 : i];
        if (rPostParam.m_RatingInitParams.size() == 0)
            break;

        bool bSlotFound = false;
        for (qVector<DataStoreRatingInitParamWithSlot>::const_iterator it = rPostParam.m_RatingInitParams.begin();
             it != rPostParam.m_RatingInitParams.end(); ++it)
        {
            if (it->m_iSlot == rTargets[i].m_iSlot)
                bSlotFound = true;
        }
        if (!bSlotFound)
            break;
    }

    return m_protocolClient.CallRateObjectsWithPosting(pContext, rTargets, rRateParams, rPostParams, bTransactional,
                                                       pRatingInfos != NULL, pRatingInfos, pResults);
}

bool DataStoreLogicServerClient::RateObjectsWithPosting(ProtocolCallContext* pContext,
                                                        const qVector<DataStoreRatingTarget>& rTargets,
                                                        const DataStoreRateObjectParam& rRateParam,
                                                        const DataStorePreparePostParam& rPostParam,
                                                        bool bTransactional, qVector<DataStoreRatingInfo>* pRatingInfos,
                                                        qVector<qResult>* pResults)
{
    qVector<DataStoreRateObjectParam> rateParams;
    qVector<DataStorePreparePostParam> postParams;
    rateParams.push_back(rRateParam);
    postParams.push_back(rPostParam);

    return RateObjectsWithPosting(pContext, rTargets, rateParams, postParams, bTransactional, pRatingInfos, pResults);
}

bool DataStoreLogicServerClient::RateObjectWithPosting(ProtocolCallContext* pContext,
                                                       const DataStoreRatingTarget& rTarget,
                                                       const DataStoreRateObjectParam& rRateParam,
                                                       const DataStorePreparePostParam& rPostParam,
                                                       DataStoreRatingInfo* pRatingInfo)
{
    bool bSlotFound = false;
    for (qVector<DataStoreRatingInitParamWithSlot>::const_iterator it = rPostParam.m_RatingInitParams.begin();
         it != rPostParam.m_RatingInitParams.end(); ++it)
    {
        if (it->m_iSlot == rTarget.m_iSlot)
            bSlotFound = true;
    }
    if (!bSlotFound)
        return false;

    return m_protocolClient.CallRateObjectWithPosting(pContext, rTarget, rRateParam, rPostParam, pRatingInfo != NULL,
                                                      pRatingInfo);
}

bool DataStoreLogicServerClient::PostMetaBinaryWithDataIds(ProtocolCallContext* pContext,
                                                           const qVector<uint64_t>& rDataIds,
                                                           const DataStorePreparePostParam& rParam, bool bTransactional,
                                                           qVector<qResult>* pResults)
{
    qVector<DataStorePreparePostParam> params;
    params.push_back(rParam);

    return m_protocolClient.CallPostMetaBinariesWithDataId(pContext, rDataIds, params, bTransactional, pResults);
}

bool DataStoreLogicServerClient::PostMetaBinariesWithDataId(
    ProtocolCallContext* pContext, const qVector<std::pair<uint64_t, DataStorePreparePostParam> >& rParams,
    bool bTransactional, qVector<qResult>* pResults)
{
    qVector<DataStorePreparePostParam> params;
    qVector<uint64_t> dataIds;
    for (qVector<std::pair<uint64_t, DataStorePreparePostParam> >::const_iterator it = rParams.begin();
         it != rParams.end(); it++)
    {
        dataIds.push_back(it->first);
        params.push_back(it->second);
    }

    return m_protocolClient.CallPostMetaBinariesWithDataId(pContext, dataIds, params, bTransactional, pResults);
}

bool DataStoreLogicServerClient::GetPasswordInfo(ProtocolCallContext* pContext, uint64_t uiDataId,
                                                 DataStorePasswordInfo* pPasswordInfo)
{
    return m_protocolClient.CallGetPasswordInfo(pContext, uiDataId, pPasswordInfo);
}

}
}
