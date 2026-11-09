#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECLIENTTEMPLATE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECLIENTTEMPLATE_HPP

#include <stdint.h>
#include <utility>

#include "nn/nex/DataStoreClientInterface.hpp"
#include "nn/nex/DataStoreGetObjectEventListenerTemplate.hpp"
#include "nn/nex/DataStoreLogicServerClient.hpp"

namespace nn
{
namespace nex
{

enum DATASTORE_API_VERSION
{
    DATASTORE_API_VERSION_V1 = 0,
    DATASTORE_API_VERSION_V2 = 1
};

template <DATASTORE_API_VERSION Version>
class DataStoreApiVersionTraits;

template <>
class DataStoreApiVersionTraits<DATASTORE_API_VERSION_V1>
{
public:
    typedef uint32_t TDataId;
    typedef DataStorePrepareGetParamV1 TPrepareGetParam;
    typedef DataStoreReqGetInfoV1 TReqGetInfo;
    typedef DataStorePreparePostParamV1 TPreparePostParam;
    typedef DataStoreReqPostInfoV1 TReqPostInfo;
    typedef DataStoreCompletePostParamV1 TCompletePostParam;
    typedef DataStoreNotificationV1 TNotification;
    typedef DataStoreGetSpecificMetaParamV1 TGetSpecificMetaParam;
    typedef DataStoreSpecificMetaInfoV1 TSpecificMetaInfo;
    typedef DataStoreGetObjectEventListenerTemplate<uint32_t> TGetObjectEventListener;
    typedef DataStoreReceiveNewArrivedObjectsEventListenerTemplate<uint32_t> TReceiveNewArrivedObjectsEventListener;
};

template <>
class DataStoreApiVersionTraits<DATASTORE_API_VERSION_V2>
{
public:
    typedef uint64_t TDataId;
    typedef DataStorePrepareGetParam TPrepareGetParam;
    typedef DataStoreReqGetInfo TReqGetInfo;
    typedef DataStorePreparePostParam TPreparePostParam;
    typedef DataStoreReqPostInfo TReqPostInfo;
    typedef DataStoreCompletePostParam TCompletePostParam;
    typedef DataStoreNotification TNotification;
    typedef DataStoreGetSpecificMetaParam TGetSpecificMetaParam;
    typedef DataStoreSpecificMetaInfo TSpecificMetaInfo;
    typedef DataStoreGetObjectEventListenerTemplate<uint64_t> TGetObjectEventListener;
    typedef DataStoreReceiveNewArrivedObjectsEventListenerTemplate<uint64_t> TReceiveNewArrivedObjectsEventListener;
};

template <typename T>
class DataStoreClientTemplate : public T, public DataStoreClientInterface
{
public:
    DataStoreClientTemplate();
    virtual ~DataStoreClientTemplate();

    bool SetDependentRvConnection(ProtocolCallContext* pCallContext) const;

