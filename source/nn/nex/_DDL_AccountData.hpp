#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_ACCOUNTDATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_ACCOUNTDATA_HPP

#include <stdint.h>

#include "nn/nex/DateTime.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_AccountData : public RootObject
{
public:
    _DDL_AccountData() : m_uiStructureVersion(0) {}

    virtual ~_DDL_AccountData() {}

    _DDL_AccountData& operator=(const _DDL_AccountData& rSource);
    bool operator==(const _DDL_AccountData& rSource) const;

    static void Add(Message* pMessage, const _DDL_AccountData& rSource);
    static void Extract(Message* pMessage, _DDL_AccountData* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiPid;
    String m_Name;
    uint32_t m_uiGroups;
    String m_Email;
    DateTime m_Date;
    DateTime m_EffectiveDate;
    String m_NotEffectiveMessage;
    DateTime m_ExpiryDate;
    String m_ExpiredMessage;
};

}
}

#endif
