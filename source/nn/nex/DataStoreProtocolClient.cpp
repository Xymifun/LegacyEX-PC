#include "nn/nex/DataStoreProtocolClient.hpp"

#include "nn/nex/_DDL_DataStoreChangeMetaParam.hpp"
#include "nn/nex/_DDL_DataStoreChangeMetaParamV1.hpp"
#include "nn/nex/_DDL_DataStoreCompletePostParam.hpp"
#include "nn/nex/_DDL_DataStoreCompletePostParamV1.hpp"
#include "nn/nex/_DDL_DataStoreCompleteUpdateParam.hpp"
#include "nn/nex/_DDL_DataStoreDeleteParam.hpp"
#include "nn/nex/_DDL_DataStoreGetMetaParam.hpp"
#include "nn/nex/_DDL_DataStoreGetNewArrivedNotificationsParam.hpp"
#include "nn/nex/_DDL_DataStoreGetNotificationUrlParam.hpp"
#include "nn/nex/_DDL_DataStoreGetSpecificMetaParam.hpp"
#include "nn/nex/_DDL_DataStoreGetSpecificMetaParamV1.hpp"
#include "nn/nex/_DDL_DataStoreMetaInfo.hpp"
#include "nn/nex/_DDL_DataStoreNotification.hpp"
#include "nn/nex/_DDL_DataStoreNotificationV1.hpp"
#include "nn/nex/_DDL_DataStorePasswordInfo.hpp"
#include "nn/nex/_DDL_DataStorePersistenceInfo.hpp"
#include "nn/nex/_DDL_DataStorePrepareGetParam.hpp"
#include "nn/nex/_DDL_DataStorePrepareGetParamV1.hpp"
#include "nn/nex/_DDL_DataStorePreparePostParam.hpp"
#include "nn/nex/_DDL_DataStorePreparePostParamV1.hpp"
#include "nn/nex/_DDL_DataStorePrepareUpdateParam.hpp"
#include "nn/nex/_DDL_DataStoreRateObjectParam.hpp"
#include "nn/nex/_DDL_DataStoreRatingInfo.hpp"
#include "nn/nex/_DDL_DataStoreRatingInfoWithSlot.hpp"
#include "nn/nex/_DDL_DataStoreRatingLog.hpp"
#include "nn/nex/_DDL_DataStoreRatingTarget.hpp"
#include "nn/nex/_DDL_DataStoreReqGetAdditionalMeta.hpp"
#include "nn/nex/_DDL_DataStoreReqGetInfo.hpp"
#include "nn/nex/_DDL_DataStoreReqGetInfoV1.hpp"
#include "nn/nex/_DDL_DataStoreReqGetNotificationUrlInfo.hpp"
#include "nn/nex/_DDL_DataStoreReqPostInfo.hpp"
#include "nn/nex/_DDL_DataStoreReqPostInfoV1.hpp"
#include "nn/nex/_DDL_DataStoreReqUpdateInfo.hpp"
#include "nn/nex/_DDL_DataStoreSearchParam.hpp"
#include "nn/nex/_DDL_DataStoreSearchResult.hpp"
#include "nn/nex/_DDL_DataStoreSpecificMetaInfo.hpp"
#include "nn/nex/_DDL_DataStoreSpecificMetaInfoV1.hpp"
#include "nn/nex/_DDL_DataStoreTouchObjectParam.hpp"
#include "nn/nex/_Type_qresult.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DataStoreChangeMetaParam.hpp"
#include "nn/nex/DataStoreChangeMetaParamV1.hpp"
#include "nn/nex/DataStoreCompletePostParam.hpp"
#include "nn/nex/DataStoreCompletePostParamV1.hpp"
#include "nn/nex/DataStoreCompleteUpdateParam.hpp"
#include "nn/nex/DataStoreDeleteParam.hpp"
#include "nn/nex/DataStoreGetMetaParam.hpp"
#include "nn/nex/DataStoreGetNewArrivedNotificationsParam.hpp"
#include "nn/nex/DataStoreGetNotificationUrlParam.hpp"
#include "nn/nex/DataStoreGetSpecificMetaParam.hpp"
#include "nn/nex/DataStoreGetSpecificMetaParamV1.hpp"
#include "nn/nex/DataStoreMetaInfo.hpp"
#include "nn/nex/DataStoreNotification.hpp"
#include "nn/nex/DataStoreNotificationV1.hpp"
#include "nn/nex/DataStorePasswordInfo.hpp"
#include "nn/nex/DataStorePersistenceInfo.hpp"
#include "nn/nex/DataStorePrepareGetParam.hpp"
#include "nn/nex/DataStorePrepareGetParamV1.hpp"
#include "nn/nex/DataStorePreparePostParam.hpp"
#include "nn/nex/DataStorePreparePostParamV1.hpp"
#include "nn/nex/DataStorePrepareUpdateParam.hpp"
#include "nn/nex/DataStoreRateObjectParam.hpp"
#include "nn/nex/DataStoreRatingInfo.hpp"
#include "nn/nex/DataStoreRatingInfoWithSlot.hpp"
#include "nn/nex/DataStoreRatingLog.hpp"
#include "nn/nex/DataStoreRatingTarget.hpp"
#include "nn/nex/DataStoreReqGetAdditionalMeta.hpp"
#include "nn/nex/DataStoreReqGetInfo.hpp"
#include "nn/nex/DataStoreReqGetInfoV1.hpp"
#include "nn/nex/DataStoreReqGetNotificationUrlInfo.hpp"
#include "nn/nex/DataStoreReqPostInfo.hpp"
#include "nn/nex/DataStoreReqPostInfoV1.hpp"
#include "nn/nex/DataStoreReqUpdateInfo.hpp"
#include "nn/nex/DataStoreSearchParam.hpp"
#include "nn/nex/DataStoreSearchResult.hpp"
#include "nn/nex/DataStoreSpecificMetaInfo.hpp"
#include "nn/nex/DataStoreSpecificMetaInfoV1.hpp"
#include "nn/nex/DataStoreTouchObjectParam.hpp"
#include "nn/nex/DDLClassMetaType.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

typedef void (*ProtoReturnFunction)(Message* pMessage, ProtocolCallContext* pCallContext);
const unsigned int METHOD_ID_MASK = 0x7FFF;
enum
{
    RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u)
};
const unsigned int METHOD_ID_COUNT = 47;
enum MethodId
{
    METHOD_PREPARE_GET_OBJECT_V1 = 1,
    METHOD_PREPARE_POST_OBJECT_V1 = 2,
    METHOD_COMPLETE_POST_OBJECT_V1 = 3,
    METHOD_DELETE_OBJECT = 4,
    METHOD_DELETE_OBJECTS = 5,
    METHOD_CHANGE_META_V1 = 6,
    METHOD_CHANGE_METAS_V1 = 7,
    METHOD_GET_META = 8,
    METHOD_GET_METAS = 9,
    METHOD_PREPARE_UPDATE_OBJECT = 10,
    METHOD_COMPLETE_UPDATE_OBJECT = 11,
    METHOD_SEARCH_OBJECT = 12,
    METHOD_GET_NOTIFICATION_URL = 13,
    METHOD_GET_NEW_ARRIVED_NOTIFICATIONS_V1 = 14,
    METHOD_RATE_OBJECT = 15,
    METHOD_GET_RATING = 16,
    METHOD_GET_RATINGS = 17,
    METHOD_RESET_RATING = 18,
    METHOD_RESET_RATINGS = 19,
    METHOD_GET_SPECIFIC_META_V1 = 20,
    METHOD_POST_META_BINARY = 21,
    METHOD_TOUCH_OBJECT = 22,
    METHOD_GET_RATING_WITH_LOG = 23,
    METHOD_PREPARE_POST_OBJECT = 24,
    METHOD_PREPARE_GET_OBJECT = 25,
    METHOD_COMPLETE_POST_OBJECT = 26,
    METHOD_GET_NEW_ARRIVED_NOTIFICATIONS = 27,
    METHOD_GET_SPECIFIC_META = 28,
    METHOD_GET_PERSISTENCE_INFO = 29,
    METHOD_GET_PERSISTENCE_INFOS = 30,
    METHOD_PERPETUATE_OBJECT = 31,
    METHOD_UNPERPETUATE_OBJECT = 32,
    METHOD_PREPARE_GET_OBJECT_OR_META_BINARY = 33,
    METHOD_GET_PASSWORD_INFO = 34,
    METHOD_GET_PASSWORD_INFOS = 35,
    METHOD_GET_METAS_MULTIPLE_PARAM = 36,
    METHOD_COMPLETE_POST_OBJECTS = 37,
    METHOD_CHANGE_META = 38,
    METHOD_CHANGE_METAS = 39,
    METHOD_RATE_OBJECTS = 40,
    METHOD_POST_META_BINARY_WITH_DATA_ID = 41,
    METHOD_POST_META_BINARIES_WITH_DATA_ID = 42,
    METHOD_RATE_OBJECT_WITH_POSTING = 43,
    METHOD_RATE_OBJECTS_WITH_POSTING = 44,
    METHOD_GET_OBJECT_INFOS = 45,
    METHOD_SEARCH_OBJECT_LIGHT = 46
};
ProtoReturnFunction s_ProtoReturn[METHOD_ID_COUNT];

}

