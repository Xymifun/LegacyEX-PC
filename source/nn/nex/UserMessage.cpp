#include "nn/nex/UserMessage.hpp"

#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace nex
{

StringStream& operator<<(StringStream& rStream, const UserMessage& rMessage)
{
    (((((((rStream << L" UserMessage (" << L" ID=").StreamNumber(rMessage.m_uiID) << L" ParentID=")
             .StreamNumber(rMessage.m_uiParentID)
         << L" RecipientID=")
            .StreamNumber(rMessage.m_uiRecipientID)
        << L" RecipientType=")
           .StreamNumber(rMessage.m_uiRecipientType)
       << L" SenderID=")
          .StreamNumber(rMessage.m_uiSenderID)
      << L" SenderName=" << rMessage.m_SenderName << L" RxTime=" << rMessage.m_ReceptionTime << L" LifeTime=")
         .StreamNumber(rMessage.m_uiLifeTime)
     << L" Flags=")
            .StreamNumber(rMessage.m_uiFlags)
        << L" Subject=" << rMessage.m_Subject << L") ";
    return rStream;
}

void UserMessage::Trace(uint64_t)
{
    StringStream stream;
    stream << *this;
}

}
}
