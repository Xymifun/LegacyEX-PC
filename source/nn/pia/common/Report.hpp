#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_REPORT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_REPORT_HPP

#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class Report
{
public:
    Report();
    Report(unsigned int category);
    ~Report();
    void Write(unsigned int category, const char* format, ...);
    void WriteCaption(unsigned int category);

    unsigned int m_first;
    unsigned char m_field04;
    unsigned char m_padding05[3];
    Time m_time;
    unsigned char m_enabled;
};

extern Report* g_currentReport;

}
}
}

#endif
