#include "game/ui/FJ_FocusEvent.hpp"

FJ_FocusEvent::FJ_FocusEvent(bool bFocusIn, bool bBubbles, bool bCancelable)
    : FJ_Event(bFocusIn ? eFJEventType_FocusIn : eFJEventType_FocusOut, bBubbles, bCancelable)
{
}
