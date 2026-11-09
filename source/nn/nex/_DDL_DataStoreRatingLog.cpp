#include "nn/nex/_DDL_DataStoreRatingLog.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t RATING_LOG_MAX_STRUCTURE_VERSION = 0;

}

_DDL_DataStoreRatingLog& _DDL_DataStoreRatingLog::operator=(const _DDL_DataStoreRatingLog& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_bIsRated = rSource.m_bIsRated;
    m_uiPid = rSource.m_uiPid;
    m_iRatingValue = rSource.m_iRatingValue;
    m_LockExpirationTime = rSource.m_LockExpirationTime;
    return *this;
}

void _DDL_DataStoreRatingLog::Add(Message* pMessage, const _DDL_DataStoreRatingLog& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = RATING_LOG_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    *pMessage << rSource.m_bIsRated;
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiPid), 4, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_iRatingValue), 4, 1);
    const uint64_t uiLockExpirationTime = rSource.m_LockExpirationTime;
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiLockExpirationTime), 8, 1);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_DataStoreRatingLog::Extract(Message* pMessage, _DDL_DataStoreRatingLog* pDestination)
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

    *pMessage >> pDestination->m_bIsRated;

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiPid);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_iRatingValue);

    if (pMessage->HasError())
    {
        return;
    }

    uint64_t uiLockExpirationTime;

    pMessage->ExtractPrimitive(&uiLockExpirationTime);
    const DateTime lockExpirationTime(uiLockExpirationTime);
    pDestination->m_LockExpirationTime = lockExpirationTime;

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
