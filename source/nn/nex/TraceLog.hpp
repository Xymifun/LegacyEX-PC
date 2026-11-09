#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TRACELOG_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TRACELOG_HPP

#include "nn/nex/Log.hpp"

namespace nn
{
namespace nex
{

class TraceLog : public Log
{
public:
    enum
    {
        TRACE_ALWAYS = 1
    };

    TraceLog();
    virtual ~TraceLog();

    static TraceLog* GetInstance();

    void SetFlag(unsigned int uiFlag) { m_uiFlags |= uiFlag; }

    void SetFlag(const wchar_t*) {}

private:
    unsigned int m_field10;
    unsigned int m_uiFlags;
};

}
}

#endif
