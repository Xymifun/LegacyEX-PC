#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORELOGICSERVERCLIENTINTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORELOGICSERVERCLIENTINTERFACE_HPP

#include <stdint.h>
#include <utility>

#include "nn/nex/DataStoreMetaInfo.hpp"
#include "nn/nex/DataStoreRatingInfo.hpp"
#include "nn/nex/DataStoreRatingInfoWithSlot.hpp"
#include "nn/nex/DataStoreSearchParam.hpp"
#include "nn/nex/DataStoreSearchResult.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class ProtocolCallContext;
class DataStorePrepareGetParam;
class DataStorePrepareGetParamV1;
class DataStoreReqGetInfo;
class DataStoreReqGetInfoV1;
class DataStoreReqGetAdditionalMeta;
class DataStorePreparePostParam;
class DataStorePreparePostParamV1;
class DataStoreReqPostInfo;
class DataStoreReqPostInfoV1;
class DataStoreCompletePostParam;
class DataStoreCompletePostParamV1;
class DataStorePrepareUpdateParam;
class DataStoreReqUpdateInfo;
class DataStoreCompleteUpdateParam;
class DataStoreDeleteParam;
class DataStoreChangeMetaParam;
class DataStoreGetMetaParam;
class DataStoreGetNotificationUrlParam;
class DataStoreReqGetNotificationUrlInfo;
class DataStoreGetNewArrivedNotificationsParam;
class DataStoreNotification;
class DataStoreNotificationV1;
class DataStoreRatingTarget;
class DataStoreRateObjectParam;
class DataStoreGetSpecificMetaParam;
class DataStoreGetSpecificMetaParamV1;
class DataStoreSpecificMetaInfo;
class DataStoreSpecificMetaInfoV1;
class DataStoreTouchObjectParam;
class DataStoreRatingLog;
class DataStorePersistenceInfo;
class DataStorePasswordInfo;

class DataStoreLogicServerClientInterface
{
public:
    virtual ~DataStoreLogicServerClientInterface();