void DataStoreProtocolClient::ProtoReturn_PrepareGetObject(Message* pMessage, ProtocolCallContext* pCallContext)
{
    DataStoreReqGetInfo* pReqGetInfo = static_cast<DataStoreReqGetInfo*>(pCallContext->GetReturnValuePtr(0));
    if (pReqGetInfo != 0)
    {
        _DDL_DataStoreReqGetInfo::Extract(pMessage, pReqGetInfo);
        return;
    }
    DataStoreReqGetInfo discarded[1];
    _DDL_DataStoreReqGetInfo::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallPrepareGetObject(ProtocolCallContext* pCallContext,
                                                   const DataStorePrepareGetParam& rPrepareGetParam,
                                                   DataStoreReqGetInfo* pReqGetInfo)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_PREPARE_GET_OBJECT);
    _DDL_DataStorePrepareGetParam::Add(&message, rPrepareGetParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqGetInfo);
    }
    s_ProtoReturn[METHOD_PREPARE_GET_OBJECT] = ProtoReturn_PrepareGetObject;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_PrepareGetObjectV1(Message* pMessage, ProtocolCallContext* pCallContext)
{
    DataStoreReqGetInfoV1* pReqGetInfoV1 = static_cast<DataStoreReqGetInfoV1*>(pCallContext->GetReturnValuePtr(0));
    if (pReqGetInfoV1 != 0)
    {
        _DDL_DataStoreReqGetInfoV1::Extract(pMessage, pReqGetInfoV1);
        return;
    }
    DataStoreReqGetInfoV1 discarded[1];
    _DDL_DataStoreReqGetInfoV1::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallPrepareGetObjectV1(ProtocolCallContext* pCallContext,
                                                     const DataStorePrepareGetParamV1& rPrepareGetParamV1,
                                                     DataStoreReqGetInfoV1* pReqGetInfoV1)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_PREPARE_GET_OBJECT_V1);
    _DDL_DataStorePrepareGetParamV1::Add(&message, rPrepareGetParamV1);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqGetInfoV1);
    }
    s_ProtoReturn[METHOD_PREPARE_GET_OBJECT_V1] = ProtoReturn_PrepareGetObjectV1;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_PrepareGetObjectOrMetaBinary(Message* pMessage,
                                                                       ProtocolCallContext* pCallContext)
{
    DataStoreReqGetInfo* pReqGetInfo = static_cast<DataStoreReqGetInfo*>(pCallContext->GetReturnValuePtr(0));
    if (pReqGetInfo != 0)
    {
        _DDL_DataStoreReqGetInfo::Extract(pMessage, pReqGetInfo);
        if (pMessage->HasError())
        {
            return;
        }
    }
    else
    {
        DataStoreReqGetInfo discarded[1];
        _DDL_DataStoreReqGetInfo::Extract(pMessage, discarded);
        if (pMessage->HasError())
        {
            return;
        }
    }

    DataStoreReqGetAdditionalMeta* pReqGetAdditionalMeta = static_cast<DataStoreReqGetAdditionalMeta*>(
        pCallContext->GetReturnValuePtr(1));
    if (pReqGetAdditionalMeta == 0)
    {
        DataStoreReqGetAdditionalMeta discarded[1];
        _DDL_DataStoreReqGetAdditionalMeta::Extract(pMessage, discarded);
        return;
    }
    _DDL_DataStoreReqGetAdditionalMeta::Extract(pMessage, pReqGetAdditionalMeta);
}

bool DataStoreProtocolClient::CallPrepareGetObjectOrMetaBinary(ProtocolCallContext* pCallContext,
                                                               const DataStorePrepareGetParam& rPrepareGetParam,
                                                               DataStoreReqGetInfo* pReqGetInfo,
                                                               DataStoreReqGetAdditionalMeta* pReqGetAdditionalMeta)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_PREPARE_GET_OBJECT_OR_META_BINARY);
    _DDL_DataStorePrepareGetParam::Add(&message, rPrepareGetParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqGetInfo);
        pCallContext->AddReturnValuePtr(pReqGetAdditionalMeta);
    }
    s_ProtoReturn[METHOD_PREPARE_GET_OBJECT_OR_META_BINARY] = ProtoReturn_PrepareGetObjectOrMetaBinary;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_PreparePostObject(Message* pMessage, ProtocolCallContext* pCallContext)
{
    DataStoreReqPostInfo* pReqPostInfo = static_cast<DataStoreReqPostInfo*>(pCallContext->GetReturnValuePtr(0));
    if (pReqPostInfo != 0)
    {
        _DDL_DataStoreReqPostInfo::Extract(pMessage, pReqPostInfo);
        return;
    }
    DataStoreReqPostInfo discarded[1];
    _DDL_DataStoreReqPostInfo::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallPreparePostObject(ProtocolCallContext* pCallContext,
                                                    const DataStorePreparePostParam& rPreparePostParam,
                                                    DataStoreReqPostInfo* pReqPostInfo)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_PREPARE_POST_OBJECT);
    _DDL_DataStorePreparePostParam::Add(&message, rPreparePostParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqPostInfo);
    }
    s_ProtoReturn[METHOD_PREPARE_POST_OBJECT] = ProtoReturn_PreparePostObject;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_PreparePostObjectV1(Message* pMessage, ProtocolCallContext* pCallContext)
{
    DataStoreReqPostInfoV1* pReqPostInfoV1 = static_cast<DataStoreReqPostInfoV1*>(pCallContext->GetReturnValuePtr(0));
    if (pReqPostInfoV1 != 0)
    {
        _DDL_DataStoreReqPostInfoV1::Extract(pMessage, pReqPostInfoV1);
        return;
    }
    DataStoreReqPostInfoV1 discarded[1];
    _DDL_DataStoreReqPostInfoV1::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallPreparePostObjectV1(ProtocolCallContext* pCallContext,
                                                      const DataStorePreparePostParamV1& rPreparePostParamV1,
                                                      DataStoreReqPostInfoV1* pReqPostInfoV1)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_PREPARE_POST_OBJECT_V1);
    _DDL_DataStorePreparePostParamV1::Add(&message, rPreparePostParamV1);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqPostInfoV1);
    }
    s_ProtoReturn[METHOD_PREPARE_POST_OBJECT_V1] = ProtoReturn_PreparePostObjectV1;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::CallCompletePostObject(ProtocolCallContext* pCallContext,
                                                     const DataStoreCompletePostParam& rCompletePostParam)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_COMPLETE_POST_OBJECT);
    _DDL_DataStoreCompletePostParam::Add(&message, rCompletePostParam);
    s_ProtoReturn[METHOD_COMPLETE_POST_OBJECT] = ProtoReturn_CompletePostObject;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::CallCompletePostObjectV1(ProtocolCallContext* pCallContext,
                                                       const DataStoreCompletePostParamV1& rCompletePostParamV1)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_COMPLETE_POST_OBJECT_V1);
    _DDL_DataStoreCompletePostParamV1::Add(&message, rCompletePostParamV1);
    s_ProtoReturn[METHOD_COMPLETE_POST_OBJECT_V1] = ProtoReturn_CompletePostObjectV1;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::CallCompletePostObjects(ProtocolCallContext* pCallContext,
                                                      const qVector<unsigned long long>& rDataIds)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_COMPLETE_POST_OBJECTS);
    _Type_qvector<_Type_uint64>::Add(&message, rDataIds);
    s_ProtoReturn[METHOD_COMPLETE_POST_OBJECTS] = ProtoReturn_CompletePostObjects;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_PrepareUpdateObject(Message* pMessage, ProtocolCallContext* pCallContext)
{
    DataStoreReqUpdateInfo* pReqUpdateInfo = static_cast<DataStoreReqUpdateInfo*>(pCallContext->GetReturnValuePtr(0));
    if (pReqUpdateInfo != 0)
    {
        _DDL_DataStoreReqUpdateInfo::Extract(pMessage, pReqUpdateInfo);
        return;
    }
    DataStoreReqUpdateInfo discarded[1];
    _DDL_DataStoreReqUpdateInfo::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallPrepareUpdateObject(ProtocolCallContext* pCallContext,
                                                      const DataStorePrepareUpdateParam& rPrepareUpdateParam,
                                                      DataStoreReqUpdateInfo* pReqUpdateInfo)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_PREPARE_UPDATE_OBJECT);
    _DDL_DataStorePrepareUpdateParam::Add(&message, rPrepareUpdateParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqUpdateInfo);
    }
    s_ProtoReturn[METHOD_PREPARE_UPDATE_OBJECT] = ProtoReturn_PrepareUpdateObject;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::CallCompleteUpdateObject(ProtocolCallContext* pCallContext,
                                                       const DataStoreCompleteUpdateParam& rCompleteUpdateParam)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_COMPLETE_UPDATE_OBJECT);
    _DDL_DataStoreCompleteUpdateParam::Add(&message, rCompleteUpdateParam);
    s_ProtoReturn[METHOD_COMPLETE_UPDATE_OBJECT] = ProtoReturn_CompleteUpdateObject;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::CallDeleteObject(ProtocolCallContext* pCallContext,
                                               const DataStoreDeleteParam& rDeleteParam)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_DELETE_OBJECT);
    _DDL_DataStoreDeleteParam::Add(&message, rDeleteParam);
    s_ProtoReturn[METHOD_DELETE_OBJECT] = ProtoReturn_DeleteObject;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::CallChangeMeta(ProtocolCallContext* pCallContext,
                                             const DataStoreChangeMetaParam& rChangeMetaParam)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_CHANGE_META);
    _DDL_DataStoreChangeMetaParam::Add(&message, rChangeMetaParam);
    s_ProtoReturn[METHOD_CHANGE_META] = ProtoReturn_ChangeMeta;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetMeta(Message* pMessage, ProtocolCallContext* pCallContext)
{
    DataStoreMetaInfo* pMetaInfo = static_cast<DataStoreMetaInfo*>(pCallContext->GetReturnValuePtr(0));
    if (pMetaInfo != 0)
    {
        _DDL_DataStoreMetaInfo::Extract(pMessage, pMetaInfo);
        return;
    }
    DataStoreMetaInfo discarded[1];
    _DDL_DataStoreMetaInfo::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallGetMeta(ProtocolCallContext* pCallContext, const DataStoreGetMetaParam& rGetMetaParam,
                                          DataStoreMetaInfo* pMetaInfo)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_META);
    _DDL_DataStoreGetMetaParam::Add(&message, rGetMetaParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMetaInfo);
    }
    s_ProtoReturn[METHOD_GET_META] = ProtoReturn_GetMeta;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetMetas(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qVector<DataStoreMetaInfo>* pMetaInfos = static_cast<qVector<DataStoreMetaInfo>*>(
        pCallContext->GetReturnValuePtr(0));
    if (pMetaInfos != 0)
    {
        _Type_qvector<DDLClassMetaType<DataStoreMetaInfo> >::Extract(pMessage, pMetaInfos);
        if (pMessage->HasError())
        {
            return;
        }
    }
    else
    {
        qVector<DataStoreMetaInfo> discarded[1];
        _Type_qvector<DDLClassMetaType<DataStoreMetaInfo> >::Extract(pMessage, discarded);
        if (pMessage->HasError())
        {
            return;
        }
    }

    qVector<qResult>* pResults = static_cast<qVector<qResult>*>(pCallContext->GetReturnValuePtr(1));
    if (pResults != 0)
    {
        _Type_qvector<_Type_qresult>::Extract(pMessage, pResults);
        return;
    }
    qVector<qResult> discarded[1];
    _Type_qvector<_Type_qresult>::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallGetMetas(ProtocolCallContext* pCallContext,
                                           const qVector<unsigned long long>& rDataIds,
                                           const DataStoreGetMetaParam& rGetMetaParam,
                                           qVector<DataStoreMetaInfo>* pMetaInfos, qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_METAS);
    _Type_qvector<_Type_uint64>::Add(&message, rDataIds);
    _DDL_DataStoreGetMetaParam::Add(&message, rGetMetaParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMetaInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_METAS] = ProtoReturn_GetMetas;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetMetasMultipleParam(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qVector<DataStoreMetaInfo>* pMetaInfos = static_cast<qVector<DataStoreMetaInfo>*>(
        pCallContext->GetReturnValuePtr(0));
    if (pMetaInfos != 0)
    {
        _Type_qvector<DDLClassMetaType<DataStoreMetaInfo> >::Extract(pMessage, pMetaInfos);
        if (pMessage->HasError())
        {
            return;
        }
    }
    else
    {
        qVector<DataStoreMetaInfo> discarded[1];
        _Type_qvector<DDLClassMetaType<DataStoreMetaInfo> >::Extract(pMessage, discarded);
        if (pMessage->HasError())
        {
            return;
        }
    }

    qVector<qResult>* pResults = static_cast<qVector<qResult>*>(pCallContext->GetReturnValuePtr(1));
    if (pResults != 0)
    {
        _Type_qvector<_Type_qresult>::Extract(pMessage, pResults);
        return;
    }
    qVector<qResult> discarded[1];
    _Type_qvector<_Type_qresult>::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallGetMetasMultipleParam(ProtocolCallContext* pCallContext,
                                                        const qVector<DataStoreGetMetaParam>& rGetMetaParams,
                                                        qVector<DataStoreMetaInfo>* pMetaInfos,
                                                        qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_METAS_MULTIPLE_PARAM);
    _Type_qvector<DDLClassMetaType<DataStoreGetMetaParam> >::Add(&message, rGetMetaParams);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMetaInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_METAS_MULTIPLE_PARAM] = ProtoReturn_GetMetasMultipleParam;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_SearchObject(Message* pMessage, ProtocolCallContext* pCallContext)
{
    DataStoreSearchResult* pSearchResult = static_cast<DataStoreSearchResult*>(pCallContext->GetReturnValuePtr(0));
    if (pSearchResult != 0)
    {
        _DDL_DataStoreSearchResult::Extract(pMessage, pSearchResult);
        return;
    }
    DataStoreSearchResult discarded[1];
    _DDL_DataStoreSearchResult::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallSearchObject(ProtocolCallContext* pCallContext,
                                               const DataStoreSearchParam& rSearchParam,
                                               DataStoreSearchResult* pSearchResult)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_SEARCH_OBJECT);
    _DDL_DataStoreSearchParam::Add(&message, rSearchParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSearchResult);
    }
    s_ProtoReturn[METHOD_SEARCH_OBJECT] = ProtoReturn_SearchObject;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_SearchObjectLight(Message* pMessage, ProtocolCallContext* pCallContext)
{
    DataStoreSearchResult* pSearchResult = static_cast<DataStoreSearchResult*>(pCallContext->GetReturnValuePtr(0));
    if (pSearchResult != 0)
    {
        _DDL_DataStoreSearchResult::Extract(pMessage, pSearchResult);
        return;
    }
    DataStoreSearchResult discarded[1];
    _DDL_DataStoreSearchResult::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallSearchObjectLight(ProtocolCallContext* pCallContext,
                                                    const DataStoreSearchParam& rSearchParam,
                                                    DataStoreSearchResult* pSearchResult)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_SEARCH_OBJECT_LIGHT);
    _DDL_DataStoreSearchParam::Add(&message, rSearchParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSearchResult);
    }
    s_ProtoReturn[METHOD_SEARCH_OBJECT_LIGHT] = ProtoReturn_SearchObjectLight;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetNotificationUrl(Message* pMessage, ProtocolCallContext* pCallContext)
{
    DataStoreReqGetNotificationUrlInfo* pReqGetNotificationUrlInfo = static_cast<DataStoreReqGetNotificationUrlInfo*>(
        pCallContext->GetReturnValuePtr(0));
    if (pReqGetNotificationUrlInfo != 0)
    {
        _DDL_DataStoreReqGetNotificationUrlInfo::Extract(pMessage, pReqGetNotificationUrlInfo);
        return;
    }
    DataStoreReqGetNotificationUrlInfo discarded[1];
    _DDL_DataStoreReqGetNotificationUrlInfo::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallGetNotificationUrl(ProtocolCallContext* pCallContext,
                                                     const DataStoreGetNotificationUrlParam& rGetNotificationUrlParam,
                                                     DataStoreReqGetNotificationUrlInfo* pReqGetNotificationUrlInfo)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_NOTIFICATION_URL);
    _DDL_DataStoreGetNotificationUrlParam::Add(&message, rGetNotificationUrlParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqGetNotificationUrlInfo);
    }
    s_ProtoReturn[METHOD_GET_NOTIFICATION_URL] = ProtoReturn_GetNotificationUrl;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetNewArrivedNotifications(Message* pMessage,
                                                                     ProtocolCallContext* pCallContext)
{
    qVector<DataStoreNotification>* pNotifications = static_cast<qVector<DataStoreNotification>*>(
        pCallContext->GetReturnValuePtr(0));
    if (pNotifications != 0)
    {
        _Type_qvector<DDLClassMetaType<DataStoreNotification> >::Extract(pMessage, pNotifications);
        if (pMessage->HasError())
        {
            return;
        }
    }
    else
    {
        qVector<DataStoreNotification> discarded[1];
        _Type_qvector<DDLClassMetaType<DataStoreNotification> >::Extract(pMessage, discarded);
        if (pMessage->HasError())
        {
            return;
        }
    }

    bool* pFlag = static_cast<bool*>(pCallContext->GetReturnValuePtr(1));
    if (pFlag != 0)
    {
        *pMessage >> *pFlag;
        return;
    }
    bool discarded;
    *pMessage >> discarded;
}

