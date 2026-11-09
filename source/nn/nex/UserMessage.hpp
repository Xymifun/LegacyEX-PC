#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_USERMESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_USERMESSAGE_HPP

#include <stdint.h>

#include "nn/nex/_DDL_UserMessage.hpp"

namespace nn
{
namespace nex
{

class StringStream;

class UserMessage : public _DDL_UserMessage
{
public:
    UserMessage() : _DDL_UserMessage()
    {
        m_uiID = 0;
        m_uiParentID = 0;
        m_uiSenderID = 0;
        m_uiRecipientID = 0;
        m_uiRecipientType = 0;
        m_ReceptionTime = DateTime::s_never;
        m_uiLifeTime = 0;
        m_uiFlags = 0;
        m_Subject = L"";
    }

    virtual ~UserMessage() {}

    virtual void Trace(uint64_t uiFilter);
};

StringStream& operator<<(StringStream& rStream, const UserMessage& rMessage);

}
}

#endif
