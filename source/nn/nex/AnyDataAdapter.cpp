#include "nn/nex/AnyDataAdapter.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/DynamicData.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/_Type_buffer.hpp"

namespace nn
{
namespace nex
{

AnyDataAdapter AnyDataAdapter::s_instance;
unsigned int AnyDataAdapter::s_uiUseCount = 0;
AnyDataAdapter::PrototypeList* AnyDataAdapter::s_pPrototypes = 0;
CriticalSection AnyDataAdapter::s_criticalSection;

AnyDataAdapter::AnyDataAdapter()
{
}

AnyDataAdapter::~AnyDataAdapter()
{
    if (s_pPrototypes != 0)
        UnregisterAllPrototypes();
}

Data* AnyDataAdapter::FindPrototype(const String& rType) const
{
    PrototypeList::iterator it = s_pPrototypes->begin();
    if (it != s_pPrototypes->end())
    {
        do
        {
            if (String::IsEqual((*it)->GetDataType(), rType))
                return *it;
            ++it;
        } while (it != s_pPrototypes->end());
    }
    return 0;
}

bool AnyDataAdapter::RegisterPrototype(Data* pPrototype)
{
    if (FindPrototype(pPrototype->GetDataType()) != 0)
        return false;
    Data* pClone = pPrototype->Clone();
    const bool bSameType = String::IsEqual(pClone->GetDataType(), pPrototype->GetDataType());
    delete pClone;
    if (bSameType)
        s_pPrototypes->push_back(pPrototype);
    return bSameType;
}

bool AnyDataAdapter::UnregisterPrototype(const String& rType)
{
    for (PrototypeList::iterator it = s_pPrototypes->begin(); it != s_pPrototypes->end(); ++it)
    {
        if (String::IsEqual((*it)->GetDataType(), rType))
        {
            delete *it;
            s_pPrototypes->erase(it);
            return true;
        }
    }
    return false;
}

void AnyDataAdapter::UnregisterAllPrototypes()
{
    PrototypeList::iterator it;
    while ((it = s_pPrototypes->begin()) != s_pPrototypes->end())
        UnregisterPrototype((*it)->GetDataType());
}

Data* AnyDataAdapter::CreateObject(const String& rType, bool bCompactFormat) const
{
    Data* pPrototype = FindPrototype(rType);
    if (pPrototype != 0)
        return pPrototype->Clone();
    pPrototype = FindPrototype(String(L"DynamicData"));
    if (pPrototype != 0)
    {
        pPrototype = pPrototype->Clone();
        static_cast<DynamicData*>(pPrototype)->m_Type = rType;
        static_cast<DynamicData*>(pPrototype)->m_bCompactFormat = bCompactFormat;
    }
    return pPrototype;
}

void AnyDataAdapter::StreamIn(Message* pMessage, const Data& rData) const
{
    _Type_string::Add(pMessage, rData.GetDataType());
    Message dataMessage;
    dataMessage.m_bCompactFormat = pMessage->m_bCompactFormat;
    rData.StreamIn(&dataMessage);
    Buffer* pBuffer = dataMessage.GetBuffer();
    const unsigned int uiLength = pBuffer->GetLength();
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiLength), sizeof(uiLength), 1);
    pMessage->Append(pBuffer->GetData(), 1, pBuffer->GetLength());
}

void AnyDataAdapter::StreamOut(Message* pMessage, Data** ppData) const
{
    String type;
    _Type_string::Extract(pMessage, &type);
    *ppData = CreateObject(type, pMessage->m_bCompactFormat);
    Buffer buffer;
    _Type_buffer::Extract(pMessage, &buffer);
    Message dataMessage(&buffer);
    dataMessage.m_bCompactFormat = pMessage->m_bCompactFormat;
    (*ppData)->StreamOut(&dataMessage);
}

void AnyDataAdapter::Register()
{
    ScopedCS lock(&s_criticalSection);
    if (s_uiUseCount == 0)
        s_pPrototypes = new PrototypeList;
    ++s_uiUseCount;
    AnyObjectAdapter<Data, String>::s_pInstance = &s_instance;
}

void AnyDataAdapter::Unregister()
{
    ScopedCS lock(&s_criticalSection);
    --s_uiUseCount;
    if (s_uiUseCount == 0)
    {
        delete s_pPrototypes;
        s_pPrototypes = 0;
        AnyObjectAdapter<Data, String>::s_pInstance = 0;
    }
}

}
}
