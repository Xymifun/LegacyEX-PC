#include "game/ui/FJ_Event.hpp"
#include "game/ui/fui.hpp"

#include <vector>

FJ_Event::FJ_Event(eFJEventType eType, bool bBubbles, bool bCancelable)
    : m_eType(eType),
      m_bBubbles(bBubbles),
      m_bCancelable(bCancelable)
{
}
