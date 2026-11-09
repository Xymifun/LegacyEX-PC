#include "nn/nex/_DDL_DataStoreReqPostInfo.hpp"

#include <algorithm>

#include "nn/nex/DDLClassMetaType.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t REQ_POST_INFO_MAX_STRUCTURE_VERSION = 0;

}

_DDL_DataStoreReqPostInfo& _DDL_DataStoreReqPostInfo::operator=(const _DDL_DataStoreReqPostInfo& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiDataId = rSource.m_uiDataId;
    m_Url = rSource.m_Url;
    m_RequestHeaders = rSource.m_RequestHeaders;
    m_FormFields = rSource.m_FormFields;
    m_RootCaCert = rSource.m_RootCaCert;
    return *this;
}

void _DDL_DataStoreReqPostInfo::Add(Message* pMessage, const _DDL_DataStoreReqPostInfo& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = REQ_POST_INFO_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiDataId), 8, 1);
    _Type_string::Add(pMessage, rSource.m_Url);
    _Type_qvector<DDLClassMetaType<DataStoreKeyValue> >::Add(pMessage, rSource.m_RequestHeaders);
    _Type_qvector<DDLClassMetaType<DataStoreKeyValue> >::Add(pMessage, rSource.m_FormFields);
    _Type_qvector<_Type_byte>::Add(pMessage, rSource.m_RootCaCert);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_DataStoreReqPostInfo::Extract(Message* pMessage, _DDL_DataStoreReqPostInfo* pDestination)
{
    pDestination->m_uiStructureVersion = 0;

    const bool bCompactFormat = pMessage->IsCompactFormat();
    uint32_t uiSize = 0;

    if (!bCompactFormat)
    {
        pMessage->ExtractPrimitive(&pDestination->m_uiStructureVersion);

        if (pMessage->HasError())
        {
            return;
        }

        pMessage->ExtractPrimitive(&uiSize);

        if (pMessage->HasError())
        {
            return;
        }
    }

    const unsigned int uiStartPosition = pMessage->GetPosition();

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiDataId);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_string::Extract(pMessage, &pDestination->m_Url);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_qvector<DDLClassMetaType<DataStoreKeyValue> >::Extract(pMessage, &pDestination->m_RequestHeaders);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_qvector<DDLClassMetaType<DataStoreKeyValue> >::Extract(pMessage, &pDestination->m_FormFields);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_qvector<_Type_byte>::Extract(pMessage, &pDestination->m_RootCaCert);

    if (pMessage->HasError() || bCompactFormat)
    {
        return;
    }

    const unsigned int uiEndPosition = uiStartPosition + uiSize;

    if (pMessage->GetPosition() <= uiEndPosition && uiEndPosition <= pMessage->GetLength())
    {
        pMessage->SetPosition(uiEndPosition);
        return;
    }

    pMessage->SetError();
}

}
}