bool DataStoreProtocolClient::CallGetNewArrivedNotifications(
    ProtocolCallContext* pCallContext, const DataStoreGetNewArrivedNotificationsParam& rGetNewArrivedNotificationsParam,
    qVector<DataStoreNotification>* pNotifications, bool* pFlag)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_NEW_ARRIVED_NOTIFICATIONS);
    _DDL_DataStoreGetNewArrivedNotificationsParam::Add(&message, rGetNewArrivedNotificationsParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pNotifications);
        pCallContext->AddReturnValuePtr(pFlag);
    }
    s_ProtoReturn[METHOD_GET_NEW_ARRIVED_NOTIFICATIONS] = ProtoReturn_GetNewArrivedNotifications;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetNewArrivedNotificationsV1(Message* pMessage,
                                                                       ProtocolCallContext* pCallContext)
{
    qVector<DataStoreNotificationV1>* pNotificationV1s = static_cast<qVector<DataStoreNotificationV1>*>(
        pCallContext->GetReturnValuePtr(0));
    if (pNotificationV1s != 0)
    {
        _Type_qvector<DDLClassMetaType<DataStoreNotificationV1> >::Extract(pMessage, pNotificationV1s);
        if (pMessage->HasError())
        {
            return;
        }
    }
    else
    {
        qVector<DataStoreNotificationV1> discarded[1];
        _Type_qvector<DDLClassMetaType<DataStoreNotificationV1> >::Extract(pMessage, discarded);
        if (pMessage->HasError())
        {
            return;
        }
    }

    bool* pFlag = static_cast<bool*>(pCallContext->GetReturnValuePtr(1));
    if (pFlag != 0)
    {
        *pMessage >> *pFlag;
        return;
    }
    bool discarded;
    *pMessage >> discarded;
}

