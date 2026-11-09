#ifndef GAME_UI_FJ_FOCUSEVENT_HPP
#define GAME_UI_FJ_FOCUSEVENT_HPP

#include "game/ui/FJ_Event.hpp"

class FJ_FocusEvent : public FJ_Event
{
public:
    FJ_FocusEvent(bool bFocusIn, bool bBubbles, bool bCancelable);
};

#endif
