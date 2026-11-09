#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_TRACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_TRACE_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class Trace
{
public:
    Trace();
    static Result CreateInstance();
    static void DestroyInstance();
    static Trace* GetInstance() { return s_instance; }
    static void Write(const char* format, ...);
    static void WriteCaption();
    static void WriteCaption(const char* left, const char* right);

    void SetFlag(unsigned long long flag)
    {
        m_enabled = true;
        m_flags |= flag;
    }

private:
    static Trace* s_instance;
    static unsigned char* s_storage;
    unsigned long long m_flags;
    Time m_start;
    bool m_enabled;
};

}
}
}

#endif
