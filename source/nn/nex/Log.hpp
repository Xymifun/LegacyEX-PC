#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LOG_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LOG_HPP

#include <cstdarg>

#include "nn/nex/LogDevice.hpp"
#include "nn/nex/LogFilter.hpp"
#include "nn/nex/OutputFormat.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Log : public RootObject
{
public:
    Log();
    virtual ~Log();

    virtual void Output(const wchar_t* format, ...);
    virtual void OutputString(const wchar_t* format, ...);

    virtual void AddCustomPrefix(wchar_t*, int, void*) {}

    void SetLogDevice(LogDevice* device, bool deleteOld);
    void SetOutputFormat(OutputFormat* format);
    void SetLogFilter(LogFilter* filter);

    LogDevice* GetLogDevice() { return m_logDevice; }

    LogFilter* GetLogFilter() { return m_logFilter; }

    OutputFormat* GetOutputFormat();
    void PrepareOutput(wchar_t* buffer, unsigned int capacity, void* context);
    void OutputImpl(const wchar_t* format, va_list arguments, void* context);

private:
    LogDevice* m_logDevice;
    OutputFormat* m_outputFormat;
    LogFilter* m_logFilter;
};

}
}

#endif
