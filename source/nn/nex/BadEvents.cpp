#include "nn/nex/BadEvents.hpp"

#include <stddef.h>

namespace nn
{
namespace nex
{

BadEvents* BadEvents::s_pInstance = NULL;

void BadEvents::SignalEvent(_ID id)
{
    ++m_eventCounts[id];
}

void BadEvents::Signal(_ID id)
{
    if (s_pInstance == NULL)
        return;
    s_pInstance->SignalEvent(id);
}

}
}
