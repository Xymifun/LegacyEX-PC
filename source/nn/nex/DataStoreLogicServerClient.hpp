#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORELOGICSERVERCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORELOGICSERVERCLIENT_HPP

#include <stdint.h>
#include <utility>

#include "nn/nex/DataStoreLogicServerClientInterface.hpp"
#include "nn/nex/DataStoreProtocolClient.hpp"
#include "nn/nex/ServiceClient.hpp"

namespace nn
{
namespace nex
{

class String;
class DataStoreChangeMetaParamV1;

class DataStoreLogicServerClient : public ServiceClient, public DataStoreLogicServerClientInterface
{
public:
    DataStoreLogicServerClient();
    virtual ~DataStoreLogicServerClient();

    virtual bool Bind(Credentials* pCredentials);
    virtual bool Unbind();
    virtual void UpdateProtocolsDefaultCredentials(Credentials* pCredentials);

    virtual bool PrepareGetObject(ProtocolCallContext* pContext, const DataStorePrepareGetParam& rParam,
                                  DataStoreReqGetInfo* pReqGetInfo);
    virtual bool PrepareGetObject(ProtocolCallContext* pContext, const DataStorePrepareGetParamV1& rParam,
                                  DataStoreReqGetInfoV1* pReqGetInfo);
    virtual bool PrepareGetObjectOrMetaBinary(ProtocolCallContext* pContext, const DataStorePrepareGetParam& rParam,
                                              DataStoreReqGetInfo* pReqGetInfo,
                                              DataStoreReqGetAdditionalMeta* pAdditionalMeta);
    virtual bool PrepareGetObjectOrMetaBinary(ProtocolCallContext* pContext, const DataStorePrepareGetParamV1& rParam,
                                              DataStoreReqGetInfoV1* pReqGetInfo,
                                              DataStoreReqGetAdditionalMeta* pAdditionalMeta);
    virtual bool PreparePostObject(ProtocolCallContext* pContext, const DataStorePreparePostParam& rParam,
                                   DataStoreReqPostInfo* pReqPostInfo);
    virtual bool PreparePostObject(ProtocolCallContext* pContext, const DataStorePreparePostParamV1& rParam,
                                   DataStoreReqPostInfoV1* pReqPostInfo);
    virtual bool CompletePostObject(ProtocolCallContext* pContext, const DataStoreCompletePostParam& rParam);
    virtual bool CompletePostObject(ProtocolCallContext* pContext, const DataStoreCompletePostParamV1& rParam);
    virtual bool CompleteSuspendedPostObject(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds);
    virtual bool PostMetaBinary(ProtocolCallContext* pContext, const DataStorePreparePostParam& rParam,
                                uint64_t* pDataId);
    virtual bool PostMetaBinaryWithDataId(ProtocolCallContext* pContext, uint64_t uiDataId,
                                          const DataStorePreparePostParam& rParam);
    virtual bool PostMetaBinaryWithDataIds(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                           const DataStorePreparePostParam& rParam, bool bTransactional,
                                           qVector<qResult>* pResults);
    virtual bool PostMetaBinariesWithDataId(ProtocolCallContext* pContext,
                                            const qVector<std::pair<uint64_t, DataStorePreparePostParam> >& rParams,
                                            bool bTransactional, qVector<qResult>* pResults);
    virtual bool PrepareUpdateObject(ProtocolCallContext* pContext, const DataStorePrepareUpdateParam& rParam,
                                     DataStoreReqUpdateInfo* pReqUpdateInfo);
    virtual bool CompleteUpdateObject(ProtocolCallContext* pContext, const DataStoreCompleteUpdateParam& rParam);
    virtual bool DeleteObject(ProtocolCallContext* pContext, const DataStoreDeleteParam& rParam);
    virtual bool DeleteObject(ProtocolCallContext* pContext, const uint64_t& rDataId);
    virtual bool DeleteObject(ProtocolCallContext* pContext, const qVector<DataStoreDeleteParam>& rParams,
                              bool bTransactional, qVector<qResult>* pResults);
    virtual bool DeleteObject(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds, bool bTransactional,
                              qVector<qResult>* pResults);
    virtual bool ChangeMeta(ProtocolCallContext* pContext, const DataStoreChangeMetaParam& rParam);
    virtual bool ChangeMeta(ProtocolCallContext* pContext, const DataStoreChangeMetaParamV1& rParam);
    virtual bool ChangeMeta(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                            const DataStoreChangeMetaParam& rParam, bool bTransactional, qVector<qResult>* pResults);
    virtual bool ChangeMeta(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                            const DataStoreChangeMetaParamV1& rParam, bool bTransactional, qVector<qResult>* pResults);
    virtual bool ChangeMeta(ProtocolCallContext* pContext, const qVector<DataStoreChangeMetaParam>& rParams,
                            bool bTransactional, qVector<qResult>* pResults);
    virtual bool ChangeMeta(ProtocolCallContext* pContext, const qVector<DataStoreChangeMetaParamV1>& rParams,
                            bool bTransactional, qVector<qResult>* pResults);
    virtual bool GetMeta(ProtocolCallContext* pContext, const DataStoreGetMetaParam& rParam,
                         DataStoreMetaInfo* pMetaInfo);
    virtual bool GetMeta(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                         const DataStoreGetMetaParam& rParam, qVector<DataStoreMetaInfo>* pMetaInfos,
                         qVector<qResult>* pResults);
    virtual bool GetMeta(ProtocolCallContext* pContext, const qVector<DataStoreGetMetaParam>& rParams,
                         qVector<DataStoreMetaInfo>* pMetaInfos, qVector<qResult>* pResults);
    virtual bool SearchObject(ProtocolCallContext* pContext, const DataStoreSearchParam& rParam,
                              DataStoreSearchResult* pSearchResult);
    virtual bool SearchObjectLight(ProtocolCallContext* pContext, const DataStoreSearchParam& rParam,
                                   DataStoreSearchResult* pSearchResult);
    virtual bool GetNotificationUrl(ProtocolCallContext* pContext, const DataStoreGetNotificationUrlParam& rParam,
                                    DataStoreReqGetNotificationUrlInfo* pInfo);
    virtual bool GetNewArrivedNotifications(ProtocolCallContext* pContext,
                                            const DataStoreGetNewArrivedNotificationsParam& rParam,
                                            qVector<DataStoreNotification>* pNotifications, bool* pbHasNext);
    virtual bool GetNewArrivedNotifications(ProtocolCallContext* pContext,
                                            const DataStoreGetNewArrivedNotificationsParam& rParam,
                                            qVector<DataStoreNotificationV1>* pNotifications, bool* pbHasNext);
    virtual bool RateObject(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                            const DataStoreRateObjectParam& rParam, DataStoreRatingInfo* pRatingInfo);
    virtual bool RateObject(ProtocolCallContext* pContext, const qVector<DataStoreRatingTarget>& rTargets,
                            const DataStoreRateObjectParam& rParam, bool bTransactional,
                            qVector<DataStoreRatingInfo>* pRatingInfos, qVector<qResult>* pResults);
    virtual bool RateObject(ProtocolCallContext* pContext, const qVector<DataStoreRatingTarget>& rTargets,
                            const qVector<DataStoreRateObjectParam>& rParams, bool bTransactional,
                            qVector<DataStoreRatingInfo>* pRatingInfos, qVector<qResult>* pResults);
    virtual bool GetRating(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                           DataStoreRatingInfo* pRatingInfo, uint64_t uiAccessPassword);
    virtual bool GetRating(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                           qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfos, qVector<qResult>* pResults,
                           uint64_t uiAccessPassword);
    virtual bool RateObjectsWithPosting(ProtocolCallContext* pContext, const qVector<DataStoreRatingTarget>& rTargets,
                                        const qVector<DataStoreRateObjectParam>& rRateParams,
                                        const qVector<DataStorePreparePostParam>& rPostParams, bool bTransactional,
                                        qVector<DataStoreRatingInfo>* pRatingInfos, qVector<qResult>* pResults);
    virtual bool RateObjectsWithPosting(ProtocolCallContext* pContext, const qVector<DataStoreRatingTarget>& rTargets,
                                        const DataStoreRateObjectParam& rRateParam,
                                        const DataStorePreparePostParam& rPostParam, bool bTransactional,
                                        qVector<DataStoreRatingInfo>* pRatingInfos, qVector<qResult>* pResults);
    virtual bool RateObjectWithPosting(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                                       const DataStoreRateObjectParam& rRateParam,
                                       const DataStorePreparePostParam& rPostParam, DataStoreRatingInfo* pRatingInfo);
    virtual bool ResetRating(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                             uint64_t uiUpdatePassword);
    virtual bool ResetRating(ProtocolCallContext* pContext, const uint64_t& rDataId);
    virtual bool ResetRating(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds, bool bTransactional,
                             qVector<qResult>* pResults);
    virtual bool GetSpecificMeta(ProtocolCallContext* pContext, const DataStoreGetSpecificMetaParam& rParam,
                                 qVector<DataStoreSpecificMetaInfo>* pMetaInfos);
    virtual bool GetSpecificMeta(ProtocolCallContext* pContext, const DataStoreGetSpecificMetaParamV1& rParam,
                                 qVector<DataStoreSpecificMetaInfoV1>* pMetaInfos);
    virtual bool TouchObject(ProtocolCallContext* pContext, const DataStoreTouchObjectParam& rParam);
    virtual bool GetRatingWithLog(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                                  DataStoreRatingInfo* pRatingInfo, DataStoreRatingLog* pRatingLog,
                                  uint64_t uiAccessPassword);
    virtual bool GetPersistenceInfo(ProtocolCallContext* pContext, uint32_t uiOwnerId, uint16_t uiPersistenceSlotId,
                                    DataStorePersistenceInfo* pInfo);
    virtual bool GetPersistenceInfo(ProtocolCallContext* pContext, uint32_t uiOwnerId,
                                    qVector<uint16_t> persistenceSlotIds, qVector<DataStorePersistenceInfo>* pInfos,
                                    qVector<qResult>* pResults);
    virtual bool PerpetuateObject(ProtocolCallContext* pContext, uint16_t uiPersistenceSlotId, uint64_t uiDataId,
                                  bool bDeleteLastObject);
    virtual bool UnperpetuateObject(ProtocolCallContext* pContext, uint16_t uiPersistenceSlotId,
                                    bool bDeleteLastObject);
    virtual bool GetPasswordInfo(ProtocolCallContext* pContext, uint64_t uiDataId,
                                 DataStorePasswordInfo* pPasswordInfo);
    virtual bool GetPasswordInfo(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                 qVector<DataStorePasswordInfo>* pPasswordInfos, qVector<qResult>* pResults);
    virtual bool GetObjectInfo(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                               qVector<DataStoreReqGetInfo>* pReqGetInfos, qVector<qResult>* pResults);
    virtual uint32_t GetMyPrincipalId() const;
    virtual bool IsCredentialsNeeded() const;

    void CreateExtraData(qVector<String>& rExtraData) const;

protected:
    enum ExtraDataIndex
    {
        EXTRA_DATA_PLATFORM = 0,
        EXTRA_DATA_REGION_ID = 1,
        EXTRA_DATA_REGION_NAME = 2,
        EXTRA_DATA_COUNTRY_ID = 3,
        EXTRA_DATA_COUNTRY_CODE = 4,
        EXTRA_DATA_COUNT = 6
    };

    enum PlatformRegion
    {
        PLATFORM_REGION_INVALID = 0,
        PLATFORM_REGION_JPN = 1,
        PLATFORM_REGION_USA = 2,
        PLATFORM_REGION_EUR = 4,
        PLATFORM_REGION_AUS = 8,
        PLATFORM_REGION_CHN = 16,
        PLATFORM_REGION_KOR = 32,
        PLATFORM_REGION_TWN = 64
    };

    enum
    {
        SCI_STATUS_SUCCESS = 1
    };

    bool m_bBound;
    DataStoreProtocolClient m_protocolClient;
};

}
}

#endif
