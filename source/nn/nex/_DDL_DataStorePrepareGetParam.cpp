#include "nn/nex/_DDL_DataStorePrepareGetParam.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t PREPARE_GET_PARAM_MAX_STRUCTURE_VERSION = 0;

}

_DDL_DataStorePrepareGetParam& _DDL_DataStorePrepareGetParam::operator=(const _DDL_DataStorePrepareGetParam& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiDataId = rSource.m_uiDataId;
    m_uiLockId = rSource.m_uiLockId;
    m_PersistenceTarget = rSource.m_PersistenceTarget;
    m_uiAccessPassword = rSource.m_uiAccessPassword;
    m_ExtraData = rSource.m_ExtraData;
    return *this;
}

void _DDL_DataStorePrepareGetParam::Add(Message* pMessage, const _DDL_DataStorePrepareGetParam& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = PREPARE_GET_PARAM_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiDataId), 8, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiLockId), 4, 1);
    _DDL_DataStorePersistenceTarget::Add(pMessage, rSource.m_PersistenceTarget);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiAccessPassword), 8, 1);
    _Type_qvector<_Type_string>::Add(pMessage, rSource.m_ExtraData);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_DataStorePrepareGetParam::Extract(Message* pMessage, _DDL_DataStorePrepareGetParam* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_uiLockId);

    if (pMessage->HasError())
    {
        return;
    }

    _DDL_DataStorePersistenceTarget::Extract(pMessage, &pDestination->m_PersistenceTarget);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiAccessPassword);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_qvector<_Type_string>::Extract(pMessage, &pDestination->m_ExtraData);

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
