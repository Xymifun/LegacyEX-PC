#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECHECKCONSISTENCYEVENTLISTENERADAPTOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORECHECKCONSISTENCYEVENTLISTENERADAPTOR_HPP

#include "nn/nex/CallContext.hpp"
#include "nn/nex/DataStoreGetObjectEventListenerTemplate.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

template <typename TTraits>
class DataStoreCheckConsistencyEventListenerAdaptor
    : public DataStoreGetObjectEventListenerTemplate<typename TTraits::TDataId>,
      public RootObject
{
public:
    DataStoreCheckConsistencyEventListenerAdaptor(typename TTraits::TGetObjectEventListener* pListener,
                                                  CallContext* pContext,
                                                  const typename TTraits::TSpecificMetaInfo& rMetaInfo)
        : m_pListener(pListener),
          m_pContext(pContext),
          m_metaInfo(rMetaInfo)
    {
    }

    virtual ~DataStoreCheckConsistencyEventListenerAdaptor() {}

    virtual void BeginInternal(typename TTraits::TDataId dataId, unsigned int uiSize, unsigned int uiValue,
                               unsigned short usDataType, unsigned short usVersion, bool bFlag);
    virtual void Begin(typename TTraits::TDataId dataId, unsigned int uiSize);
    virtual bool ProcessResponse(const void* pData, unsigned int uiSize);

private:
    typename TTraits::TGetObjectEventListener* m_pListener;
    CallContext* m_pContext;
    typename TTraits::TSpecificMetaInfo m_metaInfo;
};

template <typename TTraits>
void DataStoreCheckConsistencyEventListenerAdaptor<TTraits>::BeginInternal(typename TTraits::TDataId dataId,
                                                                           unsigned int uiSize, unsigned int uiValue,
                                                                           unsigned short usDataType,
                                                                           unsigned short usVersion, bool bFlag)
{
    if (uiSize != m_metaInfo.m_uiSize || usDataType != m_metaInfo.m_uiDataType || usVersion != m_metaInfo.m_uiVersion)
    {
        m_pContext->Cancel(CallContext::CallCanceled);
        return;
    }

    m_pListener->BeginInternal(dataId, uiSize, uiValue, usDataType, usVersion, bFlag);
}

template <typename TTraits>
void DataStoreCheckConsistencyEventListenerAdaptor<TTraits>::Begin(typename TTraits::TDataId dataId,
                                                                   unsigned int uiSize)
{
    m_pListener->Begin(dataId, uiSize);
}

template <typename TTraits>
bool DataStoreCheckConsistencyEventListenerAdaptor<TTraits>::ProcessResponse(const void* pData, unsigned int uiSize)
{
    return m_pListener->ProcessResponse(pData, uiSize);
}

}
}

#endif
