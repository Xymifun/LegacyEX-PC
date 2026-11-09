#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_USERMESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_USERMESSAGE_HPP

#include <stdint.h>

#include "nn/nex/Data.hpp"
#include "nn/nex/DateTime.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_UserMessage : public Data
{
public:
    _DDL_UserMessage() : Data(), m_uiStructureVersion(0), m_ReceptionTime(), m_Subject(), m_SenderName() {}

    virtual ~_DDL_UserMessage() {}

    _DDL_UserMessage& operator=(const _DDL_UserMessage& rSource);
    bool operator==(const _DDL_UserMessage& rSource) const;

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_UserMessage& rSource);
    static void Extract(Message* pMessage, _DDL_UserMessage* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiID;
    uint32_t m_uiRecipientID;
    uint32_t m_uiRecipientType;
    uint32_t m_uiParentID;
    uint32_t m_uiSenderID;
    DateTime m_ReceptionTime;
    uint32_t m_uiLifeTime;
    uint32_t m_uiFlags;
    String m_Subject;
    String m_SenderName;
};

}
}

#endif