    virtual bool PrepareGetObject(ProtocolCallContext* pContext, const DataStorePrepareGetParam& rParam,
                                  DataStoreReqGetInfo* pReqGetInfo) = 0;
    virtual bool PrepareGetObject(ProtocolCallContext* pContext, const DataStorePrepareGetParamV1& rParam,
                                  DataStoreReqGetInfoV1* pReqGetInfo) = 0;
    virtual bool PrepareGetObjectOrMetaBinary(ProtocolCallContext* pContext, const DataStorePrepareGetParam& rParam,
                                              DataStoreReqGetInfo* pReqGetInfo,
                                              DataStoreReqGetAdditionalMeta* pAdditionalMeta) = 0;
    virtual bool PrepareGetObjectOrMetaBinary(ProtocolCallContext* pContext, const DataStorePrepareGetParamV1& rParam,
                                              DataStoreReqGetInfoV1* pReqGetInfo,
                                              DataStoreReqGetAdditionalMeta* pAdditionalMeta) = 0;
    virtual bool PreparePostObject(ProtocolCallContext* pContext, const DataStorePreparePostParam& rParam,
                                   DataStoreReqPostInfo* pReqPostInfo) = 0;
    virtual bool PreparePostObject(ProtocolCallContext* pContext, const DataStorePreparePostParamV1& rParam,
                                   DataStoreReqPostInfoV1* pReqPostInfo) = 0;
    virtual bool CompletePostObject(ProtocolCallContext* pContext, const DataStoreCompletePostParam& rParam) = 0;
    virtual bool CompletePostObject(ProtocolCallContext* pContext, const DataStoreCompletePostParamV1& rParam) = 0;
    virtual bool CompleteSuspendedPostObject(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds) = 0;
    virtual bool PrepareUpdateObject(ProtocolCallContext* pContext, const DataStorePrepareUpdateParam& rParam,
                                     DataStoreReqUpdateInfo* pReqUpdateInfo) = 0;
    virtual bool CompleteUpdateObject(ProtocolCallContext* pContext, const DataStoreCompleteUpdateParam& rParam) = 0;
    virtual bool DeleteObject(ProtocolCallContext* pContext, const DataStoreDeleteParam& rParam) = 0;
    virtual bool ChangeMeta(ProtocolCallContext* pContext, const DataStoreChangeMetaParam& rParam) = 0;
    virtual bool GetMeta(ProtocolCallContext* pContext, const DataStoreGetMetaParam& rParam,
                         DataStoreMetaInfo* pMetaInfo) = 0;
    virtual bool GetMeta(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                         const DataStoreGetMetaParam& rParam, qVector<DataStoreMetaInfo>* pMetaInfos,
                         qVector<qResult>* pResults) = 0;
    virtual bool GetMeta(ProtocolCallContext* pContext, const qVector<DataStoreGetMetaParam>& rParams,
                         qVector<DataStoreMetaInfo>* pMetaInfos, qVector<qResult>* pResults) = 0;
    virtual bool SearchObject(ProtocolCallContext* pContext, const DataStoreSearchParam& rParam,
                              DataStoreSearchResult* pSearchResult) = 0;
    virtual bool SearchObjectLight(ProtocolCallContext* pContext, const DataStoreSearchParam& rParam,
                                   DataStoreSearchResult* pSearchResult) = 0;
    virtual bool GetNotificationUrl(ProtocolCallContext* pContext, const DataStoreGetNotificationUrlParam& rParam,
                                    DataStoreReqGetNotificationUrlInfo* pInfo) = 0;
    virtual bool GetNewArrivedNotifications(ProtocolCallContext* pContext,
                                            const DataStoreGetNewArrivedNotificationsParam& rParam,
                                            qVector<DataStoreNotification>* pNotifications, bool* pbHasNext) = 0;
    virtual bool GetNewArrivedNotifications(ProtocolCallContext* pContext,
                                            const DataStoreGetNewArrivedNotificationsParam& rParam,
                                            qVector<DataStoreNotificationV1>* pNotifications, bool* pbHasNext) = 0;
    virtual bool RateObject(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                            const DataStoreRateObjectParam& rParam, DataStoreRatingInfo* pRatingInfo) = 0;
    virtual bool GetRating(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                           DataStoreRatingInfo* pRatingInfo, uint64_t uiAccessPassword) = 0;
    virtual bool GetRating(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                           qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfos, qVector<qResult>* pResults,
                           uint64_t uiAccessPassword) = 0;
    virtual bool ResetRating(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                             uint64_t uiUpdatePassword) = 0;
    virtual bool ResetRating(ProtocolCallContext* pContext, const uint64_t& rDataId) = 0;
    virtual bool ResetRating(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds, bool bTransactional,
                             qVector<qResult>* pResults) = 0;
    virtual bool GetSpecificMeta(ProtocolCallContext* pContext, const DataStoreGetSpecificMetaParam& rParam,
                                 qVector<DataStoreSpecificMetaInfo>* pMetaInfos) = 0;
    virtual bool GetSpecificMeta(ProtocolCallContext* pContext, const DataStoreGetSpecificMetaParamV1& rParam,
                                 qVector<DataStoreSpecificMetaInfoV1>* pMetaInfos) = 0;
    virtual bool PostMetaBinary(ProtocolCallContext* pContext, const DataStorePreparePostParam& rParam,
                                uint64_t* pDataId) = 0;
    virtual bool PostMetaBinaryWithDataId(ProtocolCallContext* pContext, uint64_t uiDataId,
                                          const DataStorePreparePostParam& rParam) = 0;
    virtual bool PostMetaBinaryWithDataIds(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                           const DataStorePreparePostParam& rParam, bool bTransactional,
                                           qVector<qResult>* pResults) = 0;
    virtual bool PostMetaBinariesWithDataId(ProtocolCallContext* pContext,
                                            const qVector<std::pair<uint64_t, DataStorePreparePostParam> >& rParams,
                                            bool bTransactional, qVector<qResult>* pResults) = 0;
    virtual bool TouchObject(ProtocolCallContext* pContext, const DataStoreTouchObjectParam& rParam) = 0;
    virtual bool GetRatingWithLog(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                                  DataStoreRatingInfo* pRatingInfo, DataStoreRatingLog* pRatingLog,
                                  uint64_t uiAccessPassword) = 0;
    virtual bool GetPersistenceInfo(ProtocolCallContext* pContext, uint32_t uiOwnerId, uint16_t uiPersistenceSlotId,
                                    DataStorePersistenceInfo* pInfo) = 0;
    virtual bool GetPersistenceInfo(ProtocolCallContext* pContext, uint32_t uiOwnerId,
                                    qVector<uint16_t> persistenceSlotIds, qVector<DataStorePersistenceInfo>* pInfos,
                                    qVector<qResult>* pResults) = 0;
    virtual bool PerpetuateObject(ProtocolCallContext* pContext, uint16_t uiPersistenceSlotId, uint64_t uiDataId,
                                  bool bDeleteLastObject) = 0;
    virtual bool UnperpetuateObject(ProtocolCallContext* pContext, uint16_t uiPersistenceSlotId,
                                    bool bDeleteLastObject) = 0;
    virtual bool GetPasswordInfo(ProtocolCallContext* pContext, uint64_t uiDataId,
                                 DataStorePasswordInfo* pPasswordInfo) = 0;
    virtual bool GetPasswordInfo(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                                 qVector<DataStorePasswordInfo>* pPasswordInfos, qVector<qResult>* pResults) = 0;
    virtual bool GetObjectInfo(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                               qVector<DataStoreReqGetInfo>* pReqGetInfos, qVector<qResult>* pResults) = 0;
    virtual uint32_t GetMyPrincipalId() const = 0;
    virtual bool IsCredentialsNeeded() const = 0;
};

}
}

#endif
