#include "nn/pia/transport/AnalysisPrinter.hpp"

#include <cstdarg>
#include <cstdio>

namespace nn
{
namespace pia
{
namespace transport
{

namespace
{
char g_analysisBuffer[2000];
void (*g_printCallback)(const char*) = 0;
}

void AnalysisPrinter::Write(const char* format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    std::vsprintf(g_analysisBuffer, format, arguments);
    va_end(arguments);

    if (g_printCallback != 0)
        g_printCallback(g_analysisBuffer);
}

}
}
}