bool DataStoreProtocolClient::CallGetNewArrivedNotificationsV1(
    ProtocolCallContext* pCallContext, const DataStoreGetNewArrivedNotificationsParam& rGetNewArrivedNotificationsParam,
    qVector<DataStoreNotificationV1>* pNotificationV1s, bool* pFlag)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_NEW_ARRIVED_NOTIFICATIONS_V1);
    _DDL_DataStoreGetNewArrivedNotificationsParam::Add(&message, rGetNewArrivedNotificationsParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pNotificationV1s);
        pCallContext->AddReturnValuePtr(pFlag);
    }
    s_ProtoReturn[METHOD_GET_NEW_ARRIVED_NOTIFICATIONS_V1] = ProtoReturn_GetNewArrivedNotificationsV1;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_RateObject(Message* pMessage, ProtocolCallContext* pCallContext)
{
    DataStoreRatingInfo* pRatingInfo = static_cast<DataStoreRatingInfo*>(pCallContext->GetReturnValuePtr(0));
    if (pRatingInfo != 0)
    {
        _DDL_DataStoreRatingInfo::Extract(pMessage, pRatingInfo);
        return;
    }
    DataStoreRatingInfo discarded[1];
    _DDL_DataStoreRatingInfo::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallRateObject(ProtocolCallContext* pCallContext,
                                             const DataStoreRatingTarget& rRatingTarget,
                                             const DataStoreRateObjectParam& rRateObjectParam, const bool& rFlag,
                                             DataStoreRatingInfo* pRatingInfo)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_RATE_OBJECT);
    _DDL_DataStoreRatingTarget::Add(&message, rRatingTarget);
    _DDL_DataStoreRateObjectParam::Add(&message, rRateObjectParam);
    message << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfo);
    }
    s_ProtoReturn[METHOD_RATE_OBJECT] = ProtoReturn_RateObject;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetRating(Message* pMessage, ProtocolCallContext* pCallContext)
{
    DataStoreRatingInfo* pRatingInfo = static_cast<DataStoreRatingInfo*>(pCallContext->GetReturnValuePtr(0));
    if (pRatingInfo != 0)
    {
        _DDL_DataStoreRatingInfo::Extract(pMessage, pRatingInfo);
        return;
    }
    DataStoreRatingInfo discarded[1];
    _DDL_DataStoreRatingInfo::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallGetRating(ProtocolCallContext* pCallContext,
                                            const DataStoreRatingTarget& rRatingTarget,
                                            DataStoreRatingInfo* pRatingInfo, const unsigned long long& rDataId)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_RATING);
    _DDL_DataStoreRatingTarget::Add(&message, rRatingTarget);
    _Type_uint64::Add(&message, rDataId);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfo);
    }
    s_ProtoReturn[METHOD_GET_RATING] = ProtoReturn_GetRating;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetRatings(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfoWithSlotss =
        static_cast<qVector<qVector<DataStoreRatingInfoWithSlot> >*>(pCallContext->GetReturnValuePtr(0));
    if (pRatingInfoWithSlotss != 0)
    {
        _Type_qvector<_Type_qvector<DDLClassMetaType<DataStoreRatingInfoWithSlot> > >::Extract(pMessage,
                                                                                               pRatingInfoWithSlotss);
        if (pMessage->HasError())
        {
            return;
        }
    }
    else
    {
        qVector<qVector<DataStoreRatingInfoWithSlot> > discarded[1];
        _Type_qvector<_Type_qvector<DDLClassMetaType<DataStoreRatingInfoWithSlot> > >::Extract(pMessage, discarded);
        if (pMessage->HasError())
        {
            return;
        }
    }

    qVector<qResult>* pResults = static_cast<qVector<qResult>*>(pCallContext->GetReturnValuePtr(1));
    if (pResults != 0)
    {
        _Type_qvector<_Type_qresult>::Extract(pMessage, pResults);
        return;
    }
    qVector<qResult> discarded[1];
    _Type_qvector<_Type_qresult>::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallGetRatings(ProtocolCallContext* pCallContext,
                                             const qVector<unsigned long long>& rDataIds,
                                             qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfoWithSlotss,
                                             qVector<qResult>* pResults, const unsigned long long& rDataId)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_RATINGS);
    _Type_qvector<_Type_uint64>::Add(&message, rDataIds);
    _Type_uint64::Add(&message, rDataId);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfoWithSlotss);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_RATINGS] = ProtoReturn_GetRatings;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::CallResetRating(ProtocolCallContext* pCallContext,
                                              const DataStoreRatingTarget& rRatingTarget,
                                              const unsigned long long& rDataId)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_RESET_RATING);
    _DDL_DataStoreRatingTarget::Add(&message, rRatingTarget);
    _Type_uint64::Add(&message, rDataId);
    s_ProtoReturn[METHOD_RESET_RATING] = ProtoReturn_ResetRating;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_ResetRatings(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pValue = pCallContext->GetReturnValuePtr(0);
    if (pValue != 0)
    {
        _Type_qvector<_Type_qresult>::Extract(pMessage, static_cast<qVector<qResult>*>(pValue));
        return;
    }
    qVector<qResult> discarded[1];
    _Type_qvector<_Type_qresult>::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallResetRatings(ProtocolCallContext* pCallContext,
                                               const qVector<unsigned long long>& rDataIds, const bool& rFlag,
                                               qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_RESET_RATINGS);
    _Type_qvector<_Type_uint64>::Add(&message, rDataIds);
    message << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_RESET_RATINGS] = ProtoReturn_ResetRatings;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetSpecificMeta(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pValue = pCallContext->GetReturnValuePtr(0);
    if (pValue != 0)
    {
        _Type_qvector<DDLClassMetaType<DataStoreSpecificMetaInfo> >::Extract(
            pMessage, static_cast<qVector<DataStoreSpecificMetaInfo>*>(pValue));
        return;
    }
    qVector<DataStoreSpecificMetaInfo> discarded[1];
    _Type_qvector<DDLClassMetaType<DataStoreSpecificMetaInfo> >::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallGetSpecificMeta(ProtocolCallContext* pCallContext,
                                                  const DataStoreGetSpecificMetaParam& rGetSpecificMetaParam,
                                                  qVector<DataStoreSpecificMetaInfo>* pSpecificMetaInfos)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_SPECIFIC_META);
    _DDL_DataStoreGetSpecificMetaParam::Add(&message, rGetSpecificMetaParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSpecificMetaInfos);
    }
    s_ProtoReturn[METHOD_GET_SPECIFIC_META] = ProtoReturn_GetSpecificMeta;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetSpecificMetaV1(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pValue = pCallContext->GetReturnValuePtr(0);
    if (pValue != 0)
    {
        _Type_qvector<DDLClassMetaType<DataStoreSpecificMetaInfoV1> >::Extract(
            pMessage, static_cast<qVector<DataStoreSpecificMetaInfoV1>*>(pValue));
        return;
    }
    qVector<DataStoreSpecificMetaInfoV1> discarded[1];
    _Type_qvector<DDLClassMetaType<DataStoreSpecificMetaInfoV1> >::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallGetSpecificMetaV1(ProtocolCallContext* pCallContext,
                                                    const DataStoreGetSpecificMetaParamV1& rGetSpecificMetaParamV1,
                                                    qVector<DataStoreSpecificMetaInfoV1>* pSpecificMetaInfoV1s)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_SPECIFIC_META_V1);
    _DDL_DataStoreGetSpecificMetaParamV1::Add(&message, rGetSpecificMetaParamV1);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSpecificMetaInfoV1s);
    }
    s_ProtoReturn[METHOD_GET_SPECIFIC_META_V1] = ProtoReturn_GetSpecificMetaV1;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_PostMetaBinary(Message* pMessage, ProtocolCallContext* pCallContext)
{
    unsigned long long discarded;
    unsigned long long* pDataId = static_cast<unsigned long long*>(pCallContext->GetReturnValuePtr(0));
    unsigned long long* pDestination = &discarded;
    if (pDataId != 0)
        pDestination = pDataId;
    pMessage->ExtractPrimitive(pDestination);
}

bool DataStoreProtocolClient::CallPostMetaBinary(ProtocolCallContext* pCallContext,
                                                 const DataStorePreparePostParam& rPreparePostParam,
                                                 unsigned long long* pDataId)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_POST_META_BINARY);
    _DDL_DataStorePreparePostParam::Add(&message, rPreparePostParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pDataId);
    }
    s_ProtoReturn[METHOD_POST_META_BINARY] = ProtoReturn_PostMetaBinary;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::CallPostMetaBinaryWithDataId(ProtocolCallContext* pCallContext,
                                                           const unsigned long long& rDataId,
                                                           const DataStorePreparePostParam& rPreparePostParam)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_POST_META_BINARY_WITH_DATA_ID);
    _Type_uint64::Add(&message, rDataId);
    _DDL_DataStorePreparePostParam::Add(&message, rPreparePostParam);
    s_ProtoReturn[METHOD_POST_META_BINARY_WITH_DATA_ID] = ProtoReturn_PostMetaBinaryWithDataId;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_PostMetaBinariesWithDataId(Message* pMessage,
                                                                     ProtocolCallContext* pCallContext)
{
    void* pValue = pCallContext->GetReturnValuePtr(0);
    if (pValue != 0)
    {
        _Type_qvector<_Type_qresult>::Extract(pMessage, static_cast<qVector<qResult>*>(pValue));
        return;
    }
    qVector<qResult> discarded[1];
    _Type_qvector<_Type_qresult>::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallPostMetaBinariesWithDataId(
    ProtocolCallContext* pCallContext, const qVector<unsigned long long>& rDataIds,
    const qVector<DataStorePreparePostParam>& rPreparePostParams, const bool& rFlag, qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_POST_META_BINARIES_WITH_DATA_ID);
    _Type_qvector<_Type_uint64>::Add(&message, rDataIds);
    _Type_qvector<DDLClassMetaType<DataStorePreparePostParam> >::Add(&message, rPreparePostParams);
    message << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_POST_META_BINARIES_WITH_DATA_ID] = ProtoReturn_PostMetaBinariesWithDataId;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::CallTouchObject(ProtocolCallContext* pCallContext,
                                              const DataStoreTouchObjectParam& rTouchObjectParam)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_TOUCH_OBJECT);
    _DDL_DataStoreTouchObjectParam::Add(&message, rTouchObjectParam);
    s_ProtoReturn[METHOD_TOUCH_OBJECT] = ProtoReturn_TouchObject;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetRatingWithLog(Message* pMessage, ProtocolCallContext* pCallContext)
{
    DataStoreRatingInfo* pRatingInfo = static_cast<DataStoreRatingInfo*>(pCallContext->GetReturnValuePtr(0));
    if (pRatingInfo == 0)
    {
        DataStoreRatingInfo discarded[1];
        pRatingInfo = discarded;
    }
    _DDL_DataStoreRatingInfo::Extract(pMessage, pRatingInfo);
    if (pMessage->HasError())
    {
        return;
    }

    DataStoreRatingLog* pRatingLog = static_cast<DataStoreRatingLog*>(pCallContext->GetReturnValuePtr(1));
    if (pRatingLog == 0)
    {
        DataStoreRatingLog discarded[1];
        _DDL_DataStoreRatingLog::Extract(pMessage, discarded);
    }
    else
    {
        _DDL_DataStoreRatingLog::Extract(pMessage, pRatingLog);
    }
}

