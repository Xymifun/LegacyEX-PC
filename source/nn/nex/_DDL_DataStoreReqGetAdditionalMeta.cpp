#include "nn/nex/_DDL_DataStoreReqGetAdditionalMeta.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t REQ_GET_ADDITIONAL_META_MAX_STRUCTURE_VERSION = 0;

}

_DDL_DataStoreReqGetAdditionalMeta& _DDL_DataStoreReqGetAdditionalMeta::operator=(
    const _DDL_DataStoreReqGetAdditionalMeta& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiOwnerId = rSource.m_uiOwnerId;
    m_uiDataType = rSource.m_uiDataType;
    m_uiVersion = rSource.m_uiVersion;
    m_MetaBinary = rSource.m_MetaBinary;
    return *this;
}

void _DDL_DataStoreReqGetAdditionalMeta::Add(Message* pMessage, const _DDL_DataStoreReqGetAdditionalMeta& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = REQ_GET_ADDITIONAL_META_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiOwnerId), 4, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiDataType), 2, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiVersion), 2, 1);
    _Type_qBuffer::Add(pMessage, rSource.m_MetaBinary);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_DataStoreReqGetAdditionalMeta::Extract(Message* pMessage, _DDL_DataStoreReqGetAdditionalMeta* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_uiOwnerId);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiDataType);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiVersion);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_qBuffer::Extract(pMessage, &pDestination->m_MetaBinary);

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
