#include "nn/dbg/Break.hpp"

#include <coreinit/debug.h>

namespace nn
{
namespace dbg
{

int Break(BreakReason reason)
{
    OSPanic("CAFE/dbg_BreakImpl.cpp", 29, "Break(%d) is not impremented.", reason);
    return 0;
}

}
}