    virtual bool GetObject(ProtocolCallContext* pContext, const DataStorePrepareGetParam& rParam,
                           DataStoreGetObjectEventListenerTemplate<uint64_t>* pListener);
    virtual bool GetObject(ProtocolCallContext* pContext, const DataStorePrepareGetParam& rParam, void* pBuffer,
                           uint32_t uiBufferSize, uint32_t* pReceivedSize);
    virtual bool GetObjectOrMetaBinary(ProtocolCallContext* pContext, const DataStorePrepareGetParam& rParam,
                                       DataStoreGetObjectEventListenerTemplate<uint64_t>* pListener);
    virtual bool GetObjectV1(ProtocolCallContext* pContext, const DataStorePrepareGetParamV1& rParam,
                             DataStoreGetObjectEventListenerTemplate<uint32_t>* pListener);
    virtual bool PostObject(ProtocolCallContext* pContext, const DataStorePreparePostParam& rParam, uint64_t* pDataId,
                            DataStorePostObjectEventListener* pListener);
    virtual bool PostObject(ProtocolCallContext* pContext, const DataStorePreparePostParam& rParam, uint64_t* pDataId,
                            const void* pData, bool bCopyData);
    virtual bool PostObject(ProtocolCallContext* pContext, const DataStorePreparePostParam& rParam, uint64_t* pDataId);
    virtual bool PostObject(ProtocolCallContext* pContext, uint64_t uiDataId, const DataStorePreparePostParam& rParam);
    virtual bool PostObject(ProtocolCallContext* pContext,
                            const qVector<std::pair<uint64_t, DataStorePreparePostParam> >& rParams,
                            bool bTransactional, qVector<qResult>* pResults);
    virtual bool PostObject(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                            const DataStorePreparePostParam& rParam, bool bTransactional, qVector<qResult>* pResults);
    virtual bool PostObjectV1(ProtocolCallContext* pContext, const DataStorePreparePostParamV1& rParam,
                              uint32_t* pDataId, DataStorePostObjectEventListener* pListener);
    virtual bool UpdateObject(ProtocolCallContext* pContext, const DataStorePrepareUpdateParam& rParam,
                              DataStorePostObjectEventListener* pListener);
    virtual bool UpdateObject(ProtocolCallContext* pContext, const DataStorePrepareUpdateParam& rParam,
                              const void* pData, bool bCopyData);
    virtual bool CheckNotification(ProtocolCallContext* pContext, const String& rNotificationUrl,
                                   String* pNextNotificationUrl, uint64_t uiLastDataId, uint64_t* pLastDataId,
                                   uint32_t uiSequence, uint32_t* pSequence, uint32_t uiKey, bool* pbUrlRequestFailed,
                                   bool bForceUrlRequest);
    virtual qResult CheckNewArrival(ProtocolCallContext* pContext, const unsigned char* pNotificationBuffer,
                                    DataStoreReceiveNewArrivedObjectsEventListenerTemplate<uint64_t>* pListener,
                                    bool* pbHasNext);
    virtual qResult ReceiveNewArrivedObjects(
        ProtocolCallContext* pContext, const unsigned char* pNotificationBuffer,
        qVector<DataStoreGetObjectEventListenerTemplate<uint64_t>*> getObjectListeners,
        DataStoreReceiveNewArrivedObjectsEventListenerTemplate<uint64_t>* pListener, bool* pbHasNext);
    virtual qResult ReceiveNewArrivedObjectsV1(
        ProtocolCallContext* pContext, const unsigned char* pNotificationBuffer,
        qVector<DataStoreGetObjectEventListenerTemplate<uint32_t>*> getObjectListeners,
        DataStoreReceiveNewArrivedObjectsEventListenerTemplate<uint32_t>* pListener, bool* pbHasNext);
    virtual bool GetRating(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                           DataStoreRatingInfo* pRatingInfo, uint64_t uiAccessPassword);
    virtual bool GetRating(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                           qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfos, qVector<qResult>* pResults,
                           uint64_t uiAccessPassword);
    virtual bool GetRating(ProtocolCallContext* pContext, uint64_t uiDataId,
                           qMap<signed char, DataStoreRatingInfo>* pRatings, uint64_t uiAccessPassword);
    virtual bool GetRating(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                           qVector<qMap<signed char, DataStoreRatingInfo> >* pRatings, qVector<qResult>* pResults);
    virtual bool GetRating(ProtocolCallContext* pContext, const DataStoreRatingTarget& rTarget,
                           DataStoreRatingInfo* pRatingInfo, DataStoreRatingLog* pRatingLog, uint64_t uiAccessPassword);
    virtual bool GetObjectByInfo(ProtocolCallContext* pContext, const DataStoreReqGetInfo& rReqGetInfo,
                                 DataStoreGetObjectEventListenerTemplate<uint64_t>* pListener);
    virtual bool GetObjectByInfo(ProtocolCallContext* pContext, const DataStoreReqGetInfo& rReqGetInfo, void* pBuffer,
                                 uint32_t uiBufferSize, uint32_t* pReceivedSize);

private:
    template <typename TTraits>
    bool GetObjectImpl(ProtocolCallContext* pContext, const typename TTraits::TPrepareGetParam& rParam,
                       typename TTraits::TGetObjectEventListener* pListener, unsigned char* pBuffer,
                       uint32_t uiBufferSize, bool bOrMetaBinary, uint32_t* pReceivedSize);
    template <typename TTraits>
    bool PostObjectImpl(ProtocolCallContext* pContext, const typename TTraits::TPreparePostParam& rParam,
                        typename TTraits::TDataId* pDataId, DataStorePostObjectEventListener* pListener,
                        const unsigned char* pData, bool bCopyData);
    bool UpdateObjectImpl(ProtocolCallContext* pContext, const DataStorePrepareUpdateParam& rParam,
                          DataStorePostObjectEventListener* pListener, const unsigned char* pData, bool bCopyData);
    bool CheckNotificationImpl(ProtocolCallContext* pContext, const String& rNotificationUrl,
                               String* pNextNotificationUrl, uint64_t uiLastDataId, uint64_t* pLastDataId,
                               uint32_t uiSequence, uint32_t* pSequence, uint32_t uiKey, bool* pbUrlRequestFailed,
                               bool bForceUrlRequest);
    template <typename TTraits>
    qResult CheckNewArrivalImpl(ProtocolCallContext* pContext, const unsigned char* pNotificationBuffer,
                                DataStoreReceiveNewArrivedObjectsEventListenerTemplate<uint64_t>* pListener,
                                bool* pbHasNext);
    template <typename TTraits>
    qResult ReceiveNewArrivedObjectsImpl(ProtocolCallContext* pContext, const unsigned char* pNotificationBuffer,
                                         qVector<typename TTraits::TGetObjectEventListener*> getObjectListeners,
                                         typename TTraits::TReceiveNewArrivedObjectsEventListener* pListener,
                                         bool* pbHasNext);
    template <typename TTraits>
    bool GetObjectByInfoImpl(ProtocolCallContext* pContext, const DataStoreReqGetInfo& rReqGetInfo,
                             typename TTraits::TGetObjectEventListener* pListener, unsigned char* pBuffer,
                             uint32_t uiBufferSize, uint32_t* pReceivedSize);
    bool GetRatingImpl(ProtocolCallContext* pContext, uint64_t uiDataId,
                       qMap<signed char, DataStoreRatingInfo>* pRatings, uint64_t uiAccessPassword);
    bool GetRatingImpl(ProtocolCallContext* pContext, const qVector<uint64_t>& rDataIds,
                       qVector<qMap<signed char, DataStoreRatingInfo> >* pRatings, qVector<qResult>* pResults);

    enum
    {
        DATASTORE_PROTOCOL_ID = 115
    };

    unsigned int m_uiThreadPriority;
    bool m_bUseHttps;
    unsigned int m_uiMinimumTransferRate;
    int m_iMinimumTimeout;
    unsigned int m_uiBufferSize;
    unsigned int m_uiTransferBufferSize;
};

typedef DataStoreClientTemplate<DataStoreLogicServerClient> DataStoreClient;

}
}

#endif
