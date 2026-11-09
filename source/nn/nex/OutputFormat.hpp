#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_OUTPUTFORMAT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_OUTPUTFORMAT_HPP

#include <cstdarg>
#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class OutputFormat : public RootObject
{
public:
    OutputFormat();
    virtual ~OutputFormat();

    virtual void StartString(wchar_t* buffer, unsigned int capacity);
    virtual void StartPrefixes(wchar_t* buffer, unsigned int capacity);
    virtual void AddPrefixes(wchar_t* buffer, unsigned int capacity);
    virtual void EndPrefixes(wchar_t* buffer, unsigned int capacity);
    virtual void AddIndent(wchar_t* buffer, unsigned int capacity);
    virtual void AddMessage(wchar_t* buffer, unsigned int capacity, const wchar_t* format, va_list arguments);
    virtual void EndString(wchar_t* buffer, unsigned int capacity);

    static void AppendToString(wchar_t* buffer, const wchar_t* text, unsigned int capacity);
    static unsigned int AddMessageImpl(wchar_t* buffer, unsigned int capacity, const wchar_t* format,
                                       va_list arguments);
    void IncreaseIndent(unsigned int amount);
    void DecreaseIndent(unsigned int amount);

    void EnableNumberTraces(bool enabled);
    void ShowProcessID(bool enabled);
    void ShowThreadID(bool enabled);
    void ShowLocalTime(bool enabled);
    void ShowDateTime(bool enabled);
    void ShowSessionTime(bool enabled);
    void ShowSystemThreadName(bool enabled);
    void ShowLocalStationHandle(bool enabled);
    void ShowCID(bool enabled);
    void ShowPID(bool enabled);
    void AddPrefix(const wchar_t* prefix);
    void ShowCurrentContext(bool enabled);
    static void PreparePrefix(wchar_t* buffer, unsigned int capacity, const wchar_t* format, ...);

private:
    uint32_t m_indent;
    uint32_t m_field4;
    bool m_flags[10];
    uint32_t m_field14;
    uint32_t m_startTime;
};

}
}

#endif