bool DataStoreProtocolClient::CallGetRatingWithLog(ProtocolCallContext* pCallContext,
                                                   const DataStoreRatingTarget& rRatingTarget,
                                                   DataStoreRatingInfo* pRatingInfo, DataStoreRatingLog* pRatingLog,
                                                   const unsigned long long& rDataId)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_RATING_WITH_LOG);
    _DDL_DataStoreRatingTarget::Add(&message, rRatingTarget);
    _Type_uint64::Add(&message, rDataId);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfo);
        pCallContext->AddReturnValuePtr(pRatingLog);
    }
    s_ProtoReturn[METHOD_GET_RATING_WITH_LOG] = ProtoReturn_GetRatingWithLog;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetPersistenceInfo(Message* pMessage, ProtocolCallContext* pCallContext)
{
    DataStorePersistenceInfo* pPersistenceInfo = static_cast<DataStorePersistenceInfo*>(
        pCallContext->GetReturnValuePtr(0));
    if (pPersistenceInfo != 0)
    {
        _DDL_DataStorePersistenceInfo::Extract(pMessage, pPersistenceInfo);
        return;
    }
    DataStorePersistenceInfo discarded[1];
    _DDL_DataStorePersistenceInfo::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallGetPersistenceInfo(ProtocolCallContext* pCallContext, const unsigned int& rValue,
                                                     const unsigned short& rSlot,
                                                     DataStorePersistenceInfo* pPersistenceInfo)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_PERSISTENCE_INFO);
    _Type_uint32::Add(&message, rValue);
    _Type_uint16::Add(&message, rSlot);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPersistenceInfo);
    }
    s_ProtoReturn[METHOD_GET_PERSISTENCE_INFO] = ProtoReturn_GetPersistenceInfo;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetPersistenceInfos(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qVector<DataStorePersistenceInfo>* pPersistenceInfos = static_cast<qVector<DataStorePersistenceInfo>*>(
        pCallContext->GetReturnValuePtr(0));
    if (pPersistenceInfos != 0)
    {
        _Type_qvector<DDLClassMetaType<DataStorePersistenceInfo> >::Extract(pMessage, pPersistenceInfos);
        if (pMessage->HasError())
        {
            return;
        }
    }
    else
    {
        qVector<DataStorePersistenceInfo> discarded[1];
        _Type_qvector<DDLClassMetaType<DataStorePersistenceInfo> >::Extract(pMessage, discarded);
        if (pMessage->HasError())
        {
            return;
        }
    }

    qVector<qResult>* pResults = static_cast<qVector<qResult>*>(pCallContext->GetReturnValuePtr(1));
    if (pResults != 0)
    {
        _Type_qvector<_Type_qresult>::Extract(pMessage, pResults);
        return;
    }
    qVector<qResult> discarded[1];
    _Type_qvector<_Type_qresult>::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallGetPersistenceInfos(ProtocolCallContext* pCallContext, const unsigned int& rValue,
                                                      const qVector<unsigned short>& rSlots,
                                                      qVector<DataStorePersistenceInfo>* pPersistenceInfos,
                                                      qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_PERSISTENCE_INFOS);
    _Type_uint32::Add(&message, rValue);
    _Type_qvector<_Type_uint16>::Add(&message, rSlots);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPersistenceInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_PERSISTENCE_INFOS] = ProtoReturn_GetPersistenceInfos;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::CallPerpetuateObject(ProtocolCallContext* pCallContext, const unsigned short& rSlot,
                                                   const unsigned long long& rDataId, const bool& rFlag)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_PERPETUATE_OBJECT);
    _Type_uint16::Add(&message, rSlot);
    _Type_uint64::Add(&message, rDataId);
    message << rFlag;
    s_ProtoReturn[METHOD_PERPETUATE_OBJECT] = ProtoReturn_PerpetuateObject;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::CallUnperpetuateObject(ProtocolCallContext* pCallContext, const unsigned short& rSlot,
                                                     const bool& rFlag)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UNPERPETUATE_OBJECT);
    _Type_uint16::Add(&message, rSlot);
    message << rFlag;
    s_ProtoReturn[METHOD_UNPERPETUATE_OBJECT] = ProtoReturn_UnperpetuateObject;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetPasswordInfo(Message* pMessage, ProtocolCallContext* pCallContext)
{
    DataStorePasswordInfo* pPasswordInfo = static_cast<DataStorePasswordInfo*>(pCallContext->GetReturnValuePtr(0));
    if (pPasswordInfo != 0)
    {
        _DDL_DataStorePasswordInfo::Extract(pMessage, pPasswordInfo);
        return;
    }
    DataStorePasswordInfo discarded[1];
    _DDL_DataStorePasswordInfo::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallGetPasswordInfo(ProtocolCallContext* pCallContext, const unsigned long long& rDataId,
                                                  DataStorePasswordInfo* pPasswordInfo)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_PASSWORD_INFO);
    _Type_uint64::Add(&message, rDataId);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPasswordInfo);
    }
    s_ProtoReturn[METHOD_GET_PASSWORD_INFO] = ProtoReturn_GetPasswordInfo;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetPasswordInfos(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qVector<DataStorePasswordInfo>* pPasswordInfos = static_cast<qVector<DataStorePasswordInfo>*>(
        pCallContext->GetReturnValuePtr(0));
    if (pPasswordInfos != 0)
    {
        _Type_qvector<DDLClassMetaType<DataStorePasswordInfo> >::Extract(pMessage, pPasswordInfos);
        if (pMessage->HasError())
        {
            return;
        }
    }
    else
    {
        qVector<DataStorePasswordInfo> discarded[1];
        _Type_qvector<DDLClassMetaType<DataStorePasswordInfo> >::Extract(pMessage, discarded);
        if (pMessage->HasError())
        {
            return;
        }
    }

    qVector<qResult>* pResults = static_cast<qVector<qResult>*>(pCallContext->GetReturnValuePtr(1));
    if (pResults != 0)
    {
        _Type_qvector<_Type_qresult>::Extract(pMessage, pResults);
        return;
    }
    qVector<qResult> discarded[1];
    _Type_qvector<_Type_qresult>::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallGetPasswordInfos(ProtocolCallContext* pCallContext,
                                                   const qVector<unsigned long long>& rDataIds,
                                                   qVector<DataStorePasswordInfo>* pPasswordInfos,
                                                   qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_PASSWORD_INFOS);
    _Type_qvector<_Type_uint64>::Add(&message, rDataIds);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPasswordInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_PASSWORD_INFOS] = ProtoReturn_GetPasswordInfos;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

void DataStoreProtocolClient::ProtoReturn_GetObjectInfos(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qVector<DataStoreReqGetInfo>* pReqGetInfos = static_cast<qVector<DataStoreReqGetInfo>*>(
        pCallContext->GetReturnValuePtr(0));
    if (pReqGetInfos != 0)
    {
        _Type_qvector<DDLClassMetaType<DataStoreReqGetInfo> >::Extract(pMessage, pReqGetInfos);
        if (pMessage->HasError())
        {
            return;
        }
    }
    else
    {
        qVector<DataStoreReqGetInfo> discarded[1];
        _Type_qvector<DDLClassMetaType<DataStoreReqGetInfo> >::Extract(pMessage, discarded);
        if (pMessage->HasError())
        {
            return;
        }
    }

    qVector<qResult>* pResults = static_cast<qVector<qResult>*>(pCallContext->GetReturnValuePtr(1));
    if (pResults != 0)
    {
        _Type_qvector<_Type_qresult>::Extract(pMessage, pResults);
        return;
    }
    qVector<qResult> discarded[1];
    _Type_qvector<_Type_qresult>::Extract(pMessage, discarded);
}

