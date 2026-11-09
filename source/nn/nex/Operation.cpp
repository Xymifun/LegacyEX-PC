#include "nn/nex/Operation.hpp"

namespace nn
{
namespace nex
{

Operation::Operation(unsigned int flags) : m_userContext(), m_flags(flags)
{
    m_signalState = false;
}

unsigned short Operation::GetSignal() const
{
    return static_cast<unsigned short>(GetType());
}

}
}
