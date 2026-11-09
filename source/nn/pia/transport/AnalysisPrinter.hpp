#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_ANALYSISPRINTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_ANALYSISPRINTER_HPP

namespace nn
{
namespace pia
{
namespace transport
{

class AnalysisPrinter
{
public:
    static void Write(const char* format, ...);
    static void RegisterPrintCallback(void (*callback)(const char*));
    void Trace(unsigned long long value) const;
};

}
}
}

#endif