bool DataStoreProtocolClient::CallGetObjectInfos(ProtocolCallContext* pCallContext,
                                                 const qVector<unsigned long long>& rDataIds,
                                                 qVector<DataStoreReqGetInfo>* pReqGetInfos, qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_OBJECT_INFOS);
    _Type_qvector<_Type_uint64>::Add(&message, rDataIds);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqGetInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_OBJECT_INFOS] = ProtoReturn_GetObjectInfos;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::PrepareGetObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                              const DataStorePrepareGetParam& rPrepareGetParam,
                                                              DataStoreReqGetInfo* pReqGetInfo)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_PREPARE_GET_OBJECT);
    _DDL_DataStorePrepareGetParam::Add(pMessage, rPrepareGetParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqGetInfo);
    }
    s_ProtoReturn[METHOD_PREPARE_GET_OBJECT] = ProtoReturn_PrepareGetObject;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::PrepareGetObjectV1_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                const DataStorePrepareGetParamV1& rPrepareGetParamV1,
                                                                DataStoreReqGetInfoV1* pReqGetInfoV1)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_PREPARE_GET_OBJECT_V1);
    _DDL_DataStorePrepareGetParamV1::Add(pMessage, rPrepareGetParamV1);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqGetInfoV1);
    }
    s_ProtoReturn[METHOD_PREPARE_GET_OBJECT_V1] = ProtoReturn_PrepareGetObjectV1;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::PrepareGetObjectOrMetaBinary_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const DataStorePrepareGetParam& rPrepareGetParam,
    DataStoreReqGetInfo* pReqGetInfo, DataStoreReqGetAdditionalMeta* pReqGetAdditionalMeta)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_PREPARE_GET_OBJECT_OR_META_BINARY);
    _DDL_DataStorePrepareGetParam::Add(pMessage, rPrepareGetParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqGetInfo);
        pCallContext->AddReturnValuePtr(pReqGetAdditionalMeta);
    }
    s_ProtoReturn[METHOD_PREPARE_GET_OBJECT_OR_META_BINARY] = ProtoReturn_PrepareGetObjectOrMetaBinary;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::PreparePostObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                               const DataStorePreparePostParam& rPreparePostParam,
                                                               DataStoreReqPostInfo* pReqPostInfo)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_PREPARE_POST_OBJECT);
    _DDL_DataStorePreparePostParam::Add(pMessage, rPreparePostParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqPostInfo);
    }
    s_ProtoReturn[METHOD_PREPARE_POST_OBJECT] = ProtoReturn_PreparePostObject;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::PreparePostObjectV1_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                 const DataStorePreparePostParamV1& rPreparePostParamV1,
                                                                 DataStoreReqPostInfoV1* pReqPostInfoV1)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_PREPARE_POST_OBJECT_V1);
    _DDL_DataStorePreparePostParamV1::Add(pMessage, rPreparePostParamV1);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqPostInfoV1);
    }
    s_ProtoReturn[METHOD_PREPARE_POST_OBJECT_V1] = ProtoReturn_PreparePostObjectV1;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CompletePostObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                const DataStoreCompletePostParam& rCompletePostParam)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_COMPLETE_POST_OBJECT);
    _DDL_DataStoreCompletePostParam::Add(pMessage, rCompletePostParam);
    s_ProtoReturn[METHOD_COMPLETE_POST_OBJECT] = ProtoReturn_CompletePostObject;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CompletePostObjectV1_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const DataStoreCompletePostParamV1& rCompletePostParamV1)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_COMPLETE_POST_OBJECT_V1);
    _DDL_DataStoreCompletePostParamV1::Add(pMessage, rCompletePostParamV1);
    s_ProtoReturn[METHOD_COMPLETE_POST_OBJECT_V1] = ProtoReturn_CompletePostObjectV1;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CompletePostObjects_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                 const qVector<unsigned long long>& rDataIds)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_COMPLETE_POST_OBJECTS);
    _Type_qvector<_Type_uint64>::Add(pMessage, rDataIds);
    s_ProtoReturn[METHOD_COMPLETE_POST_OBJECTS] = ProtoReturn_CompletePostObjects;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::PrepareUpdateObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                 const DataStorePrepareUpdateParam& rPrepareUpdateParam,
                                                                 DataStoreReqUpdateInfo* pReqUpdateInfo)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_PREPARE_UPDATE_OBJECT);
    _DDL_DataStorePrepareUpdateParam::Add(pMessage, rPrepareUpdateParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqUpdateInfo);
    }
    s_ProtoReturn[METHOD_PREPARE_UPDATE_OBJECT] = ProtoReturn_PrepareUpdateObject;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CompleteUpdateObject_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const DataStoreCompleteUpdateParam& rCompleteUpdateParam)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_COMPLETE_UPDATE_OBJECT);
    _DDL_DataStoreCompleteUpdateParam::Add(pMessage, rCompleteUpdateParam);
    s_ProtoReturn[METHOD_COMPLETE_UPDATE_OBJECT] = ProtoReturn_CompleteUpdateObject;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::DeleteObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                          const DataStoreDeleteParam& rDeleteParam)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_DELETE_OBJECT);
    _DDL_DataStoreDeleteParam::Add(pMessage, rDeleteParam);
    s_ProtoReturn[METHOD_DELETE_OBJECT] = ProtoReturn_DeleteObject;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::ChangeMeta_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                        const DataStoreChangeMetaParam& rChangeMetaParam)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_CHANGE_META);
    _DDL_DataStoreChangeMetaParam::Add(pMessage, rChangeMetaParam);
    s_ProtoReturn[METHOD_CHANGE_META] = ProtoReturn_ChangeMeta;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetMeta_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                     const DataStoreGetMetaParam& rGetMetaParam,
                                                     DataStoreMetaInfo* pMetaInfo)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_META);
    _DDL_DataStoreGetMetaParam::Add(pMessage, rGetMetaParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMetaInfo);
    }
    s_ProtoReturn[METHOD_GET_META] = ProtoReturn_GetMeta;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetMetas_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                      const qVector<unsigned long long>& rDataIds,
                                                      const DataStoreGetMetaParam& rGetMetaParam,
                                                      qVector<DataStoreMetaInfo>* pMetaInfos,
                                                      qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_METAS);
    _Type_qvector<_Type_uint64>::Add(pMessage, rDataIds);
    _DDL_DataStoreGetMetaParam::Add(pMessage, rGetMetaParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMetaInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_METAS] = ProtoReturn_GetMetas;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetMetasMultipleParam_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   const qVector<DataStoreGetMetaParam>& rGetMetaParams,
                                                                   qVector<DataStoreMetaInfo>* pMetaInfos,
                                                                   qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_METAS_MULTIPLE_PARAM);
    _Type_qvector<DDLClassMetaType<DataStoreGetMetaParam> >::Add(pMessage, rGetMetaParams);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMetaInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_METAS_MULTIPLE_PARAM] = ProtoReturn_GetMetasMultipleParam;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::SearchObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                          const DataStoreSearchParam& rSearchParam,
                                                          DataStoreSearchResult* pSearchResult)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_SEARCH_OBJECT);
    _DDL_DataStoreSearchParam::Add(pMessage, rSearchParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSearchResult);
    }
    s_ProtoReturn[METHOD_SEARCH_OBJECT] = ProtoReturn_SearchObject;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::SearchObjectLight_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                               const DataStoreSearchParam& rSearchParam,
                                                               DataStoreSearchResult* pSearchResult)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_SEARCH_OBJECT_LIGHT);
    _DDL_DataStoreSearchParam::Add(pMessage, rSearchParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSearchResult);
    }
    s_ProtoReturn[METHOD_SEARCH_OBJECT_LIGHT] = ProtoReturn_SearchObjectLight;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetNotificationUrl_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage,
    const DataStoreGetNotificationUrlParam& rGetNotificationUrlParam,
    DataStoreReqGetNotificationUrlInfo* pReqGetNotificationUrlInfo)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_NOTIFICATION_URL);
    _DDL_DataStoreGetNotificationUrlParam::Add(pMessage, rGetNotificationUrlParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqGetNotificationUrlInfo);
    }
    s_ProtoReturn[METHOD_GET_NOTIFICATION_URL] = ProtoReturn_GetNotificationUrl;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetNewArrivedNotifications_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage,
    const DataStoreGetNewArrivedNotificationsParam& rGetNewArrivedNotificationsParam,
    qVector<DataStoreNotification>* pNotifications, bool* pFlag)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_NEW_ARRIVED_NOTIFICATIONS);
    _DDL_DataStoreGetNewArrivedNotificationsParam::Add(pMessage, rGetNewArrivedNotificationsParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pNotifications);
        pCallContext->AddReturnValuePtr(pFlag);
    }
    s_ProtoReturn[METHOD_GET_NEW_ARRIVED_NOTIFICATIONS] = ProtoReturn_GetNewArrivedNotifications;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetNewArrivedNotificationsV1_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage,
    const DataStoreGetNewArrivedNotificationsParam& rGetNewArrivedNotificationsParam,
    qVector<DataStoreNotificationV1>* pNotificationV1s, bool* pFlag)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_NEW_ARRIVED_NOTIFICATIONS_V1);
    _DDL_DataStoreGetNewArrivedNotificationsParam::Add(pMessage, rGetNewArrivedNotificationsParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pNotificationV1s);
        pCallContext->AddReturnValuePtr(pFlag);
    }
    s_ProtoReturn[METHOD_GET_NEW_ARRIVED_NOTIFICATIONS_V1] = ProtoReturn_GetNewArrivedNotificationsV1;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::RateObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                        const DataStoreRatingTarget& rRatingTarget,
                                                        const DataStoreRateObjectParam& rRateObjectParam,
                                                        const bool& rFlag, DataStoreRatingInfo* pRatingInfo)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_RATE_OBJECT);
    _DDL_DataStoreRatingTarget::Add(pMessage, rRatingTarget);
    _DDL_DataStoreRateObjectParam::Add(pMessage, rRateObjectParam);
    *pMessage << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfo);
    }
    s_ProtoReturn[METHOD_RATE_OBJECT] = ProtoReturn_RateObject;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetRating_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                       const DataStoreRatingTarget& rRatingTarget,
                                                       DataStoreRatingInfo* pRatingInfo,
                                                       const unsigned long long& rDataId)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_RATING);
    _DDL_DataStoreRatingTarget::Add(pMessage, rRatingTarget);
    _Type_uint64::Add(pMessage, rDataId);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfo);
    }
    s_ProtoReturn[METHOD_GET_RATING] = ProtoReturn_GetRating;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetRatings_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const qVector<unsigned long long>& rDataIds,
    qVector<qVector<DataStoreRatingInfoWithSlot> >* pRatingInfoWithSlotss, qVector<qResult>* pResults,
    const unsigned long long& rDataId)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_RATINGS);
    _Type_qvector<_Type_uint64>::Add(pMessage, rDataIds);
    _Type_uint64::Add(pMessage, rDataId);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfoWithSlotss);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_RATINGS] = ProtoReturn_GetRatings;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::ResetRating_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                         const DataStoreRatingTarget& rRatingTarget,
                                                         const unsigned long long& rDataId)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_RESET_RATING);
    _DDL_DataStoreRatingTarget::Add(pMessage, rRatingTarget);
    _Type_uint64::Add(pMessage, rDataId);
    s_ProtoReturn[METHOD_RESET_RATING] = ProtoReturn_ResetRating;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::ResetRatings_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                          const qVector<unsigned long long>& rDataIds,
                                                          const bool& rFlag, qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_RESET_RATINGS);
    _Type_qvector<_Type_uint64>::Add(pMessage, rDataIds);
    *pMessage << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_RESET_RATINGS] = ProtoReturn_ResetRatings;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetSpecificMeta_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                             const DataStoreGetSpecificMetaParam& rGetSpecificMetaParam,
                                                             qVector<DataStoreSpecificMetaInfo>* pSpecificMetaInfos)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_SPECIFIC_META);
    _DDL_DataStoreGetSpecificMetaParam::Add(pMessage, rGetSpecificMetaParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSpecificMetaInfos);
    }
    s_ProtoReturn[METHOD_GET_SPECIFIC_META] = ProtoReturn_GetSpecificMeta;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetSpecificMetaV1_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage,
    const DataStoreGetSpecificMetaParamV1& rGetSpecificMetaParamV1,
    qVector<DataStoreSpecificMetaInfoV1>* pSpecificMetaInfoV1s)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_SPECIFIC_META_V1);
    _DDL_DataStoreGetSpecificMetaParamV1::Add(pMessage, rGetSpecificMetaParamV1);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSpecificMetaInfoV1s);
    }
    s_ProtoReturn[METHOD_GET_SPECIFIC_META_V1] = ProtoReturn_GetSpecificMetaV1;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::PostMetaBinary_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                            const DataStorePreparePostParam& rPreparePostParam,
                                                            unsigned long long* pDataId)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_POST_META_BINARY);
    _DDL_DataStorePreparePostParam::Add(pMessage, rPreparePostParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pDataId);
    }
    s_ProtoReturn[METHOD_POST_META_BINARY] = ProtoReturn_PostMetaBinary;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::PostMetaBinaryWithDataId_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const unsigned long long& rDataId,
    const DataStorePreparePostParam& rPreparePostParam)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_POST_META_BINARY_WITH_DATA_ID);
    _Type_uint64::Add(pMessage, rDataId);
    _DDL_DataStorePreparePostParam::Add(pMessage, rPreparePostParam);
    s_ProtoReturn[METHOD_POST_META_BINARY_WITH_DATA_ID] = ProtoReturn_PostMetaBinaryWithDataId;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::PostMetaBinariesWithDataId_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const qVector<unsigned long long>& rDataIds,
    const qVector<DataStorePreparePostParam>& rPreparePostParams, const bool& rFlag, qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_POST_META_BINARIES_WITH_DATA_ID);
    _Type_qvector<_Type_uint64>::Add(pMessage, rDataIds);
    _Type_qvector<DDLClassMetaType<DataStorePreparePostParam> >::Add(pMessage, rPreparePostParams);
    *pMessage << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_POST_META_BINARIES_WITH_DATA_ID] = ProtoReturn_PostMetaBinariesWithDataId;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::TouchObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                         const DataStoreTouchObjectParam& rTouchObjectParam)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_TOUCH_OBJECT);
    _DDL_DataStoreTouchObjectParam::Add(pMessage, rTouchObjectParam);
    s_ProtoReturn[METHOD_TOUCH_OBJECT] = ProtoReturn_TouchObject;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetRatingWithLog_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                              const DataStoreRatingTarget& rRatingTarget,
                                                              DataStoreRatingInfo* pRatingInfo,
                                                              DataStoreRatingLog* pRatingLog,
                                                              const unsigned long long& rDataId)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_RATING_WITH_LOG);
    _DDL_DataStoreRatingTarget::Add(pMessage, rRatingTarget);
    _Type_uint64::Add(pMessage, rDataId);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfo);
        pCallContext->AddReturnValuePtr(pRatingLog);
    }
    s_ProtoReturn[METHOD_GET_RATING_WITH_LOG] = ProtoReturn_GetRatingWithLog;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetPersistenceInfo_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                const unsigned int& rValue, const unsigned short& rSlot,
                                                                DataStorePersistenceInfo* pPersistenceInfo)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_PERSISTENCE_INFO);
    _Type_uint32::Add(pMessage, rValue);
    _Type_uint16::Add(pMessage, rSlot);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPersistenceInfo);
    }
    s_ProtoReturn[METHOD_GET_PERSISTENCE_INFO] = ProtoReturn_GetPersistenceInfo;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetPersistenceInfos_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                 const unsigned int& rValue,
                                                                 const qVector<unsigned short>& rSlots,
                                                                 qVector<DataStorePersistenceInfo>* pPersistenceInfos,
                                                                 qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_PERSISTENCE_INFOS);
    _Type_uint32::Add(pMessage, rValue);
    _Type_qvector<_Type_uint16>::Add(pMessage, rSlots);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPersistenceInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_PERSISTENCE_INFOS] = ProtoReturn_GetPersistenceInfos;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::PerpetuateObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                              const unsigned short& rSlot,
                                                              const unsigned long long& rDataId, const bool& rFlag)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_PERPETUATE_OBJECT);
    _Type_uint16::Add(pMessage, rSlot);
    _Type_uint64::Add(pMessage, rDataId);
    *pMessage << rFlag;
    s_ProtoReturn[METHOD_PERPETUATE_OBJECT] = ProtoReturn_PerpetuateObject;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::UnperpetuateObject_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                const unsigned short& rSlot, const bool& rFlag)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_UNPERPETUATE_OBJECT);
    _Type_uint16::Add(pMessage, rSlot);
    *pMessage << rFlag;
    s_ProtoReturn[METHOD_UNPERPETUATE_OBJECT] = ProtoReturn_UnperpetuateObject;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetPasswordInfo_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                             const unsigned long long& rDataId,
                                                             DataStorePasswordInfo* pPasswordInfo)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_PASSWORD_INFO);
    _Type_uint64::Add(pMessage, rDataId);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPasswordInfo);
    }
    s_ProtoReturn[METHOD_GET_PASSWORD_INFO] = ProtoReturn_GetPasswordInfo;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetPasswordInfos_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                              const qVector<unsigned long long>& rDataIds,
                                                              qVector<DataStorePasswordInfo>* pPasswordInfos,
                                                              qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_PASSWORD_INFOS);
    _Type_qvector<_Type_uint64>::Add(pMessage, rDataIds);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPasswordInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_PASSWORD_INFOS] = ProtoReturn_GetPasswordInfos;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::GetObjectInfos_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                            const qVector<unsigned long long>& rDataIds,
                                                            qVector<DataStoreReqGetInfo>* pReqGetInfos,
                                                            qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_GET_OBJECT_INFOS);
    _Type_qvector<_Type_uint64>::Add(pMessage, rDataIds);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pReqGetInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_GET_OBJECT_INFOS] = ProtoReturn_GetObjectInfos;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void DataStoreProtocolClient::ProtoReturn_DeleteObjects(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pValue = pCallContext->GetReturnValuePtr(0);
    if (pValue != 0)
    {
        _Type_qvector<_Type_qresult>::Extract(pMessage, static_cast<qVector<qResult>*>(pValue));
        return;
    }
    qVector<qResult> discarded[1];
    _Type_qvector<_Type_qresult>::Extract(pMessage, discarded);
}

