#include "nn/nex/_DDL_DataStoreChangeMetaParam.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t CHANGE_META_PARAM_MAX_STRUCTURE_VERSION = 1;

}

_DDL_DataStoreChangeMetaParam& _DDL_DataStoreChangeMetaParam::operator=(const _DDL_DataStoreChangeMetaParam& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiDataId = rSource.m_uiDataId;
    m_uiModifiesFlag = rSource.m_uiModifiesFlag;
    m_Name = rSource.m_Name;
    m_Permission = rSource.m_Permission;
    m_DelPermission = rSource.m_DelPermission;
    m_uiPeriod = rSource.m_uiPeriod;
    m_MetaBinary = rSource.m_MetaBinary;
    m_Tags = rSource.m_Tags;
    m_uiUpdatePassword = rSource.m_uiUpdatePassword;
    m_uiReferredCnt = rSource.m_uiReferredCnt;
    m_uiDataType = rSource.m_uiDataType;
    m_uiStatus = rSource.m_uiStatus;
    m_CompareParam = rSource.m_CompareParam;
    m_PersistenceTarget = rSource.m_PersistenceTarget;
    return *this;
}

void _DDL_DataStoreChangeMetaParam::Add(Message* pMessage, const _DDL_DataStoreChangeMetaParam& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = CHANGE_META_PARAM_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiDataId), 8, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiModifiesFlag), 4, 1);
    _Type_string::Add(pMessage, rSource.m_Name);
    _DDL_DataStorePermission::Add(pMessage, rSource.m_Permission);
    _DDL_DataStorePermission::Add(pMessage, rSource.m_DelPermission);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiPeriod), 2, 1);
    _Type_qBuffer::Add(pMessage, rSource.m_MetaBinary);
    _Type_qvector<_Type_string>::Add(pMessage, rSource.m_Tags);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiUpdatePassword), 8, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiReferredCnt), 4, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiDataType), 2, 1);
    pMessage->Append(&rSource.m_uiStatus, 1, 1);
    _DDL_DataStoreChangeMetaCompareParam::Add(pMessage, rSource.m_CompareParam);

    if (!bCompactFormat)
    {
        if (rSource.m_uiStructureVersion != 0)
        {
            _DDL_DataStorePersistenceTarget::Add(pMessage, rSource.m_PersistenceTarget);
        }

        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_DataStoreChangeMetaParam::Extract(Message* pMessage, _DDL_DataStoreChangeMetaParam* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_uiModifiesFlag);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_string::Extract(pMessage, &pDestination->m_Name);

    if (pMessage->HasError())
    {
        return;
    }

    _DDL_DataStorePermission::Extract(pMessage, &pDestination->m_Permission);

    if (pMessage->HasError())
    {
        return;
    }

    _DDL_DataStorePermission::Extract(pMessage, &pDestination->m_DelPermission);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiPeriod);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_qBuffer::Extract(pMessage, &pDestination->m_MetaBinary);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_qvector<_Type_string>::Extract(pMessage, &pDestination->m_Tags);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiUpdatePassword);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiReferredCnt);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiDataType);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiStatus);

    if (pMessage->HasError())
    {
        return;
    }

    _DDL_DataStoreChangeMetaCompareParam::Extract(pMessage, &pDestination->m_CompareParam);

    if (pMessage->HasError())
    {
        return;
    }

    if (pDestination->m_uiStructureVersion != 0)
    {
        _DDL_DataStorePersistenceTarget::Extract(pMessage, &pDestination->m_PersistenceTarget);

        if (pMessage->HasError())
        {
            return;
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
