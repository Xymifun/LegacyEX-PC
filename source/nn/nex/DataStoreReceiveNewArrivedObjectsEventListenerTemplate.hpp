#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORERECEIVENEWARRIVEDOBJECTSEVENTLISTENERTEMPLATE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORERECEIVENEWARRIVEDOBJECTSEVENTLISTENERTEMPLATE_HPP

#include <stdint.h>

#include "nn/nex/DataStoreGetObjectEventListenerTemplate.hpp"

namespace nn
{
namespace nex
{

template <typename TDataId>
class DataStoreReceiveNewArrivedObjectsEventListenerTemplate
{
public:
    virtual ~DataStoreReceiveNewArrivedObjectsEventListenerTemplate() {}

    virtual void BeginObject(TDataId dataId, unsigned int uiOwnerId, unsigned int uiSize, unsigned short usDataType,
                             unsigned int uiVersion, uint64_t uiNotificationId) = 0;

    virtual void UpdateCheckToken(const unsigned char* pCheckToken,
                                  DataStoreGetObjectEventListenerTemplate<TDataId>* pGetObjectListener,
                                  uint64_t uiLastNotificationId) = 0;
};

}
}

#endif
