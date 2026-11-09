#include "nn/nex/_DDL_DataStoreCompleteUpdateParam.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t COMPLETE_UPDATE_PARAM_MAX_STRUCTURE_VERSION = 0;

}

_DDL_DataStoreCompleteUpdateParam& _DDL_DataStoreCompleteUpdateParam::operator=(
    const _DDL_DataStoreCompleteUpdateParam& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiDataId = rSource.m_uiDataId;
    m_uiVersion = rSource.m_uiVersion;
    m_bIsSuccess = rSource.m_bIsSuccess;
    return *this;
}

void _DDL_DataStoreCompleteUpdateParam::Add(Message* pMessage, const _DDL_DataStoreCompleteUpdateParam& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = COMPLETE_UPDATE_PARAM_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiDataId), 8, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiVersion), 4, 1);
    *pMessage << rSource.m_bIsSuccess;

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_DataStoreCompleteUpdateParam::Extract(Message* pMessage, _DDL_DataStoreCompleteUpdateParam* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_uiVersion);

    if (pMessage->HasError())
    {
        return;
    }

    *pMessage >> pDestination->m_bIsSuccess;

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
