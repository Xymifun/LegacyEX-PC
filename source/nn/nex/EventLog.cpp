#include "nn/nex/EventLog.hpp"

#include <stdint.h>
#include <cwchar>
#include <netinet/in.h>

#include "nn/nex/BerkeleySocketDriver.hpp"
#include "nn/nex/GlobalVariables.hpp"

namespace nn
{
namespace nex
{

EventLog::EventLog() : m_logLevel(Info), m_field14(false)
{
}

EventLog::~EventLog()
{
}

EventLog* EventLog::GetInstance()
{
    return &GlobalVariables::GetInstancePtr()->GetEventLog();
}

const wchar_t* EventLog::LogLevelToString(LogLevel level)
{
    switch (level)
    {
    case Debug:
        return L"Debug";
    case Info:
        return L"Info";
    case Warning:
        return L"Warning";
    case Error:
        return L"Error";
    case Fatal:
        return L"Fatal";
    case Always:
        return L"Always";
    default:
        return L"Unknown";
    }
}

void EventLog::SetLogLevel(LogLevel level)
{
    m_logLevel = level;
}

void EventLog::AddCustomPrefix(wchar_t* buffer, int capacity, void* context)
{
    const LogLevel level = static_cast<LogLevel>(reinterpret_cast<intptr_t>(context));
    if (level != Always)
    {
        const wchar_t* name = LogLevelToString(level);
        if (name != 0)
            std::swprintf(buffer + std::wcslen(buffer), capacity, L"%ls|", name);
    }
}

}
}

in_addr_t inet_addr(const char* cp)
{
    in_addr address;
    int result = inet_pton(AF_INET, cp, &address);
    if (result > 0)
        return address.s_addr;
    return INADDR_NONE;
}
