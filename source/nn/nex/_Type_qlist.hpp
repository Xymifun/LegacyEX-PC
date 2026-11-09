#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__TYPE_QLIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__TYPE_QLIST_HPP

#include <stdint.h>

#include "nn/nex/Message.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

template <typename Type>
class _Type_qlist
{
public:
    typedef typename Type::CPPType CPPType;

    static void Add(Message* pMessage, const qList<CPPType>& rSource)
    {
        const uint32_t uiCount = static_cast<uint32_t>(rSource.size());
        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiCount), sizeof(uiCount), 1);
        const typename qList<CPPType>::const_iterator itEnd = rSource.end();
        for (typename qList<CPPType>::const_iterator it = rSource.begin(); it != itEnd; ++it)
        {
            Type::Add(pMessage, *it);
        }
    }

    static void Extract(Message* pMessage, qList<CPPType>* pDestination)
    {
        pDestination->clear();
        uint32_t uiCount;
        if (!pMessage->ExtractPrimitive(&uiCount))
        {
            return;
        }
        for (uint32_t uiIndex = 0; uiIndex < uiCount && !pMessage->HasError(); ++uiIndex)
        {
            CPPType element;
            Type::Extract(pMessage, &element);
            if (pMessage->HasError())
            {
                pDestination->clear();
                return;
            }
            pDestination->push_back(element);
        }
    }
};

}
}

#endif
