#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPROTOCOLCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREPROTOCOLCLIENT_HPP

#include <utility>

#include "nn/nex/ClientProtocol.hpp"
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

class DataStoreChangeMetaParam;
class DataStoreChangeMetaParamV1;
class DataStoreCompletePostParam;
class DataStoreCompletePostParamV1;
class DataStoreCompleteUpdateParam;
class DataStoreDeleteParam;
class DataStoreGetMetaParam;
class DataStoreGetNewArrivedNotificationsParam;
class DataStoreGetNotificationUrlParam;
class DataStoreGetSpecificMetaParam;
class DataStoreGetSpecificMetaParamV1;
class DataStoreNotification;
class DataStoreNotificationV1;
class DataStorePasswordInfo;
class DataStorePersistenceInfo;
class DataStorePrepareGetParam;
class DataStorePrepareGetParamV1;
class DataStorePreparePostParam;
class DataStorePreparePostParamV1;
class DataStorePrepareUpdateParam;
class DataStoreRateObjectParam;
class DataStoreRatingLog;
class DataStoreRatingTarget;
class DataStoreReqGetAdditionalMeta;
class DataStoreReqGetInfo;
class DataStoreReqGetInfoV1;
class DataStoreReqGetNotificationUrlInfo;
class DataStoreReqPostInfo;
class DataStoreReqPostInfoV1;
class DataStoreReqUpdateInfo;
class DataStoreSpecificMetaInfo;
class DataStoreSpecificMetaInfoV1;
class DataStoreTouchObjectParam;

class DataStoreProtocolClient : public ClientProtocol
{
public:
    explicit DataStoreProtocolClient(unsigned short uiProtocolID = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);
    virtual Protocol* CreateResponder() const;

