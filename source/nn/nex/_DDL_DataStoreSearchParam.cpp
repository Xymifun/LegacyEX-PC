#include "nn/nex/_DDL_DataStoreSearchParam.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t SEARCH_PARAM_MAX_STRUCTURE_VERSION = 3;
const unsigned int SIZE_FIELD_LENGTH = 4;

}

void _DDL_DataStoreSearchParam::Add(Message* pMessage, const _DDL_DataStoreSearchParam& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = SEARCH_PARAM_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    pMessage->Append(&rSource.m_uiSearchTarget, 1, 1);
    _Type_qvector<_Type_uint32>::Add(pMessage, rSource.m_OwnerIds);
    pMessage->Append(&rSource.m_uiOwnerType, 1, 1);
    _Type_qvector<_Type_uint32>::Add(pMessage, rSource.m_DestinationIds);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiDataType), 2, 1);

    const uint64_t uiCreatedAfter = rSource.m_CreatedAfter;

    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiCreatedAfter), 8, 1);

    const uint64_t uiCreatedBefore = rSource.m_CreatedBefore;

    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiCreatedBefore), 8, 1);

    const uint64_t uiUpdatedAfter = rSource.m_UpdatedAfter;

    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiUpdatedAfter), 8, 1);

    const uint64_t uiUpdatedBefore = rSource.m_UpdatedBefore;

    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiUpdatedBefore), 8, 1);

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiReferDataId), 4, 1);
    _Type_qvector<_Type_string>::Add(pMessage, rSource.m_Tags);
    pMessage->Append(&rSource.m_uiResultOrderColumn, 1, 1);
    pMessage->Append(&rSource.m_uiResultOrder, 1, 1);
    _DDL_ResultRange::Add(pMessage, rSource.m_ResultRange);
    pMessage->Append(&rSource.m_uiResultOption, 1, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiMinimalRatingFrequency), 4, 1);

    if (!bCompactFormat)
    {
        if (rSource.m_uiStructureVersion >= 1)
        {
            *pMessage << rSource.m_bUseCache;

            if (rSource.m_uiStructureVersion >= 2)
            {
                *pMessage << rSource.m_bTotalCountEnabled;

                if (rSource.m_uiStructureVersion >= 3)
                {
                    _Type_qvector<_Type_uint16>::Add(pMessage, rSource.m_DataTypes);
                }
            }
        }

        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - SIZE_FIELD_LENGTH);
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_DataStoreSearchParam::Extract(Message* pMessage, _DDL_DataStoreSearchParam* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_uiSearchTarget);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_qvector<_Type_uint32>::Extract(pMessage, &pDestination->m_OwnerIds);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiOwnerType);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_qvector<_Type_uint32>::Extract(pMessage, &pDestination->m_DestinationIds);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiDataType);

    if (pMessage->HasError())
    {
        return;
    }

    uint64_t uiCreatedAfter;

    pMessage->ExtractPrimitive(&uiCreatedAfter);
    const DateTime createdAfter(uiCreatedAfter);
    pDestination->m_CreatedAfter = createdAfter;

    if (pMessage->HasError())
    {
        return;
    }

    uint64_t uiCreatedBefore;

    pMessage->ExtractPrimitive(&uiCreatedBefore);
    const DateTime createdBefore(uiCreatedBefore);
    pDestination->m_CreatedBefore = createdBefore;

    if (pMessage->HasError())
    {
        return;
    }

    uint64_t uiUpdatedAfter;

    pMessage->ExtractPrimitive(&uiUpdatedAfter);
    const DateTime updatedAfter(uiUpdatedAfter);
    pDestination->m_UpdatedAfter = updatedAfter;

    if (pMessage->HasError())
    {
        return;
    }

    uint64_t uiUpdatedBefore;

    pMessage->ExtractPrimitive(&uiUpdatedBefore);
    const DateTime updatedBefore(uiUpdatedBefore);
    pDestination->m_UpdatedBefore = updatedBefore;

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiReferDataId);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_qvector<_Type_string>::Extract(pMessage, &pDestination->m_Tags);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiResultOrderColumn);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiResultOrder);

    if (pMessage->HasError())
    {
        return;
    }

    _DDL_ResultRange::Extract(pMessage, &pDestination->m_ResultRange);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiResultOption);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiMinimalRatingFrequency);

    if (pMessage->HasError())
    {
        return;
    }

    if (pDestination->m_uiStructureVersion != 0)
    {
        *pMessage >> pDestination->m_bUseCache;

        if (pMessage->HasError())
        {
            return;
        }

        if (pDestination->m_uiStructureVersion > 1)
        {
            *pMessage >> pDestination->m_bTotalCountEnabled;

            if (pMessage->HasError())
            {
                return;
            }

            if (pDestination->m_uiStructureVersion > 2)
            {
                _Type_qvector<_Type_uint16>::Extract(pMessage, &pDestination->m_DataTypes);

                if (pMessage->HasError())
                {
                    return;
                }
            }
        }
    }

    if (bCompactFormat)
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
