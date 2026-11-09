#ifndef LEGACY_CONSOLE_WIIU_NN_DBG_BREAK_HPP
#define LEGACY_CONSOLE_WIIU_NN_DBG_BREAK_HPP

enum nndbgBreakReason
{
    NN_DBG_BREAK_REASON_PANIC,
    NN_DBG_BREAK_REASON_ASSERT,
    NN_DBG_BREAK_REASON_USER
};

extern "C" void nndbgBreak(nndbgBreakReason reason);

namespace nn
{
namespace dbg
{

enum BreakReason
{
    BREAK_REASON_PANIC = NN_DBG_BREAK_REASON_PANIC,
    BREAK_REASON_ASSERT = NN_DBG_BREAK_REASON_ASSERT,
    BREAK_REASON_USER = NN_DBG_BREAK_REASON_USER
};

int Break(BreakReason reason);

}
}

#endif