    static void ProtoReturn_PrepareGetObject(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallPrepareGetObject(ProtocolCallContext* pCallContext, const DataStorePrepareGetParam& rPrepareGetParam,
                              DataStoreReqGetInfo* pReqGetInfo);
    static void ProtoReturn_PrepareGetObjectV1(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallPrepareGetObjectV1(ProtocolCallContext* pCallContext, const DataStorePrepareGetParamV1& rPrepareGetParamV1,
                                DataStoreReqGetInfoV1* pReqGetInfoV1);
    static void ProtoReturn_PrepareGetObjectOrMetaBinary(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallPrepareGetObjectOrMetaBinary(ProtocolCallContext* pCallContext,
                                          const DataStorePrepareGetParam& rPrepareGetParam,
                                          DataStoreReqGetInfo* pReqGetInfo,
                                          DataStoreReqGetAdditionalMeta* pReqGetAdditionalMeta);
    static void ProtoReturn_PreparePostObject(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallPreparePostObject(ProtocolCallContext* pCallContext, const DataStorePreparePostParam& rPreparePostParam,
                               DataStoreReqPostInfo* pReqPostInfo);
    static void ProtoReturn_PreparePostObjectV1(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallPreparePostObjectV1(ProtocolCallContext* pCallContext,
                                 const DataStorePreparePostParamV1& rPreparePostParamV1,
                                 DataStoreReqPostInfoV1* pReqPostInfoV1);
    bool CallCompletePostObject(ProtocolCallContext* pCallContext,
                                const DataStoreCompletePostParam& rCompletePostParam);
    bool CallCompletePostObjectV1(ProtocolCallContext* pCallContext,
                                  const DataStoreCompletePostParamV1& rCompletePostParamV1);
    bool CallCompletePostObjects(ProtocolCallContext* pCallContext, const qVector<unsigned long long>& rDataIds);
    static void ProtoReturn_PrepareUpdateObject(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallPrepareUpdateObject(ProtocolCallContext* pCallContext,
                                 const DataStorePrepareUpdateParam& rPrepareUpdateParam,
                                 DataStoreReqUpdateInfo* pReqUpdateInfo);
    bool CallCompleteUpdateObject(ProtocolCallContext* pCallContext,
                                  const DataStoreCompleteUpdateParam& rCompleteUpdateParam);
    bool CallDeleteObject(ProtocolCallContext* pCallContext, const DataStoreDeleteParam& rDeleteParam);
    bool CallChangeMeta(ProtocolCallContext* pCallContext, const DataStoreChangeMetaParam& rChangeMetaParam);
    static void ProtoReturn_GetMeta(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetMeta(ProtocolCallContext* pCallContext, const DataStoreGetMetaParam& rGetMetaParam,
                     DataStoreMetaInfo* pMetaInfo);
    static void ProtoReturn_GetMetas(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetMetas(ProtocolCallContext* pCallContext, const qVector<unsigned long long>& rDataIds,
                      const DataStoreGetMetaParam& rGetMetaParam, qVector<DataStoreMetaInfo>* pMetaInfos,
                      qVector<qResult>* pResults);
    static void ProtoReturn_GetMetasMultipleParam(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetMetasMultipleParam(ProtocolCallContext* pCallContext,
                                   const qVector<DataStoreGetMetaParam>& rGetMetaParams,
                                   qVector<DataStoreMetaInfo>* pMetaInfos, qVector<qResult>* pResults);
    static void ProtoReturn_SearchObject(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallSearchObject(ProtocolCallContext* pCallContext, const DataStoreSearchParam& rSearchParam,
                          DataStoreSearchResult* pSearchResult);
    static void ProtoReturn_SearchObjectLight(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallSearchObjectLight(ProtocolCallContext* pCallContext, const DataStoreSearchParam& rSearchParam,
                               DataStoreSearchResult* pSearchResult);
    static void ProtoReturn_GetNotificationUrl(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetNotificationUrl(ProtocolCallContext* pCallContext,
                                const DataStoreGetNotificationUrlParam& rGetNotificationUrlParam,
                                DataStoreReqGetNotificationUrlInfo* pReqGetNotificationUrlInfo);
    static void ProtoReturn_GetNewArrivedNotifications(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetNewArrivedNotifications(
        ProtocolCallContext* pCallContext,
        const DataStoreGetNewArrivedNotificationsParam& rGetNewArrivedNotificationsParam,
        qVector<DataStoreNotification>* pNotifications, bool* pFlag);
    static void ProtoReturn_GetNewArrivedNotificationsV1(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetNewArrivedNotificationsV1(
        ProtocolCallContext* pCallContext,
        const DataStoreGetNewArrivedNotificationsParam& rGetNewArrivedNotificationsParam,
        qVector<DataStoreNotificationV1>* pNotificationV1s, bool* pFlag);
    static void ProtoReturn_RateObject(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallRateObject(ProtocolCallContext* pCallContext, const DataStoreRatingTarget& rRatingTarget,
                        const DataStoreRateObjectParam& rRateObjectParam, const bool& rFlag,
                        DataStoreRatingInfo* pRatingInfo);
    static void ProtoReturn_GetRating(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetRating(ProtocolCallContext* pCallContext, const DataStoreRatingTarget& rRatingTarget,
                       DataStoreRatingInfo* pRatingInfo, const unsigned long long& rDataId);
    static void ProtoReturn_GetRatings(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetRatings(ProtocolCallContext* pCallContext, const qVector<unsigned long long>& rDataIds,
                        qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfoWithSlotss,
                        qVector<qResult>* pResults, const unsigned long long& rDataId);
    bool CallResetRating(ProtocolCallContext* pCallContext, const DataStoreRatingTarget& rRatingTarget,
                         const unsigned long long& rDataId);
    static void ProtoReturn_ResetRatings(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallResetRatings(ProtocolCallContext* pCallContext, const qVector<unsigned long long>& rDataIds,
                          const bool& rFlag, qVector<qResult>* pResults);
    static void ProtoReturn_GetSpecificMeta(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetSpecificMeta(ProtocolCallContext* pCallContext,
                             const DataStoreGetSpecificMetaParam& rGetSpecificMetaParam,
                             qVector<DataStoreSpecificMetaInfo>* pSpecificMetaInfos);
    static void ProtoReturn_GetSpecificMetaV1(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetSpecificMetaV1(ProtocolCallContext* pCallContext,
                               const DataStoreGetSpecificMetaParamV1& rGetSpecificMetaParamV1,
                               qVector<DataStoreSpecificMetaInfoV1>* pSpecificMetaInfoV1s);
    static void ProtoReturn_PostMetaBinary(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallPostMetaBinary(ProtocolCallContext* pCallContext, const DataStorePreparePostParam& rPreparePostParam,
                            unsigned long long* pDataId);
    bool CallPostMetaBinaryWithDataId(ProtocolCallContext* pCallContext, const unsigned long long& rDataId,
                                      const DataStorePreparePostParam& rPreparePostParam);
    static void ProtoReturn_PostMetaBinariesWithDataId(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallPostMetaBinariesWithDataId(ProtocolCallContext* pCallContext, const qVector<unsigned long long>& rDataIds,
                                        const qVector<DataStorePreparePostParam>& rPreparePostParams, const bool& rFlag,
                                        qVector<qResult>* pResults);
    bool CallTouchObject(ProtocolCallContext* pCallContext, const DataStoreTouchObjectParam& rTouchObjectParam);
    static void ProtoReturn_GetRatingWithLog(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetRatingWithLog(ProtocolCallContext* pCallContext, const DataStoreRatingTarget& rRatingTarget,
                              DataStoreRatingInfo* pRatingInfo, DataStoreRatingLog* pRatingLog,
                              const unsigned long long& rDataId);
    static void ProtoReturn_GetPersistenceInfo(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetPersistenceInfo(ProtocolCallContext* pCallContext, const unsigned int& rValue,
                                const unsigned short& rSlot, DataStorePersistenceInfo* pPersistenceInfo);
    static void ProtoReturn_GetPersistenceInfos(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetPersistenceInfos(ProtocolCallContext* pCallContext, const unsigned int& rValue,
                                 const qVector<unsigned short>& rSlots,
                                 qVector<DataStorePersistenceInfo>* pPersistenceInfos, qVector<qResult>* pResults);
    bool CallPerpetuateObject(ProtocolCallContext* pCallContext, const unsigned short& rSlot,
                              const unsigned long long& rDataId, const bool& rFlag);
    bool CallUnperpetuateObject(ProtocolCallContext* pCallContext, const unsigned short& rSlot, const bool& rFlag);
    static void ProtoReturn_GetPasswordInfo(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetPasswordInfo(ProtocolCallContext* pCallContext, const unsigned long long& rDataId,
                             DataStorePasswordInfo* pPasswordInfo);
    static void ProtoReturn_GetPasswordInfos(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetPasswordInfos(ProtocolCallContext* pCallContext, const qVector<unsigned long long>& rDataIds,
                              qVector<DataStorePasswordInfo>* pPasswordInfos, qVector<qResult>* pResults);
    static void ProtoReturn_GetObjectInfos(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetObjectInfos(ProtocolCallContext* pCallContext, const qVector<unsigned long long>& rDataIds,
                            qVector<DataStoreReqGetInfo>* pReqGetInfos, qVector<qResult>* pResults);
    static bool PrepareGetObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                const DataStorePrepareGetParam& rPrepareGetParam,
                                                DataStoreReqGetInfo* pReqGetInfo);
    static bool PrepareGetObjectV1_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                  const DataStorePrepareGetParamV1& rPrepareGetParamV1,
                                                  DataStoreReqGetInfoV1* pReqGetInfoV1);
    static bool PrepareGetObjectOrMetaBinary_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                            const DataStorePrepareGetParam& rPrepareGetParam,
                                                            DataStoreReqGetInfo* pReqGetInfo,
                                                            DataStoreReqGetAdditionalMeta* pReqGetAdditionalMeta);
    static bool PreparePostObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                 const DataStorePreparePostParam& rPreparePostParam,
                                                 DataStoreReqPostInfo* pReqPostInfo);
    static bool PreparePostObjectV1_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                   const DataStorePreparePostParamV1& rPreparePostParamV1,
                                                   DataStoreReqPostInfoV1* pReqPostInfoV1);
    static bool CompletePostObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                  const DataStoreCompletePostParam& rCompletePostParam);
    static bool CompletePostObjectV1_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                    const DataStoreCompletePostParamV1& rCompletePostParamV1);
    static bool CompletePostObjects_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                   const qVector<unsigned long long>& rDataIds);
    static bool PrepareUpdateObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                   const DataStorePrepareUpdateParam& rPrepareUpdateParam,
                                                   DataStoreReqUpdateInfo* pReqUpdateInfo);
    static bool CompleteUpdateObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                    const DataStoreCompleteUpdateParam& rCompleteUpdateParam);
    static bool DeleteObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                            const DataStoreDeleteParam& rDeleteParam);
    static bool ChangeMeta_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                          const DataStoreChangeMetaParam& rChangeMetaParam);
    static bool GetMeta_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                       const DataStoreGetMetaParam& rGetMetaParam, DataStoreMetaInfo* pMetaInfo);
    static bool GetMetas_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                        const qVector<unsigned long long>& rDataIds,
                                        const DataStoreGetMetaParam& rGetMetaParam,
                                        qVector<DataStoreMetaInfo>* pMetaInfos, qVector<qResult>* pResults);
    static bool GetMetasMultipleParam_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                     const qVector<DataStoreGetMetaParam>& rGetMetaParams,
                                                     qVector<DataStoreMetaInfo>* pMetaInfos,
                                                     qVector<qResult>* pResults);
    static bool SearchObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                            const DataStoreSearchParam& rSearchParam,
                                            DataStoreSearchResult* pSearchResult);
    static bool SearchObjectLight_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                 const DataStoreSearchParam& rSearchParam,
                                                 DataStoreSearchResult* pSearchResult);
    static bool GetNotificationUrl_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                  const DataStoreGetNotificationUrlParam& rGetNotificationUrlParam,
                                                  DataStoreReqGetNotificationUrlInfo* pReqGetNotificationUrlInfo);
    static bool GetNewArrivedNotifications_PrepareRequest(
        ProtocolCallContext* pCallContext, Message* pMessage,
        const DataStoreGetNewArrivedNotificationsParam& rGetNewArrivedNotificationsParam,
        qVector<DataStoreNotification>* pNotifications, bool* pFlag);
    static bool GetNewArrivedNotificationsV1_PrepareRequest(
        ProtocolCallContext* pCallContext, Message* pMessage,
        const DataStoreGetNewArrivedNotificationsParam& rGetNewArrivedNotificationsParam,
        qVector<DataStoreNotificationV1>* pNotificationV1s, bool* pFlag);
    static bool RateObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                          const DataStoreRatingTarget& rRatingTarget,
                                          const DataStoreRateObjectParam& rRateObjectParam, const bool& rFlag,
                                          DataStoreRatingInfo* pRatingInfo);
    static bool GetRating_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                         const DataStoreRatingTarget& rRatingTarget, DataStoreRatingInfo* pRatingInfo,
                                         const unsigned long long& rDataId);
    static bool GetRatings_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                          const qVector<unsigned long long>& rDataIds,
                                          qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfoWithSlotss,
                                          qVector<qResult>* pResults, const unsigned long long& rDataId);
    static bool ResetRating_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                           const DataStoreRatingTarget& rRatingTarget,
                                           const unsigned long long& rDataId);
    static bool ResetRatings_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                            const qVector<unsigned long long>& rDataIds, const bool& rFlag,
                                            qVector<qResult>* pResults);
    static bool GetSpecificMeta_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                               const DataStoreGetSpecificMetaParam& rGetSpecificMetaParam,
                                               qVector<DataStoreSpecificMetaInfo>* pSpecificMetaInfos);
    static bool GetSpecificMetaV1_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                 const DataStoreGetSpecificMetaParamV1& rGetSpecificMetaParamV1,
                                                 qVector<DataStoreSpecificMetaInfoV1>* pSpecificMetaInfoV1s);
    static bool PostMetaBinary_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                              const DataStorePreparePostParam& rPreparePostParam,
                                              unsigned long long* pDataId);
    static bool PostMetaBinaryWithDataId_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                        const unsigned long long& rDataId,
                                                        const DataStorePreparePostParam& rPreparePostParam);
    static bool PostMetaBinariesWithDataId_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                          const qVector<unsigned long long>& rDataIds,
                                                          const qVector<DataStorePreparePostParam>& rPreparePostParams,
                                                          const bool& rFlag, qVector<qResult>* pResults);
    static bool TouchObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                           const DataStoreTouchObjectParam& rTouchObjectParam);
    static bool GetRatingWithLog_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                const DataStoreRatingTarget& rRatingTarget,
                                                DataStoreRatingInfo* pRatingInfo, DataStoreRatingLog* pRatingLog,
                                                const unsigned long long& rDataId);
    static bool GetPersistenceInfo_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                  const unsigned int& rValue, const unsigned short& rSlot,
                                                  DataStorePersistenceInfo* pPersistenceInfo);
    static bool GetPersistenceInfos_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                   const unsigned int& rValue, const qVector<unsigned short>& rSlots,
                                                   qVector<DataStorePersistenceInfo>* pPersistenceInfos,
                                                   qVector<qResult>* pResults);
    static bool PerpetuateObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                const unsigned short& rSlot, const unsigned long long& rDataId,
                                                const bool& rFlag);
    static bool UnperpetuateObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                  const unsigned short& rSlot, const bool& rFlag);
    static bool GetPasswordInfo_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                               const unsigned long long& rDataId, DataStorePasswordInfo* pPasswordInfo);
    static bool GetPasswordInfos_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                const qVector<unsigned long long>& rDataIds,
                                                qVector<DataStorePasswordInfo>* pPasswordInfos,
                                                qVector<qResult>* pResults);
    static bool GetObjectInfos_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                              const qVector<unsigned long long>& rDataIds,
                                              qVector<DataStoreReqGetInfo>* pReqGetInfos, qVector<qResult>* pResults);
    static void ProtoReturn_DeleteObjects(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_ChangeMetasV1(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_ChangeMetas(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_RateObjects(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_RateObjectWithPosting(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_RateObjectsWithPosting(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallDeleteObjects(ProtocolCallContext* pCallContext, const qVector<DataStoreDeleteParam>& rDeleteParams,
                           const bool& rFlag, qVector<qResult>* pResults);
    static bool DeleteObjects_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                             const qVector<DataStoreDeleteParam>& rDeleteParams, const bool& rFlag,
                                             qVector<qResult>* pResults);
    bool CallChangeMetaV1(ProtocolCallContext* pCallContext, const DataStoreChangeMetaParamV1& rChangeMetaParamV1);
    static bool ChangeMetaV1_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                            const DataStoreChangeMetaParamV1& rChangeMetaParamV1);
    bool CallChangeMetasV1(ProtocolCallContext* pCallContext, const qVector<unsigned long long>& rDataIds,
                           const qVector<DataStoreChangeMetaParamV1>& rChangeMetaParamV1s, const bool& rFlag,
                           qVector<qResult>* pResults);
    static bool ChangeMetasV1_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                             const qVector<unsigned long long>& rDataIds,
                                             const qVector<DataStoreChangeMetaParamV1>& rChangeMetaParamV1s,
                                             const bool& rFlag, qVector<qResult>* pResults);
    bool CallChangeMetas(ProtocolCallContext* pCallContext, const qVector<unsigned long long>& rDataIds,
                         const qVector<DataStoreChangeMetaParam>& rChangeMetaParams, const bool& rFlag,
                         qVector<qResult>* pResults);
    static bool ChangeMetas_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                           const qVector<unsigned long long>& rDataIds,
                                           const qVector<DataStoreChangeMetaParam>& rChangeMetaParams,
                                           const bool& rFlag, qVector<qResult>* pResults);
    bool CallRateObjects(ProtocolCallContext* pCallContext, const qVector<DataStoreRatingTarget>& rRatingTargets,
                         const qVector<DataStoreRateObjectParam>& rRateObjectParams, const bool& rFlag,
                         const bool& rFlag2, qVector<DataStoreRatingInfo>* pRatingInfos, qVector<qResult>* pResults);
    static bool RateObjects_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                           const qVector<DataStoreRatingTarget>& rRatingTargets,
                                           const qVector<DataStoreRateObjectParam>& rRateObjectParams,
                                           const bool& rFlag, const bool& rFlag2,
                                           qVector<DataStoreRatingInfo>* pRatingInfos, qVector<qResult>* pResults);
    bool CallRateObjectWithPosting(ProtocolCallContext* pCallContext, const DataStoreRatingTarget& rRatingTarget,
                                   const DataStoreRateObjectParam& rRateObjectParam,
                                   const DataStorePreparePostParam& rPreparePostParam, const bool& rFlag,
                                   DataStoreRatingInfo* pRatingInfo);
    static bool RateObjectWithPosting_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                     const DataStoreRatingTarget& rRatingTarget,
                                                     const DataStoreRateObjectParam& rRateObjectParam,
                                                     const DataStorePreparePostParam& rPreparePostParam,
                                                     const bool& rFlag, DataStoreRatingInfo* pRatingInfo);
    bool CallRateObjectsWithPosting(ProtocolCallContext* pCallContext,
                                    const qVector<DataStoreRatingTarget>& rRatingTargets,
                                    const qVector<DataStoreRateObjectParam>& rRateObjectParams,
                                    const qVector<DataStorePreparePostParam>& rPreparePostParams, const bool& rFlag,
                                    const bool& rFlag2, qVector<DataStoreRatingInfo>* pRatingInfos,
                                    qVector<qResult>* pResults);
    static bool RateObjectsWithPosting_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                      const qVector<DataStoreRatingTarget>& rRatingTargets,
                                                      const qVector<DataStoreRateObjectParam>& rRateObjectParams,
                                                      const qVector<DataStorePreparePostParam>& rPreparePostParams,
                                                      const bool& rFlag, const bool& rFlag2,
                                                      qVector<DataStoreRatingInfo>* pRatingInfos,
                                                      qVector<qResult>* pResults);
    static void ProtoReturn_CompletePostObject(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_CompletePostObjectV1(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_CompletePostObjects(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_CompleteUpdateObject(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_DeleteObject(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_ChangeMeta(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_ResetRating(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_PostMetaBinaryWithDataId(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_TouchObject(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_PerpetuateObject(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_UnperpetuateObject(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_ChangeMetaV1(Message* pMessage, ProtocolCallContext* pCallContext);
};

}
}

#endif
