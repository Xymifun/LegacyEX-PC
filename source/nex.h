#ifndef LEGACY_CONSOLE_WIIU_NEX_H
#define LEGACY_CONSOLE_WIIU_NEX_H

#include <stdint.h>
#include "nn/nex/CallbackRoot.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/Job.hpp"
#include "nn/nex/JobConnectEndPoint.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/NotificationEvent.hpp"
#include "nn/nex/NotificationEventHandler.hpp"
#include "nn/nex/StringStream.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/UserContext.hpp"
#include "vendor/__N_19_OnlineCore_Blob_cpp_9428d484.hpp"

namespace nn
{
namespace nex
{
class EndPoint;
class String;
class Time;
class Socket;
class SignatureBytes;
class DateTime;
class Variant;
class HTTPURL;
class StationURL;
class UserMessage;
class MessageRecipient;
class IOCompletionContext;

StringStream& __ls(StringStream& stream, const String& value);
StringStream& __ls(StringStream& stream, const InetAddress& value);
StringStream& __ls(StringStream& stream, const qResult& value);
StringStream& __ls(StringStream& stream, const Time& value);
StringStream& __ls(StringStream& stream, Socket& value);
StringStream& __ls(StringStream& stream, const IOCompletionContext& value);
StringStream& __ls(StringStream& stream, const SignatureBytes& value);
StringStream& __ls(StringStream& stream, const DateTime& value);
StringStream& __ls(StringStream& stream, const Variant& value);
StringStream& __ls(StringStream& stream, const HTTPURL& value);
StringStream& __ls(StringStream& stream, const StationURL& value);
StringStream& __ls(StringStream& stream, const UserMessage& value);
StringStream& __ls(StringStream& stream, const MessageRecipient& value);
StringStream& __ls(StringStream& stream, const StringStream& value);
StringStream& hex(StringStream& stream);
StringStream& dec(StringStream& stream);
StringStream& showbase(StringStream& stream);
StringStream& noshowbase(StringStream& stream);
StringStream& endl(StringStream& stream);
StringStream& boolalpha(StringStream& stream);
StringStream& noboolalpha(StringStream& stream);

}
}

#include "nn/nex/BadEvents.hpp"

#endif
