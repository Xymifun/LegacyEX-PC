#include "game/ui/FJ_KeyboardEvent.hpp"

FJ_KeyboardEvent::FJ_KeyboardEvent(bool bKeyUp, int iKeyCode, bool bBubbles, bool bCancelable)
    : FJ_Event(bKeyUp ? eFJEventType_KeyUp : eFJEventType_KeyDown, bBubbles, bCancelable),
      m_iKeyCode(iKeyCode)
{
}
