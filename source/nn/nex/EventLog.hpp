#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_EVENTLOG_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_EVENTLOG_HPP

#include <cstdarg>

#include "nn/nex/Log.hpp"

namespace nn
{
namespace nex
{

class EventLog : public Log
{
public:
    enum LogLevel
    {
        Unknown = 0,
        Debug = 1,
        Info = 2,
        Warning = 3,
        Error = 4,
        Fatal = 5,
        Always = 6
    };

    EventLog();
    virtual ~EventLog();

    virtual void AddCustomPrefix(wchar_t* buffer, int capacity, void* context);

    static EventLog* GetInstance();
    static const wchar_t* LogLevelToString(LogLevel level);

    void SetLogLevel(LogLevel level);

    LogLevel GetLogLevel() { return m_logLevel; }

    void TraceOutput(LogLevel, const wchar_t*, va_list) {}

private:
    LogLevel m_logLevel;
    bool m_field14;
};

}
}

#endif
