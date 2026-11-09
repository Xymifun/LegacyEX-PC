#include "nn/nex/_DDL_MatchmakeParam.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/Variant.hpp"
#include "nn/nex/_Type_std_map.hpp"
#include "nn/nex/_Type_variant.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t MAX_STRUCTURE_VERSION = 0;

}

_DDL_MatchmakeParam& _DDL_MatchmakeParam::operator=(const _DDL_MatchmakeParam& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_Parameters = rSource.m_Parameters;
    return *this;
}

void _DDL_MatchmakeParam::Add(Message* pMessage, const _DDL_MatchmakeParam& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();
    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = MAX_STRUCTURE_VERSION;
        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));
        uiSizePosition = pMessage->GetLength();
        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }
    _Type_std_map<_Type_string, _Type_variant>::Add(pMessage, rSource.m_Parameters);
    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);
        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_MatchmakeParam::Extract(Message* pMessage, _DDL_MatchmakeParam* pDestination)
{
    pDestination->m_uiStructureVersion = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();
    uint32_t uiSize = 0;
    if (!bCompactFormat)
    {
        _Type_byte::Extract(pMessage, &pDestination->m_uiStructureVersion);
        if (pMessage->HasError())
            return;
        _Type_uint32::Extract(pMessage, &uiSize);
        if (pMessage->HasError())
            return;
    }
    const unsigned int uiStartPosition = pMessage->GetPosition();
    if (pMessage->HasError())
        return;
    _Type_std_map<_Type_string, _Type_variant>::Extract(pMessage, &pDestination->m_Parameters);
    if (pMessage->HasError() || bCompactFormat)
        return;
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
