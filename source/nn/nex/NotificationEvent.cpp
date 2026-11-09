#include "nn/nex/NotificationEvent.hpp"
#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace nex
{

StringStream& operator<<(StringStream& stream, const NotificationEvent& event)
{
    ((((((stream << L"NotificationEvent (" << L" PIDSource=").StreamNumber(event.m_uiValue) << L" Type=")
            .StreamNumber(event.GetType() / 1000)
        << L" SubType=")
           .StreamNumber(event.GetType() - event.GetType() / 1000 * 1000)
       << L" Param1=")
          .StreamNumber(event.GetParam1())
      << L" Param2=")
         .StreamNumber(event.GetParam2())
     << L" Param3=")
            .StreamNumber(event.GetParam3())
        << L" UInt64Param1=" << ((static_cast<unsigned long long>(event.GetParam1()) << 32) | event.GetParam2())
        << L" StrParam=" << event.m_StringParam << L") ";
    return stream;
}

}
}
