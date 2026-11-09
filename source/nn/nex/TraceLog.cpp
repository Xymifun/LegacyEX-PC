#include "nn/nex/TraceLog.hpp"

#include "nn/nex/GlobalVariables.hpp"

namespace nn
{
namespace nex
{

TraceLog::TraceLog() : m_field10(0), m_uiFlags(TRACE_ALWAYS)
{
}

TraceLog::~TraceLog()
{
}

TraceLog* TraceLog::GetInstance()
{
    return &GlobalVariables::GetInstancePtr()->GetTraceLog();
}

}
}
