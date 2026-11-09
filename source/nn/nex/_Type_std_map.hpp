#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__TYPE_STD_MAP_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__TYPE_STD_MAP_HPP

#include <stdint.h>

#include "nn/nex/Message.hpp"
#include "nn/nex/qMap.hpp"

namespace nn
{
namespace nex
{

template <typename KeyType, typename ValueType>
class _Type_std_map
{
public:
    typedef typename KeyType::CPPType KeyCPPType;
    typedef typename ValueType::CPPType ValueCPPType;

    static void Add(Message* pMessage, const qMap<KeyCPPType, ValueCPPType>& rSource)
    {
        const uint32_t uiCount = static_cast<uint32_t>(rSource.size());
        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiCount), sizeof(uiCount), 1);
        const typename qMap<KeyCPPType, ValueCPPType>::const_iterator itEnd = rSource.end();
        for (typename qMap<KeyCPPType, ValueCPPType>::const_iterator it = rSource.begin(); it != itEnd; ++it)
        {
            KeyType::Add(pMessage, it->first);
            ValueType::Add(pMessage, it->second);
        }
    }

    static void Extract(Message* pMessage, qMap<KeyCPPType, ValueCPPType>* pDestination)
    {
        pDestination->clear();
        uint32_t uiCount = 0;
        pMessage->ExtractPrimitive(&uiCount);
        for (uint32_t uiIndex = 0; uiIndex < uiCount; ++uiIndex)
        {
            KeyCPPType key;
            ValueCPPType value;
            KeyType::Extract(pMessage, &key);
            ValueType::Extract(pMessage, &value);
            (*pDestination)[key] = value;
        }
    }
};

}
}

#endif
