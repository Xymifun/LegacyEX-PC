#include "game/ui/FJ_MouseMoveEvent.hpp"

FJ_MouseMoveEvent::FJ_MouseMoveEvent(float fX, float fY, bool bBubbles, bool bCancelable)
    : FJ_Event(eFJEventType_MouseMove, bBubbles, bCancelable),
      m_fX(fX),
      m_fY(fY)
{
}
