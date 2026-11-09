#include "nn/nex/Event.hpp"

#include "nn/nex/EventHandler.hpp"

namespace nn
{
namespace nex
{

Event::Event(EventHandler* pEventHandler, unsigned int type, unsigned int context)
    : m_pEventHandler(pEventHandler),
      m_type(type),
      m_context(context)
{
}

Event::~Event()
{
}

void Event::Set()
{
    m_pEventHandler->SetEvent(this);
}

void Event::Reset()
{
    m_pEventHandler->ResetEvent(this);
}

}
}
