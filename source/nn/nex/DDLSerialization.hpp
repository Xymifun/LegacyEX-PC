#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DDLSERIALIZATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DDLSERIALIZATION_HPP

#include <stdint.h>

#include "nn/nex/AnyObjectAdapter.hpp"
#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class String;
class StationURL;
class qBuffer;
class SystemSetting;

extern SystemSetting g_ddlConnectionEncoding;
extern unsigned int g_ddlStringEncoding;

class _Type_byte
{
public:
    typedef unsigned char CPPType;

    static void Add(Message* pMessage, const CPPType& rSource)
    {
        pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource), sizeof(CPPType), 1);
    }

    static void Extract(Message* pMessage, CPPType* pDestination) { pMessage->ExtractPrimitive(pDestination); }
};

class _Type_uint16
{
public:
    typedef uint16_t CPPType;

    static void Add(Message* pMessage, const CPPType& rSource)
    {
        pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource), sizeof(CPPType), 1);
    }

    static void Extract(Message* pMessage, CPPType* pDestination) { pMessage->ExtractPrimitive(pDestination); }
};

class _Type_uint32
{
public:
    typedef uint32_t CPPType;

    static void Add(Message* pMessage, const CPPType& rSource)
    {
        pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource), sizeof(CPPType), 1);
    }

    static void Extract(Message* pMessage, CPPType* pDestination) { pMessage->ExtractPrimitive(pDestination); }
};

class _Type_uint64
{
public:
    typedef uint64_t CPPType;

    static void Add(Message* pMessage, const CPPType& rSource)
    {
        pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource), sizeof(CPPType), 1);
    }

    static void Extract(Message* pMessage, CPPType* pDestination) { pMessage->ExtractPrimitive(pDestination); }
};

class _Type_float
{
public:
    typedef float CPPType;

    static void Add(Message* pMessage, const CPPType& rSource)
    {
        pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource), sizeof(CPPType), 1);
    }

    static void Extract(Message* pMessage, CPPType* pDestination) { pMessage->ExtractPrimitive(pDestination); }
};

class _Type_string
{
public:
    typedef String CPPType;

    static void Add(Message* pMessage, const String& rSource);
    static bool Extract(Message* pMessage, String* pDestination);
};

class _Type_qBuffer
{
public:
    typedef qBuffer CPPType;

    static void Add(Message* pMessage, const qBuffer& rSource);
    static void Extract(Message* pMessage, qBuffer* pDestination);
};

class _Type_buffertail
{
public:
    typedef Buffer CPPType;

    static void Add(Message* pMessage, const Buffer& rSource);
    static void Extract(Message* pMessage, Buffer* pDestination);
};

class _Type_stationurl
{
public:
    typedef StationURL CPPType;

    static void Add(Message* pMessage, const StationURL& rSource);
    static bool Extract(Message* pMessage, StationURL* pDestination);
};

template <typename Meta, typename KeyType>
class _Type_any
{
public:
    typedef typename Meta::CPPType ObjectType;
    typedef AnyObjectHolder<ObjectType, typename KeyType::CPPType> CPPType;

    static void Add(Message* pMessage, const CPPType& rSource)
    {
        AnyObjectAdapter<ObjectType, typename KeyType::CPPType>::s_pInstance->StreamIn(pMessage, *rSource.GetObject());
    }

    static void Extract(Message* pMessage, CPPType* pDestination)
    {
        ObjectType* pObject;
        AnyObjectAdapter<ObjectType, typename KeyType::CPPType>::s_pInstance->StreamOut(pMessage, &pObject);
        pDestination->SetObject(pObject);
    }
};

template <typename Type>
class _Type_std_list
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
        uint32_t uiCount = 0;
        pMessage->ExtractPrimitive(&uiCount);
        for (uint32_t uiIndex = 0; uiIndex < uiCount; ++uiIndex)
        {
            CPPType element;
            Type::Extract(pMessage, &element);
            pDestination->push_back(element);
        }
    }
};

template <typename Type>
class _Type_qvector
{
public:
    typedef qVector<typename Type::CPPType> CPPType;

    static void Add(Message* pMessage, const CPPType& rSource)
    {
        const uint32_t uiCount = static_cast<uint32_t>(rSource.size());

        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiCount), sizeof(uiCount), 1);

        const typename CPPType::const_iterator itEnd = rSource.end();
        for (typename CPPType::const_iterator it = rSource.begin(); it != itEnd; ++it)
        {
            Type::Add(pMessage, *it);
        }
    }

    static void Extract(Message* pMessage, CPPType* pDestination)
    {
        pDestination->clear();

        uint32_t uiCount;

        if (!pMessage->ExtractPrimitive(&uiCount))
        {
            return;
        }

        pDestination->reserve(uiCount);

        for (uint32_t uiIndex = 0; uiIndex < uiCount && !pMessage->HasError(); ++uiIndex)
        {
            typename Type::CPPType element;

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
