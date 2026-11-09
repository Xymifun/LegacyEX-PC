#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREGETOBJECTEVENTLISTENERTEMPLATE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREGETOBJECTEVENTLISTENERTEMPLATE_HPP

namespace nn
{
namespace nex
{

template <typename TDataId>
class DataStoreGetObjectEventListenerTemplate
{
public:
    virtual ~DataStoreGetObjectEventListenerTemplate() {}

    virtual void BeginInternal(TDataId dataId, unsigned int uiSize, unsigned int uiOwnerId, unsigned short usDataType,
                               unsigned short usVersion, bool bIsObject)
    {
    }

    virtual void Begin(TDataId dataId, unsigned int uiSize) {}

    virtual bool ProcessResponse(const void* pData, unsigned int uiSize) = 0;
};

}
}

#endif
