#include "nn/nex/AnyGatheringAdapter.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/DynamicGathering.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

AnyGatheringAdapter AnyGatheringAdapter::s_instance;
AnyGatheringAdapter::PrototypeList* AnyGatheringAdapter::s_pPrototypes = 0;
CriticalSection AnyGatheringAdapter::s_criticalSection;

AnyGatheringAdapter::AnyGatheringAdapter() : m_uiUseCount(0)
{
}

AnyGatheringAdapter::~AnyGatheringAdapter()
{
}

Gathering* AnyGatheringAdapter::FindPrototype(const String& rType) const
{
    ScopedCS lock(&s_criticalSection);
    for (PrototypeList::const_iterator it = s_pPrototypes->begin(); it != s_pPrototypes->end(); ++it)
    {
        if (String::IsEqual((*it)->GetGatheringType(), rType))
            return *it;
    }

    return 0;
}

bool AnyGatheringAdapter::RegisterPrototype(Gathering* pPrototype)
{
    ScopedCS lock(&s_criticalSection);
    if (FindPrototype(pPrototype->GetGatheringType()) != 0)
        return false;

    Gathering* pClone = pPrototype->Clone();
    const bool bSameType = String::IsEqual(pClone->GetGatheringType(), pPrototype->GetGatheringType());
    delete pClone;
    if (bSameType)
        s_pPrototypes->push_back(pPrototype);

    return bSameType;
}

bool AnyGatheringAdapter::UnregisterPrototype(const String& rType)
{
    ScopedCS lock(&s_criticalSection);
    for (PrototypeList::iterator it = s_pPrototypes->begin(); it != s_pPrototypes->end(); ++it)
    {
        if (String::IsEqual((*it)->GetGatheringType(), rType))
        {
            delete *it;
            s_pPrototypes->erase(it);
            return true;
        }
    }

    return false;
}

void AnyGatheringAdapter::UnregisterAllPrototypes()
{
    ScopedCS lock(&s_criticalSection);
    while (s_pPrototypes->begin() != s_pPrototypes->end())
        UnregisterPrototype((*s_pPrototypes->begin())->GetGatheringType());
}

Gathering* AnyGatheringAdapter::CreateObject(const String& rType) const
{
    Gathering* pPrototype = FindPrototype(rType);
    if (pPrototype != 0)
        return pPrototype->Clone();

    pPrototype = FindPrototype(String(L"DynamicGathering"));
    if (pPrototype != 0)
    {
        pPrototype = pPrototype->Clone();
        static_cast<DynamicGathering*>(pPrototype)->m_Type = rType;
    }

    return pPrototype;
}

void AnyGatheringAdapter::StreamIn(Message* pMessage, const Gathering& rGathering) const
{
    String type = rGathering.GetGatheringType();
    if (String::IsEqual(type, L"PersistentGathering"))
        type = L"Community";

    _Type_string::Add(pMessage, type);
    Message gatheringMessage;
    rGathering.StreamIn(&gatheringMessage);
    Buffer* pBuffer = gatheringMessage.GetBuffer();
    const unsigned int uiLength = pBuffer->GetLength();
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiLength), sizeof(uiLength), 1);
    pMessage->Append(pBuffer->GetData(), 1, pBuffer->GetLength());
}

void AnyGatheringAdapter::StreamOut(Message* pMessage, Gathering** ppGathering) const
{
    String type;
    _Type_string::Extract(pMessage, &type);
    if (String::IsEqual(type, L"Community"))
        type = L"PersistentGathering";

    *ppGathering = CreateObject(type);
    Buffer buffer;
    unsigned int uiLength = 0;
    if (pMessage->ExtractPrimitive(&uiLength) && pMessage->ValidateBufferLimit(uiLength))
    {
        buffer.AppendData(pMessage->GetBuffer()->GetData() + pMessage->GetPosition(), uiLength, 0xFFFFFFFF);
        const unsigned int uiNewPosition = pMessage->GetPosition() + uiLength;
        if (uiNewPosition <= pMessage->GetLength())
            pMessage->SetPosition(uiNewPosition);
    }

    Message gatheringMessage(&buffer);
    (*ppGathering)->StreamOut(&gatheringMessage);
}

unsigned int AnyGatheringAdapter::RegisterUse()
{
    ScopedCS lock(&s_criticalSection);
    if (s_instance.m_uiUseCount == 0)
        s_pPrototypes = new PrototypeList;

    AnyObjectAdapter<Gathering, String>::s_pInstance = &s_instance;
    return ++s_instance.m_uiUseCount;
}

unsigned int AnyGatheringAdapter::UnregisterUse()
{
    ScopedCS lock(&s_criticalSection);
    if (--s_instance.m_uiUseCount == 0)
    {
        s_instance.UnregisterAllPrototypes();
        AnyObjectAdapter<Gathering, String>::s_pInstance = 0;
        delete s_pPrototypes;
        s_pPrototypes = 0;
    }

    return s_instance.m_uiUseCount;
}

}
}