void DataStoreProtocolClient::ProtoReturn_ChangeMetasV1(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pValue = pCallContext->GetReturnValuePtr(0);
    if (pValue != 0)
    {
        _Type_qvector<_Type_qresult>::Extract(pMessage, static_cast<qVector<qResult>*>(pValue));
        return;
    }
    qVector<qResult> discarded[1];
    _Type_qvector<_Type_qresult>::Extract(pMessage, discarded);
}

void DataStoreProtocolClient::ProtoReturn_ChangeMetas(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pValue = pCallContext->GetReturnValuePtr(0);
    if (pValue != 0)
    {
        _Type_qvector<_Type_qresult>::Extract(pMessage, static_cast<qVector<qResult>*>(pValue));
        return;
    }
    qVector<qResult> discarded[1];
    _Type_qvector<_Type_qresult>::Extract(pMessage, discarded);
}

void DataStoreProtocolClient::ProtoReturn_RateObjects(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qVector<DataStoreRatingInfo>* pRatingInfos = static_cast<qVector<DataStoreRatingInfo>*>(
        pCallContext->GetReturnValuePtr(0));
    if (pRatingInfos != 0)
    {
        _Type_qvector<DDLClassMetaType<DataStoreRatingInfo> >::Extract(pMessage, pRatingInfos);
        if (pMessage->HasError())
        {
            return;
        }
    }
    else
    {
        qVector<DataStoreRatingInfo> discarded[1];
        _Type_qvector<DDLClassMetaType<DataStoreRatingInfo> >::Extract(pMessage, discarded);
        if (pMessage->HasError())
        {
            return;
        }
    }

    qVector<qResult>* pResults = static_cast<qVector<qResult>*>(pCallContext->GetReturnValuePtr(1));
    if (pResults != 0)
    {
        _Type_qvector<_Type_qresult>::Extract(pMessage, pResults);
        return;
    }
    qVector<qResult> discarded[1];
    _Type_qvector<_Type_qresult>::Extract(pMessage, discarded);
}

void DataStoreProtocolClient::ProtoReturn_RateObjectWithPosting(Message* pMessage, ProtocolCallContext* pCallContext)
{
    DataStoreRatingInfo* pRatingInfo = static_cast<DataStoreRatingInfo*>(pCallContext->GetReturnValuePtr(0));
    if (pRatingInfo != 0)
    {
        _DDL_DataStoreRatingInfo::Extract(pMessage, pRatingInfo);
        return;
    }
    DataStoreRatingInfo discarded[1];
    _DDL_DataStoreRatingInfo::Extract(pMessage, discarded);
}

void DataStoreProtocolClient::ProtoReturn_RateObjectsWithPosting(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qVector<DataStoreRatingInfo>* pRatingInfos = static_cast<qVector<DataStoreRatingInfo>*>(
        pCallContext->GetReturnValuePtr(0));
    if (pRatingInfos != 0)
    {
        _Type_qvector<DDLClassMetaType<DataStoreRatingInfo> >::Extract(pMessage, pRatingInfos);
        if (pMessage->HasError())
        {
            return;
        }
    }
    else
    {
        qVector<DataStoreRatingInfo> discarded[1];
        _Type_qvector<DDLClassMetaType<DataStoreRatingInfo> >::Extract(pMessage, discarded);
        if (pMessage->HasError())
        {
            return;
        }
    }

    qVector<qResult>* pResults = static_cast<qVector<qResult>*>(pCallContext->GetReturnValuePtr(1));
    if (pResults != 0)
    {
        _Type_qvector<_Type_qresult>::Extract(pMessage, pResults);
        return;
    }
    qVector<qResult> discarded[1];
    _Type_qvector<_Type_qresult>::Extract(pMessage, discarded);
}

DataStoreProtocolClient::DataStoreProtocolClient(unsigned short uiProtocolID) : ClientProtocol(1)
{
    if (uiProtocolID == 0)
        return;
    SetProtocolID(uiProtocolID);
    RVClientCore::GetInstance()->GetProtocolRegistry()->RegisterProtocol(uiProtocolID, new DataStoreProtocolClient());
}

void DataStoreProtocolClient::ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext)
{
    const unsigned int uiMethodID = Protocol::ExtractMethodID(pMessage) & METHOD_ID_MASK;
    if (uiMethodID - 1 >= METHOD_ID_COUNT - 1)
    {
        ClientProtocol::SetCallError(qResult(RESULT_NOT_IMPLEMENTED));
        return;
    }
    if (s_ProtoReturn[uiMethodID] != 0)
        s_ProtoReturn[uiMethodID](pMessage, pCallContext);
}

