#include "nn/dbg/Break.hpp"

extern "C" void nndbgBreak(nndbgBreakReason reason)
{
    nn::dbg::Break(static_cast<nn::dbg::BreakReason>(reason));
}
