#include "nn/pia/common/Report.hpp"

#include <cstdarg>

namespace nn
{
namespace pia
{
namespace common
{

Report* g_currentReport;

Report::Report() : m_first(0xffffffffu), m_time(), m_enabled(1)
{
    m_time.SetNow();
}

Report::~Report()
{
}

void Report::Write(unsigned int category, const char* format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    va_end(arguments);
}

void Report::WriteCaption(unsigned int category)
{
    (void)category;
}

}
}
}
