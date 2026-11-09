#include "nn/nex/_DDL_DataStorePersistenceInitParam.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t PERSISTENCE_INIT_PARAM_MAX_STRUCTURE_VERSION = 0;

}

_DDL_DataStorePersistenceInitParam& _DDL_DataStorePersistenceInitParam::operator=(
    const _DDL_DataStorePersistenceInitParam& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiPersistenceSlotId = rSource.m_uiPersistenceSlotId;
    m_bDeleteLastObject = rSource.m_bDeleteLastObject;
    return *this;
}

void _DDL_DataStorePersistenceInitParam::Add(Message* pMessage, const _DDL_DataStorePersistenceInitParam& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = PERSISTENCE_INIT_PARAM_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiPersistenceSlotId), 2, 1);
    *pMessage << rSource.m_bDeleteLastObject;

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_DataStorePersistenceInitParam::Extract(Message* pMessage, _DDL_DataStorePersistenceInitParam* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_uiPersistenceSlotId);

    if (pMessage->HasError())
    {
        return;
    }

    *pMessage >> pDestination->m_bDeleteLastObject;

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