bool DataStoreProtocolClient::CallDeleteObjects(ProtocolCallContext* pCallContext,
                                                const qVector<DataStoreDeleteParam>& rDeleteParams, const bool& rFlag,
                                                qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_DELETE_OBJECTS);
    _Type_qvector<DDLClassMetaType<DataStoreDeleteParam> >::Add(&message, rDeleteParams);
    message << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_DELETE_OBJECTS] = ProtoReturn_DeleteObjects;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::DeleteObjects_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                           const qVector<DataStoreDeleteParam>& rDeleteParams,
                                                           const bool& rFlag, qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_DELETE_OBJECTS);
    _Type_qvector<DDLClassMetaType<DataStoreDeleteParam> >::Add(pMessage, rDeleteParams);
    *pMessage << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_DELETE_OBJECTS] = ProtoReturn_DeleteObjects;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallChangeMetaV1(ProtocolCallContext* pCallContext,
                                               const DataStoreChangeMetaParamV1& rChangeMetaParamV1)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_CHANGE_META_V1);
    _DDL_DataStoreChangeMetaParamV1::Add(&message, rChangeMetaParamV1);
    s_ProtoReturn[METHOD_CHANGE_META_V1] = ProtoReturn_ChangeMetaV1;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::ChangeMetaV1_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                          const DataStoreChangeMetaParamV1& rChangeMetaParamV1)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_CHANGE_META_V1);
    _DDL_DataStoreChangeMetaParamV1::Add(pMessage, rChangeMetaParamV1);
    s_ProtoReturn[METHOD_CHANGE_META_V1] = ProtoReturn_ChangeMetaV1;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallChangeMetasV1(ProtocolCallContext* pCallContext,
                                                const qVector<unsigned long long>& rDataIds,
                                                const qVector<DataStoreChangeMetaParamV1>& rChangeMetaParamV1s,
                                                const bool& rFlag, qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_CHANGE_METAS_V1);
    _Type_qvector<_Type_uint64>::Add(&message, rDataIds);
    _Type_qvector<DDLClassMetaType<DataStoreChangeMetaParamV1> >::Add(&message, rChangeMetaParamV1s);
    message << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_CHANGE_METAS_V1] = ProtoReturn_ChangeMetasV1;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::ChangeMetasV1_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const qVector<unsigned long long>& rDataIds,
    const qVector<DataStoreChangeMetaParamV1>& rChangeMetaParamV1s, const bool& rFlag, qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_CHANGE_METAS_V1);
    _Type_qvector<_Type_uint64>::Add(pMessage, rDataIds);
    _Type_qvector<DDLClassMetaType<DataStoreChangeMetaParamV1> >::Add(pMessage, rChangeMetaParamV1s);
    *pMessage << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_CHANGE_METAS_V1] = ProtoReturn_ChangeMetasV1;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallChangeMetas(ProtocolCallContext* pCallContext,
                                              const qVector<unsigned long long>& rDataIds,
                                              const qVector<DataStoreChangeMetaParam>& rChangeMetaParams,
                                              const bool& rFlag, qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_CHANGE_METAS);
    _Type_qvector<_Type_uint64>::Add(&message, rDataIds);
    _Type_qvector<DDLClassMetaType<DataStoreChangeMetaParam> >::Add(&message, rChangeMetaParams);
    message << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_CHANGE_METAS] = ProtoReturn_ChangeMetas;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::ChangeMetas_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                         const qVector<unsigned long long>& rDataIds,
                                                         const qVector<DataStoreChangeMetaParam>& rChangeMetaParams,
                                                         const bool& rFlag, qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_CHANGE_METAS);
    _Type_qvector<_Type_uint64>::Add(pMessage, rDataIds);
    _Type_qvector<DDLClassMetaType<DataStoreChangeMetaParam> >::Add(pMessage, rChangeMetaParams);
    *pMessage << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_CHANGE_METAS] = ProtoReturn_ChangeMetas;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallRateObjects(ProtocolCallContext* pCallContext,
                                              const qVector<DataStoreRatingTarget>& rRatingTargets,
                                              const qVector<DataStoreRateObjectParam>& rRateObjectParams,
                                              const bool& rFlag, const bool& rFlag2,
                                              qVector<DataStoreRatingInfo>* pRatingInfos, qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_RATE_OBJECTS);
    _Type_qvector<DDLClassMetaType<DataStoreRatingTarget> >::Add(&message, rRatingTargets);
    _Type_qvector<DDLClassMetaType<DataStoreRateObjectParam> >::Add(&message, rRateObjectParams);
    message << rFlag;
    message << rFlag2;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_RATE_OBJECTS] = ProtoReturn_RateObjects;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::RateObjects_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                         const qVector<DataStoreRatingTarget>& rRatingTargets,
                                                         const qVector<DataStoreRateObjectParam>& rRateObjectParams,
                                                         const bool& rFlag, const bool& rFlag2,
                                                         qVector<DataStoreRatingInfo>* pRatingInfos,
                                                         qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_RATE_OBJECTS);
    _Type_qvector<DDLClassMetaType<DataStoreRatingTarget> >::Add(pMessage, rRatingTargets);
    _Type_qvector<DDLClassMetaType<DataStoreRateObjectParam> >::Add(pMessage, rRateObjectParams);
    *pMessage << rFlag;
    *pMessage << rFlag2;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_RATE_OBJECTS] = ProtoReturn_RateObjects;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallRateObjectWithPosting(ProtocolCallContext* pCallContext,
                                                        const DataStoreRatingTarget& rRatingTarget,
                                                        const DataStoreRateObjectParam& rRateObjectParam,
                                                        const DataStorePreparePostParam& rPreparePostParam,
                                                        const bool& rFlag, DataStoreRatingInfo* pRatingInfo)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_RATE_OBJECT_WITH_POSTING);
    _DDL_DataStoreRatingTarget::Add(&message, rRatingTarget);
    _DDL_DataStoreRateObjectParam::Add(&message, rRateObjectParam);
    _DDL_DataStorePreparePostParam::Add(&message, rPreparePostParam);
    message << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfo);
    }
    s_ProtoReturn[METHOD_RATE_OBJECT_WITH_POSTING] = ProtoReturn_RateObjectWithPosting;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::RateObjectWithPosting_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   const DataStoreRatingTarget& rRatingTarget,
                                                                   const DataStoreRateObjectParam& rRateObjectParam,
                                                                   const DataStorePreparePostParam& rPreparePostParam,
                                                                   const bool& rFlag, DataStoreRatingInfo* pRatingInfo)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_RATE_OBJECT_WITH_POSTING);
    _DDL_DataStoreRatingTarget::Add(pMessage, rRatingTarget);
    _DDL_DataStoreRateObjectParam::Add(pMessage, rRateObjectParam);
    _DDL_DataStorePreparePostParam::Add(pMessage, rPreparePostParam);
    *pMessage << rFlag;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfo);
    }
    s_ProtoReturn[METHOD_RATE_OBJECT_WITH_POSTING] = ProtoReturn_RateObjectWithPosting;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

bool DataStoreProtocolClient::CallRateObjectsWithPosting(ProtocolCallContext* pCallContext,
                                                         const qVector<DataStoreRatingTarget>& rRatingTargets,
                                                         const qVector<DataStoreRateObjectParam>& rRateObjectParams,
                                                         const qVector<DataStorePreparePostParam>& rPreparePostParams,
                                                         const bool& rFlag, const bool& rFlag2,
                                                         qVector<DataStoreRatingInfo>* pRatingInfos,
                                                         qVector<qResult>* pResults)
{
    Message message;
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, GetProtocolID(), COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(&message, METHOD_RATE_OBJECTS_WITH_POSTING);
    _Type_qvector<DDLClassMetaType<DataStoreRatingTarget> >::Add(&message, rRatingTargets);
    _Type_qvector<DDLClassMetaType<DataStoreRateObjectParam> >::Add(&message, rRateObjectParams);
    _Type_qvector<DDLClassMetaType<DataStorePreparePostParam> >::Add(&message, rPreparePostParams);
    message << rFlag;
    message << rFlag2;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_RATE_OBJECTS_WITH_POSTING] = ProtoReturn_RateObjectsWithPosting;
    if (m_bLocalLoopback && FlagIsSet(0x10))
    {
        pScheduler->GetCriticalSection().Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pScheduler->GetCriticalSection().Leave();
    return bSent;
}

bool DataStoreProtocolClient::RateObjectsWithPosting_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const qVector<DataStoreRatingTarget>& rRatingTargets,
    const qVector<DataStoreRateObjectParam>& rRateObjectParams,
    const qVector<DataStorePreparePostParam>& rPreparePostParams, const bool& rFlag, const bool& rFlag2,
    qVector<DataStoreRatingInfo>* pRatingInfos, qVector<qResult>* pResults)
{
    Scheduler* pScheduler = Scheduler::GetInstance();
    pScheduler->GetCriticalSection().Enter();
    if (!RegisterCallContext(pMessage, pCallContext))
    {
        pScheduler->GetCriticalSection().Leave();
        return false;
    }
    AddMethodID(pMessage, METHOD_RATE_OBJECTS_WITH_POSTING);
    _Type_qvector<DDLClassMetaType<DataStoreRatingTarget> >::Add(pMessage, rRatingTargets);
    _Type_qvector<DDLClassMetaType<DataStoreRateObjectParam> >::Add(pMessage, rRateObjectParams);
    _Type_qvector<DDLClassMetaType<DataStorePreparePostParam> >::Add(pMessage, rPreparePostParams);
    *pMessage << rFlag;
    *pMessage << rFlag2;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRatingInfos);
        pCallContext->AddReturnValuePtr(pResults);
    }
    s_ProtoReturn[METHOD_RATE_OBJECTS_WITH_POSTING] = ProtoReturn_RateObjectsWithPosting;
    pScheduler->GetCriticalSection().Leave();
    return true;
}

void DataStoreProtocolClient::ProtoReturn_CompletePostObject(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_CompletePostObjectV1(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_CompletePostObjects(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_CompleteUpdateObject(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_DeleteObject(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_ChangeMeta(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_ResetRating(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_PostMetaBinaryWithDataId(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_TouchObject(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_PerpetuateObject(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_UnperpetuateObject(Message*, ProtocolCallContext*)
{
}

void DataStoreProtocolClient::ProtoReturn_ChangeMetaV1(Message*, ProtocolCallContext*)
{
}

Protocol* DataStoreProtocolClient::CreateResponder() const
{
    return new DataStoreProtocolClient();
}

}
}
