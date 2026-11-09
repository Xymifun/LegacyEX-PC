#include "nn/nex/StateMachine.hpp"

namespace nn
{
namespace nex
{

StateMachine::~StateMachine()
{
}

StateMachine::QSimpleEvent::~QSimpleEvent()
{
}

unsigned short StateMachine::QSimpleEvent::GetSignal() const
{
    return m_signal;
}

}
}
