#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECLIENTINTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECLIENTINTERFACE_HPP

#include <stdint.h>
#include <utility>

#include "nn/nex/DataStoreRatingInfo.hpp"
#include "nn/nex/DataStoreRatingInfoWithSlot.hpp"
#include "nn/nex/qMap.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class ProtocolCallContext;
class String;
class DataStorePrepareGetParam;
class DataStorePrepareGetParamV1;
class DataStorePreparePostParam;
class DataStorePreparePostParamV1;
class DataStorePrepareUpdateParam;
class DataStoreReqGetInfo;
class DataStoreRatingTarget;
class DataStoreRatingLog;
class DataStorePostObjectEventListener;

template <typename TDataId>
class DataStoreGetObjectEventListenerTemplate;

template <typename TDataId>
class DataStoreReceiveNewArrivedObjectsEventListenerTemplate;

class DataStoreClientInterface
{
public:
    virtual ~DataStoreClientInterface();

    virtual bool GetObject(ProtocolCallContext* pContext, const DataStorePrepareGetParam& rParam,
                           DataStoreGetObjectEventListenerTemplate<uint64_t>* pListener) = 0;
    virtual bool GetObject(ProtocolCallContext* pContext, const DataStorePrepareGetParam& rParam, void* pBuffer,
                           uint32_t uiBufferSize, uint32_t* pReceivedSize) = 0;
    virtual bool GetObjectOrMetaBinary(ProtocolCallContext* pContext, const DataStorePrepareGetParam& rParam,
                                       DataStoreGetObjectEventListenerTemplate<uint64_t>* pListener) = 0;
    virtual bool GetObjectV1(ProtocolCallContext* pContext, const DataStorePrepareGetParamV1& rParam,
                             DataStoreGetObjectEventListenerTemplate<uint32_t>* pListener) = 0;
    virtual bool PostObject(ProtocolCallContext* pContext, const DataStorePreparePostParam& rParam, uint64_t* pDataId,
                            DataStorePostObjectEventListener* pListener) = 0;
    virtual bool PostObject(ProtocolCallContext* pContext, const DataStorePreparePostParam& rParam, uint64_t* pDataId,
                            const void* pData, bool bCopyData) = 0;
    virtual bool PostObject(ProtocolCallContext* pContext, const DataStorePreparePostParam& rParam,
                            uint64_t* pDataId) = 0;
    virtual bool PostObject(ProtocolCallContext* pContext, uint64_t uiDataId,
                            const DataStorePreparePostParam& rParam) = 0;
    virtual bool PostObject(ProtocolCallContext* pContext,
                            const qVector<std::pair<uint64_t, DataStorePreparePostParam> >& rParams,
                            bool bTransactional, qVector<qResult>* pResults) = 0;
    virtual bool PostObjectV1(ProtocolCallContext* pContext, const DataStorePreparePostParamV1& rParam,
                              uint32_t* pDataId, DataStorePostObjectEventListener* pListener) = 0;
    virtual bool UpdateObject(ProtocolCallContext* pContext, const DataStorePrepareUpdateParam& rParam,
                              DataStorePostObjectEventListener* pListener) = 0;
    virtual bool UpdateObject(ProtocolCallContext* pContext, const DataStorePrepareUpdateParam& rParam,
                              const void* pData, bool bCopyData) = 0;
    virtual bool CheckNotification(ProtocolCallContext* pContext, const String& rNotificationUrl,
                                   String* pNextNotificationUrl, uint64_t uiLastDataId, uint64_t* pDataIds,
                                   uint32_t uiDataIdCapacity, uint32_t* pDataIdCount, uint32_t uiTimeout,
                                   bool* pbHasNext, bool bWait) = 0;
    virtual qResult ReceiveNewArrivedObjects(
        ProtocolCallContext* pContext, const unsigned char* pNotificationBuffer,
        qVector<DataStoreGetObjectEventListenerTemplate<uint64_t>*> getObjectListeners,
        DataStoreReceiveNewArrivedObjectsEventListenerTemplate<uint64_t>* pListener, bool* pbHasNext) = 0;
    virtual qResult ReceiveNewArrivedObjectsV1(
        ProtocolCallContext* pContext, const unsigned char* pNotificationBuffer,
        qVector<DataStoreGetObjectEventListenerTemplate<uint32_t>*> getObjectListeners,
        DataStoreReceiveNewArrivedObjectsEventListenerTemplate<uint32_t>* pListener, bool* pbHasNext) = 0;
    virtual bool GetRating(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                           DataStoreRatingInfo* pRatingInfo, uint64_t uiAccessPassword) = 0;
    virtual bool GetRating(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                           qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfos, qVector<qResult>* pResults,
                           uint64_t uiAccessPassword) = 0;
    virtual bool GetRating(ProtocolCallContext* pContext, uint64_t uiDataId,
                           qMap<signed char, DataStoreRatingInfo>* pRatings, uint64_t uiAccessPassword) = 0;
    virtual bool GetRating(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                           qVector<qMap<signed char, DataStoreRatingInfo> >* pRatings, qVector<qResult>* pResults) = 0;
    virtual bool GetRating(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                           DataStoreRatingInfo* pRatingInfo, DataStoreRatingLog* pRatingLog,
                           uint64_t uiAccessPassword) = 0;
    virtual bool GetObjectByInfo(ProtocolCallContext* pContext, const DataStoreReqGetInfo& rReqGetInfo,
                                 DataStoreGetObjectEventListenerTemplate<uint64_t>* pListener) = 0;
    virtual bool GetObjectByInfo(ProtocolCallContext* pContext, const DataStoreReqGetInfo& rReqGetInfo, void* pBuffer,
                                 uint32_t uiBufferSize, uint32_t* pReceivedSize) = 0;
};

}
}

#endif
